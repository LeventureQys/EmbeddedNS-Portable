
#include "drv_alg.h"
#include "app_alg_dsp_ans.h"
#include "algDspUser.h"
#include "alg_filter.h"
#include "drv_uac10.h"
#include "lib_drv_uac10.h"
#include "app_dma.h"
#include "app_i2s.h"
#include "app_alg_dsp_config.h"
#include "app_alg_dsp_filter.h"
#include "app_alg_dsp_gate_knee.h"
#include "app_alg_dsp_split_filter.h"


#include "para_default_tb.c"
#include "para_default_filter.c"

#include "alg_gamemode4.h"
#include "alg_game71.h"

#include "app_alg_dsp_mbd.h"

#include "para_default_game71.c"
#include "Timer.h"
#include "UsbUSerInfoSet.h"

#include "drv_alg_ans.c"
#include "drv_alg_api.c"

#include "alg_dcblock.h"
#include "alg_freqshift.h"
#include "alg_rms.h"              //Garry


#include "mcu_alg_comp.h"
#include "mcu_alg_limiter.h"

#include "ParamTable.h"

#include "enc.h"

#define ALG_LIB_VER_STR_SIZE                (0x12)
// 1, 16bit
// 2, 24bit
#define I2S_INOUT_RESOLUTION_LEVEL          (2)//(1)

#define FIXP_CLIP_MAX_16BIT                 (32767)
#define FIXP_CLIP_MIN_16BIT                 (-32767)

#define FIXP_CLIP_MAX_24BIT                 (8388607)
#define FIXP_CLIP_MIN_24BIT                 (-8388607)

uint8_t enableAns;
uint8_t enableGtae;

//增益调整：对输入信号的强度进行调整，确保信号在合适范围内
void AlgGainStepChangeF(float32_t *gain, float32_t step);

//int32_t IfxSelect[1]={1}; //0 bypass,1 game71,2 game4
int32_t IfxSelect[1] = {0};

extern int32_t     compParaGroup0[5];

//Garry   RMS电平显示
__RAM02 ALG_PARA_RMSCALC algParaRMSCalc[2];
__RAM02 int32_t algVarRMSCalc[2][ALG_VAR_SIZE_RMS_MONO];
__RAM02 float rmsValue[2];

__RAM02 int32_t dspAlgAnsVar[DSP_ALG_ANS_VAR_SIZE];
__RAM01 int32_t dspAnsBuf[DSP_ALG_ANS_CALCBUFF_SIZE];
__RAM01 int32_t ansRingBuf[ANS_RINGBUF_LEN];


ALG_PARA_GAMEMD4 algParaGameMode4 ;
__RAM1 int32_t algVarGameMode4[ALG_GAMEMD4_VAR_SIZE];
__RAM01 int32_t algDlyLineGameMode4[ALG_GAMEMD4_DELAYLINE_SIZE];


ALG_PARA_GAME71 algParaGame71;
__RAM1 int32_t algVarGame71[ALG_GAME71_VAR_SIZE];
__RAM01 int32_t algDlyLineGame71[ALG_GAME71_DELAY_SIZE];


ALG_PARA_DCBLOCK algParaDCBlk[2];
int32_t algVarDCBlk[2][ALG_VAR_SIZE_DCBLOCK_STEREO];


__RAM1 int32_t algVarFreqShift[1][ALG_VAR_SIZE_FREQSHIFT];
__RAM01 ALG_PARA_FREQ algParaFreqShift[1];
int32_t freqOnOff = 0;


__RAM1 int16_t SinTemp[514];

//Limit
ALG_DSP_PARA_MBD algParaMbd =
{
    .fc1 = 1000,
    .fc2 = 3000,
    // freq < fc1
	
	/*
	    .preGain = 128,
        .threshold = -86,
        .attackTime = 10,
        .releaseTime = 20,
        .ratio = 1000,
	*/
    .algParaDrc[0] =
    {
        .preGain = 128,
        .threshold = 0,    //-86
        .attackTime = 10,
        .releaseTime = 20,
        .ratio = 100,
    },
    // freq > fc2
    .algParaDrc[1] =
    {
        .preGain = 128,
        .threshold = 0,
        .attackTime = 10,
        .releaseTime = 20,
        .ratio = 100,
    },
    // fc1 < freq < fc2
    .algParaDrc[2] =
    {
        .preGain = 128,
        .threshold = 0, // -86
        .attackTime = 10,
        .releaseTime = 20,
        .ratio = 100,
    },
		
    /*
        .preGain = 128,
        .threshold = -86, //0,
        .attackTime = 10,
        .releaseTime = 20,
        .ratio = 1000,
    */
    .dbgG1 = 1.0f,
    .dbgG2 = 1.0f,
    .dbgG3 = 1.0f,
};



__RAM1  int32_t algVarMbd[ALG_VAR_SIZE_MBD];

__RAM1 int32_t algVarGate[10];


DSP_ALG_PARA_ANS dspAlgParaAns;

DSP_ALG_PARA_SPLT_FILT	dspAlgParaSplitFilter;
__RAM01 int32_t algDlyLineSplitFilt[DSP_ALG_SPLFILTER_DELAYLINE_SIZE];




uint8_t dbg_i2s_dma_first_enter_true_mic = 0;
uint8_t dbg_i2s_dma_first_enter_true_spk = 0;
uint8_t dbg_i2s_dma_first_enter_frame_disable = 0;

uint32_t dsp_not_finished_time_2 = 0;
extern uint32_t dbg_dma_int_enter_cnt;

// var for dspmode switch usage
// [0] real, [1] target
// perform gradual change
float g_gainNewAddMcu[ALG_GAIN_NUM][2] =
{

    {0.0f, 1.0f}, // gainInAdcL             0
    {0.0f, 1.0f}, // gainInAdcR             1
     
    {0.0f, 1.0f}, // gainInSpkL             2  不给1.0f，声音太大
    {0.0f, 1.0f}, // gainInSpkR             3

    {0.0f, 0.0f}, // Gain_USB_15            4  0：USB Play不混进电脑Mic
    {0.0f, 1.0f}, // Gian_Direct_12   		5

    {0.0f, 1.0f}, // Gain_Mic_L       6
    {0.0f, 1.0f}, // Gain_Mic_R       7

    {0.0f, 1.0f}, // Gain_DacLout      8
    {0.0f, 1.0f}, // Gain_DacRout      9
    
    {0.0f, 1.0f}, // Gain_Mix2_USB      10
  
    
    
};


float_t GainMix11VolCur;
int32_t dbgVarSize;
uint8_t AnsOnOffDefault = 0; //1;  //ans 默认打开


// 0, dsp run ans (codec set at 48K)
// 1, dsp do NOT run ans (codec set at 96K)
//uint8_t dspCurRunMode = 0;

/***************eq para********************/
// dsp with ans eq para
float *pEqGroupAnsPara[DSP_EQ_GROUP_NUM] =
{
    (float *) paraDefaultAdcEq,
};



ALG_DSP_EQ_PARA eqPara[1] =
{
    {
        // ans eq para
        .pGroupPara = (float **) &pEqGroupAnsPara,
    },

};


/***************gate para********************/
//int32_t noiseGateParaGroup0[1][7] =
//{
//    // TODO : add complete para
//    {
//        128,            // preGain
//        128,            // preGainEnergy
//        -90,    //-90,          // threshold
//        40,         // attackTime
//        60,         // releaseTime
//        0,          // level
//        0,          // varAddr, NOT used
//
//    }
//};

//int32_t *pNoiseGateGroupPara[DSP_GATE_GROUP_NUM] =
//{
//    (int32_t *) &noiseGateParaGroup0,
//
//};


//ALG_DSP_NGATE_PARA noiseGatePara =
//{
//    .pGroupPara = (int32_t **) &pNoiseGateGroupPara,
//
//
//};



/***************gate knee para********************/

int32_t algVarGateKnee[DSP_GATE_KNEE_GROUP_NUM][ALG_DSP_VAR_SIZE_GATE_KNEE];

ALG_DSP_PARA_GATE_KNEE algParaGateKnee[DSP_GATE_KNEE_GROUP_NUM] =
{
	{
		.threshold_db = -70.0f,
		
		.ratio =  1/3.0f,
		.attack_ms = 10.0f,
		.release_ms = 200.0f,
		.knee_width_db = 12.0f,
		.hp_freq = 150.0f,
		.varAddr = (int)&algVarGateKnee[0][0],
	},
};



//增益值修改    
//2                 gain para
//gain值修改
float gainDspPara[DSP_GAIN_NUM] =
{
    1.0f,   // Gain_ADC_1_04         
    1.0f,   // Gain_Direct_10
    0.0f,   // Gain_09
    1.0f,   // Gain_Mixer_11
    1.0f,   // GainAdcIn
};

uint8_t gainMixParaReFreshFlag = 0;

float dacFilterMemL[4 * DACOUT_FILTER_NUM];
float dacFilterMemR[4 * DACOUT_FILTER_NUM];
float dacFilterCoeff[5 * DACOUT_FILTER_NUM];
float dacFilterCoeffSet[5 * DACOUT_FILTER_NUM];

float spkFilterMemL[4 * SPK_FILTER_NUM];
float spkFilterMemR[4 * SPK_FILTER_NUM];
float spkFilterCoeff[5 * SPK_FILTER_NUM];
float spkFilterCoeffSet[5 * SPK_FILTER_NUM];

float spkFilterMemL2[4 * SPK_FILTER_NUM];
float spkFilterMemR2[4 * SPK_FILTER_NUM];
float spkFilterCoeff2[5 * SPK_FILTER_NUM];
float spkFilterCoeffSet2[5 * SPK_FILTER_NUM];


// stereo
ALG_FILTER_F32 DACOutFilterL[1] =
{
    {
        .numStages = DACOUT_FILTER_NUM,
        .pState = dacFilterMemL,
        .pCoeffs = dacFilterCoeff,

    },
};

ALG_FILTER_F32 DACOutFilterR[1] =
{
    {
        .numStages = DACOUT_FILTER_NUM,
        .pState = dacFilterMemR,
        .pCoeffs = dacFilterCoeff,

    },
};



// stereo
ALG_FILTER_F32 SpkInFilterL[1] =
{
    {
        .numStages = SPK_FILTER_NUM,
        .pState = spkFilterMemL,
        .pCoeffs = spkFilterCoeff,

    },
};

ALG_FILTER_F32 SpkInFilterR[1] =
{
    {
        .numStages = SPK_FILTER_NUM,
        .pState = spkFilterMemR,
        .pCoeffs = spkFilterCoeff,

    },
};

ALG_FILTER_F32 SpkInFilterR2[1] =
{
    {
        .numStages = SPK_FILTER_NUM,
        .pState = spkFilterMemR2,
        .pCoeffs = spkFilterCoeff2,

    },
};

ALG_FILTER_F32 SpkInFilterL2[1] =
{
    {
        .numStages = SPK_FILTER_NUM,
        .pState = spkFilterMemL2,
        .pCoeffs = spkFilterCoeff2,

    },
};




void  AppAlgMcuFilterInit(void)
{
    for (int i = 0; i < SPK_FILTER_NUM; i++)
    {
        // 调用AlgFilterDesign函数计算滤波器系数，并存储在spkFilterCoeff数组中
        AlgFilterDesign((ALG_PARA_FILTER *)&paraDefaultSpkEq[0][i], &spkFilterCoeff[5 * i], ALG_SAMPLE_RATE);
    }
		//Garry
    for (int i = 0; i < SPK_FILTER_NUM; i++)
    {
        // 调用AlgFilterDesign函数计算滤波器系数，并存储在spkFilterCoeff数组中
        AlgFilterDesign((ALG_PARA_FILTER *)&paraDefaultSpkEq[1][i], &spkFilterCoeff2[5 * i], ALG_SAMPLE_RATE);
    }		
		
    for (int i = 0; i < DACOUT_FILTER_NUM; i++)
    {
        // 调用AlgFilterDesign函数计算滤波器系数，并存储在dacFilterCoeff数组中
        AlgFilterDesign((ALG_PARA_FILTER *)&paraDefaultDacEq[0][i], &dacFilterCoeff[5 * i], ALG_SAMPLE_RATE);
    }
}

uint8_t hidDrcParaUpdateFlag = 0;



//Comp压缩器
int32_t     compParaGroup0[5] =
{
    128,             //preGain;
    0,               //threshold;
    10,              //attackTime;
    20,              //releaseTime;
    100,             //ratio     100~2000

};


//int32_t *pCompGroupPara[DSP_COMP_GROUP_NUM] =
//{
//    (int32_t *) &compParaGroup0,

//};
//ALG_DSP_COMP_PARA   compPara =
//{
//    .pGroupPara = (int32_t **) &pCompGroupPara,
//};




uint32_t testvar;
/*
 * dspRunMode = 0, codec set @48K , dsp run ans  
 * 当 dspRunMode = 0 时，编解码器设置为 48K，DSP 运行 ANS
 * dspRunMode = 1, codec set @96K , dsp do NOT run ans  
 * 当 dspRunMode = 1 时，编解码器设置为 96K，DSP 不运行 ANS
 */
void DspAlgInit(void)
{
    algDspInit();
    // update ans relavant var
    dbgVarSize = DspAlgANSVarSize();    // R0 = 0x 3F  获取ANS变量大小
    dbgVarSize = DspAlgANSCalcBuffSize();   // R0 = 0x 196D    获取ANS计算缓冲区大小
    dspAlgParaAns.varAddr = (int32_t)&dspAlgAnsVar[0];    //设置ANS变量地址
    dspAlgParaAns.bufferAddr2 = (int32_t)&dspAnsBuf[0];   //设置ANS缓冲区地址
    dspAlgParaAns.ansOnOff = AnsOnOffDefault;             //设置ANS开关状态
    dspAlgParaAns.ansMode = 0;                            //设置ANS模式
    DspAlgANSVarInit((DSP_ALG_PARA_ANS *)&dspAlgParaAns);  //初始化ANS变量
    DspAlgANSCalcBufferInit((DSP_ALG_PARA_ANS *)&dspAlgParaAns);  //初始化ANS计算缓冲区
    DspAlgANSParaUpdate((int32_t *)DSP_IRAM_ADDR(DSP_ANS_INIT_ADDR));  //更新ANS参数
    DspANSAlgSinLogTbInit();                                  //初始化正弦和对数表
    // init fft bit reverse table in iram
    DspANSAlgFFTInit();                                      //初始化快速傅里叶变换FFT
    DspAlgFFTRadix4BitRvsTbInit(0);                          //初始化 FFT的基-4位反转表


	// split filter init
	dspAlgParaSplitFilter.delayLineAddr = (int)&algDlyLineSplitFilt[0];
	DspAlgSplitFilterInit(&dspAlgParaSplitFilter);


	
    AppAlgMcuFilterInit();                                  //初始化MCU 滤波器
	
    // init eq in dsp                            EQ均衡器初始化
    if (algDspInitEq(&eqPara[0]))
    {
        __breakpoint(0);                      //如果初始化失败，触发断点
    }
		// DC阻塞初始化                         
    algParaDCBlk[0].chMode = 1;
    algParaDCBlk[0].varAddr = (int32_t)&algVarDCBlk[0];
    AlgDcblockInit(&algParaDCBlk[0]);
		
		
		//噪声门初始化
    // init gate in dsp

	if (algDspInitGateKnee( &algParaGateKnee[0]) )
    {
        __breakpoint(0);
    }
		
		
    // init gain in dsp  增益控制初始化
    if (algDspInitGain(&gainDspPara[0]))
    {
        __breakpoint(0);                        //如果初始化失败，触发断点
    }
        //Drc初始化
//    algParaMbd.varAddr = (int32_t)&algVarMbd[0];
//    algDspInitMBD(&algParaMbd);
		
		//压缩器初始化
//    if (algDspInitComp(&compPara))
//    {
//        __breakpoint(0);
//    }
    
    //My Comp
    mcu_alg_comp_init(NULL);
    //My Limiter
    mcu_alg_limiter_init(NULL);

    //RMS电平显示 初始化
//    algParaRMSCalc[0].averageTime = 25;
//    algParaRMSCalc[0].varAddr = (int32_t)&algVarRMSCalc[0];
//    if (AlgRmsCalcInit(&algParaRMSCalc[0]))
//    {
//        __breakpoint(0);
//    }
		
//    // set at 10ms delay //ANS
//    algDspSetRunMode(1);
//    obsvDspRunMode = 1;
    // set at 1ms delay
	//DSP运行模式设置 为1ms延迟 //Gate
      algDspSetRunMode(0);
      obsvDspRunMode = 0;
		
		//游戏模式相关初始化：初始化4种游戏模式、7.1声道的相关变量和缓冲区
//    algParaGameMode4.varAddr = (int32_t)&algVarGameMode4;
//    algParaGameMode4.bufferAddr = (int32_t)&algDlyLineGameMode4;
//    AlgGameMode4Init(&algParaGameMode4);
//    algParaGame71 = paraDefaultGM71[0];
//    algParaGame71.varAddr = (int32_t)&algVarGame71;
//    algParaGame71.bufferAddr = (int32_t)&algDlyLineGame71;
//    AlgGame71Init(&algParaGame71);
		
		//延迟线初始化：初始化通用延迟线，并清零游戏模式相关的延迟缓冲区
    palgGeneralTmDelayLine = (ALG_GENENRAL_TM_DELAYLINE *)&ansRingBuf[0];
    palgGeneralTmDelayLine->dspBuffIdx = 0;
    palgGeneralTmDelayLine->mcuBuffIdx = 0;
//    LibDrvMemset((void*)AllDelayLine, 0x00, sizeof(AllDelayLine));

//    LibDrvMemset((void *)algDlyLineGameMode4, 0x00, sizeof(algDlyLineGameMode4));
//    LibDrvMemset((void *)algDlyLineGame71, 0x00, sizeof(algDlyLineGame71));

    AlgSinTableSet(SinTemp);
		
		//频率偏移初始化：配置频率偏移参数并初始化
//    algParaFreqShift[0].freqOffset = -10; //10;
//    algParaFreqShift[0].freqOnOff = 1;
//    algParaFreqShift[0].varAddr = (int32_t)&algVarFreqShift[0];
//    AlgFreqShiftInit(&algParaFreqShift[0]);

    //ENC init
    Enc_Init(&g_EncParam, enc_level);
		
    //
    bDspRunEn = 1;
}



void DrvAlgInit(void)
{
    DspAlgInit();
    NVIC_SetPriority(USER1_IRQn, 2);        // 开启用户软件IRQ中断1
    NVIC_EnableIRQ(USER1_IRQn);
    
    //ENC
    NVIC_SetPriority(USER2_IRQn, 2);        // 开启用户软件IRQ中断2
    NVIC_EnableIRQ(USER2_IRQn);
}


// pMem, 指向指针数组，具体定义根据实际的输入输出数据流调整
// 数据流通路上的算法处理，帧中断（DMA帧中断等中断中处理的）
/*


 */


//volatile float fAdcL,fAdcR,fI2sL,fI2sR,fUsbL,fUsbR;

#include "enc.h"
#include "a0_hal_gpio.h"
#include "Gpio.h"
extern uint8_t enc_write;
extern uint8_t enc_cnt;
bool EncOnOff = 1;//ENC开关，默认关

//#pragma push
//#pragma O0
__RAM_CODE void DrvAlgFrameProcess(void **pMem)
{
    int32_t cntI = 0;
	/* 输入信号初始化与转换
     输入信号是32位定点数，缩小到16位并转换成浮点数
	   i32InDacL 和 i32InDacR 是从 DAC（数模转换器）输出的信号数据
	   i32InAdcL 和 i32InAdcR 是从 ADC（模数转换器）输入的信号数据
	   i32TempL 和 i32TempR 用作中间缓冲区
	   */
    int32_t *i32InDacL = (int32_t *)pMem[0];
    int32_t *i32InDacR = (int32_t *)pMem[1];
    int32_t *i32InAdcL = (int32_t *)pMem[2];
    int32_t *i32InAdcR = (int32_t *)pMem[3];
    // temp buffer for gameMode
    int32_t *i32TempL = (int32_t *)pMem[6];
    int32_t *i32TempR = (int32_t *)pMem[7];
	
  	
    // spk in L and spk in R
    float *fInDacL = (float *)i32InDacL;
    float *fInDacR = (float *)i32InDacR;
    // adc in L and adc in R
    float *fInAdcL = (float *)i32InAdcL;
    float *fInAdcR = (float *)i32InAdcR;
//    float *fInTempL = (float *)i32TempL;
//    float *fInTempR = (float *)i32TempR;

    
    //adc写入enc_in 读出enc_out
    int32_t adcl_in_i = 48*enc_cnt + 480*enc_write;
    int32_t adcl_out_i = 48*enc_cnt + 480*enc_write;
    
	 /*
	  数字音量调整，不是增益
    调用 AlgGainStepChangeF 函数，逐步对音频信号的增益进行调整。
		g_gainNewAddMcu 是一个数组，存储了增益调整的参数，ALG_GAIN_NUM 是增益的数量
	 */
    for (cntI = 2; cntI < (ALG_GAIN_NUM - 2) ; cntI++)
    {
        AlgGainStepChangeF(g_gainNewAddMcu[cntI], 1.0f / 128.0f);
    }
    uint32_t sampleNum;
		
		/*
		输入信号的分辨率处理：
		根据I2S_INOUT_RESOLUTION_LEVEL的设置，输入信号的分辨率
		会被调整：
		1：信号会右移16位；
		2：信号会右移8位；
		*/
    sampleNum = ALG_FRAME_LEN;
    while (sampleNum != 0)
    {
#if (I2S_INOUT_RESOLUTION_LEVEL == 1)
        *fInDacL++ = (float)(*i32InDacL++ >> 16);
        *fInDacR++ = (float)(*i32InDacR++ >> 16);
        *fInAdcL++ = (float)(*i32InAdcL++ >> 16);
        *fInAdcR++ = (float)(*i32InAdcR++ >> 16);
#elif (I2S_INOUT_RESOLUTION_LEVEL == 2)
        // speaker input
        *fInDacL++ = (float)(*i32InDacL++ >> 8);
        *fInDacR++ = (float)(*i32InDacR++ >> 8);
        // adc in,
//        enc_in[adcl_in_i++] = (float)(*i32InAdcL << 8);
        *fInAdcL++ = (float)(*i32InAdcL++ >> 8);
        *fInAdcR++ = (float)(*i32InAdcR++ >> 8);
#endif
        sampleNum--;
    }
#if (DBG_TIME_CALC_EN != 0)
    uint32_t dTimCnt;
    uint32_t idx = 0;
#endif
    // 算法处理
    float32_t *pfBuff[12];
		
		/*
      DC阻塞处理，对输入信号进行DS偏移的阻塞处理，通常用于去除信号中的
      直流成分，保证信号的质量
     */
    //volatile float32_t *pfBuff[12];
//    pfBuff[0] = (float32_t *)pMem[2];//Adc_L
//    pfBuff[1] = (float32_t *)pMem[3];//Adc_R
//    AlgDcblockProcess(&algParaDCBlk[0], (void *)pfBuff);
    
    //ENC
    #if 1
    fInAdcL = (float *)(pMem[2]);
    for(uint8_t i = 0; i<48; i++)
    {
        enc_in[adcl_in_i++] = *fInAdcL;
        if (EncOnOff)
        {
            *fInAdcL++ =  enc_out[adcl_out_i++];
        }
    }
    #endif
		
    
    // adcL, adcR
    pfBuff[0] = (float32_t *)pMem[2];
    pfBuff[1] = (float32_t *)pMem[3];
    // spkL, spkR
    pfBuff[2] = (float32_t *)pMem[0];
    pfBuff[3] = (float32_t *)pMem[1];

		/*
    信号混合：
    在 sampleNum 循环中，将左右声道的ADC信号进行混合处理
     pfBuff[0] = (*pfBuff[0] * McuAdcLin + *pfBuff[1] * McuAdcRin) / 2
     这行代码将左右声道的信号按增益因子 McuAdcLin 和 McuAdcRin 混合成单一的信号
		 混合的过程中使用了增益系数（McuAdcLin、McuAdcRin、McuSpkLin、McuSpkRin）
		 来控制信号的增益。混合的方式是对左声道和右声道的信号进行加权平均。
		*/
    // float McuAdcLin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_LIN][0];
    // float McuAdcRin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_RIN][0];
    float McuSpkLin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_SPK_LIN][0];
    float McuSpkRin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_SPK_RIN][0];
    float fgainInAdcLCtrl = 0.0f;
    float fgainInAdcRCtrl = 0.0f;
    // mix adcL with adcR to adcL and adcR
    // usb speaker left/right data mix gain process
    sampleNum = ALG_FRAME_LEN;
		
    while (sampleNum)
    {
	    fgainInAdcLCtrl = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_LIN][0];
        AlgGainStepChangeF(g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_LIN], 0.001f);
        fgainInAdcRCtrl = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_RIN][0];
        AlgGainStepChangeF(g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_RIN], 0.001f);
        //*pfBuff[0] = (*pfBuff[0] * McuAdcLin + *pfBuff[1] * McuAdcRin) / 2;
        *pfBuff[0] = (*pfBuff[0] * fgainInAdcLCtrl + *pfBuff[1] * fgainInAdcRCtrl) ;
        *pfBuff[1] = *pfBuff[0];
        *pfBuff[2] = *pfBuff[2] * McuSpkLin;
        *pfBuff[3] = *pfBuff[3] * McuSpkRin;
        pfBuff[0]++;
        pfBuff[1]++;
        pfBuff[2]++;
        pfBuff[3]++;
        sampleNum--;
    }

    pfBuff[0] = (float32_t *)pMem[0];
    pfBuff[1] = (float32_t *)pMem[1];
    pfBuff[2] = (float32_t *)pMem[6];
    pfBuff[3] = (float32_t *)pMem[7];
		
		
		/*
		游戏模式处理：
		如果 IfxSelect[0] == 1，调用 AlgGame71ProcessF，这是7.1声道的处理函数
		如果 IfxSelect[0] == 2，调用 AlgGameMode4ProcessF，这是4种游戏模式的处理函数
		*/
//    if (IfxSelect[0] == 1)
//    {
//        AlgGame71ProcessF(&algParaGame71, (void *)pfBuff);
//    }
//    else if (IfxSelect[0] == 2)
//    {
//        AlgGameMode4ProcessF(&algParaGameMode4, (void *)pfBuff);
//    }
    
//    // spk Eq proc
//    pfBuff[0] = (float32_t *)pMem[0];
//    pfBuff[1] = (float32_t *)pMem[1];
//    AlgFilterProcess(&SpkInFilterL[0], pfBuff[0], pfBuff[0], ALG_FRAME_LEN);
//    AlgFilterProcess(&SpkInFilterR[0], pfBuff[1], pfBuff[1], ALG_FRAME_LEN);
    
    //Niko 7段的USB-SPK EQ给ADC使用
		/*
		使用 AlgFilterProcess 对左、右声道信号进行滤波处理。
		这是均衡器（EQ）处理，用于调整音频的频率响应，增强或削减某些频率范围
		*/
    pfBuff[0] = (float32_t *)pMem[2];
//    pfBuff[1] = (float32_t *)pMem[3];
    AlgFilterProcess(&SpkInFilterL[0], pfBuff[0], pfBuff[0], ALG_FRAME_LEN);
//    AlgFilterProcess(&SpkInFilterR[0], pfBuff[1], pfBuff[1], ALG_FRAME_LEN);

//    AlgFilterProcess(&SpkInFilterL2[0], pfBuff[0], pfBuff[0], ALG_FRAME_LEN);
//    AlgFilterProcess(&SpkInFilterR2[0], pfBuff[1], pfBuff[1], ALG_FRAME_LEN);		
    
    //My Comp
//    mcu_alg_comp_process((uint8_t *)pMem[2], (uint8_t *)pMem[2], ALG_FRAME_LEN*4, NULL);
    //My Limiter
//    mcu_alg_limiter_process((uint8_t *)pMem[2], (uint8_t *)pMem[2], ALG_FRAME_LEN*4, NULL);
    
    // adcL, adcR input , ans processed data out
//    pfBuff[0] = (float32_t *)pMem[2];
//    DspAlgFrameProcess(pfBuff);
		
    //Garry    RMS
//    rmsValue[0] = AlgRmsCalcProcess(&algParaRMSCalc[0], pMem[2]);    // ADC_L
		
		/*
		频率偏移处理
		这里调用AlgFreqShiftProcess，执行频率偏移处理
		可以用来模拟特定的音频效果
		*/
//    void *pFreqBuff[3];
//    pFreqBuff[0] = (float32_t *)pMem[2];//adcL
//    pFreqBuff[1] = (float32_t *)pMem[8];//Temp
//    pFreqBuff[2] = (float32_t *)pMem[9];//Temp
//    AlgFreqShiftProcess(&algParaFreqShift[0], (void *)pFreqBuff);
		
		
    /*
		gain matrix process 增益矩阵处理
		从 g_gainNewAddMcu 数组中获取 USB 输入和直接输入的增益值
		Gain_USB_15 用于调整 USB 输入信号的增益
		Gain_Direct_12 用于调整直接输入信号的增益
		*/ 
    float Gain_USB_15 = g_gainNewAddMcu[Mcu_Gain_USB_15][0];
    float Gain_Direct_12 = g_gainNewAddMcu[Mcu_Gain_Direct_12][0];
    float Gain_Mix2_USB = g_gainNewAddMcu[MCU_ALG_GAIN_Mix2_USB][0];
		//pMem[2] 和 pMem[3] 存储 ADC 左右声道的输入数据
    // adc inL
    pfBuff[0] = (float32_t *)pMem[2];
    // adc inR
    pfBuff[1] = (float32_t *)pMem[3];
		
		//pMem[0] 和 pMem[1] 存储扬声器输出数据（也可以认为是 DAC 输出信号）
    // spk inL, also dacoutL
    pfBuff[2] = (float32_t *)pMem[0];
    // spk inR, also dacoutR
    pfBuff[3] = (float32_t *)pMem[1];
		
		//pMem[4] 和 pMem[5] 存储 USB 麦克风输入信号（左、右）
   // usbMicL
    pfBuff[4] = (float32_t *)pMem[4];
    //usbMicR
    pfBuff[5] = (float32_t *)pMem[5];
		
		volatile float fAdcL, fUsbL, fUsbR;
    sampleNum = ALG_FRAME_LEN;
    while (sampleNum)
    {
        fAdcL = *pfBuff[0];
        fUsbL = *pfBuff[2];
        fUsbR = *pfBuff[3];
        *pfBuff[4] = fAdcL  + fUsbL	*	Gain_USB_15;
        *pfBuff[5] = fAdcL  + fUsbR *	Gain_USB_15;
         //*pfBuff[2] = fAdcL * Gain_Direct_12+ fUsbL;       原
         //*pfBuff[3] = fAdcL * Gain_Direct_12+ fUsbR;
        *pfBuff[2] = fAdcL * Gain_Direct_12+ fUsbL * Gain_Mix2_USB;     //Garry
        *pfBuff[3] = fAdcL * Gain_Direct_12+ fUsbR * Gain_Mix2_USB;     //Garry
        pfBuff[0]++;
        pfBuff[1]++;
        pfBuff[2]++;
        pfBuff[3]++;
	    pfBuff[4]++;
        pfBuff[5]++;
        sampleNum--;
    }

    /* DAC 输出信号滤波
		dac L/R eq process
		AlgFilterProcess 对左声道和右声道的 DAC 输出数据进行滤波处理，
		应用预定义的滤波器 DACOutFilterL 和 DACOutFilterR
		*/ 
//    pfBuff[0] = (float32_t *)pMem[0];
//    pfBuff[1] = (float32_t *)pMem[1];
//		
//		McuAlgFilterRefresh();
//    AlgFilterProcess(&DACOutFilterL[0], pfBuff[0], pfBuff[0], ALG_FRAME_LEN);
//    AlgFilterProcess(&DACOutFilterR[0], pfBuff[1], pfBuff[1], ALG_FRAME_LEN);
		
		/*
		增益调整
		从 g_gainNewAddMcu 数组中获取 USB 和 DAC 输出信号的增益值
		McuUsbLOut 和 McuUsbROut 控制 USB 输出的增益
		McuDacLOut 和 McuDacROut 控制 DAC 输出的增益
		*/
    int32_t *i32OutDacL = (int32_t *)pMem[0];
    int32_t *i32OutDacR = (int32_t *)pMem[1];
    int32_t *i32OutMicL = (int32_t *)pMem[4];
    int32_t *i32OutMicR = (int32_t *)pMem[5];
    float *fOutDacL = (float *)i32OutDacL;
    float *fOutDacR = (float *)i32OutDacR;
    float *fOutMicL = (float *)i32OutMicL;
    float *fOutMicR = (float *)i32OutMicR;
    float McuUsbLOut = g_gainNewAddMcu[MCU_ALG_GAIN_USBL_OUT][0];
    float McuUsbROut = g_gainNewAddMcu[MCU_ALG_GAIN_USBR_OUT][0];
    float fgainOutDacLCtrl = 0;
    float fgainOutDacRCtrl = 0;
    sampleNum = ALG_FRAME_LEN;
		
		/*
		USB 和 DAC 输出信号将分别根据获取的增益值进行增益调整
		*/
    for (cntI = 0; cntI < sampleNum; cntI++)
    {
        fgainOutDacLCtrl = g_gainNewAddMcu[ MCU_ALG_GAIN_DACL_OUT][0];
        AlgGainStepChangeF(g_gainNewAddMcu[ MCU_ALG_GAIN_DACL_OUT],  0.001f);
        fgainOutDacRCtrl = g_gainNewAddMcu[ MCU_ALG_GAIN_DACR_OUT][0] ;
        AlgGainStepChangeF(g_gainNewAddMcu[ MCU_ALG_GAIN_DACR_OUT],  0.001f);
        *fOutMicL = *fOutMicL * McuUsbLOut;    //USB 麦克风左声道信号乘以其增益
        *fOutMicR = *fOutMicR * McuUsbROut;     //USB 麦克风右声道信号乘以其增益
        *fOutDacL = *fOutDacL * fgainOutDacLCtrl;     //DAC 左声道信号乘以其增益
        *fOutDacR = *fOutDacR * fgainOutDacRCtrl;     //DAC 右声道信号乘以其增益
        fOutMicL++;
        fOutMicR++;
        fOutDacL++;
        fOutDacR++;
    }
		
		/*
		
		*/
#if (DBG_TIME_CALC_EN != 0)
    while (idx)
    {
        idx--;
        if (g_dbgTimCnt[idx][0] > g_dbgTimCnt[idx][1])
        {
            g_dbgTimCnt[idx][1] = g_dbgTimCnt[idx][0];
        }
    }
#endif
    fOutDacL = (float *)i32OutDacL;
    fOutDacR = (float *)i32OutDacR;
    fOutMicL = (float *)i32OutMicL;
    fOutMicR = (float *)i32OutMicR;
   sampleNum = ALG_FRAME_LEN;
    // 输出数据做剪切并还原成32位定点数
    while (sampleNum != 0)
    {
        int32_t out1L;
        int32_t out1R;
#if (I2S_INOUT_RESOLUTION_LEVEL == 1)
#elif (I2S_INOUT_RESOLUTION_LEVEL == 2)
        out1L = (int32_t)(*fOutMicL++);
        if (out1L > FIXP_CLIP_MAX_24BIT)
        {
            out1L = FIXP_CLIP_MAX_24BIT;
        }
        else if (out1L < FIXP_CLIP_MIN_24BIT)
        {
            out1L = FIXP_CLIP_MIN_24BIT;
        }
        *i32OutMicL++ = out1L << 8;
        out1R = (int32_t)(*fOutMicR++);
        if (out1R > FIXP_CLIP_MAX_24BIT)
        {
            out1R = FIXP_CLIP_MAX_24BIT;
        }
        else if (out1R < FIXP_CLIP_MIN_24BIT)
        {
            out1R = FIXP_CLIP_MIN_24BIT;
        }
        *i32OutMicR++ = out1R << 8;
        out1L = (int32_t)(*fOutDacL++);
        if (out1L > FIXP_CLIP_MAX_24BIT)
        {
            out1L = FIXP_CLIP_MAX_24BIT;
        }
        else if (out1L < FIXP_CLIP_MIN_24BIT)
        {
            out1L = FIXP_CLIP_MIN_24BIT;
        }
        *i32OutDacL++ = out1L << 8;
        out1R = (int32_t)(*fOutDacR++);
        if (out1R > FIXP_CLIP_MAX_24BIT)
        {
            out1R = FIXP_CLIP_MAX_24BIT;
        }
        else if (out1R < FIXP_CLIP_MIN_24BIT)
        {
            out1R = FIXP_CLIP_MIN_24BIT;
        }
        *i32OutDacR++ = out1R << 8;
#endif
        sampleNum--;
    }   
}
//#pragma pop


// 低于帧中断、高于主循环的调用,使用USER1_IRQHandler
__RAM_CODE void DrvAlgProcess(void)
{
    DspAlgWAnsMainProcess();
    return;
}
  
  

uint8_t testMbdParaSetFlag = 0;
void AlgParaSetTest(void)
{
    if (testMbdParaSetFlag == 1)
    {
        DspAlgMBDParaSet(&algParaMbd);
        testMbdParaSetFlag = 0;
        AlgFreqShiftParaSet(&algParaFreqShift[0]);
    }
    else if (testMbdParaSetFlag == 2)
    {
        McuAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultSpkEq[0][0], 7, 0);
//        DspAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultAdcEq[0][0], 5, 0);
//	    DspAlgGateUnitParaSet((ALG_DSP_PARA_GATE*)&noiseGateParaGroup0[0][0],(float32_t *)noiseGateParaGroup0[0][6]);
        testMbdParaSetFlag = 0;
//        AlgFreqShiftParaSet(&algParaFreqShift[0]);
    }
}

//解决压缩器拉动滑条后出现的电流声

#if(0)
extern uint32 CompTest;

void AlgParaSetTest2(void)
{
    if(CompTest && (CompTest!=0xFFFFFFFF))
    {
        CompTest --;
        if(CompTest == 0)
        {
            compParaGroup0[2]=ParamTbl.Preset->Comp_Attack;
            ALG_DSP_PARA_DRC *pParaDrc = (ALG_DSP_PARA_DRC*)compParaGroup0;
            DspAlgDrcUnitParaSet( pParaDrc , (ALG_DSP_VAR_DRC*)DSP_IRAM_ADDR(DSP_COMP_START_ADDR) );  
            CompTest =0xFFFFFFFF;
        }
    }
}



extern uint32 LimitTest;

void AlgParaSetTest3(void)
{
    if(LimitTest && (LimitTest!=0xFFFFFFFF))
    {
        LimitTest --;
        if(LimitTest == 0)
        {
            algParaMbd.algParaDrc[0].attackTime  = ParamTbl.Preset->Limit_Attack;
            algParaMbd.algParaDrc[1].attackTime  = ParamTbl.Preset->Limit_Attack;
            algParaMbd.algParaDrc[2].attackTime  = ParamTbl.Preset->Limit_Attack;
            DspAlgMBDParaSet(&algParaMbd); 
            LimitTest =0xFFFFFFFF;
        }
    }
}
#endif


