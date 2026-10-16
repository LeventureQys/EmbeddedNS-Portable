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
**	文件名	: Com.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: Communication通讯处理类。应用于PC电脑软件控制。一般是USB、串口等。
**
****************************************************************************************************
*/
#ifndef _COM_H
#define _COM_H
#include "UserRetype.h"
#include "Com.HidRx.h"
#include "Com.DataProcess.h"



typedef struct
{
	void (*Init)(void);
	void (*Task)(void);
} COM_HANDLE;

extern COM_HANDLE Com;

#endif




