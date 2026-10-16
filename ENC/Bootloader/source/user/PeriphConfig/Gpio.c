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
**	日　期  : 2023年1月12日
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
#define LED_RED_PORT				GPIO0
#define LED_RED_PIN					GPIO_PIN_18
#define LED_RED_ON()				LED_RED_PORT->BCR |= LED_RED_PIN
#define LED_RED_OFF()				LED_RED_PORT->BSR |= LED_RED_PIN

#define LED_GREEN_PORT				GPIO0
#define LED_GREEN_PIN				GPIO_PIN_19
#define LED_GREEN_ON()				LED_GREEN_PORT->BCR |= LED_GREEN_PIN
#define LED_GREEN_OFF()				LED_GREEN_PORT->BSR |= LED_GREEN_PIN

#define LED_BLUE_PORT				GPIO0
#define LED_BLUE_PIN				GPIO_PIN_20
#define LED_BLUE_ON()				LED_BLUE_PORT->BCR |= LED_BLUE_PIN
#define LED_BLUE_OFF()				LED_BLUE_PORT->BSR |= LED_BLUE_PIN


/* 函数声明 */
void _GpioInit(void);
void _GpioDefaultState(void);

void _LedOn(void);
void _LedOff(void);
void _RedLedOn(void);
void _RedLedOff(void);
void _GreenLedOn(void);
void _GreenLedOff(void);
void _BlueLedOn(void);
void _BlueLedOff(void);


/* 变量定义和声明 */
GPIO_HANDLE Gpio = 
{
	.Init = _GpioInit,
	
	.Ui = 
	{
		.RedLed =
		{
			.On  = _RedLedOn,
			.Off = _RedLedOff,
		},
		
		.GreenLed =
		{
			.On  = _GreenLedOn,
			.Off = _GreenLedOff,
		},
		
		.BlueLed =
		{
			.On  = _BlueLedOn,
			.Off = _BlueLedOff,
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
	// LED
	LED_RED_PORT  ->OER |= LED_RED_PIN;
	LED_GREEN_PORT->OER |= LED_GREEN_PIN;
	LED_BLUE_PORT ->OER |= LED_BLUE_PIN;
	
	
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
	Gpio.Ui.GreenLed.Off();
	Gpio.Ui.RedLed.Off();
	Gpio.Ui.BlueLed.Off();
	
//	while (1)
//	{
//		LED_BLINK();
//		OsTick.DelayMs(1);
//	}
}


/*
****************************************************************************************************
**
**                                         _LedOn/Off()
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
void _RedLedOn(void)
{
	LED_RED_ON();
}

void _RedLedOff(void)
{
	LED_RED_OFF();
}

void _GreenLedOn(void)
{
	LED_GREEN_ON();
}

void _GreenLedOff(void)
{
	LED_GREEN_OFF();
}

void _BlueLedOn(void)
{
	LED_BLUE_ON();
}

void _BlueLedOff(void)
{
	LED_BLUE_OFF();
}




