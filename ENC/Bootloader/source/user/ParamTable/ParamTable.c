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
**	文件名	: ParamTable.c
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: 系统参数表管理。
**
****************************************************************************************************
*/
#include "ProjectConfig.h"
#include "ParamTable.h"
#include "SctConfig.h"



/* 函数声明 */
void ParamTblInit(void);


/* 变量定义和声明 */
PARAM_TBL_HANDLE ParamTbl = 
{
	.Init = ParamTblInit,
	
	.DevInfo = {0},
};


/*
****************************************************************************************************
**
**                                        _ParamTblInit()
**
**
**	功能描述: 参数表初始化
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void ParamTblInit(void)
{
	/* 通过读 MainApp 工程中的 FwInfoArray 来初始化Model Code, Model Name
	*/
	uint8  *pt;
	uint16 *pInfo = (uint16*)(FLASH_FW_INFO_ADDR);
	
	
	ParamTbl.DevInfo[ 0] = *pInfo++;
	ParamTbl.DevInfo[ 1] = *pInfo++;
	ParamTbl.DevInfo[ 2] = *pInfo++;
							pInfo++;		// 为了4字节对齐，多了一个uint16
	ParamTbl.DevInfo[ 3] = *pInfo++;
	ParamTbl.DevInfo[ 4] = *pInfo++;
	ParamTbl.DevInfo[ 5] = *pInfo++;
	ParamTbl.DevInfo[ 6] = *pInfo++;
	ParamTbl.DevInfo[ 7] = *pInfo++;
	ParamTbl.DevInfo[ 8] = *pInfo++;
	ParamTbl.DevInfo[ 9] = *pInfo++;
	ParamTbl.DevInfo[10] = *pInfo++;
	
	
	pt = (uint8*)(&ParamTbl.DevInfo[0]);
	for (uint32 i=0; i<6; i++)
	{
		if (*pt == 0xFF)					// MainApp未初始化FwInfoArray，则参数置0
		{
			ParamTbl.DevInfo[0] = 0;
			ParamTbl.DevInfo[1] = 0;
			ParamTbl.DevInfo[2] = 0;
			break;
		}		
	}
	
	pt = (uint8*)(&ParamTbl.DevInfo[3]);
	for (uint32 i=0; i<16; i++)
	{
		if (*pt == 0xFF)
		{
			ParamTbl.DevInfo[ 3] = 0;
			ParamTbl.DevInfo[ 4] = 0;
			ParamTbl.DevInfo[ 5] = 0;
			ParamTbl.DevInfo[ 6] = 0;
			ParamTbl.DevInfo[ 7] = 0;
			ParamTbl.DevInfo[ 8] = 0;
			ParamTbl.DevInfo[ 9] = 0;
			ParamTbl.DevInfo[10] = 0;
			break;
		}		
	}
}




