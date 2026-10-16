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
**	文件名	: Dsp.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: DSP类。与DSP相关的功能、数据都由此类来管理。
**
****************************************************************************************************
*/
#ifndef	_DSP_H
#define	_DSP_H
#include "UserRetype.h"
#include "Dsp.Config.h"
#include "Dsp.Program.h"
#include "Dsp.Program.ModuleTable.h"



typedef struct
{
//	uint32 ParamInitFlag;		// Dsp参数批量设置
	
	void (*Init)(void);
	void (*Task)(void);
	
	DSP_COMM    *Comm;
	DSP_PROGRAM *Program;

} DSP_CLASS;

extern DSP_CLASS Dsp;

#endif




