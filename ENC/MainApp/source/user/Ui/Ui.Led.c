/*
*****************************************Copyright (c)**********************************************
**
**
**                                  深圳摩罗志远科技有限公司
**                             Shenzhen Maono Technology Co.,Ltd
**
**                                        www.maono.cn
**
**
**	文件名	: Ui.Led.c
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: LED应用。
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Gpio.h"
#include "Spi.h"
#include "Ui.h"
#include "ParamTable.h"
#include "Helper.h"
#include "a0_hal_gpio.h"
#include "a0_hal_def.h"


/* 宏定义 */
typedef struct
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 unused;
} COLOR_CLASS;


/* 函数声明 */
void UiLedInit(void);
void UiLedProcess(void);

void _AllLedOff(void);
void _AllLedOffAction(void);
void _AllLedBlink(void);
void _FxMuteLedDisp(uint32 mute, uint32 nsEn);
void _RgbLedDisp(uint32 onoff, uint32 mode, uint32 lightness);


/* 变量定义和声明 */
UI_LED_HANDLE  UiLed = 
{
	.Init    = UiLedInit,
	.Process = UiLedProcess,

	.AllOff       = _AllLedOff,
	.AllOffAction = _AllLedOffAction,
	.AllLedBlink  = _AllLedBlink,
	
	.FxMute       = _FxMuteLedDisp,
	.RgbShow      = _RgbLedDisp,
};

static COLOR_CLASS RgbLedStatus[2][RGB_LED_NUM] = {0};


/* ================================================================================================
 *  4颗 WS2812 灯带驱动（数据线 GPIO0.28 = SPI2_MOSI/AF4，走硬件 SPI）
 *
 *  以前是软件位翻转，灯效全灭时第1颗偶尔还会亮、开了降噪更容易出现。位翻转期间虽然关了
 *  中断，但关中断只挡得住中断，挡不住 CPU 的取指/总线停顿（降噪那条链在 RAM 里跑，又和 I2S
 *  的 DMA 抢总线），某一位的高电平被拉长，灯珠就会把 0 码判成 1 码。这也是"只有第一颗不对"
 *  的原因：帧开头那一位最容易撞上刚接上的取指；而内容不变时又不会重发，那颗就一直错下去。
 *
 *  现在改用硬件 SPI2：位时序完全由外设从 FIFO 产生，CPU 只负责往 FIFO 里塞字节，
 *  取指停顿、总线抢占都不影响输出。SPI2 的 TX 中断优先级是 0，高于降噪 USER1 的 2，
 *  一定抢得到；FIFO 16 字节(约 21us)也足够缓冲。
 *
 *  编码沿用 Spi.c 验证过的那套：一个 WS2812 位 = 一个 SPI 字节，0 码 = 0xC0、1 码 = 0xF8；
 *  SPI 时钟 = 96MHz/(clkDiv+1) = 6MHz，于是 0 码高 333ns、1 码高 833ns、位周期 1.333us，
 *  都落在 WS2812B 的容差里。每颗灯 24 位、高位先发、顺序 G R B。
 * ================================================================================================ */

#define RGB_STRIP_NUM			(4)					// 灯带上的灯珠数量，必须与 Spi2.c 的 SPI2_LED_NUM 一致

#define RGB_RESET_NS			(300000)			// 锁存低电平，这颗料实测 300us 可用

#define RGB_NS2CYC(ns)			(((ns) * 192) / 1000)	// HCLK = 192MHz，1us = 192 拍

//死等保护的迭代上限。正常最长的一次等待（复位 57600 周期）约 1 万次循环就够，这里留 20 倍余量；
//万一这颗片子的 DWT 没跑起来（CYCCNT 不递增），也不会卡死在这里。
#define RGB_WAIT_GUARD			(200000)

//兜底重发周期。内容没变也至少这么久发一帧。改硬件 SPI 后每帧本身是可靠的，这里只是保底：
//万一上电头一帧没发全、或者灯带被别的干扰错了位，最多 100ms 就纠正回来。
#define RGB_REFRESH_MS			(100)


/*
**  用 DWT 周期计数器等到 t0 之后 cyc 个周期。以绝对周期数为基准，
**  不受 Flash 取指快慢、分支预测、函数调用开销的影响。
*/
static void _RgbStripWait(uint32 t0, uint32 cyc)
{
	uint32 guard = RGB_WAIT_GUARD;

	while ((DWT->CYCCNT - t0) < cyc && --guard)
	{
	}
}


/*
**  把 RgbLedStatus[CURR] 前 RGB_STRIP_NUM 颗灯的数据交给 SPI2 发出去。
**  这里不重置 DWT->CYCCNT —— OsTick.DelayUs() 也在用它，重置会打断别处的计时。
*/
static void _RgbStripFlush(void)
{
	//使能 DWT 周期计数器当时基（OsTick.Init() 里已使能过，这里再配一遍让驱动自带前置条件）
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL       |= DWT_CTRL_CYCCNTENA_Msk;

	//时序交给硬件，不用关中断。返回时只是把数据塞进了 FIFO，剩下的由 SPI2 的发送中断接着发。
	Spi2LedTx((uint8 *)RgbLedStatus[CURR]);

	//发完 MOSI 停在低电平（0 码 0xC0、1 码 0xF8 的最后一位都是 0），灯带就是靠这段低电平锁存。
	//一帧 96 字节在 6MHz 下要 128us，等 300us 顺便也保证整帧确实发完了、不会和下一次发送重叠。
	_RgbStripWait(DWT->CYCCNT, RGB_NS2CYC(RGB_RESET_NS));
}


//上次真正发出去的内容，用来避免每轮都重发（固定单色灯效内容不变，没必要一直占着这条线）
static COLOR_CLASS _RgbStripSent[RGB_STRIP_NUM] = {0};
static uint32      _RgbStripNeedSend = 1;		// 上电后强制先发一帧


/*
**  内容有变化才返回 1，并把影子缓冲同步过来。
*/
static uint32 _RgbStripChanged(void)
{
	uint32 changed = 0;

	if (_RgbStripNeedSend)
	{
		_RgbStripNeedSend = 0;
		changed = 1;
	}

	for (uint32 i = 0; i < RGB_STRIP_NUM; i++)
	{
		if (_RgbStripSent[i].r != RgbLedStatus[CURR][i].r ||
			_RgbStripSent[i].g != RgbLedStatus[CURR][i].g ||
			_RgbStripSent[i].b != RgbLedStatus[CURR][i].b)
		{
			changed = 1;
		}

		_RgbStripSent[i] = RgbLedStatus[CURR][i];
	}

	return changed;
}


/*
****************************************************************************************************
**
**                                        UiLedInit()
**
**	功能描述: 
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void UiLedInit(void)
{
	Spi2Init();							// GPIO0.28 交给 SPI2_MOSI，4颗灯带的时序由外设产生

	/* 赋非0值，用于开机时更新一次状态 */
//	for (uint32 i=0; i<RGB_LED_NUM; i++)
//	{
//		RgbLedStatus[LAST][i].unused = 0xFF;
//	}
}


/*
****************************************************************************************************
**
**                                        UiLedProcess()
**
**	功能描述: LED处理
**
**	参  数  : 
**
**	返回值  : 
**
**
****************************************************************************************************
*/
void UiLedProcess(void)
{
//	uint32 *pCurr = (uint32*)RgbLedStatus[CURR];
//	uint32 *pLast = (uint32*)RgbLedStatus[LAST];
//	uint32 changeEvent = 0;
//	uint32 i;


//	// 所有RGB灯更新
//	for (i=0; i<RGB_LED_NUM; i++)
//	{
//		if (*pLast != *pCurr)
//		{
//			*pLast = *pCurr;
//			changeEvent = 1;
//		}

//		pCurr++;
//		pLast++;
//	}
//	
//	if (changeEvent)
//	{
//		Spi.LedReflesh((uint8*)RgbLedStatus[CURR]);
//	}
}


/*
****************************************************************************************************
**
**                                        _AllLedOff()
**
**	功能描述: 所有灯关闭
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void _AllLedOff(void)
{
	for (uint32 i=0; i<RGB_LED_NUM; i++)
	{
		RgbLedStatus[CURR][i].r = 0;
		RgbLedStatus[CURR][i].g = 0;
		RgbLedStatus[CURR][i].b = 0;
	}
}


/*
****************************************************************************************************
**
**                                        _AllLedOffAction()
**
**	功能描述: 立刻执行关闭所有灯
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void _AllLedOffAction(void)
{
	uint32 i;


	for (i=0; i<RGB_LED_NUM; i++)
	{
		RgbLedStatus[CURR][i].r = 0;
		RgbLedStatus[CURR][i].g = 0;
		RgbLedStatus[CURR][i].b = 0;
	}
	Spi.LedReflesh((uint8*)RgbLedStatus[CURR]);
	_RgbStripFlush();					// GPIO0.28 上的4颗新灯带也一起熄灭
}


/*
****************************************************************************************************
**
**                                        _AllLedBlink()
**
**	功能描述: 所有灯闪烁
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void _AllLedBlink(void)
{
	uint32 i, t;
	uint32 r = 255;


	for (t=0; t<7; t++)
	{
		if (t%2) r = 255;
		else     r = 0;
		
		for (i=0; i<RGB_LED_NUM; i++)
		{
			RgbLedStatus[CURR][i].r = r;
			RgbLedStatus[CURR][i].g = 0;
			RgbLedStatus[CURR][i].b = 0;
		}
		Spi.LedReflesh((uint8*)RgbLedStatus[CURR]);
		
		OsTick.DelayMs(150);
	}
}


/*
****************************************************************************************************
**
**                                     _FuncLedDisp()
**
**	功能描述: 复合功能灯指示
**
**	参  数  : mute  静音状态
**
**	返回值  : 
**
****************************************************************************************************
*/
void _FxMuteLedDisp(uint32 mute, uint32 nsEn)
{
	// 降噪(ENC)状态指示灯：GPIO0.2（原绿灯）。高电平点亮，低电平熄灭。
	// 只跟随降噪开关，不受静音影响——静音状态下也能看出降噪是开还是关。
	//
	// 原三色状态灯（0.29红 / 0.2绿 / 0.4蓝共用一个灯）已拆掉：
	// 0.29 和 0.4 改成了触摸按键输入，只剩 0.2 还是输出脚，现在专职做降噪指示。
	// 因此静音状态不再有任何灯指示，mute 形参保留只为兼容 Ui.Led->FxMute 接口。
	(void)mute;

	if (nsEn)
	{
		HAL_GPIO_BIT_SET(GPIO0, GPIO_PIN_2);
	}
	else
	{
		HAL_GPIO_BIT_CLR(GPIO0, GPIO_PIN_2);
	}
}


/*
****************************************************************************************************
**
**                                        _RgbLedDisp()
**
**	功能描述: 4颗WS2812灯带的显示处理（数据线 GPIO0.28）
**
**	参  数  : onoff     0=灯效总开关关闭（全灭），1=打开
**	          mode      灯效编号 0 ~ RGB_SHOW_MODE_NUM-1
**	          lightness 亮度百分比。本版按满亮度显示，参数保留以兼容 Ui.Led->RgbShow 接口
**
**	显示优先级：总开关关 > 静音 > 灯效模式。三者互相独立：
**	  总开关关  ->  4颗全灭。静音时长按RGB键熄灭红灯，用的也是这个开关
**	  静音打开  ->  4颗固定全红，与当前灯效无关；但 mode 变量照常被短按改动，
**	                一取消静音就按新模式显示
**	  其余      ->  按 mode 显示灯效
**
**	灯效编号（就是短按RGB键依次切换的7种）：
**	  0 七彩跑马灯  1 颜色渐变  2 固定紫  3 固定青  4 固定黄  5 固定绿  6 固定蓝
**
****************************************************************************************************
*/

#define RGB_SHOW_COLOR_NUM		(7)			// 七彩色基准色个数

#define RGB_MARQUEE_STEP_MS		(200)		// 跑马灯每步停留时间
#define RGB_FADE_STEP_MS		(60)		// 渐变每小步时间
#define RGB_FADE_SUBSTEP		(8)			// 两个基准色之间插几步，插得越多过渡越平滑

//七彩色基准，跑马灯和渐变共用一张表，顺序：红 橙 黄 绿 青 蓝 紫
static const uint8 _RgbRainbow[RGB_SHOW_COLOR_NUM][3] =
{
	{255,   0,   0},	// 0 红
	{255,  77,   0},	// 1 橙
	{255, 191,   0},	// 2 黄
	{  0, 255,   0},	// 3 绿
	{  0, 255, 255},	// 4 青
	{  0,   0, 255},	// 5 蓝
	{255,   0, 255},	// 6 紫
};

//固定单色灯效（编号 2~6：紫 青 黄 绿 蓝）分别对应上面 _RgbRainbow 的哪一行
static const uint8 _RgbFixedIdx[RGB_SHOW_MODE_NUM - 2] = {6, 4, 2, 3, 5};


//把第 idx 颗灯设成指定颜色
static void _RgbSetOne(uint32 idx, uint8 r, uint8 g, uint8 b)
{
	RgbLedStatus[CURR][idx].r = r;
	RgbLedStatus[CURR][idx].g = g;
	RgbLedStatus[CURR][idx].b = b;
}


//4颗都设成同一个颜色
static void _RgbSetAll(uint8 r, uint8 g, uint8 b)
{
	for (uint32 i = 0; i < RGB_STRIP_NUM; i++)
	{
		_RgbSetOne(i, r, g, b);
	}
}


/*
**  各灯效每步之间的毫秒数。固定单色不需要步进，返回 0 表示"不随时间变化"。
*/
static uint32 _RgbShowStepMs(uint32 mode)
{
	if (mode == 0)	return RGB_MARQUEE_STEP_MS;		// 七彩跑马灯
	if (mode == 1)	return RGB_FADE_STEP_MS;		// 颜色渐变

	return 0;										// 固定单色
}


/*
**  按灯效编号和步号算出4颗灯的颜色，写进 RgbLedStatus[CURR]。
*/
static void _RgbShowEffect(uint32 mode, uint32 step)
{
	if (mode == 0)
	{
		//七彩跑马灯：4颗一直全亮，每颗取彩虹里相邻的一个颜色，整体滚动。
		//第 i 颗显示 (i - step) % 7，所以每步所有灯的颜色一起往回挪一格，
		//看上去就是七彩色沿着灯带倒着跑；7步走完一轮，整条带子的颜色组合循环。
		//减法要先加一圈 RGB_SHOW_COLOR_NUM 再取模——都是无符号数，直接减会下溢。
		for (uint32 i = 0; i < RGB_STRIP_NUM; i++)
		{
			uint32 col = (i + RGB_SHOW_COLOR_NUM - (step % RGB_SHOW_COLOR_NUM)) % RGB_SHOW_COLOR_NUM;

			_RgbSetOne(i, _RgbRainbow[col][0], _RgbRainbow[col][1], _RgbRainbow[col][2]);
		}
	}
	else if (mode == 1)
	{
		//颜色渐变：4颗同色，在七彩色之间平滑过渡（不是直接跳变）。
		//每两个基准色之间再插 RGB_FADE_SUBSTEP 小步，段内按比例线性插值。
		uint32 total = RGB_SHOW_COLOR_NUM * RGB_FADE_SUBSTEP;	// 一整轮的小步数
		uint32 s     = step % total;
		uint32 seg   = s / RGB_FADE_SUBSTEP;					// 落在第几段
		uint32 frac  = s % RGB_FADE_SUBSTEP;					// 段内进度
		const uint8 *c0 = _RgbRainbow[seg];
		const uint8 *c1 = _RgbRainbow[(seg + 1) % RGB_SHOW_COLOR_NUM];

		_RgbSetAll((uint8)((int32)c0[0] + ((int32)c1[0] - (int32)c0[0]) * (int32)frac / (int32)RGB_FADE_SUBSTEP),
		           (uint8)((int32)c0[1] + ((int32)c1[1] - (int32)c0[1]) * (int32)frac / (int32)RGB_FADE_SUBSTEP),
		           (uint8)((int32)c0[2] + ((int32)c1[2] - (int32)c0[2]) * (int32)frac / (int32)RGB_FADE_SUBSTEP));
	}
	else
	{
		//固定单色
		const uint8 *c = _RgbRainbow[_RgbFixedIdx[mode - 2]];

		_RgbSetAll(c[0], c[1], c[2]);
	}
}


void _RgbLedDisp(uint32 onoff, uint32 mode, uint32 lightness)
{
	static uint32 lastMode = 0xFFFFFFFF;
	static uint32 step     = 0;
	static uint32 tick     = 0;
	static uint32 refresh  = 0;			// 上次无条件重发的时刻
	uint32 stepMs;
	uint32 now;

	(void)lightness;					// 本版按满亮度显示，不做调光

	if (!onoff)
	{
		lastMode = 0xFFFFFFFF;			// 重新打开时从第一步开始
		step     = 0;
		_RgbSetAll(0, 0, 0);
	}
	else if (ParamTbl.Preset->MicMute)
	{
		//静音打开：4颗固定全红。mode 变量不受影响——静音期间短按RGB键照样在切模式，
		//只是画面保持红色，取消静音后立刻按新模式显示。
		lastMode = 0xFFFFFFFF;
		step     = 0;
		_RgbSetAll(255, 0, 0);
	}
	else
	{
		if (mode >= RGB_SHOW_MODE_NUM)
		{
			mode = 0;
		}

		stepMs = _RgbShowStepMs(mode);

		if (lastMode != mode)			//换灯效后从第一步重新开始，不留上一种灯效的相位
		{
			lastMode = mode;
			step     = 0;
			tick     = HAL_TickGet() - stepMs;		//让下面的判断立刻成立，马上走第一步
		}

		_RgbShowEffect(mode, step);

		//用 HAL_TickGet() 的真实毫秒时基做步进。这里不能用 OsTick.TimerMs()：
		//它是扫描式的，每轮主循环才减 1，而主循环一轮要 4~5ms，定时会被拉长好几倍。
		if (stepMs != 0 && (HAL_TickGet() - tick) >= stepMs)
		{
			tick = HAL_TickGet();
			step++;
		}
	}

	//内容变了才发，固定单色和灭灯时不会每轮都占着这条线。
	//再补一个兜底：不管内容变没变，最多 RGB_REFRESH_MS 就重发一帧。
	//（以前是防"静止画面下收到错帧后永不重发"；改用硬件 SPI 后每帧本身可靠了，
	// 留着它只是保底——上电头一帧、或者灯带被外部干扰错了位，最多 100ms 自己纠正回来。
	// 一帧 128us + 300us 锁存，10 次/秒约 0.4% CPU，可以忽略。）
	now = HAL_TickGet();
	if (_RgbStripChanged() || (now - refresh) >= RGB_REFRESH_MS)
	{
		refresh = now;
		_RgbStripFlush();
	}
}




