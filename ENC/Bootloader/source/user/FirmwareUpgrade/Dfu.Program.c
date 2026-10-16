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
**	文件名	: Dfu.Program.c
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: 固件升级，数据收发处理
**
****************************************************************************************************
*/
#include "main.h"



/* 变量定义和声明 */
uint8 IapArray[MCU_FLASH_PAGE_BYTES_SIZE] = {0};	// 用来存放每个页的代码的缓冲区

struct
{
	struct
	{
		uint8  ModelCode[6];
//		uint8  ModelName[16];
		uint8  VerX;
		uint8  VerY;
		uint8  VerZ;
		uint32 FirmwareLength;
		uint32 FirmwareChecksum;
	} NewFwInfo;
	
	uint32 DfuStep;			// 0 Rx Info, 1 Rx Code
	
	uint32 DataCount;		// 用于累加数据长度，判断结束
	uint32 CheckSumCalc;	// 累加计算
	uint32 IapAddress;
	
} DfuCtrl =
{
	.NewFwInfo =
	{
		.ModelCode        = {0},
//		.ModelName        = {0},
		.VerX             = 0,
		.VerY             = 0,
		.VerZ             = 0,
		.FirmwareLength   = 0,
		.FirmwareChecksum = 0,
	},
	
	.DfuStep = 0,
	
	.DataCount    = 0,
	.CheckSumCalc = 0,
	.IapAddress   = 0,
};

fp32  ProgressIncrement = 0;


/*
****************************************************************************************************
**
**                                    DfuProgramInfoCheck()
**
**
**	功能描述: 判断新的固件信息是否符合升级条件
**
**	参  数  : data	不包含CMD和ChipID
**
**	返回值  : 当前DFU的状态
**
****************************************************************************************************
*/
uint32 DfuProgramInfoCheck(uint8 *data, uint32 len)
{
	uint32 i;
	uint32 pass;
	uint8  *pt;
	
	
	
	pt = data;
	pass = 1;
	for (i=0; i<6; i++)		// 当Model Code = "000000"时，为通用升级
	{
		if ('0' != (*pt++))
		{
			pass = 0;
			break;
		}
	}
    
    //Niko：更新Bootloader，修改DfuCtrl.IapAddress起始地址
    uint32 Boot = 1;
    uint8  BootModelCode[] = "Bootld";
    pt = data;
    for (i=0; i<6; i++)		// 当Model Code = "Bootld"时，为Bootloader升级
	{
		if (BootModelCode[i] != (*pt++))
		{
			Boot = 0;
			break;
		}
        if (i == 5)
        {//boot标志正确
            pass = 1;
        }
	}
    
	
	if (!pass)				// 非通用Model Code则判断当前项目的Code
	{
		DfuCtrl.NewFwInfo.ModelCode[0] = (ParamTbl.DevInfo[0]     ) & 0xFF;
		DfuCtrl.NewFwInfo.ModelCode[1] = (ParamTbl.DevInfo[0] >> 8) & 0xFF;
		DfuCtrl.NewFwInfo.ModelCode[2] = (ParamTbl.DevInfo[1]     ) & 0xFF;
		DfuCtrl.NewFwInfo.ModelCode[3] = (ParamTbl.DevInfo[1] >> 8) & 0xFF;
		DfuCtrl.NewFwInfo.ModelCode[4] = (ParamTbl.DevInfo[2]     ) & 0xFF;
		DfuCtrl.NewFwInfo.ModelCode[5] = (ParamTbl.DevInfo[2] >> 8) & 0xFF;
		
		
		pt = data;
		for (i=0; i<6; i++)
		{
			if (DfuCtrl.NewFwInfo.ModelCode[i] != (*pt++))
			{
				return DFU_STATUS_INFO_ERR;
			}
		}
		
		/* for (i=0; i<16; i++)		// DFU只要判断Model Code，不需要判断Model Name
		{
			if (DfuCtrl.NewFwInfo.ModelName[i] != '\0')
			{
				if (DfuCtrl.NewFwInfo.ModelName[i] != (*pt++))
				{
					return DFU_STATUS_INFO_ERR;
				}
			}
		} */
	}
	
	pt = data + 6 + 64;
	DfuCtrl.NewFwInfo.VerX = *pt++;
	DfuCtrl.NewFwInfo.VerY = *pt++;
	DfuCtrl.NewFwInfo.VerZ = *pt++;
	
	LibDrvMemcpy(&DfuCtrl.NewFwInfo.FirmwareLength,   pt, 4);
	pt += 4;
	LibDrvMemcpy(&DfuCtrl.NewFwInfo.FirmwareChecksum, pt, 4);
	
	DfuCtrl.DfuStep      = 1;
    if (Boot == 1)
    {
        DfuCtrl.IapAddress   = FLASH_BL_ADDR;
    }
    else
    {
        DfuCtrl.IapAddress   = FLASH_APP_ADDR;
    }
	DfuCtrl.CheckSumCalc = 0;
	DfuCtrl.DataCount    = 0;
	Crc32.Reset();
	
	Dfu.StatusBytes.SetEndFlag(DFU_END_FAIL);
	
	// 计算升级进度的变化量 ProgressIncrement
	uint32 div = DfuCtrl.NewFwInfo.FirmwareLength / sizeof(IapArray);
	uint32 res = DfuCtrl.NewFwInfo.FirmwareLength % sizeof(IapArray);

	if (res) div++;

	ProgressIncrement  = 100.0f / div;
	Dfu.Progress.Value = 0;
    Dfu.Progress.DfuTimeOut = 0;
	
	return DFU_STATUS_UPGRADE_CONTINUE;
}


/*
****************************************************************************************************
**
**                                       DfuProgram()
**
**
**	功能描述: 写入Flash
**
**	参  数  : data	不包含CMD和ChipID
**
**	返回值  : 当前DFU的状态
**
****************************************************************************************************
*/
uint32 DfuProgram(uint8 *data, uint32 len)
{
	uint32 ret = DFU_STATUS_CODE_ERR;



	if (DfuCtrl.DfuStep == 0)		// 必须先经过信息校验后，才能进行Program
	{
		return DFU_STATUS_CODE_ERR;
	}
	
	
	// 1. 数据解密，写入Flash -----------------------------------------------------
	LibDrvMemcpy(IapArray, data, len);
	DfuCtrl.DataCount += len;
	
	#ifdef GAMMA_DECRYPT				// Gamma解密
	if (DfuCtrl.DataCount <= 8*1024)
	{
		Dfu.Program.DataDecrypt(IapArray, len);
	}
	#endif
	
	if (!Iap.Write(DfuCtrl.IapAddress, (uint32*)IapArray, len))
	{
		return DFU_STATUS_CODE_ERR;
	}
	DfuCtrl.IapAddress += len;
	
	
	// 2. 计算校验和 --------------------------------------------------------------
	DfuCtrl.CheckSumCalc = Crc32.CalcContinue((uint32*)IapArray, len/4);

	
	// 3. 更新进度值 --------------------------------------------------------------
	Dfu.Progress.Value += ProgressIncrement;

	
	// 4. 比较最终校验和，并结束 --------------------------------------------------
	if (DfuCtrl.DataCount >= DfuCtrl.NewFwInfo.FirmwareLength)		// 程序升级结束
	{
		if (DfuCtrl.CheckSumCalc != DfuCtrl.NewFwInfo.FirmwareChecksum)
		{
			ret = DFU_STATUS_UPGRADE_FAIL;		// 数据校验和出错，升级不成功
		}
		else
		{
			Dfu.Program.SetCompletedFlag();
			ret = DFU_STATUS_UPGRADE_SUCCESS;
		}
		
		DfuCtrl.DfuStep = 0;					// 清除标志，如果不断电，可以拔插USB，再次升级
	}
	else
	{
		ret = DFU_STATUS_UPGRADE_CONTINUE;		// 程序升级未结束，返回正确，继续等待数据接收
        Dfu.Progress.DfuTimeOut = 1000;
	}
	
	return ret;
}




