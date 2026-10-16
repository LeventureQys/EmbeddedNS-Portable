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
#include "Ui.h"
#include "a0_hal_gpio.h"
#include "a0_hal_spi_std.h"
#include "a0_hal_conf.h"
#include "a0_hal_dma.h"
#include "a0_hal_def.h"
#include "a0_hal_gpio.h"
#include "a0_hal_spi_std.h"



/* 宏定义 */
#define SPI_IDX			(1)
#define SPIx			SPIN(SPI_IDX)

/* SPI2：GPIO0.28(AF4=SPI2_MOSI) 上的 4 颗 WS2812。
   放在本文件而不是那份没编进工程的 Spi2.c 里，是为了不必改 JAT1821.uvprojx——
   Keil 开着工程时改工程文件会被它保存时覆盖掉。编码常量、_RgbByteConvert() 这里都现成。 */
#define SPI2_IDX		(2)
#define SPI2x			SPIN(SPI2_IDX)
#define SPI2_LED_NUM	(4)			// 必须与 Ui.Led.c 的 RGB_STRIP_NUM 一致

#define DMA_CH			(8)				// DMA通道号选择
#define DMA_CH_SPI		(DMA0CH0)		// DMA地址通道选择

#define GRB_ZERO		(0xC0)
#define GRB_ONE			(0xF8)


/* 函数声明 */
void Spi1Init(void);
void Spi1LedTx(uint8 *data);

void Spi2Init(void);
void Spi2LedTx(uint8 *data);


/* 变量定义和声明 */
SPIx_HANDLE Spi =
{
	.Init       = Spi1Init,
	.LedReflesh = Spi1LedTx,
};

uint32  SpiTxCnt;
uint8  *SpiTxBuf;

uint8 PixelBuf[RGB_LED_NUM * 24] = {0};		// RGB灯的数据缓冲
uint8 DmaBuf  [RGB_LED_NUM * 24] = {0};

typedef struct
{
    uint32_t                len;
    uint32_t                srcAddr;
    uint32_t                dstAddr;
    DMA_CH_CCR_TypeDef      ccr;
    DMA_CH_CCFG_TypeDef     ccfg;

    uint8_t                 spiid;
    SPI_CR_TypeDef          scr;
    SPI_FCR_TypeDef         sfcr;
    SPI_SR_TypeDef          ssr;
} SPI_DMA;

SPI_DMA SpiDamConfig =
{
    //DMA寄存器配置
    .len               = RGB_LED_NUM * 24,
    .srcAddr           = 0,
    .dstAddr           = 0,
    .ccr.b =
    {
        .chEn          = 1,						// DMA通道使能控制 0非使能 1使能
        .dstCtrl       = 2,						// 目标地址控制 0递增 1递减 2固定
        .srcCtrl       = 0,						// 源地址控制 0递增 1递减 2固定
        .mode          = 1,						// 0普通模式 1硬件握手模式
        .dstWidth      = DMA_WIDTH_8BIT,		// 目标传输位宽 0 8位 1 16位 2 32位
        .srcWidth      = DMA_WIDTH_8BIT,		// 源传输位宽 0 8位 1 16位 2 32位
        .srcSize       = DMA_BURST_8,			// 源地址一次传输数量的选择 0 1,1 4,2 8,3 16,4 32,5 64,6 128,7 256
        .fifoTh        = DMA_FIFO_TH_1,			// DMA传输启动FIFO的阈值 0 1,1 2,2 4
        .tcMsk         = 1,						// 传输完成状态屏蔽标志位 0 传输完成TC置1 1传输完成TC不置1

    },
    .ccfg.b =
    {
        .srcHardwareEn = 0,						// 源硬件握手使能控制 0非使能 1使能
        .srcRequest    = 0,						// 源DMA请求选择0~15
        .dstHardwareEn = 1,						// 目标硬件使能控制 0非使能 1使能
        .dstRequest    = 8,						// 目标DMA请求选择0~15
        .intTcMsk      = 1,						// 通道传输完成中断屏蔽标志位 0不屏蔽中断 1屏蔽中断
        .intErrMsk     = 1,						// 通道传输错误中断屏蔽标志位 0 不屏蔽   1屏蔽
        .intAbtMsk     = 1,						// 通道传输中止中断屏蔽标志位 0 不屏蔽   1屏蔽
    },

    //SPI寄存器配置
    .spiid             = SPI_IDX,
    .scr.b =
    {
        .mode          = 2,						// SPI模式  CPHA:CPOL 1 0
        .bitDir        = 0,						// FIFO数据传输方向选择 0从高至低 1从低至高
        .slaveEn       = 0,						// 主从模式设置 0主 1从
        .enable        = 0,						// 控制器使能开关 0禁止 1使能
        .csSel         = 0,						// 必须写0
        .fifoAccWidth  = 0,						// FIFO访问位宽设置 0 8位 1 16位 2 32位
        .byteDir       = SPI_BYTE_LOW2HIGH,		// 传输字节顺序仅当位宽为16，32有效 0从高至低 1从低至高
        .transWidth    = 0,						// 传输数据位宽 0 8位 1 16位 2 32位
        .csCtrl        = 0,						// 主机模式CS输出电平控制 0低 1高
        .dmaMode       = 2,						// DMA模式 0不用DMA 1RX使用DMA 2TX使用DMA
        .clkDiv        = 15,					// 时钟分频从PCLK分频 96MHZ 最小值为8
    },
    .sfcr.b =
    {
        .rxFifoTrigCfg = SPI_RX_FIFO_NE0,		// 接收触发门限设定 0 0 1 >=4 2 >=8 3 =16
        .txFifoTrigCfg = SPI_TX_FIFO_SE8,		// 发送FIFO触发门限设置 0 0 1 <=4 2 <=8 3 <=16
        .rxTimeOutEn   = 0,						// 接收超时中断使能  1使能
        .rxFifoTrigEn  = 0,						// 接收中断使能    1使能
        .txFifoTrigEn  = 1,						// 发送中断使能    1使能
        .rwMode        = 1,						// 数据传输模式    0不读不写 1TX 2RX 3TXandRX
        .fifoRst       = 1,						// 写1清FIFO
    },
};


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
	/* 初始化IO口 */
	HAL_GPIO_AF_SET(GPIO0, 24, GPIO_AF4_SPI1_MOSI);


	SpiDamConfig.srcAddr = (uint32)&DmaBuf[0];
	SpiDamConfig.dstAddr = (uint32)(&(SPIx->DR));


	HAL_SPI_DISABLE(SPIx);
	HAL_DMA_GROUP_DISABLE(DMA0CTRL);
	
	/* SPI */
	HAL_SPI_CTRL_SET     (SPIx, SpiDamConfig.scr.w);
	HAL_SPI_FIFO_CTRL_SET(SPIx, SpiDamConfig.sfcr.w);
	HAL_SPI_TRANS_NUM_SET(SPIx, RGB_LED_NUM * 24);
	HAL_SPI_ENABLE       (SPIx);

	/* SPI DMA */
	HAL_DMA_CH_SRC_ADDR_SET  (DMA_CH_SPI, SpiDamConfig.srcAddr);		// 源地址
	HAL_DMA_CH_DST_ADDR_SET  (DMA_CH_SPI, SpiDamConfig.dstAddr);		// 目标地址
	HAL_DMA_CH_CTRL_SET      (DMA_CH_SPI, SpiDamConfig.ccr.w);
	HAL_DMA_CH_CFG_SET       (DMA_CH_SPI, SpiDamConfig.ccfg.w);
	HAL_DMA_CH_TRANS_SIZE_SET(DMA_CH_SPI, SpiDamConfig.len);			// 传输数量
	HAL_DMA_GROUP_REQ_SEL    (DMA0SELECT, DMA_CH, DMA_REQ_SPI1TX);		// DMA响应信号选择
	HAL_DMA_GROUP_ENABLE     (DMA0CTRL);
}


/*
****************************************************************************************************
**
**                                       SpiDmaTx()
**
**
**	功能描述:
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void SpiDmaTx(void)
{
	memcpy(DmaBuf, PixelBuf, RGB_LED_NUM * 24);							// 数据搬运到DMA Buf

//	HAL_SPI_DISABLE(SPIx);
	HAL_DMA_GROUP_DISABLE(DMA0CTRL);
	
	/* SPI */
	HAL_SPI_CTRL_SET     (SPIx, SpiDamConfig.scr.w);
	HAL_SPI_FIFO_CTRL_SET(SPIx, SpiDamConfig.sfcr.w);
	HAL_SPI_TRANS_NUM_SET(SPIx, RGB_LED_NUM * 24);
	HAL_SPI_ENABLE       (SPIx);

	/* SPI DMA */
	HAL_DMA_CH_SRC_ADDR_SET  (DMA_CH_SPI, SpiDamConfig.srcAddr);		// 源地址
	HAL_DMA_CH_DST_ADDR_SET  (DMA_CH_SPI, SpiDamConfig.dstAddr);		// 目标地址
	HAL_DMA_CH_CTRL_SET      (DMA_CH_SPI, SpiDamConfig.ccr.w);
	HAL_DMA_CH_CFG_SET       (DMA_CH_SPI, SpiDamConfig.ccfg.w);
	HAL_DMA_CH_TRANS_SIZE_SET(DMA_CH_SPI, SpiDamConfig.len);			// 传输数量
	HAL_DMA_GROUP_REQ_SEL    (DMA0SELECT, DMA_CH, DMA_REQ_SPI1TX);		// DMA响应信号选择
	HAL_DMA_GROUP_ENABLE     (DMA0CTRL);
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
__RAM_CODE void _RgbByteConvert(uint8 data, uint8 *pixelBuf)
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
__RAM_CODE void Spi1LedTx(uint8 *data)
{
	uint8  rr, gg, bb;


	for (uint32 i=0; i<RGB_LED_NUM; i++)
	{
		rr = *data++;
		gg = *data++;
		bb = *data++;
		data++;										// 如果 data 是定义成 rgb 结构体变量，那么会多一个字节的偏移

		_RgbByteConvert(gg, PixelBuf + i*24     );	// 硬件要求的顺序是G1 R1 B1, G2 R2 B2, ...
		_RgbByteConvert(rr, PixelBuf + i*24 +  8);
		_RgbByteConvert(bb, PixelBuf + i*24 + 16);
	}

	SpiDmaTx();
	OsTick.DelayUs(250);	// >80us
}


/* ================================================================================================
 *  SPI2 + GPIO0.28 上的 4 颗 WS2812
 *
 *  为什么不是软件位翻转：位翻转期间虽然关了中断，但关中断只挡得住中断，挡不住 CPU 的取指/
 *  总线停顿（降噪那条链跑在 RAM 里，又和 I2S 的 DMA 抢总线），某一位的高电平被拉长，灯珠就会
 *  把 0 码判成 1 码。改由外设产生时序后，CPU 只负责往 FIFO 里塞字节，停顿再大也不影响输出。
 *
 *  一个 WS2812 位 = 一个 SPI 字节：0 码 0xC0、1 码 0xF8（同上面 SPI1 那条链，已验证）。
 *  clkDiv=15 → SPI 时钟 = 96MHz/16 = 6MHz，于是 0 码高 333ns、1 码高 833ns、位周期 1.333us，
 *  都在 WS2812B 的容差内。
 * ================================================================================================ */

uint32  Spi2TxCnt;
uint8  *Spi2TxBuf;

uint8 Spi2PixelBuf[SPI2_LED_NUM * 24] = {0};


/*
****************************************************************************************************
**
**                                       Spi2Init()
**
**	功能描述: SPI2 初始化，驱动 GPIO0.28 上的 4 颗 WS2812
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
			.byteDir      = SPI_BYTE_LOW2HIGH,
			.clkDiv       = 15,						// 96MHz/(15+1) = 6MHz
			.fifoAccWidth = 0,						// 0:8bit  1:16bit  2:32bit
			.transWidth   = 0,
		},
	};
	SPI_FCR_TypeDef fcr =
	{
		.b =
		{
			.fifoRst       = 1,
			.txFifoTrigCfg = SPI_TX_FIFO_SE8,		// FIFO <=8 就触发发送中断，留 8 字节(约 10.7us)余量
			.rxFifoTrigCfg = 2,
			.rxFifoTrigEn  = 0,
			.rxTimeOutEn   = 0,
			.txFifoTrigEn  = 1,
			.rwMode        = SPI_OP_NONE,			// 每次发送前再由 Spi2LedTx 设成 SPI_OP_TX
		},
	};

	/* 初始化IO口：GPIO0.28 = SPI2_MOSI(AF4)，配成输出、无上下拉 */
	HAL_GPIO_AF_SET  (GPIO0, 28, GPIO_AF4_SPI2_MOSI);
	HAL_GPIO_OUT_EN  (GPIO0, GPIO_PIN_28);
	HAL_GPIO_PUPD_SET(GPIO0, 28, GPIO_NOPULL);

	HAL_SPI_CTRL_SET     (SPI2x, cr.w);
	HAL_SPI_TIMEOUT_SET  (SPI2x, 0x0F);
	HAL_SPI_FIFO_CTRL_SET(SPI2x, fcr.w);

	//优先级 0，高于降噪的 USER1_IRQn(=2)，降噪正跑着也抢得到。不这么做的话 FIFO 一旦喂不上，
	//SPI 时钟就会停在中途，灯珠照样判错码——和位翻转被拉长是同一个后果。
	NVIC_SetPriority(SPI2_TX_IRQn, 0);

	HAL_SPI_ENABLE(SPI2x);
}


/*
****************************************************************************************************
**
**                                       SPI2_TX_IRQHandler()
**
**	功能描述: SPI2 发送中断，把剩下的数据补进 FIFO，发完自己关掉中断
**
**	参  数  :
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
		sr.w = HAL_SPI_STATE_GET(SPI2x);

		if (sr.b.txFifoCnt < 16)
		{
			HAL_SPI_DATA_WRITE(SPI2x, *pData++);
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
		NVIC_DisableIRQ(SPI2_TX_IRQn);
	}
}


/*
****************************************************************************************************
**
**                                       Spi2LedTx()
**
**	功能描述: 把 4 颗灯的数据（R,G,B,pad 共 4 组）用 SPI2 发出去。
**	          返回时只是把 FIFO 塞满了，剩下的由 SPI2_TX_IRQHandler 接着发；
**	          调用方要等整帧发完（见 Ui.Led.c 的 _RgbStripFlush）。
**
**	参  数  : data  按常规 R1 G1 B1, R2 G2 B2, ... 来排列
**
**	返回值  :
**
****************************************************************************************************
*/
void Spi2LedTx(uint8 *data)
{
	SPI_SR_TypeDef sr;
	uint8  rr, gg, bb;
	uint32 len;
	uint8  *buf;

	for (uint32 i=0; i<SPI2_LED_NUM; i++)
	{
		rr = *data++;
		gg = *data++;
		bb = *data++;
		data++;										// 如果 data 是定义成 rgb 结构体变量，那么会多一个字节的偏移

		_RgbByteConvert(gg, Spi2PixelBuf + i*24     );	// 硬件要求的顺序是G1 R1 B1, G2 R2 B2, ...
		_RgbByteConvert(rr, Spi2PixelBuf + i*24 +  8);
		_RgbByteConvert(bb, Spi2PixelBuf + i*24 + 16);
	}

	buf = Spi2PixelBuf;
	len = SPI2_LED_NUM * 24;						// 4*24 = 96 字节，6MHz 下 128us

	HAL_SPI_OP_MODE_SET(SPI2x, SPI_OP_TX);
	HAL_SPI_FIFO_RST(SPI2x);

	HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPI2x, SPI_8BIT);
	HAL_SPI_TRANS_WIDTH_SET      (SPI2x, SPI_8BIT);
	HAL_SPI_TRANS_NUM_SET        (SPI2x, len);

	//先把 FIFO(16字节)塞满，剩下的交给发送中断
	while (len != 0)
	{
		sr.w = HAL_SPI_STATE_GET(SPI2x);

		if (sr.b.txFifoCnt < 16)
		{
			HAL_SPI_DATA_WRITE(SPI2x, *buf++);
			len--;
		}
		else
		{
			Spi2TxBuf = buf;
			Spi2TxCnt = len;

			NVIC_EnableIRQ(SPI2_TX_IRQn);
			break;
		}
	}
}




