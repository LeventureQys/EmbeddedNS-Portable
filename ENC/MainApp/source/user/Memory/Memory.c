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
**	文件名	: Memory.c
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 系统存储管理。
**
****************************************************************************************************
*/
#include "UserRetype.h"
#include "OsTick.h"
#include "Iap.h"
#include "Dsp.h"
#include "ParamTable.h"
#include "Memory.h"
#include "Memory2.h"
#include "Fw.Upgrade.h"
#include "Fw.Info.h"



/* 函数声明 */
void   MemInit(void);
void   MemHwInit(void);
void   MemTask(void);

void   MemResFactory(void);
void   MemDspAudioModuleInit(void);

uint32 MemParamRandWrite(uint16 *addr, uint16 *data);
uint32 MemParamRandRead (uint16 *addr, uint16 *data);
void   MemParamSequRead (uint16 *addr, uint8 *u8buf, uint32 len);

void   MemCfgParamAutoSaveSet(void);
void   MemPresetParamAutoSaveSet(void);
void   MemPresetParamAutoSaveClr(void);


/* 变量定义和声明 */
MEM_HANDLE Mem = 
{
	.Init = MemInit,
	.Task = MemTask,
	
	.ParamWrite =
	{
		.Rand = MemParamRandWrite,
	},
	.ParamRead =
	{
		.Rand = MemParamRandRead,
		.Sequ = MemParamSequRead,
	},
	.Save =
	{
		.DevInfoParam = MemDeviceInfoWrite,
		.ConfigParam  = MemConfigDataWrite,
		.PresetParam  = MemPresetDataWrite,
	},
	.Load =
	{
		.DevInfoParam = MemDeviceInfoRead,
		.ConfigParam  = MemConfigDataRead,
		.PresetParam  = MemPresetDataRead,
	},
	.LoadPreset = MemPresetLoad,
	
	.ResFactory = MemResFactory,
	.CfgParamAutoSavePreview = MemCfgParamAutoSaveSet,
	.PresetParamAutoSavePreview = MemPresetParamAutoSaveSet,
    .PresetParamAutoSaveCancel = MemPresetParamAutoSaveClr
};

uint32 AutoSaveTimer = 0;			// Config param
uint32 PresetAutoSaveTimer = 0;		// Preset param


/*
****************************************************************************************************
**
**                                        MemInit()
**
**
**	功能描述: 系统内存管理服务初始化。它是一个大整体，包含DevInfo，Preset和McuData等。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemInit(void)
{
	uint32 initFlag = 0;
	uint16 modelCode[3] = {0};



	// 各参数表在MCU启动后的初始化
	ParamTbl.SetDefault.DevInfo();
	ParamTbl.SetDefault.Config();
	ParamTbl.SetDefault.Preset();
	
	
	/* 读出并判断Model Code，如果不符，则需要进行第一次初始化 */
	MemReadModelCode(modelCode);
	
	if (true == ParamTbl.Get1stInitFlag(modelCode))
    //if (true)
	{
		initFlag = 1;
		goto L_INIT_PROC;
	}
	else
	{
		MemDeviceInfoRead();
		
		MemConfigDataRead();
		if (ParamTbl.GetReInitFlag(ParamTbl.Config->SysReInit) == true)	// 判断是否恢复出厂设置
		{
			initFlag = 1;
			goto L_INIT_PROC;
		}
		else
		{
//			OsTick.DelayMs(300);  // Liang
			// pConfig->PstStartupIdx = 0 调取默认预设。不需要操作即是默认预设
			if (ParamTbl.Config->PstStartupIdx > 0 && ParamTbl.Config->PstStartupIdx <= NUM_OF_PRESET)
			{
				uint16 nAddr = 0;
				uint16 nData = 0;
				nAddr = 0x0001 | (((ParamTbl.Config->PstStartupIdx-1) + 3)<<12);
				Mem.ParamRead.Rand(&nAddr, &nData);
				
				if (nData == 1)		// 只有预设有效标志有效才可以读出来。不然不执行，那就是默认预设
				{	// 预设参数有效 -> 调取预设
					MemPresetDataRead(ParamTbl.Config->PstStartupIdx - 1);
				}
			}
		}
	}

	L_INIT_PROC:
	if (initFlag)
	{
		MemHwInit();
		MemParamsInit();
	}
	
//	if (Fwu.Flag)  // Liang
//	{
//		Fwu.Flag = 0;
//		MemParamsInit4Dfu();
//	}
	
	FwInfoSetMCMN(ParamTbl.DevInfo->ModelCode, ParamTbl.DevInfo->ModelName);
    
    //设置当前软件的版本,提供上位机读取的
    ParamTbl.GetFirmwareVer();
    
	MemDspAudioModuleInit();
}


/*
****************************************************************************************************
**
**                                        MemHwInit()
**
**
**	功能描述: 底层初始化
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
//static	uint32 addr = MEM_STORE_START_ADDR;
void MemHwInit(void)
{
	uint32 addr = MEM_STORE_START_ADDR;
	
	
	while (addr < MEM_STORE_END_ADDR)
	{
		Iap.PageErase(addr);
		addr += FLASH_SECTOR_SIZE;
	}
}


/*
****************************************************************************************************
**
**                                        MemTask()
**
**
**	功能描述: Memory的任务处理
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemTask(void)
{
	/* Config param 自动保存 */
	if (OsTick.TimerMs(&AutoSaveTimer, TFB_CFG_AUTO_SAVE))
	{
		MemConfigDataWrite();
	}
	
	/* Preset param 自动保存 */
	if (OsTick.TimerMs(&PresetAutoSaveTimer, TFB_PRESET_AUTO_SAVE))
	{
		uint32 idx = ParamTbl.Config->PstSaveIdx;

	
		if (idx>0 && idx<=NUM_OF_PRESET)
		{
			ParamTbl.Preset->ValidFlag = 1;
			Mem.Save.PresetParam(idx-1);
		}
	}
}


/*
****************************************************************************************************
**
**                                        MemResFactory()
**
**
**	功能描述: 存储设备恢复出厂设置
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemResFactory(void)
{
	MemHwInit();
	MemParamsInit();
	MemDspAudioModuleInit();
	Dsp.Program->Unmute();
}


/*
****************************************************************************************************
**
**                                        MemDspAudioModuleInit()
**
**
**	功能描述: DSP 各模块根据预设重新初始化
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemDspAudioModuleInit(void)
{
	Dsp.Program->Mute();		// 调试DSP时，可以设成 Dsp.Unmute
	
#if !DBG_NO_DSP
	uint16 paramAddr = MEM_PRESET_DATA_BASE;
	Dsp.Program->AMSet.SequParam(&paramAddr, PRESET_TBL_SIZE);
#endif
}


/*
****************************************************************************************************
**
**                                     MemParamRandWrite()
**
**
**	功能描述: 写的时候，每个地址都需要判断合法，故每次只写一个参数就可以了
**
**	参  数  : 
**
**	返回值  : 0 错误，1 DEV_INFO，2 CONFIG_DATA，3 CURR PRESET DATA，4 User Preset
**
****************************************************************************************************
*/
uint32 MemParamRandWrite(uint16 *addr, uint16 *data)
{
	uint32 addrOffset = *addr & MEM_OFFSET_ADDR_MASK;
	
	
	
	switch (*addr & MEM_ADDR_TYPE_MASK)
	{
		case MEM_DEV_INFO_BASE: //---------------- Device Info Param Block
			if (addrOffset < DEV_INFO_TBL_SIZE)
			{
				ParamTbl.Set.DevInfo(addrOffset, *data);
				return IS_DEV_INFO;
			}
			break;
			
		case MEM_CONFIG_DATA_BASE: //------------- Config Param Block
			if (addrOffset < CONFIG_TBL_SIZE)
			{
				ParamTbl.Set.Config(addrOffset, *data);
				return IS_CFG_DATA;
			}
			break;
			
		case MEM_PRESET_DATA_BASE: //------------- Current Preset Param Block
			if (addrOffset < PRESET_TBL_SIZE)
			{
				ParamTbl.Set.Preset(addrOffset, *data);
				return IS_CURR_PRESET;
			}
			break;
	}
	
	return 0;
}


/*
****************************************************************************************************
**
**                                     MemParamRandRead()
**
**
**	功能描述: 对于随机读，地址是随机的，固本函数只读一个参数的值
**
**	参  数  : 
**
**	返回值  : 0 错误，1 正确
**
****************************************************************************************************
*/
uint32 MemParamRandRead(uint16 *addr, uint16 *data)
{
	uint32 memBlockType = *addr & MEM_ADDR_TYPE_MASK;
	uint32 addrOffset = *addr & MEM_OFFSET_ADDR_MASK;
	uint32 pstIdx;
	
	
	
	switch (memBlockType)
	{
		case MEM_DEV_INFO_BASE: //---------------- Device Info Param Block
			if (addrOffset < DEV_INFO_TBL_SIZE)
			{
				*data = ParamTbl.DevInfoArray[addrOffset];
			}
			else
			{
				return 0;
			}
			break;
			
		case MEM_CONFIG_DATA_BASE: //------------- Config Param Block
			if (addrOffset < CONFIG_TBL_SIZE)
			{
				*data = ParamTbl.ConfigArray[addrOffset];
			}
			else
			{
				return 0;
			}
			break;
			
		case MEM_PRESET_DATA_BASE: //------------- Current Preset Param Block
			if (addrOffset < PRESET_TBL_SIZE)
			{
				*data = ParamTbl.PresetArray[addrOffset];
			}
			else
			{
				return 0;
			}
			break;
			
		case MEM_PRESET1_DATA_BASE: //------------ Other Preset Param Block
//		case MEM_PRESET2_DATA_BASE: 
//		case MEM_PRESET3_DATA_BASE: 
//		case MEM_PRESET4_DATA_BASE: 
//		case MEM_PRESET5_DATA_BASE: 
//		case MEM_PRESET6_DATA_BASE: 
//		case MEM_PRESET7_DATA_BASE: 
//		case MEM_PRESET8_DATA_BASE: 
			
			pstIdx = ((memBlockType - MEM_PRESET1_DATA_BASE) >> 12) & 0xF;
			
			if (pstIdx < NUM_OF_PRESET && addrOffset < PRESET_TBL_SIZE)
			{
				MemPresetDataReadBytes(pstIdx, addrOffset, (uint8*)data, 1);
			}
			else
			{
				return 0;
			}
			break;
	}
	
	return 1;
}


/*
****************************************************************************************************
**
**                                     MemParamSequRead()
**
**
**	功能描述: 对于连续读，一次可以读多个。
**
**	参  数  : u8buf  读返回数组，它是高字节在前，主要用于通讯协议发送
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemParamSequRead(uint16 *addr, uint8 *u8buf, uint32 len)
{
	uint32 memBlockType = *addr & MEM_ADDR_TYPE_MASK;
	uint32 addrOffset = *addr & MEM_OFFSET_ADDR_MASK;
	uint32 pstIdx;
	uint32 temp;
	uint32 i;
	uint8  flashBuf[252] = {0};
	
	
	
	// addr, len 在调用本函数前已经限制，这里不再做判断
	switch (memBlockType)
	{
		case MEM_DEV_INFO_BASE: //---------------- Device Info Param Block
			for (i=0; i<len; i++)
			{
				temp = ParamTbl.DevInfoArray[addrOffset];
				*u8buf++ = (temp   ) & 0xFF;
				*u8buf++ = (temp>>8) & 0xFF;
				addrOffset++;
			}
			break;
			
		case MEM_CONFIG_DATA_BASE: //------------- Config Param Block
			for (i=0; i<len; i++)
			{
				temp = ParamTbl.ConfigArray[addrOffset];
				*u8buf++ = (temp   ) & 0xFF;
				*u8buf++ = (temp>>8) & 0xFF;
				addrOffset++;
			}
			break;
			
		case MEM_PRESET_DATA_BASE: //------------- Current Preset Param Block
			for (i=0; i<len; i++)
			{
				temp = ParamTbl.PresetArray[addrOffset];
				*u8buf++ = (temp   ) & 0xFF;
				*u8buf++ = (temp>>8) & 0xFF;
				addrOffset++;
			}
			break;
			
		case MEM_PRESET1_DATA_BASE: //------------ Other Preset Param Block
//		case MEM_PRESET2_DATA_BASE: 
//		case MEM_PRESET3_DATA_BASE: 
//		case MEM_PRESET4_DATA_BASE: 
//		case MEM_PRESET5_DATA_BASE: 
//		case MEM_PRESET6_DATA_BASE: 
//		case MEM_PRESET7_DATA_BASE: 
//		case MEM_PRESET8_DATA_BASE: 
			
			pstIdx = ((memBlockType - MEM_PRESET1_DATA_BASE) >> 12) & 0xF;			
		
			if (pstIdx < NUM_OF_PRESET && addrOffset < PRESET_TBL_SIZE)
			{
				MemPresetDataReadBytes(pstIdx, addrOffset, flashBuf, len);
			}
			break;
	}
}


/*
****************************************************************************************************
**
**                                    MemCfgParamAutoSaveSet()
**
**
**	功能描述: 自动保存预处理。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemCfgParamAutoSaveSet(void)
{
	AutoSaveTimer = 500;
}



/*
****************************************************************************************************
**
**                                    MemPresetParamAutoSaveSet()
**
**
**	功能描述: 预设参数自动保存预处理。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemPresetParamAutoSaveSet(void)
{
	PresetAutoSaveTimer = 500;
}


/*
****************************************************************************************************
**
**                                    MemPresetParamAutoSaveClr()
**
**
**	功能描述: 清除预设参数的计数，不保存。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemPresetParamAutoSaveClr(void)
{
	PresetAutoSaveTimer = 0;
}

