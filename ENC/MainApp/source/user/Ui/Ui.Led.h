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
**	日　期  : 2021年12月23日
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

    RGB_UP_1   = RGB_LED_7,		// 网头上3个灯
    RGB_UP_2   = RGB_LED_8,
    RGB_UP_3   = RGB_LED_9,
	
    RGB_DOWN_1 = RGB_LED_1,		// 网头下6个灯
    RGB_DOWN_2 = RGB_LED_2,
    RGB_DOWN_3 = RGB_LED_3,
    RGB_DOWN_4 = RGB_LED_4,
    RGB_DOWN_5 = RGB_LED_5,
    RGB_DOWN_6 = RGB_LED_6,

} RGB_LED_ENUM;


#define SHOW_LED_NUM			(9)
#define SHOW_LED_UP_NUM			(3)
#define SHOW_LED_DOWN_NUM		(6)

#define LED_ON					(1)
#define LED_OFF					(0)

//RGB灯效种数（短按RGB触摸键依次切换的7种：跑马灯/渐变/紫/青/黄/绿/蓝）
#define RGB_SHOW_MODE_NUM		(7)


typedef struct 
{	
	void (*Init)(void);
	void (*Process)(void);
	
	void (*AllOff      )(void);
	void (*AllOffAction)(void);
	void (*AllLedBlink )(void);
	
	void (*FxMute)(uint32 mute, uint32 nsEn);
	void (*RgbShow)(uint32 onoff, uint32 mode, uint32 lightness);

} UI_LED_HANDLE;

extern UI_LED_HANDLE  UiLed;

#endif




