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
**	文件名	: Ui.Encoder.c
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 编码器应用。
**
****************************************************************************************************
*/
#include "gd32f3x0.h"
#include "OsTick.h"
#include "Gpio.h"
#include "Ui.Encoder.h"



/* 函数声明 */
static void _EncoderInit(void);
static void _EncoderReset(void);
static void _EncoderScan(void);
static void _EncoderProcess(void);


/* 变量定义和声明 */
UI_ENC_HANDLE  UiEnc = 
{
	.Param =
	{
		.Event = 0,
		.Value = 0,
	},
	
	.Init    = _EncoderInit,
	.Reset   = _EncoderReset,
	.Scan    = _EncoderScan,
	.Process = _EncoderProcess,
};

static int32 ValueState;	// 编码器处理过程中，中间变量
	

/*
****************************************************************************************************
**
**                                        _EncoderInit()
**
**	功能描述: 
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
static void _EncoderInit(void)
{
	_EncoderReset();
}


/*
***************************************************************************************************
**
**                                      _EncoderReset()
**
**	功能描述：编码器复位
**
**	参  数  ：
**
**	返回值  ：
*
***************************************************************************************************
*/
static void _EncoderReset(void)
{
	UiEnc.Param.Event = 0;
	UiEnc.Param.Value = 0;
	ValueState        = 0;
}


/*
***************************************************************************************************
**
**                                      _EncoderScan()
**
**	功能描述：单次编码器扫描。1ms调用一次，间隔时间也相当于做了滤波功能。可在中断中调用。
**
**	参  数  ：
**
**	返回值  ：
*
***************************************************************************************************
*/
static void _EncoderScan(void)
{
	uint32 AB = Gpio.Ui.Enc.GetStatus();
	static uint32 ABold = 0;
	static uint32 encState = 0;
	
	
	if (ABold != AB)
	{
		encState = ((encState << 2) | AB) & 0x3F;	// 移2次
		
		if (encState == 0x38 || encState == 0x07)
		{
			ValueState++;
		}
		else if (encState == 0x34 || encState == 0x0B)
		{
			ValueState--;
		}
		
		ABold = AB;
	}
}


/*
***************************************************************************************************
**
**                                      _EncoderProcess()
**
**	功能描述：编码器处理。在单次扫描基础上，再做一定时间内的累计处理。
**
**	参  数  ：
**
**	返回值  ：
*
***************************************************************************************************
*/
static void _EncoderProcess(void)
{
	static uint32 time = 1;
	
	
	if (OsTick.TimerMs(&time, TFB_ENC_SCAN))  	// processing every 80ms
	{
		time = 80;
		
		if (ValueState != 0)
		{
			UiEnc.Param.Event = 1;
			UiEnc.Param.Value = ValueState;
			ValueState        = 0;
		}
	}
}




