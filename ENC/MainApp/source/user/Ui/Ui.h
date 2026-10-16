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
**	文件名	: Ui.h
**
**	程序员	:
**
**	日　期  : 2021年12月23日
**
**	文件描述: Ui处理。
**
****************************************************************************************************
*/
#ifndef _Ui_H
#define _Ui_H
#include "UserRetype.h"
#include "Ui.Led.h"
#include "Ui.Key.h"
#include "Ui.Encoder.h"



// 各菜单定义
#define LED_SHOW_MODE_NUM		(9)		// 1种彩色循环，1种呼吸，7种单色



#define CURR		(0)
#define LAST		(1)


typedef struct
{
	struct
	{
		uint16 MicMute;
		uint16 NoiseSuppressorEnabled;
//		uint16 ReverbEnabled;
		uint16 NoiseSuppressor4Levels;  // Liang
		uint16 LedShowOnOff;
		uint16 LedShowMode;
		
		uint16 UsbOutIncrease;	// Steve
        uint16 Game71OnOff;	// Niko
	} Last;		// 记录旧值，用于判断
} UI_PARAM;


// typedef struct
// {
// 	struct
// 	{
// 		uint32 LedShow;			// 按键循环变化，包含关状态
// 	} Menu;
// } UI_CTRL;


typedef struct
{
	UI_LED_HANDLE *Led;
	UI_KEY_HANDLE *Key;

	void (*Init)(void);
	void (*Task)(void);
	void (*Disable)(void);

} UI_HANDLE;

extern UI_HANDLE  Ui;

#endif




