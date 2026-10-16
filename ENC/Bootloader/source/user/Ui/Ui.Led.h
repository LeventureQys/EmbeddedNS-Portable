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
**	文件名	: Ui.Led.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: LED应用。
**
****************************************************************************************************
*/
#ifndef _Ui_LED_H
#define _Ui_LED_H
#include "UserRetype.h"



typedef enum
{
	RGB_LED_1,
	RGB_LED_2,
	RGB_LED_3,
	RGB_LED_4,
	RGB_LED_5,
	RGB_LED_6,
	RGB_LED_7,
	RGB_LED_8,
	RGB_LED_9,
    RGB_FX_MUTE,	// 功能复用灯
	
	RGB_LED_NUM,

} RGB_LED_ENUM;

#define LED_ON			(1)
#define LED_OFF			(0)


typedef struct 
{
	void (*Init)(void);
	void (*AllOff)(void);
	void (*AllLedBlink)(void);

} UI_LED_HANDLE;

extern UI_LED_HANDLE  UiLed;

#endif




