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
**	文件名	: Gpio.c
**
**	程序员	:
**
**	日　期  : 2022年12月1日
**
**	文件描述: 硬件MCU的GPIO驱动、初始化、配置等。
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Gpio.h"
#include "a0.h"
#include "a0_hal_gpio.h"



/* 宏定义 */

//触摸按键极性：0=触摸时低电平（内部上拉），1=触摸时高电平（内部下拉）
#define TOUCH_KEY_PRESSED_LEVEL			  (0)

//把IDR电平换算成按键状态：1=触摸按下，0=松开
#if (TOUCH_KEY_PRESSED_LEVEL == 0)
	#define TOUCH_KEY_IS_PRESSED(status)	(!(status))
#else
	#define TOUCH_KEY_IS_PRESSED(status)	(!!(status))
#endif

//静音触摸按键（短按开关静音，长按开关降噪）
#define KEY_MUTE_TOUCH_PORT			      GPIO0
#define KEY_MUTE_TOUCH_PIN			      GPIO_PIN_29
#define KEY_MUTE_TOUCH_PIN_NUM			  (29)
#define KEY_MUTE_TOUCH_GET_STATUS()	  (KEY_MUTE_TOUCH_PORT->IDR & KEY_MUTE_TOUCH_PIN)

//RGB灯效触摸按键（短按切换灯效，长按开关灯效）
#define KEY_RGB_TOUCH_PORT			      GPIO0
#define KEY_RGB_TOUCH_PIN			        GPIO_PIN_4
#define KEY_RGB_TOUCH_PIN_NUM			    (4)
#define KEY_RGB_TOUCH_GET_STATUS()	  (KEY_RGB_TOUCH_PORT->IDR & KEY_RGB_TOUCH_PIN)

//降噪(ENC)状态指示灯：高电平点亮，低电平熄灭。
//驱动在 Ui.Led.c 的 _FxMuteLedDisp() 里，跟随 ParamTbl.Preset->NoiseSuppressorEnabled。
#define NS_LED_PORT					        GPIO0
#define NS_LED_PIN					        GPIO_PIN_2
#define NS_LED_PIN_NUM				      (2)


/* 函数声明 */
void _GpioInit(void);
void _GpioDefaultState(void);

uint32 _FxKeyGetStatus(void);
uint32 _RgbKeyGetStatus(void);

void _LedOn(void);
void _LedOff(void);


/* 变量定义和声明 */
GPIO_HANDLE Gpio =
{
	.Init = _GpioInit,

	.Ui =
	{
		.Key =
		{
			.FxKeyGetStatus  = _FxKeyGetStatus,
			.RgbKeyGetStatus = _RgbKeyGetStatus,
		},
	},
};


/*
****************************************************************************************************
**
**                                         _GpioInit()
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
void _GpioInit(void)
{
//	// 屏蔽：原三色状态指示灯里的红/蓝（0.29红 / 0.4蓝）
//	// 这两个脚已改作触摸按键输入，见下面的触摸按键配置。
//	// 原绿灯 0.2 仍保留为输出，现在专职做降噪指示灯，见下面 NS_LED 配置。
//	// BLUE_LED
//	HAL_GPIO_AF_SET  (GPIO0, (4), GPIO_AF0);	  	    // 复用为GPIO模式
//	HAL_GPIO_OUT_EN   (GPIO0, GPIO_PIN_4);					    // 设置为输出模式
//	HAL_GPIO_PUPD_SET(GPIO0, (4),GPIO_PULLDOWN);	    // 设置下拉
//
//	//RED_LED
//	HAL_GPIO_AF_SET  (GPIO0, (29), GPIO_AF0);	  	      // 复用为GPIO模式
//	HAL_GPIO_OUT_EN   (GPIO0, GPIO_PIN_29);					      // 设置为输出模式
//	HAL_GPIO_PUPD_SET(GPIO0, (29),GPIO_PULLDOWN);	      // 设置下拉
//
//	//KEY（原GPIO0.6机械功能键已移除）
//	HAL_GPIO_AF_SET  (GPIO0, (6), GPIO_AF0);
//	HAL_GPIO_IN_EN   (GPIO0, GPIO_PIN_6);
//	HAL_GPIO_PUPD_SET(GPIO0, (6), GPIO_PULLUP);

	//触摸按键：静音开关 GPIO0.29 / RGB灯效 GPIO0.4
	HAL_GPIO_AF_SET  (KEY_MUTE_TOUCH_PORT, KEY_MUTE_TOUCH_PIN_NUM, GPIO_AF0);	  	// 复用为GPIO模式
	HAL_GPIO_IN_EN   (KEY_MUTE_TOUCH_PORT, KEY_MUTE_TOUCH_PIN);					          // 设置为输入模式
	HAL_GPIO_PUPD_SET(KEY_MUTE_TOUCH_PORT, KEY_MUTE_TOUCH_PIN_NUM, GPIO_PULLUP);	// 设置上拉

	HAL_GPIO_AF_SET  (KEY_RGB_TOUCH_PORT, KEY_RGB_TOUCH_PIN_NUM, GPIO_AF0);	  	  // 复用为GPIO模式
	HAL_GPIO_IN_EN   (KEY_RGB_TOUCH_PORT, KEY_RGB_TOUCH_PIN);					            // 设置为输入模式
	HAL_GPIO_PUPD_SET(KEY_RGB_TOUCH_PORT, KEY_RGB_TOUCH_PIN_NUM, GPIO_PULLUP);	  // 设置上拉

	//降噪(ENC)状态指示灯 GPIO0.2（原绿灯），保留为输出
	HAL_GPIO_AF_SET  (NS_LED_PORT, NS_LED_PIN_NUM, GPIO_AF0);	  	  // 复用为GPIO模式
	HAL_GPIO_OUT_EN  (NS_LED_PORT, NS_LED_PIN);					            // 设置为输出模式
	HAL_GPIO_PUPD_SET(NS_LED_PORT, NS_LED_PIN_NUM, GPIO_PULLDOWN);	// 设置下拉，默认熄灭
	HAL_GPIO_BIT_CLR (NS_LED_PORT, NS_LED_PIN);					            // 上电先熄灭，随后由 _FxMuteLedDisp() 按实际降噪状态刷新

	//test
//  HAL_GPIO_BIT_SET(GPIO0,GPIO_PIN_4);    // 原KEY_BLUE_PORT
//  HAL_GPIO_BIT_SET(GPIO0,GPIO_PIN_29);   // 原KEY_RED_PORT
//  HAL_GPIO_BIT_SET(GPIO0,GPIO_PIN_2);    // 原KEY_GREEN_PORT

    //Niko  test
    HAL_GPIO_AF_SET  (GPIO0, (28), GPIO_AF0);	  	      // 复用为GPIO模式
	HAL_GPIO_OUT_EN  (GPIO0, GPIO_PIN_28);			      // 设置为输出模式
	HAL_GPIO_PUPD_SET(GPIO0, (28), GPIO_NOPULL);	      //
//    HAL_GPIO_BIT_SET(GPIO0,GPIO_PIN_28);
//    HAL_GPIO_BIT_CLR(GPIO0,GPIO_PIN_28);

	_GpioDefaultState();
}


/*
****************************************************************************************************
**
**                                         _GpioDefaultState()
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
void _GpioDefaultState(void)
{
//	while (1)
//	{
//		LED_BLINK();
//		OsTick.DelayMs(1);
//	}
}


/*
****************************************************************************************************
**
**                                         _KeyGetStatus()
**
**
**	功能描述: 读取按键状态
**
**	参  数  :
**
**	返回值  : 1 按下，0 松开
**
****************************************************************************************************
*/
uint32 _FxKeyGetStatus(void)
{
	if (TOUCH_KEY_IS_PRESSED(KEY_MUTE_TOUCH_GET_STATUS()))
		return 1;

	return 0;
}

uint32 _RgbKeyGetStatus(void)
{
	if (TOUCH_KEY_IS_PRESSED(KEY_RGB_TOUCH_GET_STATUS()))
		return 1;

	return 0;
}


