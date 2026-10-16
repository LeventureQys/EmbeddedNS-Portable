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
**	文件名　: ParamTable.h
**
**	程序员　: 
** 
**	日　期  : 2023年1月12日
** 
**	文件描述: 系统参数表管理。
** 
***************************************************************************************************
*/
#ifndef _PARAM_TABLE_H
#define _PARAM_TABLE_H
#include "UserRetype.h"



#define DEV_INFO_TBL_SIZE             (11)


/* 设备信息 - 参数地址定义 */
#define PARAM_MODEL_CODE              (0x0000)
#define PARAM_MODEL_NAME              (0x0003)


typedef struct
{
	void (*Init)(void);
	
	uint16 DevInfo[DEV_INFO_TBL_SIZE];

} PARAM_TBL_HANDLE;

extern PARAM_TBL_HANDLE ParamTbl;

#endif




