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
**	文件名	: Mem.SaveLoad.c
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 参数保存与加载。
**
****************************************************************************************************
*/
#include <string.h>
#include "UserRetype.h"
#include "OsTick.h"
#include "Iap.h"
#include "Dsp.h"
#include "ParamTable.h"
#include "Memory.h"
#include "Memory2.h"



/*
****************************************************************************************************
**
**                                      MemParamsInit()
**
**
**	功能描述: 预设出厂初始化
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemParamsInit(void)
{
	uint32 i, j;
	uint32 status = 0;
	char name[PRESET_NAME_STRING_LEN * 2] = "Factory Preset 1";
	uint16 *pt;
	
	
	
	ParamTbl.SetDefault.DevInfo();
	ParamTbl.SetDefault.Config();
	ParamTbl.SetDefault.Preset();
	
	for (i=0; i<NUM_OF_PRESET; i++)
	{
		name[7] = '1' + i;

		pt = (uint16*)name;
		for (j=0; j<PRESET_NAME_STRING_LEN; j++)
		{
			ParamTbl.Preset->PresetName[j] = *pt++;
		}
		
		status = MemPresetDataWrite(i);
	}
	
	status = MemDeviceInfoWrite();
	status = MemConfigDataWrite();
	(void)status;
}


/*
****************************************************************************************************
**
**                                      MemParamsInit4Dfu()
**
**
**	功能描述: DFU结束后，需要对DevInfo，Config进行重新初始化。而一般Preset则不一定要这样的操作。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemParamsInit4Dfu(void)
{
	uint32 status = 0;
	
	
	ParamTbl.SetDefault.DevInfo();
	ParamTbl.SetDefault.Config();
	
	status = MemDeviceInfoWrite();
	status = MemConfigDataWrite();
	(void)status;
}


/*
****************************************************************************************************
**
**                                   MemDeviceInfoWrite()
**
**
**	功能描述: 保存DevInfo到存储器（Flash）中
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemDeviceInfoWrite(void)
{
	uint32 ret = 0;
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = 0,
		.pData      = (uint8*)ParamTbl.DevInfoArray,
		.Length     = DEV_INFO_TBL_SIZE * sizeof(uint16),
		
		.Address    = MEM_STORE_DEV_INFO_ADDR,
		.SizeOfData = DEV_INFO_TBL_SIZE * sizeof(uint16),
		.SizeOfBank = DEV_INFO_BANK_SIZE,
		.NumOfBank  = DEV_INFO_BANK_NUM,
	};
	
	
	ret = MemWrite(&MemHandle);
	return ret;
}


/*
****************************************************************************************************
**
**                                   MemDeviceInfoRead()
**
**
**	功能描述: 从存储器（Flash）中读取DevInfo
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemDeviceInfoRead(void)
{
	uint32 ret = 0;
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = 0,
		.pData      = (uint8*)ParamTbl.ReadBufTemp,
		.Length     = DEV_INFO_TBL_SIZE * sizeof(uint16),
		
		.Address    = MEM_STORE_DEV_INFO_ADDR,
		.SizeOfData = DEV_INFO_TBL_SIZE * sizeof(uint16),
		.SizeOfBank = DEV_INFO_BANK_SIZE,
		.NumOfBank  = DEV_INFO_BANK_NUM,
	};
	
	
	ret = MemRead(&MemHandle, false);
	if (ret)
	{
		memcpy(ParamTbl.DevInfoArray, ParamTbl.ReadBufTemp, DEV_INFO_TBL_SIZE * sizeof(uint16));
		ParamTbl.FwInfoSNSet();
	}
	
	return ret;
}


/*
****************************************************************************************************
**
**                                   MemReadModelCode()
**
**
**	功能描述: 从存储器（Flash）中读取Model Code
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemReadModelCode(uint16 *mc)
{
	uint32 ret = 0;
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = 0,
		.pData      = (uint8*)ParamTbl.ReadBufTemp,
		.Length     = DEV_INFO_TBL_SIZE * sizeof(uint16),
		
		.Address    = MEM_STORE_DEV_INFO_ADDR,
		.SizeOfData = DEV_INFO_TBL_SIZE * sizeof(uint16),
		.SizeOfBank = DEV_INFO_BANK_SIZE,
		.NumOfBank  = DEV_INFO_BANK_NUM,
	};
	
	
	ret = MemRead(&MemHandle, false);
	if (ret)
	{
		memcpy(mc, ParamTbl.ReadBufTemp, 3 * sizeof(uint16));
	}
	else
	{
		mc[0] = 0;
		mc[1] = 0;
		mc[2] = 0;
	}
	
	return ret;
}


/*
****************************************************************************************************
**
**                                   MemConfigDataWrite()
**
**
**	功能描述: 保存ConfigData到存储器（Flash）中
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemConfigDataWrite(void)
{
	uint32 ret = 0;
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = 0,
		.pData      = (uint8*)ParamTbl.ConfigArray,
		.Length     = CONFIG_TBL_SIZE * sizeof(uint16),
		
		.Address    = MEM_STORE_CONFIG_DATA_ADDR,
		.SizeOfData = CONFIG_TBL_SIZE * sizeof(uint16),
		.SizeOfBank = CFG_DATA_BANK_SIZE,
		.NumOfBank  = CFG_DATA_BANK_NUM,
	};
	
	
	ret = MemWrite(&MemHandle);
	return ret;
}


/*
****************************************************************************************************
**
**                                   MemConfigDataRead()
**
**
**	功能描述: 从存储器（Flash）中读取ConfigData
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemConfigDataRead(void)
{
	uint32 ret = 0;
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = 0,
		.pData      = (uint8*)ParamTbl.ReadBufTemp,
		.Length     = CONFIG_TBL_SIZE * sizeof(uint16),
		
		.Address    = MEM_STORE_CONFIG_DATA_ADDR,
		.SizeOfData = CONFIG_TBL_SIZE * sizeof(uint16),
		.SizeOfBank = CFG_DATA_BANK_SIZE,
		.NumOfBank  = CFG_DATA_BANK_NUM,
	};
	
	
	ret = MemRead(&MemHandle, false);	
	if (ret)
	{
		memcpy(ParamTbl.ConfigArray, ParamTbl.ReadBufTemp, CONFIG_TBL_SIZE * sizeof(uint16));
	}
	
	return ret;
}


/*
****************************************************************************************************
**
**                                        MemPresetDataWrite()
**
**
**	功能描述: 把当前预设参数保存到存储器（Flash）中
**
**	参  数  : presetIdx:   预设序号，从0开始
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemPresetDataWrite(uint32 presetIdx)
{
	uint32 ret = 0;	
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = 0,
		.pData      = (uint8*)ParamTbl.PresetArray,
		.Length     = PRESET_TBL_SIZE * sizeof(uint16),
		
		.Address    = 0,
		.SizeOfData = PRESET_TBL_SIZE * sizeof(uint16),	
		.SizeOfBank = PRESET_BANK_SIZE,
		.NumOfBank  = PRESET_BANK_NUM,
	};


	
	if (presetIdx > (NUM_OF_PRESET-1))
		return 0;

	MemHandle.Address = MEM_STORE_PRESET_ADDR(presetIdx);
	ret = MemWrite(&MemHandle);
	
	return ret;
}


/*
****************************************************************************************************
**
**                                        MemPresetDataRead()
**
**
**	功能描述: 从存储器（Flash）中调取一个预设参数，到当前预设参数中来
**
**	参  数  : presetIdx:   预设序号，从0开始
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemPresetDataRead(uint32 presetIdx)
{
	uint32 ret = 0;
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = 0,
		.pData      = (uint8*)ParamTbl.ReadBufTemp,
		.Length     = PRESET_TBL_SIZE * sizeof(uint16),
		
		.Address    = 0,
		.SizeOfData = PRESET_TBL_SIZE * sizeof(uint16),
		.SizeOfBank = PRESET_BANK_SIZE,
		.NumOfBank  = PRESET_BANK_NUM,
	};
	
	
	
	if (presetIdx > (NUM_OF_PRESET-1))
		return 0;
	
	MemHandle.Address = MEM_STORE_PRESET_ADDR(presetIdx);
	ret = MemRead(&MemHandle, false);
	
	if (ret)
	{
		memcpy(ParamTbl.PresetArray, ParamTbl.ReadBufTemp, PRESET_TBL_SIZE * sizeof(uint16));
	}
	
	return ret;
}


/*
****************************************************************************************************
**
**                                        MemPresetDataReadBytes()
**
**
**	功能描述: 从某个预设中，按偏移地址读取一定长度的数据。
**
**	参  数  : presetIdx  预设序号，从0开始
**            addrOffset 在预设中的偏移地址
**            length     参数数目，不超过124
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemPresetDataReadBytes(uint32 presetIdx, uint32 addrOffset, uint8 *buf, uint32 regNum)
{
	uint32 ret = 0;
	MEM_DRIVER_HANDLE MemHandle = 
	{
		.Offset     = addrOffset * 2,	// 每个参数是两个字节大小，地址偏移要乘以2
		.pData      = buf,
		.Length     = regNum * 2,  		// 每个参数是两个字节大小，这里是字节数目
		
		.Address    = 0,
		.SizeOfData = PRESET_TBL_SIZE * sizeof(uint16),
		.SizeOfBank = PRESET_BANK_SIZE,
		.NumOfBank  = PRESET_BANK_NUM,
	};
	
	
	
	if (presetIdx > (NUM_OF_PRESET-1))
		return 0;
	
	MemHandle.Address = MEM_STORE_PRESET_ADDR(presetIdx);
	ret = MemRead(&MemHandle, false);

	return ret;
}


/*
****************************************************************************************************
**
**                                   MemPresetLoad()
**
**
**	功能描述: 加载模式
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 MemPresetLoad(uint32 idx)
{
	uint32 ret = 0;
	uint16 paramAddr = MEM_PRESET_DATA_BASE;
	
	
	if (idx < NUM_OF_PRESET)
	{
		Dsp.Program->Mute();
		OsTick.DelayMs(100);
		
		ret = MemPresetDataRead(idx);
		if (ret)
		{
			Dsp.Program->AMSet.SequParam(&paramAddr, PRESET_TBL_SIZE);
			Dsp.Program->UnmuteDelay(50);
		}
	}
	return ret;
}




