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
**	文件名	: ParamTable.Preset.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 预设参数表
**
****************************************************************************************************
*/
#ifndef _PRESET_H
#define _PRESET_H
#include "UserRetype.h"



// 预设数据结构
typedef struct
{
	uint16 Checksum;							// 预设数据校验和
	uint16 ValidFlag;							// 预设有效标志
	uint16 PresetName[32];						// 预设名称
	
	uint16 MicMute;
	uint16 MicGain;
	
	uint16 NoiseSuppressorEnabled;
	uint16 NoiseSuppressor4Levels;
	
	uint16 MicMode;                 //  Garry
	uint16 Band1_Enable;
	uint16 Band1_Type;
	uint16 Band1_Freq;
	uint16 Band1_Gain;
	uint16 Band1_Q;

	uint16 Band2_Enable;
	uint16 Band2_Type;
	uint16 Band2_Freq;
	uint16 Band2_Gain;
	uint16 Band2_Q;
	
	uint16 Band3_Enable;
	uint16 Band3_Type;
	uint16 Band3_Freq;
	uint16 Band3_Gain;
	uint16 Band3_Q;
	
	uint16 Band4_Enable;
	uint16 Band4_Type;
	uint16 Band4_Freq;
	uint16 Band4_Gain;
	uint16 Band4_Q;
	
	uint16 Band5_Enable;
	uint16 Band5_Type;
	uint16 Band5_Freq;
	uint16 Band5_Gain;
	uint16 Band5_Q;

	uint16 Band6_Enable;
	uint16 Band6_Type;
	uint16 Band6_Freq;
	uint16 Band6_Gain;
	uint16 Band6_Q;
	
	uint16 Band7_Enable;
	uint16 Band7_Type;
	uint16 Band7_Freq;
	uint16 Band7_Gain;
	uint16 Band7_Q;
	
	uint16 Comp_Enable;
	uint16 Comp_MakeUpGain;
	uint16 Comp_Threshold;
	uint16 Comp_Attack;
	uint16 Comp_Release;
	uint16 Comp_Rate;
  uint16 Comp_KneeWidth;

	uint16 Limit_Enable;
	uint16 Limit_PreGain;
	uint16 Limit_Threshold;
	uint16 Limit_Attack;
	uint16 Limit_Release;
	uint16 Limit_Rate;
    
  uint16 Mic_Sig;                             //Garry  电平指示
  uint16 Mic_SigLevelMute;
  uint16 MonitorRate;                         //Garry   监听总开关
	uint16 HP_Gain;                             //Garry   监听音量
	uint16 Mic_OutPut;                          //Garry   Mic路由开关
	uint16 PC_OutPut;                           //Garry   PC路由开关

} PRESET_PARAM;

#endif




