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
**  日　期  : 2023年1月12日
**
**	文件描述: SPI总线来控制RGB灯
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Gpio.h"
#include "Spi.h"
#include "Ui.h"
#include "a0_hal_gpio.h"
#include "a0_hal_spi_std.h"



/* 宏定义 */
#define SPI_IDX			(1)
#define SPIx			SPIN(SPI_IDX)


#define GRB_ZERO		(0xC0)
#define GRB_ONE			(0xF8)


/* 函数声明 */
void Spi1Init(void);
void Spi1LedTx(uint8 *data);


/* 变量定义和声明 */
SPIx_HANDLE Spi = 
{
	.Init       = Spi1Init,
	.LedReflesh = Spi1LedTx,
};

uint32  SpiTxCnt;
uint8  *SpiTxBuf;

uint8 PixelBuf[RGB_LED_NUM * 24] = {0};		// RGB灯的数据缓冲


/*
****************************************************************************************************
**
**                                       Spi1Init()
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
void Spi1Init(void)
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


	/* 初始化IO口 */
	HAL_GPIO_AF_SET(GPIO0, 24, GPIO_AF4_SPI1_MOSI);

	/* 初始化控制寄存器 */
	HAL_SPI_CTRL_SET(SPIx, cr.w);

	/* 初始化中断控制 */
	HAL_SPI_TIMEOUT_SET(SPIx, 0x0F);
	
	HAL_SPI_FIFO_CTRL_SET(SPIx, fcr.w);

	NVIC_SetPriority(SPI1_TX_IRQn, 0);

	HAL_SPI_ENABLE(SPIx);
}


/*
****************************************************************************************************
**
**                                       SPI1_TX_IRQHandler()
**
**
**	功能描述: SPI1 发送中断处理
**
**	参  数  : buf   数据读取缓冲区
**            len	读取的数据长度
**
**	返回值  : 
**
****************************************************************************************************
*/
void SpiLedIrqHandler(void)
{
	SPI_SR_TypeDef sr;
	uint32 len = SpiTxCnt;
	uint8  *pData = SpiTxBuf;
	
	
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
	 
	SpiTxCnt = len;
	SpiTxBuf = pData;
	
	if (len == 0)
	{
		NVIC_DisableIRQ(((IRQn_Type)(SPI1_TX_IRQn + ((SPI_IDX - 1) << 1))));
	}
}

//void SPI1_TX_IRQHandler(void)
//{
//	SpiLedIrqHandler();
//}


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
**                                       Spi1LedTx()
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
void Spi1LedTx(uint8 *data)
{
	SPI_SR_TypeDef  sr;
	uint8  rr, gg, bb;
	uint32 len;
	uint8  *buf;

	
	for (uint32 i=0; i<RGB_LED_NUM; i++)
	{
		rr = *data++;
		gg = *data++;
		bb = *data++;
		data++;												// 如果 data 是定义成 rgb 结构体变量，那么会多一个字节的偏移
		
		_RgbByteConvert(gg, PixelBuf + i*24     );	// 硬件要求的顺序是G1 R1 B1, G2 R2 B2, ...
		_RgbByteConvert(rr, PixelBuf + i*24 +  8);
		_RgbByteConvert(bb, PixelBuf + i*24 + 16);
	}
	
	buf = PixelBuf;
	len = RGB_LED_NUM * 24;
	
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
				SpiTxBuf = buf;
				SpiTxCnt = len;
				
				NVIC_EnableIRQ(((IRQn_Type)(SPI1_TX_IRQn + ((SPI_IDX - 1) << 1))));
				break;
			}
		}
	}
	//--------------------------------------------------------------------------
	
	OsTick.DelayUs(250);	// >80us
}




