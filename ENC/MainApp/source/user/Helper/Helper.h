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
**	文件名	: Helper.h
**
**	程序员	: 
**
**	日　期  : 2022年4月1日
**
**	文件描述: 辅助性函数方法。
**
****************************************************************************************************
*/
#ifndef _HELPER_H
#define _HELPER_H
#include "UserRetype.h"



#define QF			(1000)		// Q format
#define QF_DIV2		(500)


typedef struct 
{
	void (*BitSet )(uint32 *var, uint32 value, uint32 bitStart, uint32 bitNum);
	void (*BitSet2)(uint32 *var, uint32 value, uint32 bitStart, uint32 bitEnd);
	void (*BitClr )(uint32 *var, uint32 bitStart, uint32 bitNum);
	void (*BitClr2)(uint32 *var, uint32 bitStart, uint32 bitEnd);

	// void (*HSV2RGB)(fp32 h, fp32 s, fp32 v, uint8 *r, uint8 *g, uint8 *b);
	void (*HSV2RGB)(uint32 h, uint32 s, uint32 v, uint8 *rr, uint8 *gg, uint8 *bb);
	void (*RGB2HSV)(uint8 r, uint8 g, uint8 b, fp32 *h, fp32 *s, fp32 *v);

} HELPER_HANDLE;

extern const HELPER_HANDLE Hp;

#endif




