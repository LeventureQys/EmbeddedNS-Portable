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
**	文件名	: Ui.Encoder.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 编码器应用。
**
****************************************************************************************************
*/
#ifndef _Ui_ENC_H
#define _Ui_ENC_H
#include "UserRetype.h"



typedef struct 
{
	struct
	{
		uint32 Event;
		int32  Value;
	} Param;
	
	void (*Init)(void);
	void (*Reset)(void);
	void (*Scan)(void);
	void (*Process)(void);

} UI_ENC_HANDLE;

extern UI_ENC_HANDLE  UiEnc;

#endif


