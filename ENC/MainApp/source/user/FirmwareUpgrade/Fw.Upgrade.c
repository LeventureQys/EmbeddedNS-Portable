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
**	文件名	: FirmwareUpgrade.c
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 固件升级处理文件。
**
****************************************************************************************************
*/
#include "a0.h"
#include "UserRetype.h"
#include "SctConfig.h"
#include "OsTick.h"
#include "Iap.h"
#include "Fw.Upgrade.h"
#include "Ui.h"
#include "Dsp.h"
#include "Memory.h"



/* 函数声明 */
void FwuNvicVectorDeinit(void);
void FwuFinishProcess(void);
void ReadOutProtectionSet(void);
void FwuJumpToDfu(void);
void FwuStart(void);
void FwuStartProcess(void);


/* 变量定义和声明 */
FWU_HANDLE Fwu =
{
	.Flag = 0,
	
	.FinishProcess    = FwuFinishProcess,
	.Start            = FwuStart,
	.Task             = FwuStartProcess,
};

struct
{
	uint32 Init1stFlag;
	uint32 BlJumpTo;
	uint32 StartFlag;
	uint32 EndFlag;
} StatusBytes = {0};

uint32 StartEnabled;
uint32 StartTimeout;


/*
****************************************************************************************************
**
**                                       DfuStatusBytesLoad()
**
**
**	功能描述: 从 FLASH_DFU_STATUS_ADDR 扇区读出来。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DfuStatusBytesLoad(void)
{	
	StatusBytes.Init1stFlag = (uint32)(*(uint8*)(FLASH_DFU_SB_ADDR + 0));
	StatusBytes.BlJumpTo    = (uint32)(*(uint8*)(FLASH_DFU_SB_ADDR + 1));
	StatusBytes.StartFlag   = (uint32)(*(uint8*)(FLASH_DFU_SB_ADDR + 2));
	StatusBytes.EndFlag     = (uint32)(*(uint8*)(FLASH_DFU_SB_ADDR + 3));
}


/*
****************************************************************************************************
**
**                                       DfuStatusBytesSave()
**
**
**	功能描述: 回写到 FLASH_DFU_STATUS_ADDR 扇区。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DfuStatusBytesSave(void)
{
	uint8 iapBuf[4];


	iapBuf[0] = StatusBytes.Init1stFlag;
	iapBuf[1] = StatusBytes.BlJumpTo;
	iapBuf[2] = StatusBytes.StartFlag;
	iapBuf[3] = StatusBytes.EndFlag;

	Iap.Write(FLASH_DFU_SB_ADDR, (uint32*)iapBuf, DFU_SB_SIZE);
}


/*
****************************************************************************************************
**
**                                       FwuFinishProc()
**
**
**	功能描述: 固件更新完成修改标志
**
**	参  数  : 
**
**	返回值  : 
**
**  备  注  : 只要运行到这里就表示升级完成。
**
****************************************************************************************************
*/
void FwuFinishProcess(void)
{
	DfuStatusBytesLoad();
	
	if (StatusBytes.Init1stFlag == DFU_SB_1ST_INIT || 
		StatusBytes.EndFlag != DFU_END_CLR)
	{
		// 清除标志
		StatusBytes.Init1stFlag = DFU_SB_1ST_INIT_CLR;
		StatusBytes.BlJumpTo    = BL_JUMP_TO_APP;
		StatusBytes.StartFlag   = DFU_START_CLR;
		StatusBytes.EndFlag     = DFU_END_CLR;
		
		DfuStatusBytesSave();
		
		Fwu.Flag = 1;
	}
}


/*
****************************************************************************************************
**
**                                        FwuJumpToDfu()
**
**
**	功能描述: 固件更新模式，需要对相应存储位置，设置一些标志。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
extern void UserEnterUpdateMode(void);
void FwuJumpToDfu(void)
{
	StatusBytes.Init1stFlag = DFU_SB_1ST_INIT_CLR;
	StatusBytes.BlJumpTo    = BL_JUMP_TO_DFU;
	StatusBytes.StartFlag   = DFU_START;
	StatusBytes.EndFlag     = DFU_END_CLR;
	
	DfuStatusBytesSave();
    
    UserEnterUpdateMode();
    
//	OsTick.DelayMs(300);	// ~100ms
//    
//	__set_PRIMASK(1);
//	//__set_FAULTMASK(1);
//	NVIC_SystemReset();
}


/*
****************************************************************************************************
**
**                                         FwuStart()
**
**
**	功能描述: 开始固件升级，只是设置变量，延时后才真正开始。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void FwuStart(void)
{
	StartEnabled  = 1;
	StartTimeout = 100;
}


/*
****************************************************************************************************
**
**                                         FwuStartProcess()
**
**
**	功能描述: 延时一定时间后，才开始固件升级。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void FwuStartProcess(void)
{
	if (StartEnabled)
	{
		if (OsTick.TimerMs(&StartTimeout, TFB_DFU_SUC))
		{
			Ui.Disable();
			Dsp.Program->Mute();
			OsTick.DelayMs(100);
			
			StartEnabled = 0;
//            return;
			FwuJumpToDfu();
		}
	}
}




