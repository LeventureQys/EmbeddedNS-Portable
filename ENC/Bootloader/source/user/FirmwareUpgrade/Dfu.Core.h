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
**	文件名	: Dfu.Core.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: 判断进入升级模式，用户程序的跳转等等。
**
****************************************************************************************************
*/
#ifndef _DFU_CORE_H
#define _DFU_CORE_H
#include "UserRetype.h"



/* DFU状态字节，定义如下: 
*/
#define DFU_SB_SIZE 			(4u)		// 字节数

#define DFU_SB_1ST_INIT     	(0xFFu)		// 是DFU状态字节第一次初始化标志
#define DFU_SB_1ST_INIT_CLR 	(0xA5u)

#define BL_JUMP_TO_DFU			(0x00u)		// 是程序跳转判断，指示引导程序跳转到哪个程序
#define BL_JUMP_TO_APP			(0xFFu)

#define DFU_START				(0x01u)		// 指示升级开始
#define DFU_START_CLR			(0x00u)

#define DFU_END_SUCCESS			(0x02u)		// 成功升级
#define DFU_END_FAIL			(0x01u)		// 已经执行升级，但未完成或失败
#define DFU_END_CLR				(0x00u)


typedef struct
{
	void (*Init)(void);
	void (*Task)(void);
	
	void (*Terminate)(void);
	
	struct
	{
		fp32 Value;		// 升级进度记录 0 - 100
        uint32 DfuTimeOut;
	} Progress;
	
	struct
	{
		void (*SetEndFlag)(uint32 flag);
	} StatusBytes;
	
	struct
	{
		uint32 (*ValidCheck)(uint8 *data, uint32 len);
		uint32 (*WriteFlash)(uint8 *data, uint32 len);
		
		void   (*SetCompletedFlag)(void);
		uint32 (*GetCompletedFlag)(void);
		
		void (*DataDecrypt)(uint8 *data, uint32 len);
	} Program;
	
} DFU_HANDLE;

extern DFU_HANDLE Dfu;

#endif




