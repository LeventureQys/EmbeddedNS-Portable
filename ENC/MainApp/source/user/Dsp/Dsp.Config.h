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
**	文件名	: Dsp_Config.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: DSP配置
**
****************************************************************************************************
*/
#ifndef	_DSP_CONFIG_H
#define	_DSP_CONFIG_H
#include "UserRetype.h"



typedef struct
{
	void (*Init)(void);
} DSP_CONFIG;


typedef struct
{
	void  (*Init)(void);
	
	uint8 (*SpiWR  )(uint8 dat);
	void  (*SpiCS  )(uint32 sla, uint32 lev);
	void  (*DelayMs)(uint32 i);
	void  (*DelayUs)(uint32 i);
} DSP_COMM;

#endif




