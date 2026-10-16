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
**	日　期  : 2023年1月12日
**
**	文件描述: Ui处理。
**
****************************************************************************************************
*/
#ifndef _Ui_H
#define _Ui_H
#include "UserRetype.h"
#include "Ui.Led.h"



typedef struct
{
	UI_LED_HANDLE *Led;

	void (*Init)(void);

} UI_HANDLE;

extern UI_HANDLE  Ui;

#endif




