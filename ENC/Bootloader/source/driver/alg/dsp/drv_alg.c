
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


#define ALG_LIB_VER_STR_SIZE                (0x12)
// 1, 16bit
// 2, 24bit
#define I2S_INOUT_RESOLUTION_LEVEL          (2)//(1)

#define FIXP_CLIP_MAX_16BIT                 (32767)
#define FIXP_CLIP_MIN_16BIT                 (-32767)

#define FIXP_CLIP_MAX_24BIT                 (8388607)
#define FIXP_CLIP_MIN_24BIT                 (-8388607)

void AlgGainStepChangeF(float32_t *gain, float32_t step);

//int32_t IfxSelect[1]={1}; //0 bypass,1 game71,2 game4
int32_t IfxSelect[1] = {0};


__RAM02 int32_t dspAlgAnsVar[DSP_ALG_ANS_VAR_SIZE];
__RAM01 int32_t dspAnsBuf[DSP_ALG_ANS_CALCBUFF_SIZE];
__RAM01 int32_t ansRingBuf[ANS_RINGBUF_LEN];



ALG_PARA_GAMEMD4 algParaGameMode4 ;
__RAM1 int32_t algVarGameMode4[ALG_GAMEMD4_VAR_SIZE];
__RAM01 int32_t algDlyLineGameMode4[ALG_GAMEMD4_DELAYLINE_SIZE];


ALG_PARA_GAME71 algParaGame71;
__RAM1 int32_t algVarGame71[ALG_GAME71_VAR_SIZE];
__RAM01 int32_t algDlyLineGame71[ALG_GAME71_DELAY_SIZE];



ALG_DSP_PARA_MBD algParaMbd =
{
    .fc1 = 1000,
    .fc2 = 3000,
    // freq < fc1
    .algParaDrc[0] =
    {
        .preGain = 128,
        .threshold = -86,
        .attackTime = 10,
        .releaseTime = 20,
        .ratio = 1000,
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
        .threshold = -86, //0,
        .attackTime = 10,
        .releaseTime = 20,
        .ratio = 1000,
    },
    .dbgG1 = 1.0f,
    .dbgG2 = 1.0f,
    .dbgG3 = 1.0f,
};
__RAM1  int32_t algVarMbd[ALG_VAR_SIZE_MBD];

__RAM1 int32_t algVarGate[10];


DSP_ALG_PARA_ANS dspAlgParaAns;

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

    {0.0f, 1.0f}, // gainInSpkL             2
    {0.0f, 1.0f}, // gainInSpkR             3

    {0.0f, 1.0f}, // Gain_USB_15            4
    {0.0f, 1.0f}, // Gian_Direct_12   		5

    {0.0f, 1.0f}, // Gain_Mic_L       6
    {0.0f, 1.0f}, // Gain_Mic_R       7

    {0.0f, 1.0f}, // Gain_DacLout      8
    {0.0f, 1.0f}, // Gain_DacRout      9
};


float_t GainMix11VolCur;
int32_t dbgVarSize;
uint8_t AnsOnOffDefault = 1; //1;  //ans 默认打开


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
int32_t noiseGateParaGroup0[1][7] =
{
    // TODO : add complete para
    {
        128,            // preGain
        128,            // preGainEnergy
        -75,    //-90,          // threshold
        40,         // attackTime
        60,         // releaseTime
        0,          // level
        0,          // varAddr, NOT used

    }
};



int32_t *pNoiseGateGroupPara[DSP_GATE_GROUP_NUM] =
{
    (int32_t *) &noiseGateParaGroup0,
};


ALG_DSP_NGATE_PARA noiseGatePara =
{
    .pGroupPara = (int32_t **) &pNoiseGateGroupPara,

};


//2                 gain para

float gainDspPara[DSP_GAIN_NUM] =
{
    1.0f,   // Gain_ADC_1_04
    1.0f,   //1.0f, // Gain_Direct_10
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






void  AppAlgMcuFilterInit(void)
{
    for (int i = 0; i < SPK_FILTER_NUM; i++)
    {
        // 调用AlgFilterDesign函数计算滤波器系数，并存储在spkFilterCoeff数组中
        AlgFilterDesign((ALG_PARA_FILTER *)&paraDefaultSpkEq[0][i], &spkFilterCoeff[5 * i], ALG_SAMPLE_RATE);
    }
    for (int i = 0; i < DACOUT_FILTER_NUM; i++)
    {
        // 调用AlgFilterDesign函数计算滤波器系数，并存储在dacFilterCoeff数组中
        AlgFilterDesign((ALG_PARA_FILTER *)&paraDefaultDacEq[0][i], &dacFilterCoeff[5 * i], ALG_SAMPLE_RATE);
    }
}

uint32_t testvar;
/*
 * dspRunMode = 0, codec set @48K , dsp run ans
 * dspRunMode = 1, codec set @96K , dsp do NOT run ans
 */
void DspAlgInit(void)
{
    algDspInit();
    // update ans relavant var
    dbgVarSize = DspAlgANSVarSize();    // R0 = 0x 3F
    dbgVarSize = DspAlgANSCalcBuffSize();   // R0 = 0x 196D
    dspAlgParaAns.varAddr = (int32_t)&dspAlgAnsVar[0];
    dspAlgParaAns.bufferAddr2 = (int32_t)&dspAnsBuf[0];
    dspAlgParaAns.ansOnOff = AnsOnOffDefault;
    dspAlgParaAns.ansMode = 2;
    DspAlgANSVarInit((DSP_ALG_PARA_ANS *)&dspAlgParaAns);
    DspAlgANSCalcBufferInit((DSP_ALG_PARA_ANS *)&dspAlgParaAns);
    DspAlgANSParaUpdate((int32_t *)DSP_IRAM_ADDR(DSP_ANS_INIT_ADDR));
    DspANSAlgSinLogTbInit();
    // init fft bit reverse table in iram
    DspANSAlgFFTInit();
    DspAlgFFTRadix4BitRvsTbInit(0);
    AppAlgMcuFilterInit();
    // init eq in dsp
    if (algDspInitEq(&eqPara[0]))
    {
        __breakpoint(0);
    }
  	noiseGateParaGroup0[0][6] = (int32_t)&algVarGate[0];
		
    // init gate in dsp
    if (algDspInitGate(&noiseGatePara))
    {
        __breakpoint(0);
    }
    // init gain in dsp
    if (algDspInitGain(&gainDspPara[0]))
    {
        __breakpoint(0);
    }
    algParaMbd.varAddr = (int32_t)&algVarMbd[0];
    algDspInitMBD(&algParaMbd);
    // set at 10ms delay
//  algDspSetRunMode(1);
//  obsvDspRunMode = 1;
    // set at 1ms delay
    algDspSetRunMode(0);
    obsvDspRunMode = 0;
    algParaGameMode4.varAddr = (int32_t)&algVarGameMode4;
    algParaGameMode4.bufferAddr = (int32_t)&algDlyLineGameMode4;
    AlgGameMode4Init(&algParaGameMode4);
    algParaGame71 = paraDefaultGM71[0];
    algParaGame71.varAddr = (int32_t)&algVarGame71;
    algParaGame71.bufferAddr = (int32_t)&algDlyLineGame71;
    AlgGame71Init(&algParaGame71);
    palgGeneralTmDelayLine = (ALG_GENENRAL_TM_DELAYLINE *)&ansRingBuf[0];
    palgGeneralTmDelayLine->dspBuffIdx = 0;
    palgGeneralTmDelayLine->mcuBuffIdx = 0;
//    LibDrvMemset((void*)AllDelayLine, 0x00, sizeof(AllDelayLine));
    LibDrvMemset((void *)algDlyLineGameMode4, 0x00, sizeof(algDlyLineGameMode4));
    LibDrvMemset((void *)algDlyLineGame71, 0x00, sizeof(algDlyLineGame71));
    bDspRunEn = 1;
}

void DrvAlgInit(void)
{
    DspAlgInit();
    NVIC_SetPriority(USER1_IRQn, 2);        // 开启用户软件IRQ中断1
    NVIC_EnableIRQ(USER1_IRQn);
}


// pMem, 指向指针数组，具体定义根据实际的输入输出数据流调整
// 数据流通路上的算法处理，帧中断（DMA帧中断等中断中处理的）
/*


 */


//volatile float fAdcL,fAdcR,fI2sL,fI2sR,fUsbL,fUsbR;


__RAM_CODE void DrvAlgFrameProcess(void **pMem)
{
    int32_t cntI = 0;
    // 输入信号是32位定点数，缩小到16位并转换成浮点数
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
    for (cntI = 0; cntI <= ALG_GAIN_NUM ; cntI++)
    {
        AlgGainStepChangeF(g_gainNewAddMcu[cntI], 1.0f / 128.0f);
    }
    uint32_t sampleNum;
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
    //volatile float32_t *pfBuff[12];
    // adcL, adcR
    pfBuff[0] = (float32_t *)pMem[2];
    pfBuff[1] = (float32_t *)pMem[3];
    // spkL, spkR
    pfBuff[2] = (float32_t *)pMem[0];
    pfBuff[3] = (float32_t *)pMem[1];
    float McuAdcLin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_LIN][0];
    float McuAdcRin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_ADC_RIN][0];
    float McuSpkLin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_SPK_LIN][0];
    float McuSpkRin = g_gainNewAddMcu[MCU_ALG_GAIN_IDX_SPK_RIN][0];
    // mix adcL with adcR to adcL and adcR
    // usb speaker left/right data mix gain process
    sampleNum = ALG_FRAME_LEN;
    while (sampleNum)
    {
        *pfBuff[0] = *pfBuff[0] * McuAdcLin + *pfBuff[1] * McuAdcRin;
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
    if (IfxSelect[0] == 1)
    {
        AlgGame71ProcessF(&algParaGame71, (void *)pfBuff);
    }
    else if (IfxSelect[0] == 2)
    {
        AlgGameMode4ProcessF(&algParaGameMode4, (void *)pfBuff);
    }
    // spk Eq proc
//    pfBuff[0] = (float32_t *)pMem[0];
//    pfBuff[1] = (float32_t *)pMem[1];
//    AlgFilterProcess(&SpkInFilterL[0], pfBuff[0], pfBuff[0], ALG_FRAME_LEN);
//    AlgFilterProcess(&SpkInFilterR[0], pfBuff[1], pfBuff[1], ALG_FRAME_LEN);
    
    //Niko 7段的USB-SPK EQ给ADC使用
    pfBuff[0] = (float32_t *)pMem[2];
    pfBuff[1] = (float32_t *)pMem[3];
    AlgFilterProcess(&SpkInFilterL[0], pfBuff[0], pfBuff[0], ALG_FRAME_LEN);
    AlgFilterProcess(&SpkInFilterR[0], pfBuff[1], pfBuff[1], ALG_FRAME_LEN);
    
    // adcL, adcR input , ans processed data out
    pfBuff[0] = (float32_t *)pMem[2];
    DspAlgFrameProcess(pfBuff);
    // gain matrix process
    float Gain_USB_15 = g_gainNewAddMcu[Mcu_Gain_USB_15][0];
    float Gain_Direct_12 = g_gainNewAddMcu[Mcu_Gain_Direct_12][0];
    // adc inL
    pfBuff[0] = (float32_t *)pMem[2];
    // adc inR
    pfBuff[1] = (float32_t *)pMem[3];
    // spk inL, also dacoutL
    pfBuff[2] = (float32_t *)pMem[0];
    // spk inR, also dacoutR
    pfBuff[3] = (float32_t *)pMem[1];
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
        *pfBuff[2] = fAdcL * Gain_Direct_12+ fUsbL ;
        *pfBuff[3] = fAdcL * Gain_Direct_12+ fUsbR ;
        pfBuff[0]++;
        pfBuff[1]++;
        pfBuff[2]++;
        pfBuff[3]++;
			  pfBuff[4]++;
        pfBuff[5]++;
        sampleNum--;
    }

    // dac L/R eq process
    pfBuff[0] = (float32_t *)pMem[0];
    pfBuff[1] = (float32_t *)pMem[1];
		
		McuAlgFilterRefresh();
    AlgFilterProcess(&DACOutFilterL[0], pfBuff[0], pfBuff[0], ALG_FRAME_LEN);
    AlgFilterProcess(&DACOutFilterR[0], pfBuff[1], pfBuff[1], ALG_FRAME_LEN);
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
    float McuDacLOut = g_gainNewAddMcu[MCU_ALG_GAIN_DACL_OUT][0];
    float McuDacROut = g_gainNewAddMcu[MCU_ALG_GAIN_DACR_OUT][0];
    sampleNum = ALG_FRAME_LEN;
    for (cntI = 0; cntI < sampleNum; cntI++)
    {
        *fOutMicL = *fOutMicL * McuUsbLOut;
        *fOutMicR = *fOutMicR * McuUsbROut;
        *fOutDacL = *fOutDacL * McuDacLOut;
        *fOutDacR = *fOutDacR * McuDacROut;
        fOutMicL++;
        fOutMicR++;
        fOutDacL++;
        fOutDacR++;
    }
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
    }
    else if (testMbdParaSetFlag == 2)
    {
        DspAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultAdcEq[0][0], 5, 0);
				DspAlgGateUnitParaSet((ALG_DSP_PARA_GATE*)&noiseGateParaGroup0[0][0],(float32_t *)noiseGateParaGroup0[0][6]);
        testMbdParaSetFlag = 0;
    }
}

