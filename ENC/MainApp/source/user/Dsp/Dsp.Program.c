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
**	文件名	: Dsp.Program.c
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: DSP编程，与DSP相关的所有操作。
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Gpio.h"
#include "Dsp.h"
#include "Dsp2.h"



#if (1)			// 定义和声明
/* 函数声明 */
void DspBootloader(void);

void DspAMSet_SequParam(uint16 *paramStartAddr, uint32 num);
void DspAMSet_RandParam(uint8  *pData, uint32 len);

void DspUnmutePostProcessSet(uint32 ms);
void DspUnmutePostProcess(void);


/* 变量定义和声明 */
DSP_PROGRAM DspProgram =
{
	.Bootloader = DspBootloader,

	.AMSet =
	{
		.SequParam = DspAMSet_SequParam,
		.RandParam = DspAMSet_RandParam,
	},
	
	.Mute   = DspMute,
	.Unmute = DspUnmute,
	
	.UnmuteDelay         = DspUnmutePostProcessSet,
	.UnmutePostProcess   = DspUnmutePostProcess,
};

uint32 UnmutePostProcessTime = 0;

#endif
//...


#if (1)			// DSP初始化，引导，复位
/*
****************************************************************************************************
**
**                                         DspBootloader()
**
**
**	功能描述: 芯片复位。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void DspBootloader(void)
{
}
#endif
//...


#if (1)			// 通过参数表(ParamTable)来与DSP通讯
/*
****************************************************************************************************
**
**                                DspAMSetForSequ()
**
**
**	功能描述: 连续地址写Dsp操作
**
**	参  数  : paramStartAddr    第一个参数的地址
**            num		        参数数目
**
**	返回值  : 1正确，0错误
**
**  备  注  : 要对所有的地址数据做出处理。
**
****************************************************************************************************
*/
void DspAMSet_SequParam(uint16 *paramStartAddr, uint32 num)
{
	uint32 i, temp;
	uint32 paramAddr   = *paramStartAddr;
	uint32 addrStart = 0;	// 当前模块对应的首地址
	uint32 addrEnd   = 0;	// 当前模块对应的尾地址，有些模块是对应几个寄存器的
	uint32 addrIsOK  = 0;


	
	for (i=0; i<num; )
	{
		if (i == 0)	// 第一次地址是不用过滤
		{
			addrIsOK = 1;
		}
		else
		{
			if (paramAddr < addrStart || paramAddr > addrEnd)	// 不在本DSP模块中的地址即是新的模块
			{
				addrIsOK = 1;
			}
			else
			{
				addrIsOK = 0;
				i++;
				paramAddr++;
			}
		}

		if (addrIsOK)
		{
			DspModuleParamSet(&paramAddr, &addrStart, &addrEnd);
			temp = addrEnd - paramAddr + 1;
			i += temp;
			paramAddr += temp;
		}
	}
}


/*
****************************************************************************************************
**
**                                DspAMSetForRand()
**
**
**	功能描述: 随机地址写Dsp操作
**
**	参  数  : pData     通讯数据，地址+数据的组合
**            len		数据长度，单位字节，它必须是4的整数倍
**
**	返回值  :
**
**  备  注  : 要对所有的地址数据做出处理。
**
****************************************************************************************************
*/
void DspAMSet_RandParam(uint8 *pData, uint32 len)
{
	uint32 i;
	uint32 lp = len/4;
	uint32 paramAddr;
	uint32 addrStart = 0;	// 当前模块对应的首地址
	uint32 addrEnd = 0;		// 当前模块对应的尾地址，有些模块是对应几个寄存器的
	uint32 addrIsOK = 0;
	uint16 *pt = (uint16*)pData;


	
	for (i=0; i<lp; i++)
	{
		paramAddr = *pt;
		pt += 2;				// 多加1是偏移掉参数值，到下一个参数地址

		if (i == 0)				// 第一次地址是不用过滤
		{
			addrIsOK = 1;
		}
		else
		{
			if (paramAddr < addrStart || paramAddr > addrEnd)	// 不在本DSP模块中的地址即是新的模块
				addrIsOK = 1;
			else
				addrIsOK = 0;
		}

		if (addrIsOK)
		{
			DspModuleParamSet(&paramAddr, &addrStart, &addrEnd);
		}
	}
}
#endif
//...


#if (1)			// 其它
/*
****************************************************************************************************
**
**                                    UnmutePostProcessSet()
**
**
**	功能描述: 设置Unmute后处理的延迟时间
**
**	参  数  : ms  需要的延时时间，单位ms
**
**	返回值  :
**
****************************************************************************************************
*/
void DspUnmutePostProcessSet(uint32 ms)
{
	UnmutePostProcessTime = ms;
}


/*
****************************************************************************************************
**
**                                      UnmutePostProcess()
**
**
**	功能描述: Unmute后处理。延迟一些时间后，再处理Unmute
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void DspUnmutePostProcess(void)
{
	if (OsTick.TimerMs(&UnmutePostProcessTime, TFB_DSP_UNMUTE_PROC))
	{
		DspUnmute();
	}
}
#endif
//...




