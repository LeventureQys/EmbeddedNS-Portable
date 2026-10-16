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
**	文件名	: Dsp.Program.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: DSP编程，与DSP相关的所有操作。
**
****************************************************************************************************
*/
#ifndef	_DSP_PROGRAM_H
#define	_DSP_PROGRAM_H
#include "UserRetype.h"



typedef struct
{
	void (*Bootloader)(void);

	struct
	{
		void (*SequParam)(uint16 *paramStartAddr, uint32 num);
		void (*RandParam)(uint8  *pData, uint32 len);
	} AMSet;
	
	void (*Mute  )(void);
	void (*Unmute)(void);
	
	void (*UnmuteDelay)(uint32 ms);
	void (*UnmutePostProcess)(void);
	
	void (*RxAckProcess)(uint8 *data);

	int32 (*ReadMicSigLev)(void);

} DSP_PROGRAM;

#endif




