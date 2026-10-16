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
**	文件名	: Dfu.Core.c
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: 判断进入升级模式，用户程序的跳转等等。
**
****************************************************************************************************
*/
#include "main.h"



/* 函数声明 */
void   DfuInit(void);
void   DfuTask(void);

void   DfuSetStatusBytesEndFlag(uint32 flag);
void   DfuSetProgramCompletedFlag(void);
uint32 DfuGetProgramCompletedFlag(void);
void   DfuTerminate(void);

void   StatusBytesLoad(void);
void   StatusBytesSave(void);

extern uint32 DfuProgramInfoCheck(uint8 *data, uint32 len);
extern uint32 DfuProgram(uint8 *data, uint32 len);
extern void   GammaDecrypt(uint8 *data, uint32 len);


/* 变量定义和声明 */
DFU_HANDLE Dfu =
{
	.Init = DfuInit,
	.Task = DfuTask,
	.Terminate = DfuTerminate,
	
	.StatusBytes =
	{
		.SetEndFlag = DfuSetStatusBytesEndFlag,
	},
	.Program =
	{
		.ValidCheck = DfuProgramInfoCheck,
		.WriteFlash = DfuProgram,

		.SetCompletedFlag = DfuSetProgramCompletedFlag,
		.GetCompletedFlag = DfuGetProgramCompletedFlag,

		.DataDecrypt = GammaDecrypt,
	},
};

struct
{
	uint32 Init1stFlag;
	uint32 BlJumpTo;
	uint32 StartFlag;
	uint32 EndFlag;
} StatusBytes = {0};

uint32 ProgramCompleted = 0;


/*
****************************************************************************************************
**
**                                         DfuInit()
**
**
**	功能描述: Dfu初始化
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DfuInit(void)
{
	StatusBytesLoad();
	
	if (StatusBytes.Init1stFlag == DFU_SB_1ST_INIT)
	{
		// DFU状态字第一次初始化标志
		StatusBytes.Init1stFlag = DFU_SB_1ST_INIT_CLR;
		StatusBytes.BlJumpTo    = BL_JUMP_TO_DFU;
		StatusBytes.StartFlag   = DFU_START_CLR;
		StatusBytes.EndFlag     = DFU_END_CLR;
	}
	else
	{
		// 清除 DFU_START
		StatusBytes.StartFlag = DFU_START_CLR;
		
		if (StatusBytes.EndFlag == DFU_END_FAIL)
		{
			StatusBytes.BlJumpTo = BL_JUMP_TO_DFU;
		}
		else
		{
			StatusBytes.BlJumpTo = BL_JUMP_TO_APP;
		}
	}
	
	StatusBytesSave();
}


/*
****************************************************************************************************
**
**                                         DfuTask()
**
**
**	功能描述: Dfu任务
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
__RAM_CODE void DfuTask(void)
{
	// DFU completed
	if (Dfu.Program.GetCompletedFlag())
	{
		OsTick.DelayMs(1000);
		Dfu.Terminate();
	}
}


/*
****************************************************************************************************
**
**                                       DfuTerminate()
**
**
**	功能描述: Dfu结束
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DfuTerminate(void)
{
	StatusBytes.Init1stFlag = DFU_SB_1ST_INIT_CLR;
	StatusBytes.BlJumpTo    = BL_JUMP_TO_APP;
	StatusBytes.StartFlag   = DFU_START_CLR;
	StatusBytes.EndFlag     = DFU_END_SUCCESS;

	StatusBytesSave();

	/* 升级完毕，自动重启设备。*/
	OsTick.DelayMs(1000);

	__set_PRIMASK(1);
	__set_FAULTMASK(1);		//关闭所有中断
	NVIC_SystemReset();
	
	while (1)		// 升级成功后，选择不软件复位。设备需要重新上电
	{
	}
}


#if (1)		//
/*
****************************************************************************************************
**
**                              DfuSetStatusBytesEndFlag()
**
**
**	功能描述: 设置 DfuEndFlag 值，在不同阶段设置不同状态值
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DfuSetStatusBytesEndFlag(uint32 flag)
{
	StatusBytes.EndFlag = flag;
	
	if (StatusBytes.EndFlag == DFU_END_FAIL)
	{
		StatusBytes.BlJumpTo = BL_JUMP_TO_DFU;
	}
	StatusBytesSave();
}


/*
****************************************************************************************************
**
**                                   DfuSetProgramCompletedFlag()
**
**
**	功能描述: 编程结束，设置标志
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DfuSetProgramCompletedFlag(void)
{
	ProgramCompleted = 1;
}


/*
****************************************************************************************************
**
**                                   DfuGetProgramCompletedFlag()
**
**
**	功能描述: 获取编程结束标志
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
uint32 DfuGetProgramCompletedFlag(void)
{
	return ProgramCompleted;
}
#endif
//...


#if (1)		// StatusBytes 保存与加载
/*
****************************************************************************************************
**
**                                       StatusBytesLoad()
**
**
**	功能描述: 从 FLASH_DFU_SB_ADDR 扇区读出来。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void StatusBytesLoad(void)
{
	uint8 iapBuf[DFU_SB_SIZE];


	Iap.Read(FLASH_DFU_SB_ADDR, (uint32*)iapBuf, DFU_SB_SIZE);
	
	StatusBytes.Init1stFlag = iapBuf[0];
	StatusBytes.BlJumpTo    = iapBuf[1];
	StatusBytes.StartFlag   = iapBuf[2];
	StatusBytes.EndFlag     = iapBuf[3];
}


/*
****************************************************************************************************
**
**                                       StatusBytesSave()
**
**
**	功能描述: 回写到 FLASH_DFU_SB_ADDR 扇区。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void StatusBytesSave(void)
{
	uint8 iapBuf[DFU_SB_SIZE];


	iapBuf[0] = StatusBytes.Init1stFlag;
	iapBuf[1] = StatusBytes.BlJumpTo;
	iapBuf[2] = StatusBytes.StartFlag;
	iapBuf[3] = StatusBytes.EndFlag;

	Iap.Write(FLASH_DFU_SB_ADDR, (uint32*)iapBuf, DFU_SB_SIZE);
}
#endif
//...




