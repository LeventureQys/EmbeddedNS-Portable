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
**	文件名	: Fw.Info.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 固件信息
**
****************************************************************************************************
*/
#include "Iap.h"
#include "SctConfig.h"
#include "ParamTable.h"
#include "Fw.Info.h"



/* 固件信息，主要存放
	Model Code
	Model Name
	Device SN
   
   固件信息，DFU工程也可以访问。
*/
#if 0		// 只有调试才开启
const uint8 FwInfoArray[88]__attribute__((at(FLASH_FW_INFO_ADDR))) = 
{
	(MODEL_CODE_PARAM1 & 0xFF), ((MODEL_CODE_PARAM1 >> 8) & 0xFF),
	(MODEL_CODE_PARAM2 & 0xFF), ((MODEL_CODE_PARAM2 >> 8) & 0xFF),
	(MODEL_CODE_PARAM3 & 0xFF), ((MODEL_CODE_PARAM3 >> 8) & 0xFF),
	0xFF, 0xFF,							// 补两字节，用于4字节对齐
	
	(MODEL_NAME_PARAM1 & 0xFF), ((MODEL_NAME_PARAM1 >> 8) & 0xFF),
	(MODEL_NAME_PARAM2 & 0xFF), ((MODEL_NAME_PARAM2 >> 8) & 0xFF),
	(MODEL_NAME_PARAM3 & 0xFF), ((MODEL_NAME_PARAM3 >> 8) & 0xFF),
	(MODEL_NAME_PARAM4 & 0xFF), ((MODEL_NAME_PARAM4 >> 8) & 0xFF),
	(MODEL_NAME_PARAM5 & 0xFF), ((MODEL_NAME_PARAM5 >> 8) & 0xFF),
	(MODEL_NAME_PARAM6 & 0xFF), ((MODEL_NAME_PARAM6 >> 8) & 0xFF),
	(MODEL_NAME_PARAM7 & 0xFF), ((MODEL_NAME_PARAM7 >> 8) & 0xFF),
	(MODEL_NAME_PARAM8 & 0xFF), ((MODEL_NAME_PARAM8 >> 8) & 0xFF),
	
	0xFF, 0xFF, 0xFF, 0xFF, 			// Flash地址为：0x08022000 + 24 = 0x08022018
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
};
#endif


/*
****************************************************************************************************
**
**                                     FwInfoSetMCMN()
**
**
**	功能描述: 
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void FwInfoSetMCMN(uint16 *mc, uint16 *mn)
{
	uint32 i;
	uint32 reset = 0;
	uint16 *info;
	uint8  buf[256];
	
	
	Iap.Read(FLASH_FW_INFO_ADDR, (uint32*)buf, sizeof(buf));
	
	info = (uint16*)&buf[0];
	for (i=0; i<3; i++)
	{
		if (info[i] != mc[i])
		{
			reset = 1;
			break;
		}
	}
	
	if (reset)
	{
		info = (uint16*)&buf[0];
		for (i=0; i<3; i++)
		{
			info[i] = mc[i];
		}
		
		info = (uint16*)&buf[4*2];
		for (i=0; i<8; i++)
		{
			info[i] = mn[i];
		}
		
		Iap.Write(FLASH_FW_INFO_ADDR, (uint32*)buf, sizeof(buf));
	}
}


/*
****************************************************************************************************
**
**                                     FwInfoSNSave()
**
**
**	功能描述: 
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void FwInfoSNSave(void)
{
	uint32 i;
	uint8  buf[256];
	
	
	Iap.Read(FLASH_FW_INFO_ADDR, (uint32*)buf, sizeof(buf));

	for (i=0; i<32; i++)
	{
		buf[2*i + 24] = ParamTbl.DevInfo->SN[i];
		buf[2*i + 25] = ParamTbl.DevInfo->SN[i] >> 8;
	}

	Iap.Write(FLASH_FW_INFO_ADDR, (uint32*)buf, sizeof(buf));
}




