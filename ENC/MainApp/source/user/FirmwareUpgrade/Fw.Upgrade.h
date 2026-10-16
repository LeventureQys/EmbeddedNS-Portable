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
**	文件名	: Fw.Upgrade.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 固件升级处理文件。
**
****************************************************************************************************
*/
#ifndef	_FW_UPGRADE_H
#define	_FW_UPGRADE_H
#include "UserRetype.h"



/* DFU状态字，定义如下: 
*/
#define DFU_SB_SIZE 			(4u)		// 字节数

#define DFU_SB_1ST_INIT     	(0xFFu)		// 是DFU状态字第一次初始化标志
#define DFU_SB_1ST_INIT_CLR 	(0xA5u)

#define BL_JUMP_TO_DFU			(0x00u)		// 是程序跳转判断，指示引导程序跳转到哪个程序
#define BL_JUMP_TO_APP			(0xFFu)

#define DFU_START				(0x01u)		// 指示升级开始
#define DFU_START_CLR			(0x00u)

#define DFU_SUCCESS_END			(0x02u)		// 成功升级
#define DFU_FAIL_END			(0x01u)		// 已经执行升级，但未完成或失败
#define DFU_END_CLR				(0x00u)


typedef struct
{
	uint32 Flag;			// 1 已经完成新的一次DFU，0 则没有
	
	void (*FinishProcess)(void);
	void (*Start)(void);
	void (*Task)(void);
	
} FWU_HANDLE;

extern FWU_HANDLE Fwu;

#endif




