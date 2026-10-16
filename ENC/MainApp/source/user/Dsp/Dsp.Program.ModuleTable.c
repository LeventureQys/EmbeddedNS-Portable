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
**	文件名	: Dsp.Program.ModuleTable.c
**
**	程序员	:
**
**	日　期  : 2022年7月22日
**
**	文件描述: DSP模块处理文件。
**
****************************************************************************************************
*/
#include "Helper.h"
#include "OsTick.h"
#include "Gpio.h"
#include "Memory.h"
#include "ParamTable.h"
#include "Ui.h"
#include "Dsp.h"
#include "Dsp2.h"
#include "UserFunction.h"
#include "alg_filter.h"
#include "drv_alg.h"

#include "app_alg_dsp_ans.h"
#include "mcu_alg_comp.h"
#include "mcu_alg_limiter.h"


extern void DspAlgEqParaSet(ALG_PARA_FILTER *pEqPara, uint32_t segNumMax, uint32_t EqIdx);
extern __RAM_CODE void McuAlgEqParaSet(ALG_PARA_FILTER *pPara, uint32_t segNumMax, uint32_t EqIdx);
extern ALG_PARA_FILTER paraDefaultAdcEq[1][5];
extern ALG_PARA_FILTER paraDefaultSpkEq[2][7];
extern ALG_PARA_FILTER paraDefaultDacEq[1][5];

extern float g_gainNewAddMcu[ALG_GAIN_NUM][2] ;

extern void UserEnterUpdateMode(void);

extern DSP_ALG_PARA_ANS dspAlgParaAns;
extern void DspAlgANSVarInit(DSP_ALG_PARA_ANS *pPara);  

extern int32_t noiseGateParaGroup0[1][6];

#if (1)		// Head
/* 函数声明 */
typedef void (*DMParamSet)(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);

void DMParamSetNothing(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);
void DMParamSetPresetName(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);
void DMParamSetMicMute(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);
void DMParamSetHpMute (uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);

void DMParamSetHpGain (uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);
void DMParamSetMonRate(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);
void DMParamSetNoiseSuppressor(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);
//void DMParamSetReverb (uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);  // Liang

void DMParamLedShow      (uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);

void DMParamSetUsbOutInc (uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);  // Steve
void DMParamSetGame71 (uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);  // Niko


void DMParamSetMicGain(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);                   //Garry
void DMParamSetMicEqualizerMode(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);          //Garry
void DMParamSet_Band(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);                     //Garry
void DMParamSet_Comp(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);                     //Garry
void DMParamSet_Limit(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);                    //Garry

void DMParamSet_enable_comp(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);     // MicCompBypass 
void DMParamSet_comp_treshold(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);   // MicCompTreshold 
void DMParamSet_comp_ratio(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);      // MicCompRatio   
void DMParamSet_comp_kneew(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);      // MicCompKneeWidth  
void DMParamSet_comp_attack(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);     // MicCompAttack
void DMParamSet_comp_release(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);    // MicCompRelease
void DMParamSet_comp_mkg(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);        // MicCompMakeUpGain

void DMParamSet_enable_limiter(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);   //MicLimiterBypass  
void DMParamSet_limiter_treshold(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd); //MicLimiterThreshold
void DMParamSet_limiter_attack(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);   //MicLimiterAttack
void DMParamSet_limiter_release(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);  //MicLimiterRelease





/* 变量定义和声明 */
/* Python 自动生成的起始行 */
const DMParamSet DMConfigParamSetTable[] =
{
	DMParamSetNothing,     // Heartbeat  ~~~~~~~~ 心跳 ~~~~~~~~
	DMParamSetNothing,     // Addr  ~~~~~~~~ 设备地址 ~~~~~~~~
	DMParamSetNothing,     // Number(N)  ~~~~~~~~ 系统预设总数 ~~~~~~~~
	DMParamSetNothing,     // StartupNo  ~~~~~~~~ 系统启动加载的预设 ~~~~~~~~
	DMParamSetNothing,     // SaveNo  ~~~~~~~~ 保存（存储器）预设 ~~~~~~~~
	DMParamSetNothing,     // LoadNo  ~~~~~~~~ 加载（存储器）预设 ~~~~~~~~
	DMParamSetNothing,     // DeleNo  ~~~~~~~~ 删除（存储器）预设 ~~~~~~~~
	DMParamSetNothing,     // ComErrAck  ~~~~~~~~ 通讯错误 ~~~~~~~~
	DMParamSetNothing,     // DFU  ~~~~~~~~ 进入升级模式 ~~~~~~~~
	DMParamSetNothing,     // ReInit  ~~~~~~~~ 系统重新初始化 ~~~~~~~~
	DMParamSetNothing,     // Set  ~~~~~~~~ 恢复出厂设置 ~~~~~~~~
	DMParamSetNothing,     // SyncFlag  ~~~~~~~~ 软件同步数据标志 ~~~~~~~~
	DMParamSetNothing,     // SysErr1  ~~~~~~~~ 系统错误状态记录 ~~~~~~~~
	DMParamSetNothing,     // SysErr2
};


const DMParamSet DMPresetParamSetTable[] =
{
	DMParamSetNothing,                           // Checksum  ~~~~~~~~ 预设数据校验和 ~~~~~~~~
	DMParamSetNothing,                           // ValidFlag  ~~~~~~~~ 预设有效标志 ~~~~~~~~
	DMParamSetPresetName,                        // PresetName1  ~~~~~~~~ 预设名称（64字节） ~~~~~~~~
	DMParamSetPresetName,                        // PresetName2
	DMParamSetPresetName,                        // PresetName3
	DMParamSetPresetName,                        // PresetName4
	DMParamSetPresetName,                        // PresetName5
	DMParamSetPresetName,                        // PresetName6
	DMParamSetPresetName,                        // PresetName7
	DMParamSetPresetName,                        // PresetName8
	DMParamSetPresetName,                        // PresetName9
	DMParamSetPresetName,                        // PresetName10
	DMParamSetPresetName,                        // PresetName11
	DMParamSetPresetName,                        // PresetName12
	DMParamSetPresetName,                        // PresetName13
	DMParamSetPresetName,                        // PresetName14
	DMParamSetPresetName,                        // PresetName15
	DMParamSetPresetName,                        // PresetName16
	DMParamSetPresetName,                        // PresetName17
	DMParamSetPresetName,                        // PresetName18
	DMParamSetPresetName,                        // PresetName19
	DMParamSetPresetName,                        // PresetName20
	DMParamSetPresetName,                        // PresetName21
	DMParamSetPresetName,                        // PresetName22
	DMParamSetPresetName,                        // PresetName23
	DMParamSetPresetName,                        // PresetName24
	DMParamSetPresetName,                        // PresetName25
	DMParamSetPresetName,                        // PresetName26
	DMParamSetPresetName,                        // PresetName27
	DMParamSetPresetName,                        // PresetName28
	DMParamSetPresetName,                        // PresetName29
	DMParamSetPresetName,                        // PresetName30
	DMParamSetPresetName,                        // PresetName31
	DMParamSetPresetName,                        // PresetName32
	DMParamSetMicMute,                           // MIC Mute  ~~~~~~~~ MIC静音(Mute) ~~~~~~~~
	DMParamSetMicGain,                           // MIC Gain  ~~~~~~~~ MIC音量(Gain) ~~~~~~~~
	DMParamSetNoiseSuppressor,                   // OnOff  ~~~~~~~~ 降噪开关 ~~~~~~~~
	DMParamSetNoiseSuppressor,                   // level  ~~~~~~~~ 4档降噪 ~~~~~~~~  
	DMParamSetMicEqualizerMode,                  // MicMode  ~~~~~~~~ 麦克风均衡器场景 ~~~~~~~~ 
	//~~~~~~~~ Band_1 ~~~~~~~~
	DMParamSet_Band,                            //Band1 Enable
	DMParamSet_Band,                            //Band1 Type
	DMParamSet_Band,                            //Band1 Freq
	DMParamSet_Band,                            //Band1 Gain
	DMParamSet_Band,                            //Band1 Q
	
	//~~~~~~~~ Band_2 ~~~~~~~~
	DMParamSet_Band,                            //Band2 Enable
	DMParamSet_Band,                            //Band2 Type
	DMParamSet_Band,                            //Band2 Freq
	DMParamSet_Band,                            //Band2 Gain
	DMParamSet_Band,                            //Band2 Q
	
	//~~~~~~~~ Band_3~~~~~~~~
	DMParamSet_Band,                            //Band3 Enable
	DMParamSet_Band,                            //Band3 Type
	DMParamSet_Band,                            //Band3 Freq
	DMParamSet_Band,                            //Band3 Gain
	DMParamSet_Band,                            //Band3 Q

	//~~~~~~~~ Band_4~~~~~~~~ 
	DMParamSet_Band,                            //Band4 Enable
	DMParamSet_Band,                            //Band4 Type
	DMParamSet_Band,                            //Band4 Freq
	DMParamSet_Band,                            //Band4 Gain
	DMParamSet_Band,                            //Band4 Q

	//~~~~~~~~ Band_5 ~~~~~~~~
	DMParamSet_Band,                            //Band5 Enable
	DMParamSet_Band,                            //Band5 Type
	DMParamSet_Band,                            //Band5 Freq
	DMParamSet_Band,                            //Band5 Gain
	DMParamSet_Band,                            //Band5 Q
	
	//~~~~~~~~ Band_6 ~~~~~~~~
	DMParamSet_Band,                            //Band6 Enable
	DMParamSet_Band,                            //Band6 Type
	DMParamSet_Band,                            //Band6 Freq
	DMParamSet_Band,                            //Band6 Gain
	DMParamSet_Band,                            //Band6 Q
	
	//~~~~~~~~ Band_7 ~~~~~~~~
	DMParamSet_Band,                            //Band7 Enable
	DMParamSet_Band,                            //Band7 Type
	DMParamSet_Band,                            //Band7 Freq
	DMParamSet_Band,                            //Band7 Gain
	DMParamSet_Band,                            //Band7 Q
	
	//~~~~~~~~ Comp ~~~~~~~~
    DMParamSet_enable_comp,                     //Enable 
    DMParamSet_comp_mkg,                        //preGain
    DMParamSet_comp_treshold,                   //Threshold
    DMParamSet_comp_attack,                     //Attack
    DMParamSet_comp_release,                    //Release
    DMParamSet_comp_ratio,                      //Rate
    DMParamSet_comp_kneew,                      //KneeWidth
    
                        
    //~~~~~~~~ Limit ~~~~~~~~	                
    DMParamSet_enable_limiter,                  //Enable  
    DMParamSetNothing,                          //preGain
    DMParamSet_limiter_treshold,                //Threshold
    DMParamSet_limiter_attack,                  //Attack
    DMParamSet_limiter_release,                 //Release
    DMParamSetNothing,                          //Rate

    //~~~~~~~~ Rms ~~~~~~~~
   	DMParamSetNothing,                  //Garry    电平指示
   	DMParamSetNothing,                  //Garry    电平指示
		
	  DMParamSetMonRate,                           // MIC/PC Rate   ~~~~~~~~ 监听混音比例 ~~~~~~~~ 
		DMParamSetMonRate,
		DMParamSetMonRate,
		DMParamSetMonRate,
};

/* Python 自动生成的结束行 */
#endif
//...


#if (1)		// 基本处理
/*
****************************************************************************************************
**
**                                   DspModuleParamSet()
**
**
**	功能描述: 对DSP音频模块进行写，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
void DspModuleParamSet(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	uint32 addr = (*paramAddr);
	uint32 idx  = addr & MEM_OFFSET_ADDR_MASK;


	/*if (addr >= PARAM_CONFIG_START && addr <= PARAM_CONFIG_END)
	{
		DMConfigParamSetTable[idx](paramAddr, addrStart, addrEnd);
	}
	else*/
	if (addr >= PARAM_PRESET_START && addr <= PARAM_PRESET_END)
	{
		DMPresetParamSetTable[idx](paramAddr, addrStart, addrEnd);
	}
	else
	{	
        *addrStart = (*paramAddr);
		*addrEnd   = (*paramAddr);
	}
}
#endif
//...


#if (1)		// DevInfo参数的处理
/*
****************************************************************************************************
**
**                                   DMParamSetNothing()
**
**
**	功能描述: 一个默认不做DSP底层处理的空函数。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
void DMParamSetNothing(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
}
#endif
//...


#if (1)		// Config参数的处理
#endif
//...


#if (1)		// Preset参数的处理
/*
****************************************************************************************************
**
**                                 DMParamSetPresetName()
**
**
**	功能描述: 对DSP音频模块进行写，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
void DMParamSetPresetName(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr) + 31;
}

/*
****************************************************************************************************
**
**                                   DMParamSetMonRate()
**
**
**	功能描述: 对DSP音频模块进行写，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
void DMParamSetMonRate(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

	float mic=0.0075f;
	float pc =0.001f;	
	
	DspDrv_Mixer_11(1.0f);

	//Mix2_USB (Gain)：这一路为PC伴奏增益 0.72f：-3.4dBV 14mW
	float usbGain = ParamTbl.Preset->PC_OutPut ? 0.0072f*ParamTbl.Preset->HP_Gain : 0.0f;    //0.75
	g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][1] = usbGain;

	//Mix2_adc:这一路为ADC输出至DAC，耳机监听修改这一路，MAX:0.86f  
	float adcGain = ParamTbl.Preset->Mic_OutPut ? 0.01f*ParamTbl.Preset->HP_Gain : 0.0f;
	g_gainNewAddMcu[Mcu_Gain_Direct_12][1] = adcGain;
	
	//加入监听总开关的做法
	#if(0)	
	if (ParamTbl.Preset->MonitorRate)
	{
		//DspDrv_MicMonModule (1);
		
		//Mix1_IN：Mixer11:这一路为人声输入至PC，不动
		DspDrv_Mixer_11(1.0f);
		
		//Mix2_USB (Gain)：这一路为PC伴奏增益
    float usbGain = ParamTbl.Preset->PC_OutPut ? 0.0075f*ParamTbl.Preset->HP_Gain : 0.0f;
    g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][1] = usbGain;
		
    //Mix2_adc:这一路为ADC输出至DAC，耳机监听修改这一路，MAX:0.86f  
    float adcGain = ParamTbl.Preset->Mic_OutPut ? 0.01f*ParamTbl.Preset->HP_Gain : 0.0f;
    g_gainNewAddMcu[Mcu_Gain_Direct_12][1] = adcGain;
	}
	else 
	{
		//DspDrv_MicMonModule (0);
		
		//Mix2_USB (Gain)：这一路为PC伴奏增益
		g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][1] = 0.0f;   //1.0f		
		
		//Mix2_adc:这一路为ADC输出至DAC，耳机监听修改这一路，MAX:0.86f  
		g_gainNewAddMcu[Mcu_Gain_Direct_12][1] = 0.0f;	
	}
#endif
	
}

/*
****************************************************************************************************
**
**                                   DMParamSetMicMute()
**
**
**	功能描述: 对DSP音频模块进行写，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
__RAM_CODE void DMParamSetMicMute(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
	
	uint32 offset = (*paramAddr) & MEM_OFFSET_ADDR_MASK;	// 与掉最高的4位，因为表格只需要偏移量
	
   if (ParamTbl.Preset->MicMute)
	{
        DspDrv_MicMuteModule(1);
		  
	}
	else 
	{
   		  DspDrv_MicMuteModule(0);
		
	}
	
 
}
/*
****************************************************************************************************
**
**                                   DMParamSetNoiseSuppressor()
**
**
**	功能描述: 对DSP音频模块进行写，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
__RAM_CODE void DMParamSetNoiseSuppressor(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
	float threshold=0.91f;
	float aa=0.5f;
#if 0 
    //菁音降噪
	if (ParamTbl.Preset->NoiseSuppressorEnabled==1)  // Garry
	{
        AppAlgDspRunModeSwitch(1);
        enableAns = 1;//        ControlAns(1);
        dspAlgParaAns.ansOnOff = ParamTbl.Preset->NoiseSuppressorEnabled;
        //dspAlgParaAns.ansMode = (ParamTbl.Preset->NoiseSuppressor4Levels<0)?(0):(ParamTbl.Preset->NoiseSuppressor4Levels>3)?(3):(ParamTbl.Preset->NoiseSuppressor4Levels);
        dspAlgParaAns.ansMode = ParamTbl.Preset->NoiseSuppressor4Levels; //
        DspAlgANSVarInit((DSP_ALG_PARA_ANS *)&dspAlgParaAns);
        DspAlgANSCalcBufferInit((DSP_ALG_PARA_ANS *)&dspAlgParaAns);
        DspAlgANSParaUpdate((int32_t *)DSP_IRAM_ADDR(DSP_ANS_INIT_ADDR));
 	}
	else if(ParamTbl.Preset->NoiseSuppressorEnabled==0)  // Garry
	{   
        AppAlgDspRunModeSwitch(0);
        enableAns = 0;//        ControlAns(1);
        dspAlgParaAns.ansOnOff = ParamTbl.Preset->NoiseSuppressorEnabled;
 	}
#else
    //降噪门
    // if (ParamTbl.Preset->NoiseSuppressorEnabled==1)
	// {
    //     noiseGateParaGroup0[0][0] = 128;  // preGain
    //     noiseGateParaGroup0[0][1] = 18;  // preGainEnergy
    //     //noiseGateParaGroup0[0][2] = ((int32)ParamTbl.Preset->NoiseSuppressor4Levels*threshold) - 91;  // threshold   -91-0
	// 			noiseGateParaGroup0[0][2] =(int32)((ParamTbl.Preset->NoiseSuppressor4Levels+1)*aa-90);       // threshold   -91- -40
    //     noiseGateParaGroup0[0][3] = 20 ;  // attackTime
    //     noiseGateParaGroup0[0][4] = 30 ;  // releaseTime
    //     noiseGateParaGroup0[0][5] = 0  ;  // level
        
    //     DspAlgGateUnitParaSet((ALG_DSP_PARA_GATE*)&noiseGateParaGroup0[0][0],(float32_t *)noiseGateParaGroup0[0][6]);
    //     enableGtae = 1;
	// 			//Dsp_Noise();
 	// }
	// else if(ParamTbl.Preset->NoiseSuppressorEnabled==0)
	// {   
    //     enableGtae = 0;
	// 	    Mcu_Original();
	// 			Dsp_Original();
 	// }

    //ENC
    Enc_SetOnOff(ParamTbl.Preset->NoiseSuppressorEnabled);

#endif
}

/*
****************************************************************************************************
**
**                                   DMParamSetMicEqualizerMode()
**
**
**	功能描述: 麦克风均衡器模式(场景)，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
void DMParamSetMicEqualizerMode(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
	
	uint32 offset = (*paramAddr) & MEM_OFFSET_ADDR_MASK;	// 与掉最高的4位，因为表格只需要偏移量

    if (ParamTbl.PresetArray[offset])                  //
	{
		switch(ParamTbl.Preset->MicMode)
		{
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;			
		}	
	}
	
}


/*
****************************************************************************************************
**
**                                   DMParamSet_Band()
**
**
**	功能描述: Band，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
__RAM_CODE void DMParamSet_Band(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
	
	//uint32_t *pParaBuff= (uint32_t *)((*paramAddr) & MEM_OFFSET_ADDR_MASK);	// 与掉最高的4位，因为表格只需要偏移量
	uint32 offset = (*paramAddr) & MEM_OFFSET_ADDR_MASK;

	uint32 band,Param;
	band = (offset-0x27)/5;
	Param = (offset-0x27)%5;
    
#if(1)
	
	switch(Param)
	{
		case 0: 
            {
                paraDefaultSpkEq[0][band].type  = ParamTbl.PresetArray[offset +1];
                paraDefaultSpkEq[0][band].fc    = ParamTbl.PresetArray[offset +2];
                paraDefaultSpkEq[0][band].gain  = (float)ParamTbl.PresetArray[offset +3]*0.1f-32;
                paraDefaultSpkEq[0][band].q     = (float)ParamTbl.PresetArray[offset +4]*0.1f;
            }break;                      //Enable 上位机不需要
		case 1:
			paraDefaultSpkEq[0][band].type       =  ParamTbl.PresetArray[offset]; break;
	  case 2:           
      paraDefaultSpkEq[0][band].fc         =  ParamTbl.PresetArray[offset];
		  if(paraDefaultSpkEq[0][0].fc==20||paraDefaultSpkEq[0][1].fc==20000)
			{
				paraDefaultSpkEq[0][band].type=0;
			}	
			break;
    case 3:                                                      
			paraDefaultSpkEq[0][band].gain       =  (float)ParamTbl.PresetArray[offset]*0.1f-32; break;  //Niko 精度0.1
		case 4:                                                               
			paraDefaultSpkEq[0][band].q          =  (float)ParamTbl.PresetArray[offset]*0.1f; break;  //Niko 精度0.1
	}
    paraDefaultSpkEq[0][0].q=0.7f;
    paraDefaultSpkEq[0][1].q=0.7f;
    McuAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultSpkEq[0][0], 7, 0);
#endif		

}




/*
****************************************************************************************************
**
**                                   DMParamSet_Comp()
**
**
**	功能描述: 压缩器Comp的操作，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
uint32 CompTest = 0xFFFFFFFF;
__RAM_CODE void DMParamSet_Comp(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

	/*
	判断使能Enable，使能时发默认参数
	失能发门限0，ratio 0,pregain 128
    把使能、失能的分两部分全写在一起
	*/ 
    
    
	 if(ParamTbl.Preset->Comp_Enable==0)
     {   //直通
        compParaGroup0[0]=128;
        compParaGroup0[1]=0;
        compParaGroup0[4]=100;
     }
    else  if(ParamTbl.Preset->Comp_Enable==1)
    {
        compParaGroup0[0]=ParamTbl.Preset->Comp_MakeUpGain;  
        compParaGroup0[1]=ParamTbl.Preset->Comp_Threshold-91;
        //compParaGroup0[1]=(ParamTbl.Preset->Comp_Threshold-91<0)?(0):(ParamTbl.Preset->Comp_Threshold-91>91)?(91):(ParamTbl.Preset->Comp_Threshold-91);
        compParaGroup0[2]=0xFF;//ParamTbl.Preset->Comp_Attack;
        compParaGroup0[3]=ParamTbl.Preset->Comp_Release;
        compParaGroup0[4]=(ParamTbl.Preset->Comp_Rate<100)?(100):((ParamTbl.Preset->Comp_Rate>2000)?(2000):(ParamTbl.Preset->Comp_Rate));
    }
    
    //传入底层
//    g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_LIN][0] = 0.5f        ;
//    g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_RIN][0] = 0.5f        ;
    
//    CompTest = 1000000;
//    ALG_DSP_PARA_DRC *pParaDrc = (ALG_DSP_PARA_DRC*)compParaGroup0;
//    DspAlgDrcUnitParaSet( pParaDrc , (ALG_DSP_VAR_DRC*)DSP_IRAM_ADDR(DSP_COMP_START_ADDR) );  
} 




/*
****************************************************************************************************
**
**                                   DMParamSet_Limit()
**
**
**	功能描述: Limit限幅器的操作，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
uint32 LimitTest = 0xFFFFFFFF;
__RAM_CODE void DMParamSet_Limit(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

	uint32 offset = (*paramAddr) & MEM_OFFSET_ADDR_MASK;
	
	uint32 Param;
	Param = (offset-0x50)%5;
	
        // 失能时参数
        if (ParamTbl.Preset->Limit_Enable == 0) 
        {
            for (int i = 0; i < 3; i++) 
            {
                algParaMbd.algParaDrc[i].preGain   = 128;
                algParaMbd.algParaDrc[i].threshold = 0;
                algParaMbd.algParaDrc[i].ratio     = 100;
            }
        }
        else if(ParamTbl.Preset->Limit_Enable == 1)  //发1的时候读参数表
        {
            for (int i = 0; i < 3; i++) 
            {
                switch (Param) 
                {   
                    //case0的逻辑屏蔽掉由上方if实现		algParaMbd.algParaDrc[i].preGain     = ParamTbl.PresetArray[offset];
                    case 0:    
                    {
                         algParaMbd.algParaDrc[i].preGain     = ParamTbl.Preset->Limit_PreGain;        
                         algParaMbd.algParaDrc[i].threshold   = ParamTbl.Preset->Limit_Threshold-91;   
                         algParaMbd.algParaDrc[i].attackTime  = 0xFF;//ParamTbl.Preset->Limit_Attack;         
                         algParaMbd.algParaDrc[i].releaseTime = ParamTbl.Preset->Limit_Release;        
                         algParaMbd.algParaDrc[i].ratio       = (ParamTbl.Preset->Limit_Rate<100)?(100):((ParamTbl.Preset->Limit_Rate>2000)?(2000):(ParamTbl.Preset->Limit_Rate));                   
                    }
                        
                    break;
                    case 1: algParaMbd.algParaDrc[i].preGain     = ParamTbl.Preset->Limit_PreGain;        break;
                    case 2: algParaMbd.algParaDrc[i].threshold   = ParamTbl.Preset->Limit_Threshold-91;   break;
                    case 3: algParaMbd.algParaDrc[i].attackTime  = 0xFF;break;//ParamTbl.Preset->Limit_Attack;         break;
                    case 4: algParaMbd.algParaDrc[i].releaseTime = ParamTbl.Preset->Limit_Release;        break;
                    case 5: algParaMbd.algParaDrc[i].ratio       = (ParamTbl.Preset->Limit_Rate<100)?(100):((ParamTbl.Preset->Limit_Rate>2000)?(2000):(ParamTbl.Preset->Limit_Rate)); break;
                    
                }
            }	
        }
     //传入底层
//    g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_LIN][0] = 0.5f        ;
//    g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_RIN][0] = 0.5f        ;
        
      LimitTest=1000000;  
      DspAlgMBDParaSet(&algParaMbd);  
                
    
    

}


#endif
//...





#if (1) 	// 其它处理
/*
****************************************************************************************************
**
**                                           DspMute()
**
**
**	功能描述: 声音渐变，进入静音
**
**	参  数  :
**
**	返回值  :
**
**  备  注  :
**
****************************************************************************************************
*/
void DspMute(void)
{
	DspDrv_DacMuteModule(1);
}


/*
****************************************************************************************************
**
**                                           DspUnmute()
**
**
**	功能描述: 声音渐变，退出静音
**
**	参  数  :
**
**	返回值  :
**
**  备  注  :
**
****************************************************************************************************
*/
void DspUnmute(void)
{
	DspDrv_DacMuteModule(0);
}
#endif

/*
****************************************************************************************************
**
**                                           Compression
**
**
**	功能描述: 压缩参数设置
**
**	参  数  :
**
**	返回值  :
**
**  备  注  :
**
****************************************************************************************************
*/
__RAM_CODE void DMParamSet_enable_comp(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
    if(ParamTbl.Preset->Comp_Enable<=1)
    {
        mcu_alg_enable_comp(0,0, !ParamTbl.Preset->Comp_Enable);
    }
}

__RAM_CODE void DMParamSet_comp_treshold(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd) 
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

    set_mcu_alg_comp_treshold(0,0, ParamTbl.Preset->Comp_Threshold);//取值：0-600    range: -50.0dB to 0.0dB, step: 0.1dB
}

__RAM_CODE void DMParamSet_comp_ratio(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd) 
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
    
    set_mcu_alg_comp_ratio(0,0, ParamTbl.Preset->Comp_Rate);//取值：10-1000     range: 1.0 to 100.0, step: 0.1
}    

__RAM_CODE void DMParamSet_comp_kneew(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd) 
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

    set_mcu_alg_comp_kneew(0,0,ParamTbl.Preset->Comp_KneeWidth);
}    

__RAM_CODE void DMParamSet_comp_attack(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)   
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

    set_muc_alg_comp_attack(0,0, ParamTbl.Preset->Comp_Attack);//取值：1-2000   range: 0.0001s to 0.2s, step: 0.0001s
}

__RAM_CODE void DMParamSet_comp_release(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)  
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

    set_muc_alg_comp_release(0,0, ParamTbl.Preset->Comp_Release);//取值：50-20000    range: 0.005s to 2s, step: 0.0001s
}

__RAM_CODE void DMParamSet_comp_mkg(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd) 
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

    set_muc_alg_comp_mkg(0,0, ParamTbl.Preset->Comp_MakeUpGain);//取值：0-100  range: 0.0dB to 60.0dB, step: 0.1dB
}    

/*
****************************************************************************************************
**
**                                           Limiter
**
**
**	功能描述: 限幅器参数设置
**
**	参  数  :
**
**	返回值  :
**
**  备  注  :
**
****************************************************************************************************
*/
__RAM_CODE void DMParamSet_enable_limiter(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)   
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

    if(ParamTbl.Preset->Limit_Enable<=1)
    {
        mcu_alg_enable_limiter(0,0, !ParamTbl.Preset->Limit_Enable);
    }
}
__RAM_CODE void DMParamSet_limiter_treshold(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
    
    set_mcu_alg_limiter_treshold(0,0, ParamTbl.Preset->Limit_Threshold);//取值：0-900  range: -90.0dB to 0.0dB, step: 0.1dB
}
__RAM_CODE void DMParamSet_limiter_attack(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)  
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
    
    set_muc_alg_limiter_attack(0,0, ParamTbl.Preset->Limit_Attack);//取值：1-2000  range: 0.0001s to 0.2s, step: 0.0001s
}
__RAM_CODE void DMParamSet_limiter_release(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd) 
{
    *addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);

    set_muc_alg_limiter_release(0,0, ParamTbl.Preset->Limit_Release);//取值：50-20000  range: 0.005s to 2s, step: 0.0001s
}

#if(1)
/*
****************************************************************************************************
**
**                                   DMParamSetMicGain()
**
**
**	功能描述: 对DSP音频模块进行写，会按模块参数操作（会涉及多个参数）。
**
**	参  数  : paramAddr    参数地址
**            addrStart    返回相应 Audio Module 的开始参数地址，因为一个模块会有多个参数
**            addrEnd      返回相应 Audio Module 的结束参数地址
**
**	返回值  :
**
****************************************************************************************************
*/
__RAM_CODE void DMParamSetMicGain(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd)
{
	// 按 Audio Module 参数块，更新其参数的开始和结束地址
	*addrStart = (*paramAddr);
	*addrEnd   = (*paramAddr);
	//uint32 offset = (*paramAddr) & MEM_OFFSET_ADDR_MASK;
//	if(ParamTbl.Preset->MicGain <= 100)
//	{
//        if (ParamTbl.Preset->MicMute)
//        {
//        	DspDrv_MicMuteModule(1);
//        }
//        else 
//        {
//            DspDrv_MicVol_Add_GainModule(ParamTbl.Preset->MicGain);
//        }

//	}
    
}
#endif
//...

