/*
*****************************************Copyright (c)**********************************************
**
**
**
**
**
**
**
**	文件名	: Spi.c
**
**	程序员	: 
**
**  日　期  : 2022年12月1日
**
**	文件描述: SPI总线来控制RGB灯
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Gpio.h"
#include "Spi.h"
#include "a0_hal_gpio.h"
#include "a0_hal_spi_std.h"



/* ！！这个文件目前【不在】Keil 工程里，也不要往里加 ！！
**
** 它里面这套 SPI2 驱动已经搬到 Spi.c 里了（Spi.c 本来就在工程里，还现成有 GRB_ZERO/GRB_ONE
** 和 _RgbByteConvert()，也就不用改 JAT1821.uvprojx —— Keil 开着工程时改工程文件会被它保存时覆盖）。
** Ui.Led.c 调的 Spi2Init()/Spi2LedTx() 现在来自 Spi.c。
**
** 所以：把本文件加进工程会直接报 Spi2Init/Spi2LedTx 重复定义的链接错误。留着只是备查。
*/

/* 宏定义 */
#define SPI_IDX			(2)
#define SPIx			SPIN(SPI_IDX)


#define GRB_ZERO		(0xC0)
#define GRB_ONE			(0xF8)

//GPIO0.28 这条带子上的灯珠数，必须和 Ui.Led.c 的 RGB_STRIP_NUM 一致。
//不能像 Spi.c 那样叫 RGB_LED_NUM——那是 Ui.Led.h 里的枚举(=10，另一条带子用)，同名会冲突。
#define SPI2_LED_NUM	(4)


/* 函数声明 */
void Spi2Init(void);
void Spi2LedTx(uint8 *data);


/* 变量定义和声明 */
/* 这里不再定义 SPIx_HANDLE Spi —— Spi.c 已经定义了一个，同名会撞符号。
   本文件只对外提供 Spi2Init()/Spi2LedTx()，由 Ui.Led.c 直接调用。 */

uint32  Spi2TxCnt;
uint8  *Spi2TxBuf;

uint8 Spi2PixelBuf[SPI2_LED_NUM * 24] = {0};	// RGB灯的数据缓冲


/*
****************************************************************************************************
**
**                                       Spi2Init()
**
**
**	功能描述: SPI 初始化
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void Spi2Init(void)
{
	SPI_CR_TypeDef cr =
	{
		.b =
		{
			.mode         = SPI_CPHA1CPOL0,
			.bitDir       = 0,
			.enable       = 0,
			.byteDir      = SPI_BYTE_LOW2HIGH,		// default, little endian
			.clkDiv       = 15,
			.fifoAccWidth = 0,						// 0:8bit  1:16bit  2 :32bit
			.transWidth   = 0,						// 0:8bit  1:16bit  2 :32bit
		},
	};	
	SPI_FCR_TypeDef fcr =
	{
		.b =
		{    
			.fifoRst       = 1,
			.txFifoTrigCfg = SPI_TX_FIFO_SE8,
			.rxFifoTrigCfg = 2,
			.rxFifoTrigEn  = 0,
			.rxTimeOutEn   = 0,       				// default, little endian
			.txFifoTrigEn  = 1,
			.rwMode        = SPI_OP_NONE,
		},
	};


	/* 初始化IO口：GPIO0.28 = SPI2_MOSI(AF4)，配成输出、无上下拉 */
	HAL_GPIO_AF_SET  (GPIO0, 28, GPIO_AF4_SPI2_MOSI);
	HAL_GPIO_OUT_EN  (GPIO0, GPIO_PIN_28);
	HAL_GPIO_PUPD_SET(GPIO0, 28, GPIO_NOPULL);

	/* 初始化控制寄存器 */
	HAL_SPI_CTRL_SET(SPIx, cr.w);

	/* 初始化中断控制 */
	HAL_SPI_TIMEOUT_SET(SPIx, 0x0F);
	
	HAL_SPI_FIFO_CTRL_SET(SPIx, fcr.w);

	NVIC_SetPriority(SPI2_TX_IRQn, 0);

	HAL_SPI_ENABLE(SPIx);
}


/*
****************************************************************************************************
**
**                                       SPI2_TX_IRQHandler()
**
**
**	功能描述: SPI2 发送中断处理
**
**	参  数  : buf   数据读取缓冲区
**            len	读取的数据长度
**
**	返回值  : 
**
****************************************************************************************************
*/
void SPI2_TX_IRQHandler(void)
{
	SPI_SR_TypeDef sr;
	uint32 len = Spi2TxCnt;
	uint8  *pData = Spi2TxBuf;
	
	
	while (len != 0)
	{
		sr.w = HAL_SPI_STATE_GET(SPIx);
		
		if (sr.b.txFifoCnt < 16)
		{
			HAL_SPI_DATA_WRITE(SPIx, *pData++);
			len--;
		}
		else
		{
			break;
		}
	}
	 
	Spi2TxCnt = len;
	Spi2TxBuf = pData;
	
	if (len == 0)
	{
		NVIC_DisableIRQ(((IRQn_Type)(SPI1_TX_IRQn + ((SPI_IDX - 1) << 1))));
	}
}


/*
****************************************************************************************************
**
**                                        _RgbByteConvert()
**
**	功能描述: 送8位，等于1颗灯的一种颜色，WS2812灯数据为一串24位二进制数，每种颜色8位二进制数。
**           先是绿色，然后是红色，最后是蓝色
**
**	参  数  : data  一个字节，8位数据
**
**	返回值  : 
**
****************************************************************************************************
*/
void _RgbByteConvert(uint8 data, uint8 *pixelBuf)
{
	for (uint32 i=0; i<8; i++)
	{
		if (data & 0x80) *pixelBuf = GRB_ONE;			// 高位先发
		else             *pixelBuf = GRB_ZERO;

		data <<= 1;
		pixelBuf++;
	}
}


/*
****************************************************************************************************
**
**                                       Spi2LedTx()
**
**
**	功能描述: 发送RGB灯所需的时序
**
**	参  数  : data  按常规R1 G1 B1, R2 G2 B2, ... 来排列
**
**	返回值  : 
**
****************************************************************************************************
*/
void Spi2LedTx(uint8 *data)
{
	SPI_SR_TypeDef  sr;
	uint8  rr, gg, bb;
	uint32 len;
	uint8  *buf;

	
	for (uint32 i=0; i<SPI2_LED_NUM; i++)
	{
		rr = *data++;
		gg = *data++;
		bb = *data++;
		data++;												// 如果 data 是定义成 rgb 结构体变量，那么会多一个字节的偏移

		_RgbByteConvert(gg, Spi2PixelBuf + i*24     );	// 硬件要求的顺序是G1 R1 B1, G2 R2 B2, ...
		_RgbByteConvert(rr, Spi2PixelBuf + i*24 +  8);
		_RgbByteConvert(bb, Spi2PixelBuf + i*24 + 16);
	}

	buf = Spi2PixelBuf;
	len = SPI2_LED_NUM * 24;
	
	//--------------------------------------------------------------------------
	HAL_SPI_OP_MODE_SET(SPIx, SPI_OP_TX);
	HAL_SPI_FIFO_RST(SPIx);

	if (len != 0)
	{
		HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPIx, SPI_8BIT);		// 剩下数据按照字节传输		
		HAL_SPI_TRANS_WIDTH_SET      (SPIx, SPI_8BIT);
		HAL_SPI_TRANS_NUM_SET        (SPIx, len);
		
		while (len != 0)
		{
			sr.w = HAL_SPI_STATE_GET(SPIx);
			
			if (sr.b.txFifoCnt < 16)
			{
				HAL_SPI_DATA_WRITE(SPIx, *buf++);
				len--;
			}
			else
			{
				Spi2TxBuf = buf;
				Spi2TxCnt = len;
				
				NVIC_EnableIRQ(((IRQn_Type)(SPI1_TX_IRQn + ((SPI_IDX - 1) << 1))));
				break;
			}
		}
	}
	//--------------------------------------------------------------------------
	
	OsTick.DelayUs(120);	// >80us
}




