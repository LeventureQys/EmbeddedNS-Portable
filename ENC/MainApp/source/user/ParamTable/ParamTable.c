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
**	日　期  : 2021年12月23日
**
**	文件描述: 系统参数表管理。
**
****************************************************************************************************
*/
#include "ParamTable.h"



/* 函数声明 */
extern void DevInfoSetDefault(void);
extern void ConfigSetDefault(void);
extern void PresetSetDefault(void);

extern uint32 _GetReInitFlag(uint32 flag);
extern uint32 _Get1stInitFlag(uint16 *modelCode);
extern void _GetFirmwareVer(void);
extern void DevInfoSNSet(void);

void   ParamSetDevInfo(uint32 addr, uint32 data);
uint32 ParamGetDevInfo(uint32 addr);
void   ParamSetConfig (uint32 addr, uint32 data);
uint32 ParamGetConfig (uint32 addr);
void   ParamSetPreset (uint32 addr, uint32 data);
uint32 ParamGetPreset (uint32 addr);


/* 变量定义和声明 */
#define READ_BUF_SIZE	(512)
uint8 ParamTable4Read[READ_BUF_SIZE];	// 从存储设备中读取参数时，设一个临时缓冲区

#if PRESET_TBL_SIZE > READ_BUF_SIZE
error()
#endif

extern uint16 DevInfoTable[DEV_INFO_TBL_SIZE];
extern uint16 ConfigTable [CONFIG_TBL_SIZE  ];
extern uint16 PresetTable [PRESET_TBL_SIZE  ];


PARAM_TBL_HANDLE ParamTbl = 
{
	.ReadBufTemp = ParamTable4Read,
	
	.DevInfo = (DEV_INFO_PARAM*)DevInfoTable,
	.Config  = (CONFIG_PARAM  *)ConfigTable,
	.Preset  = (PRESET_PARAM  *)PresetTable,
	
	.DevInfoArray = DevInfoTable,
	.ConfigArray  = ConfigTable,
	.PresetArray  = PresetTable,
	
	.Set =
	{
		.DevInfo = ParamSetDevInfo,
		.Config  = ParamSetConfig,
		.Preset  = ParamSetPreset,
	},
	.SetDefault =
	{
		.DevInfo = DevInfoSetDefault,
		.Config  = ConfigSetDefault,
		.Preset  = PresetSetDefault,
	},
	.Get = 
	{
		.DevInfo = ParamGetDevInfo,
		.Config  = ParamGetConfig,
		.Preset  = ParamGetPreset,
	},
	
	.GetReInitFlag  = _GetReInitFlag,
	.Get1stInitFlag = _Get1stInitFlag,
    .GetFirmwareVer = _GetFirmwareVer,
	
	.FwInfoSNSet = DevInfoSNSet,
};


/*
****************************************************************************************************
**
**                                        ParamSetDevInfo()
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
void ParamSetDevInfo(uint32 addr, uint32 data)
{
	addr &= 0x0FFF;
	DevInfoTable[addr] = data;
}

uint32 ParamGetDevInfo(uint32 addr)
{
	addr &= 0x0FFF;
	return DevInfoTable[addr];
}


/*
****************************************************************************************************
**
**                                        ParamSetConfig()
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
void ParamSetConfig(uint32 addr, uint32 data)
{
	addr &= 0x0FFF;
	ConfigTable[addr] = data;
}

uint32 ParamGetConfig(uint32 addr)
{
	addr &= 0x0FFF;
	return ConfigTable[addr];
}


/*
****************************************************************************************************
**
**                                        ParamSetPreset()
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
void ParamSetPreset(uint32 addr, uint32 data)
{
	addr &= 0x0FFF;
	PresetTable[addr] = data;
}

uint32 ParamGetPreset(uint32 addr)
{
	addr &= 0x0FFF;
	return PresetTable[addr];
}




