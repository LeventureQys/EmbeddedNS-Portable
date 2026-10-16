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
**	日　期  : 2023年1月12日
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



/* 宏定义 */
typedef struct
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 unused;
} COLOR_CLASS;


/* 函数声明 */
void _LedInit(void);
void _AllLedOff(void);
void _AllLedBlink(void);


/* 变量定义和声明 */
UI_LED_HANDLE  UiLed = 
{
	.Init        = _LedInit,
	.AllOff      = _AllLedOff,
	.AllLedBlink = _AllLedBlink,
};

COLOR_CLASS RgbLedStatus[RGB_LED_NUM] = {0};


/*
****************************************************************************************************
**
**                                        _LedInit()
**
**	功能描述: 
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void _LedInit(void)
{
	_AllLedOff();
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
		RgbLedStatus[i].r = 0;
		RgbLedStatus[i].g = 0;
		RgbLedStatus[i].b = 0;
	}
	Spi.LedReflesh((uint8*)RgbLedStatus);
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
			RgbLedStatus[i].r = r;
			RgbLedStatus[i].g = 0;
			RgbLedStatus[i].b = 0;
		}
		Spi.LedReflesh((uint8*)RgbLedStatus);
		
		OsTick.DelayMs(150);
	}
}




