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
**	文件名	: Fw.DfuStatusBytes.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: 指定DFU Status Bytes的内容。
**
****************************************************************************************************
*/
#include "main.h"



/* 
#define DFU_SB_1ST_INIT     	(0xFFu)		// 是DFU状态字节第一次初始化标志
#define DFU_SB_1ST_INIT_CLR 	(0xA5u)

#define BL_JUMP_TO_DFU			(0x00u)		// 是程序跳转判断，指示引导程序跳转到哪个程序
#define BL_JUMP_TO_APP			(0xFFu)

#define DFU_START				(0x01u)		// 指示升级开始
#define DFU_START_CLR			(0x00u)

#define DFU_SUCCESS_END			(0x02u)		// 成功升级
#define DFU_FAIL_END			(0x01u)		// 已经执行升级，但未完成或失败
#define DFU_END_CLR				(0x00u)
*/

const uint8 DfuStatusBytesArray[DFU_SB_SIZE]__attribute__((at(FLASH_DFU_SB_ADDR))) = 
{
	DFU_SB_1ST_INIT_CLR,
	BL_JUMP_TO_DFU,
	DFU_START_CLR,
	DFU_END_CLR,
};




