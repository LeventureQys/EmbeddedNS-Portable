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
**	文件名	: Dsp.Driver.c
**
**	程序员	: 
**
**	日　期  : 
**
**	文件描述: 
**
****************************************************************************************************
*/
#include "UserRetype.h"
#include "Helper.h"
#include "ParamTable.h"
#include "Dsp2.h"
#include "a0_hal_dsp.h"

#include "alg_filter.h"
#include "drv_alg.h"

#include "lib_drv_uac10.h"
//#include "drv_uac10_lib.h"

//增益Gain修改



extern ALG_PARA_FILTER paraDefaultAdcEq[1][5];
extern ALG_PARA_FILTER paraDefaultSpkEq[2][7];       //Garry
extern void DspAlgEqParaSet(ALG_PARA_FILTER *pEqPara, uint32_t segNumMax, uint32_t EqIdx);
extern void setAdcEqValue(int row, int col, float gain, float fc, float q, uint32_t type);

extern float g_gainNewAddMcu[ALG_GAIN_NUM][2] ;        //Garry
extern void McuAlgEqParaSet(ALG_PARA_FILTER *pPara, uint32_t segNumMax, uint32_t EqIdx);
//
//typedef struct {
//    float gain;
//    float freq;
//    float q;
//    ALG_FILTER_TYPE type;
//} EqSetting;

//static EqSetting lastSettings[3] = {
//    {.gain = -1, .freq = -1, .q = -1, .type = FILTER_NONE},  // 频段0
//    {.gain = -1, .freq = -1, .q = -1, .type = FILTER_NONE},  // 频段1
//    {.gain = -1, .freq = -1, .q = -1, .type = FILTER_NONE}   // 频段2
//};



/* 宏定义 */
#define GAIN_4_NS		(1.0f)
#define GAIN_Dir        (1.0f)          //Garry

/* 函数声明 */
//extern void my_hid_gate(uint8_t *pRecv);

//extern void test_eq(uint8_t index, uint8_t type);	// Steve test 调试EQ
//extern void test_eq(uint8_t type);	// Steve test 调试EQ

/* 变量定义和声明 */
//extern float32_t gainParaGrp0[DSP_GAIN_NUM];

//extern float32_t gainMixParaGrp0[DSP_GM_GAIN_NUM];  // Steve 调整混音以开启关闭监听


#if (1)		// 算法控制逻辑

/*
****************************************************************************************************
**
**                                 DspDrv_MicVolModule()
**
**	功能描述: MIC音量控制
**
**	参  数  : 
**
****************************************************************************************************
*/
void DspDrv_MicVolModule(fp32 Vol)
{
//    dsp_gain_set(ALG_GAIN_IDX_ADC_LIN, gain);
//    dsp_gain_set(ALG_GAIN_IDX_ADC_RIN, gain);
		gainDspPara[0]=Vol;
		LibDrvMemcpy((uint8_t *)DSP_IRAM_ADDR(DSP_GAIN_ADC_1_04_ADDR), (uint8_t *)&gainDspPara[0], 4);

		//上面那两个是旧的 gainDspPara 机制：刷新开关 dspAlgGainRefreshEn 全工程没人置 1，
		//DSP 侧也不再读 DSP_GAIN_ADC_1_04_ADDR，所以只写它听不出静音（静音键按下去灯变红、
		//MicMute 也翻转了，但声音一点不变，就是这个原因）。
		//现在真正管 ADC 输入增益的是 g_gainNewAddMcu 这张表——drv_alg.c 每帧调
		//AlgGainStepChangeF 把 [0] 往 [1] 上靠，MIC→USB 通路取的是 ADC_LIN / ADC_RIN 两项，
		//所以静音必须同时改这两项的目标值才会真正生效。留 [0] 不动，让 DSP 自己斜坡过渡，
		//避免直接切零产生爆音。
		g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_LIN][1] = Vol;
		g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_RIN][1] = Vol;
}


/*
****************************************************************************************************
**
**                                 DspDrv_Mixer_11()
**
**	功能描述: Mixer_11
**
**	参  数  : 移植DGM20算法，用于USB输出增强
**
****************************************************************************************************
*/
void DspDrv_Mixer_11(fp32 Vol)
{
//    dsp_gain_set(ALG_GAIN_IDX_ADC_LIN, gain);
//    dsp_gain_set(ALG_GAIN_IDX_ADC_RIN, gain);
	  gainDspPara[3]=Vol;
	  LibDrvMemcpy((uint8_t *)DSP_IRAM_ADDR(DSP_GAIN_MIXER11_ADDR), (uint8_t *)&gainDspPara[3], 4);
}



/*
****************************************************************************************************
**
**                                 DspDrv_MicMuteModule()
**
**	功能描述: MIC静音
**
**	参  数  : 
**
****************************************************************************************************
*/
void DspDrv_MicMuteModule(uint32 mute)
{
	if (mute)
	{
		DspDrv_MicVolModule(0);
	}
    else{
        DspDrv_MicVolModule(GAIN_Dir);
//		if (ParamTbl.Preset->NoiseSuppressorEnabled)
//		{
//			DspDrv_MicVolModule(GAIN_Dir);
//		}
//		else
//		{
//			DspDrv_MicVolModule(GAIN_Dir);			
//		}
	}
}
/*
****************************************************************************************************
**
**                                 DspDrv_NoiseReduction4Levels()
**
**	功能描述: 4档降噪
**
**	参  数  : 
**
****************************************************************************************************
*/
//void DspDrv_NoiseSuppModule(uint32 en)
//en取值[0,1]  threshold取值[0,3]
void DspDrv_4LevelsNoiseReduction(uint32 en, int32 levels)  // Garry
{  
	if (en)
	{
     switch(levels)
		 {
			 //第一档   
			 case 0:
			 break;
			 
			//第二档
			 case 1:
			 break;
			 
			 //第三档
			 case 2:
			 break;
			 
			 //第四档
			 case 3:
			 break;
		 }
	}
	//若关掉降噪使能，声音正常
	else
	{
        
		if (ParamTbl.Preset->MicMute == 0)
		{
			DspDrv_MicVolModule(1.0f);
		}
	}
	  //补充传入参数的函数......
}




/*
****************************************************************************************************
**
**                                 DspDrv_MicMonModule()  // Steve
**
**	功能描述: MIC监听控制
**
**	参  数  : 
**
****************************************************************************************************
*/
void DspDrv_MicMonModule (uint32 en)
{
#if 0
	float aa;                             //存储链路计算变量
	aa=0.0086f*ParamTbl.Preset->HP_Gain;
	if (en)
	{
		    //Mix2_USB (Gain)：这一路为PC伴奏增益，不动
        g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][1] = 0.57f;  
		    //Mix1_IN：Mixer11:这一路为人声输入至PC，不动
        DspDrv_Mixer_11(1.37f);
		
		    //Mix2_adc:这一路为ADC输出至DAC，耳机监听修改这一路，MAX:0.86f
        g_gainNewAddMcu[Mcu_Gain_Direct_12][1] = (aa<0?0:aa>100?100:aa);		
	}
	else
	{
        g_gainNewAddMcu[Mcu_Gain_Direct_12][1] = 0.0f        ;                          
	}
#else
	if (en)
	{
			  //Mix1_IN：Mixer11:这一路为人声输入至PC，不动
        DspDrv_Mixer_11(1.0f);
		    //Mix2_USB (Gain)：这一路为PC伴奏增益，不动
        g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][1] = 0.85f;   //1.0f		0.75f
		    //Mix2_adc:这一路为ADC输出至DAC，耳机监听修改这一路，MAX:0.86f  
        g_gainNewAddMcu[Mcu_Gain_Direct_12][1] = 1.0f;
	}
	else
	{
        g_gainNewAddMcu[Mcu_Gain_Direct_12][1] = 0.0f;                          
	}
#endif
}




/*
****************************************************************************************************
**
**                                 DspDrv_UsbOutIncModule() // Steve
**
**	功能描述: USB输出音频增强
**
**	参  数  : 
**
****************************************************************************************************
*/
void DspDrv_UsbOutIncModule (uint32 en)
{
//	if (en)
//	{
//        //Mix2_USB (Gain)：     
//        g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][1] = 0.57f;                                       
//        //Mix1_IN：Mixer11
//        DspDrv_Mixer_11(2.87f);
//	}
//	else
//	{       
//        //Mix2_USB (Gain)：
//        g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][1] = 0.57f;  
//        //Mix1_IN：Mixer11
//        DspDrv_Mixer_11(1.37f);                                 
//	}

//	if(ParamTbl.Preset->MonitorRate == 3000) // Steve test 新增控制监听音量
//	{
//		DspDrv_MicMonModule (0); 
//	}
//	else
//	{
//		DspDrv_MicMonModule (1);
//	}
}


void Mcu_setFilterParams(int filterIndex, float gain, float fc, float q, uint32_t type) 
{
    if (filterIndex >= 0 && filterIndex < 7) 
		{
        paraDefaultSpkEq[1][filterIndex].gain = gain;
        paraDefaultSpkEq[1][filterIndex].fc = fc;
        paraDefaultSpkEq[1][filterIndex].q = q;
        paraDefaultSpkEq[1][filterIndex].type = type;
    }
		
		McuAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultSpkEq[1][0], 7, 2);
}


#if(0)
// 预定义的场景设置函数
void Mcu_Original(void)                //原声
{   
    Mcu_setFilterParams(0, -2.0f, 150.0f,  1.4f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(1, 3.0f,  260.0f,  0.7f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(2, -2.0f, 4000.0f, 0.9f, (uint32_t)FILTER_LS);
    Mcu_setFilterParams(3, 0.5f,  500.0f,  0.9f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(4, 1.0f,  8000.0f, 1.2f, (uint32_t)FILTER_HS);
    Mcu_setFilterParams(5, 1.5f,  1000.0f, 0.8f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(6, 0.5f,  80.0f,   0.8f, (uint32_t)FILTER_PEAK);
	
}
#endif

void Mcu_Original(void)                //原声
{   
    Mcu_setFilterParams(0, -2.5f, 120.0f,  0.9f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(1, 1.7f,  260.0f,  0.8f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(2, -2.0f, 4000.0f, 0.9f, (uint32_t)FILTER_LS);
    Mcu_setFilterParams(3, 0.3f,  300.0f,  0.7f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(4, 1.3f,  8600.0f, 1.0f, (uint32_t)FILTER_HS);
    Mcu_setFilterParams(5, 1.4f,  1000.0f, 0.8f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(6, 0.5f,  80.0f,   0.9f, (uint32_t)FILTER_PEAK);
	
}

void Mcu_Noise(void)               //降噪补偿
{
    Mcu_setFilterParams(0, 0.0f,  1000.0f,  0.7f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(1, 0.0f,  1000.0f,  0.7f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(2, 0.0f,  1000.0f,  0.7f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(3, 0.0f,  1000.0f,  0.7f, (uint32_t)FILTER_PEAK);          
    Mcu_setFilterParams(4, 0.0f,  1000.0f,  0.7f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(5, 0.0f,  1000.0f,  0.7f, (uint32_t)FILTER_PEAK);
    Mcu_setFilterParams(6, 0.0f,  1000.0f,  0.7f, (uint32_t)FILTER_PEAK);
}


//void Mcu_setFilterParams2(int filterIndex, float gain, float fc, float q, uint32_t type) 
//{
//    if (filterIndex >= 0 && filterIndex < 7) 
//		{
//        paraDefaultSpkEq[1][filterIndex].gain = gain;
//        paraDefaultSpkEq[1][filterIndex].fc = fc;
//        paraDefaultSpkEq[1][filterIndex].q = q;
//        paraDefaultSpkEq[1][filterIndex].type = type;
//    }
//		
//		McuAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultSpkEq[1][0], 7, 2);
//}



void Dsp_setFilterParams(int filterIndex, float gain, float fc, float q, uint32_t type) 
{

    if (filterIndex >= 0 && filterIndex < 5) 
		{
        paraDefaultAdcEq[0][filterIndex].gain = gain;
        paraDefaultAdcEq[0][filterIndex].fc = fc;
        paraDefaultAdcEq[0][filterIndex].q = q;
        paraDefaultAdcEq[0][filterIndex].type = type;
    }
		
		DspAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultAdcEq[0][0], 5, 0);

		
}
#if(0)
// 预定义的场景设置函数
void Dsp_Original(void)                //原声
{
    Dsp_setFilterParams(0, 3.0f, 3000.0f, 0.9f, (uint32_t)FILTER_HS);
    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);	
}
#endif
void Dsp_Original(void)                //原声
{
    Dsp_setFilterParams(0, 1.4f, 4000.0f, 1.0f, (uint32_t)FILTER_HS);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);	       
}

void Dsp_Noise(void)               //降噪补偿
{
    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);
//    Dsp_setFilterParams(0, 0.0f, 1000.0f, 0.7f, (uint32_t)FILTER_PEAK);	
}











//******************************************以下模块暂时用不上*****************************************************

/*
****************************************************************************************************
**
**                                 DspDrv_MicGainModule()
**
**	功能描述: MIC增益控制  PD100U Gen2增益固定，暂时用不上该增益调节
**
**	参  数  : 
**
****************************************************************************************************
*/
void DspDrv_MicGainModule(int8 Gain)
{
    AppCodecMicVolCfg(Gain,Gain);
}


/*
****************************************************************************************************
**
**                                 DspDrv_EQModule()  // Niko
**
**	功能描述: EQ设置
**
**	参  数  : 
**
****************************************************************************************************
*/
void DspDrv_EQModule(int32_t group,int32_t  seg, ALG_PARA_FILTER *pFilter)
{
//	dsp_eq_set(group,  seg,  pFilter);
}


/*
****************************************************************************************************
**
**                                 DspDrv_Game71Module()  // Niko
**
**	功能描述: 71音效设置
**
**	参  数  : 0:关闭，1:打开
**
****************************************************************************************************
*/
void DspDrv_Game71Module(uint32 en)
{
//	if (en)
//	{//打开
//        dsp_game71_set(true);
//	}
//	else
//	{//关闭
//        dsp_game71_set(false);
//	}
}


/*
****************************************************************************************************
**
**                                 DspDrv_NoiseSuppModule()
**
**	功能描述: 噪声抑制
**
**	参  数  : 
**
****************************************************************************************************
*/
//void DspDrv_NoiseSuppModule(uint32 en)
//en取值[0,1]  threshold取值[0,3]
void DspDrv_NoiseSuppModule(uint32 en, int32 threshold)  // Liang
{  
//      ALG_DSP_PARA_GATE algParaGatePara;

}


/*
****************************************************************************************************
**
**                                 DspDrv_DacMuteModule()
**
**	功能描述: DAC输出静音
**
**	参  数  : 
**
****************************************************************************************************
*/
void DspDrv_DacMuteModule(uint32 mute)
{
//	if (mute)
//	{
//        dsp_gain_set(ALG_GAIN_IDX_DAC_LOUT, 0.0f);
//        dsp_gain_set(ALG_GAIN_IDX_DAC_ROUT, 0.0f);
//	}
//	else
//	{
//        dsp_gain_set(ALG_GAIN_IDX_DAC_LOUT, 1.0f);
//        dsp_gain_set(ALG_GAIN_IDX_DAC_ROUT, 1.0f);
//	}
}

#endif
//...





