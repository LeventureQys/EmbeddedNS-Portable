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
**	文件名	: Ui.Key.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 按键应用。
**
****************************************************************************************************
*/
#ifndef _Ui_KEY_H
#define _Ui_KEY_H
#include "UserRetype.h"



typedef enum
{
	KEY_FX,			// 静音，降噪开关
	KEY_RGB,		// 切换灯的显示模式
	
	KEY_NUM,
} KEY_ENUM;


typedef struct 
{
	void (*Init)(void);
	void (*Reset)(void);
	void (*Process)(void);
	
	void   (*EventCleanAll)(void);	
	uint32 (*KeyClick     )(KEY_ENUM key);
	uint32 (*KeyLongClick )(KEY_ENUM key);
	uint32 (*KeyResClick  )(void);

} UI_KEY_HANDLE;

extern UI_KEY_HANDLE  UiKey;

#endif


