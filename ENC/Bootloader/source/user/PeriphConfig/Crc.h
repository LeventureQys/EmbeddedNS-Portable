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
**	文件名	: Crc.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: CRC计算
**
****************************************************************************************************
*/
#ifndef _CRC_H
#define _CRC_H
#include "UserRetype.h"



typedef struct 
{
	void   (*Init )(void);
	void   (*Reset)(void);
    void   (*Add  )(uint32 data);
    uint32 (*Get  )(void);
	uint32 (*Calc )(uint32 *data, uint32 length);
	uint32 (*CalcContinue)(uint32 *data, uint32 length);
} CRC32_HANDLE;

extern CRC32_HANDLE Crc32;

#endif




