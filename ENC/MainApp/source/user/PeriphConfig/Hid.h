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
**	文件名	: Hid.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: USB HID应用
**
****************************************************************************************************
*/
#ifndef _HID_H
#define _HID_H
#include "UserRetype.h"



typedef struct 
{
	void   (*Init )(void);
	
	void   (*SendBytes  )(uint8 *data, uint32 len);
	void   (*RxFiFoWrite)(uint8 *data, uint32 len);
	uint32 (*RxFiFoRead )(uint8 *data);
} HID_HANDLE;

extern HID_HANDLE Hid;

#endif




