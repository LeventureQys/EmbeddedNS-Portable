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
**	文件名	: ParamTable.DevInfo.c
**
**	程序员	: 
**
**	日　期  : 2022年7月22日
**
**	文件描述: 设备信息参数表
**
****************************************************************************************************
*/
#include "Iap.h"
#include "ParamTable.h"
#include "SctConfig.h"
#include "Fw.Info.h"
#include "Fw.Version.h"

#include "lib_drv_uac10_config.h"



/*	参数地址减去0x0000，就是本表的索引地址。
	给DevInfoTable赋值前，DevInfoTableDef的值就是出厂缺省值。
*/
uint16 DevInfoTable[DEV_INFO_TBL_SIZE];

const uint16 DevInfoTableDef[DEV_INFO_TBL_SIZE] =
{
/* Python 自动生成的起始行 */
	MODEL_CODE_PARAM1,     // MC1 (0x3050) ~~~~~~~~ Model Code ~~~~~~~~
	MODEL_CODE_PARAM2,     // MC2 (0x594A) 
	MODEL_CODE_PARAM3,     // MC3 (0x3330) 
	MODEL_NAME_PARAM1,     // DN1 (0x0000) ~~~~~~~~ Device Name ~~~~~~~~
	MODEL_NAME_PARAM2,     // DN2 (0x0000) 
	MODEL_NAME_PARAM3,     // DN3 (0x0030)	
	MODEL_NAME_PARAM4,     // DN4 (0x0000) 
	MODEL_NAME_PARAM5,     // DN5 (0x0000) 
	MODEL_NAME_PARAM6,     // DN6 (0x0000) 
	MODEL_NAME_PARAM7,     // DN7 (0x0000) 
	MODEL_NAME_PARAM8,     // DN8 (0x0000) 
	FW_VERSION_X,     // X        ~~~~~~~~ Firmware Rev ~~~~~~~~
	FW_VERSION_Y,     // Y        
	FW_VERSION_Z,     // Z        
	FW_VERSION_BUILD,     // Build  
	0x0001,     // Type           
	0x0001,     // X              ~~~~~~~~ DSP1 Rev ~~~~~~~~
	0x0000,     // Y              
	0x0000,     // Z              
	0x0001,     // X              ~~~~~~~~ DSP2 Rev ~~~~~~~~
	0x0000,     // Y              
	0x0000,     // Z              
	USB_DEFAULT_VID,  // VID            ~~~~~~~~ USB ID ~~~~~~~~
	USB_DEFAULT_PID,  // PID    
	0x07E9,     // Year           ~~~~~~~~ Date ~~~~~~~~
	0x0001,     // Month          
	0x0001,     // Day            
	0x0000,     // Hour           ~~~~~~~~ Time ~~~~~~~~
	0x0000,     // Minute         
	0x0000,     // Second         

	0x0000,     // SN1            ~~~~~~~~ Device SN ~~~~~~~~
	0x0000,     // SN2            
	0x0000,     // SN3            
	0x0000,     // SN4            
	0x0000,     // SN5            
	0x0000,     // SN6            
	0x0000,     // SN7            
	0x0000,     // SN8            
	0x0000,     // SN9            
	0x0000,     // SN10           
	0x0000,     // SN11           
	0x0000,     // SN12           
	0x0000,     // SN13

	0x0000,     // SN14           
	0x0000,     // SN15           
	0x0000,     // SN16           
	0x0000,     // SN17           
	0x0000,     // SN18           
	0x0000,     // SN19           
	0x0000,     // SN20           
	0x0000,     // SN21           
	0x0000,     // SN22           
	0x0000,     // SN23           
	0x0000,     // SN24           
	0x0000,     // SN25           
	0x0000,     // SN26           
	0x0000,     // SN27           
	0x0000,     // SN28           
	0x0000,     // SN29           
	0x0000,     // SN30           
	0x0000,     // SN31           
	0x0000,     // SN32           
/* Python 自动生成的结束行 */
};


/*
****************************************************************************************************
**
**                                        DevInfoDateSet()
**
**
**	功能描述: 设置当前日期
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
const uint8 LocalDateStr[] = __DATE__;	// Keil有时不更新的，需要Rebuilt All
const uint8 LocalTimeStr[] = __TIME__;

void DevInfoDateSet(void)
{
	uint32 t1, t2, t3, t4;
	
	
	// Date
	switch (LocalDateStr[0])
	{
		case 'J': 
			if      (LocalDateStr[1] == 'a') ParamTbl.DevInfo->Date.Month = 1;
			else if (LocalDateStr[2] == 'n') ParamTbl.DevInfo->Date.Month = 6;
			else                             ParamTbl.DevInfo->Date.Month = 7;
			break;
			
		case 'M': 
			if (LocalDateStr[2] == 'r')  ParamTbl.DevInfo->Date.Month = 3;
			else                         ParamTbl.DevInfo->Date.Month = 5;
			break;
			
		case 'A': 
			if (LocalDateStr[1] == 'p') ParamTbl.DevInfo->Date.Month = 4;
			else                        ParamTbl.DevInfo->Date.Month = 8;
			break;
			
		case 'F': ParamTbl.DevInfo->Date.Month = 2;  break;
		case 'S': ParamTbl.DevInfo->Date.Month = 9;  break;
		case 'O': ParamTbl.DevInfo->Date.Month = 10; break;
		case 'N': ParamTbl.DevInfo->Date.Month = 11; break;
		case 'D': ParamTbl.DevInfo->Date.Month = 12; break;
	}
	
	if (LocalDateStr[4] > '0') t1 = LocalDateStr[4] - '0';
	else                       t1 =0;
	t2 = LocalDateStr[5] - '0';
	ParamTbl.DevInfo->Date.Day = t1*10 + t2;
	
	t1 = LocalDateStr[7]  - '0';
	t2 = LocalDateStr[8]  - '0';
	t3 = LocalDateStr[9]  - '0';
	t4 = LocalDateStr[10] - '0';
	ParamTbl.DevInfo->Date.Year = t1*1000 + t2*100 + t3*10 + t4;
	
	// Time
	#if 0
	t1 = LocalTimeStr[0] - '0';
	t2 = LocalTimeStr[1] - '0';	
	ParamTbl.DevInfo->Time.Hour = t1*10 + t2;
	
	t1 = LocalTimeStr[3] - '0';
	t2 = LocalTimeStr[4] - '0';	
	ParamTbl.DevInfo->Time.Minute = t1*10 + t2;
	
	t1 = LocalTimeStr[6] - '0';
	t2 = LocalTimeStr[7] - '0';	
	ParamTbl.DevInfo->Time.Second = t1*10 + t2;
	#else
	ParamTbl.DevInfo->Time.Hour   = 0;		// 这个时分秒感觉意义不是很大。反而在某些情况下，造成不必要的麻烦
	ParamTbl.DevInfo->Time.Minute = 0;		// 当对比两个hex文件的差异时，经常会造成困扰
	ParamTbl.DevInfo->Time.Second = 0;		// 如果全设成0，那么就只有日期的差异。这样容易对比一些
	#endif
}


/*
****************************************************************************************************
**
**                                       DevInfoSNSet()
**
**
**	功能描述: 设置SN
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DevInfoSNSet(void)
{
	uint32 flag = 0;
	uint32 i;
	uint8  *SNCode = (uint8*)(FLASH_FW_INFO_ADDR + 24);
	
	
	for (i=0; i<64; i++)				// 判断SNCode是否初始化过
	{		
		if (SNCode[i] != 0xFF)
		{
			flag = 1;
			break;
		}
	}
	
	if (flag)
	{
		uint8 *p1 = SNCode;
		uint8 *p2 = (uint8*)ParamTbl.DevInfo->SN;
		for (i=0; i<64; i++)
		{
			if (*p1 != 0xFF)
			{
				*(p2++) = *(p1++);
			}
			else
			{
				break;
			}
		}
		for (; i<64; i++)
		{
			*(p2++) = 0;
		}
	}
	else
	{
		for (i=0; i<32; i++)
		{		
			ParamTbl.DevInfo->SN[i] = 0xFFFF;		// 未初始化，SN全设为0
		}
	}
}


/*
****************************************************************************************************
**
**                                     _Get1stInitFlag()
**
**
**	功能描述: 判断是否需要做第一次（一次性初始化标志）初始化。
**            用Model Code来做判断。第一次烧程序，存储区的Model Code都是FF，这样程序会对存储区进行
**            一次性初始化。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
uint32 _Get1stInitFlag(uint16 *modelCode)
{
	DEV_INFO_PARAM *Curr = (DEV_INFO_PARAM*)DevInfoTableDef;
	
	
	/* modelCode       是存储区中的值，意味着是旧固件的值，未升级前。
	   DevInfoTableDef 是新固件的值
	   两者进行比较
	*/
	if (modelCode[0] != Curr->ModelCode[0] ||
		modelCode[1] != Curr->ModelCode[1] ||
		modelCode[2] != Curr->ModelCode[2])
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
****************************************************************************************************
**
**                                     _GetFirmwareVer()
**
**
**	功能描述:获取当前固件的版本。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void _GetFirmwareVer(void)
{
    ParamTbl.DevInfo->FirmwareVer.X = FW_VERSION_X;
    ParamTbl.DevInfo->FirmwareVer.Y = FW_VERSION_Y;
    ParamTbl.DevInfo->FirmwareVer.Z = FW_VERSION_Z;
    ParamTbl.DevInfo->FirmwareVer.Build = FW_VERSION_BUILD;
}

/*
****************************************************************************************************
**
**                                        DevInfoReSet()
**
**
**	功能描述: 设备信息中部分是需要重新设置的
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DevInfoReSet(void)
{
	DevInfoDateSet();
	
	// SN
	DevInfoSNSet();
	
	/* 0 Demo 演示版、测试版，在未量产前的版本
	   1 Release 发布版，量产后的版本
	
	   需在《参数表.xlsm》中修改，然后再更新到数组 DevInfoTableDef 中来
	*/
//	ParamTbl.DevInfo->FirmwareRev.Type = 0;
}


/*
****************************************************************************************************
**
**                                        DevInfoSetDefault()
**
**
**	功能描述: 对各设备信息参数赋初始值。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DevInfoSetDefault(void)
{
	uint32 i;
	
	
	for (i=0; i<DEV_INFO_TBL_SIZE; i++)
	{
		DevInfoTable[i] = DevInfoTableDef[i];
	}
	
	
	// 下面的是根据实际来赋值
	DevInfoReSet();
}




