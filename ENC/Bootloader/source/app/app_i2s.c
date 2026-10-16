

#include "app_i2s.h"
#include "app_dma.h"
#include "app_codec.h"
#include "drv_uac10.h"
#include "lib_drv_uac10.h"
#include "a0_hal_i2s.h"
#include "a0_hal_otg.h"
#include "a0_hal_gpio.h"
#include "a0_hal_timer.h"
#include "a0_hal_sys_ctrl.h"
#include "drv_alg.h"
#include "app_i2s_resample.h"


uint32_t DrvAlgAnsTmCalcChk(void);

// debug cycle count
#define USER0_IRQ_CLK_RECORD_EN         (1)



/* Private define ------------------------------------------------------------*/
#define I2S_CLK_CHG_EN                  (0) //(1) //(0) must set to '0'
#define I2S_STREAMING_BYPASS_EN         (0) //(1)//(0)                 // 1, 配置成直接ADC2DAC, 0, 配置成带数据流的处理
#define I2S_TEMP_GROUP_NUM              (6)                 // 临时缓存组数，用于DMA帧中断中
#define I2S_CH_NUM                      (DMA_CH_I2S_USE)    // I2S需要让DMA搬运的数据流单声道通道数

/* Private type --------------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#if USER0_IRQ_CLK_RECORD_EN
uint32_t g_user0CycleCnt[5];      // 0, last clk count value, 1, dma cycle, 2, dma cycle max, 3, dma irq cycle use, 4, dma irq cycle use max
#endif
// audioFifo[0~3] , dacL/R out , D3L/R out
// audioFifo[4~7], adcL/R in, D4L/R in
__RAM1 int32_t audioFifo[I2S_CH_NUM][I2S_BUFF_MAX]; // N路单声道数据，每路数据3级缓存，用于DMA访问
__RAM1 int32_t audioFifoTemp[I2S_TEMP_GROUP_NUM][I2S_PACK_NUM];     // 临时缓存一帧浮点数数据


static int32_t audioFifoIdx = 0;                            // 数据缓存的idx，0~2
static uint32_t flagUser0IrqHandler;                        // 进入USER0_IRQHandler标志

static uint32_t i2sPackCur = I2S_PACK_NUM;                  // 用于I2S时钟更改（采样率改变192K/96K），调整DMA长度，让DMA帧中断基本维持在1ms左右

uint32_t user0IrqSpdRunOutTime = 0;
extern  int32_t audioFifoTempS1[I2S_SALVE_TEMP_GROUP_NUM][I2S_S1_PACK_NUM];


#if I2S_CLK_CHG_EN
static uint32_t i2sPackCurSet = I2S_PACK_NUM;
#endif

/* Private constants ---------------------------------------------------------*/
//I2S寄存器配置 48K
static const I2S_InitTypeDef i2sCfg1 =
{
    .cr = {
        .b.clkEn = 1,                               /* enable mclk output */
        .b.mclkDiv = I2S_MCLK_DIV4,                 /* config mclk div4 from i2s clk */
        .b.isMaster = 1,                            /* config as i2s master */
        .b.mode = I2S_MODE_I2S,                     /* config mode */
        .b.mclkOutDisable = 0,
        .b.lrclkDiv = I2S_FS_48K,                   /* config lrclk as 48k or 44.1k */
        .b.frameNum = 7,                            /* 8 sample */
#if (I2S_IFX_CLOCK == 0)
        .b.clkEnS1 = 1,
        .b.intEnS1 = 0,
        .b.dmaEnS1 = 1,
#else
        .b.clkEnS1 = 0,
        .b.intEnS1 = 0,
        .b.dmaEnS1 = 0,
#endif

        .b.clkEnS2 = 0,
        .b.intEnS2 = 0,
        .b.dmaEnS2 = 0,
        .b.d6OutDacEn = 0,
        .b.d6FifoSel = 0,                           /* for dac */
        .b.d5FifoSel = 0,                           /* for adc */
        .b.d4FifoSel = 1,                           /* for i2s d4 */
    },
    .ccr0 = {
        .b.fifoReset = 0x7F,
        .b.dOutEn = I2S_D3_OUT_EN,
    },
    .ccr1 = {
        .b.d0Clk = 3,
        .b.d1Clk = 3,
        .b.d2Clk = 3,
#if (I2S_IFX_CLOCK == 0)
        .b.d3Clk = 1,
        .b.d4Clk = 1,
#else
        .b.d3Clk = 0,
        .b.d4Clk = 0,
#endif
        .b.d5Clk = 0,
        .b.d6Clk = 0,
        .b.bclkSelS2 = 0,
        .b.lrclkSelS2 = 0,
    },
};

#if (I2S_CLK_CHG_EN != 0)   //不进行时钟切换
static const uint32_t i2sSampleRateCfgTb[][5] =
{
    //samplerate, i2sCfg, codecCfg, clkMode(44.1/48)
    {44100,     I2S_FS_48K,     CODEC_256FS,    0, 48},
    {48000,     I2S_FS_48K,     CODEC_256FS,    1, 48},
    {88200,     I2S_FS_96K,     CODEC_128FS,    0, 96},
    {96000,     I2S_FS_96K,     CODEC_128FS,    1, 96},
    {176400,    I2S_FS_192K,    0xFFFFFFFF,     0, 192},
    {192000,    I2S_FS_192K,    0xFFFFFFFF,     1, 192},
    {0,         0,              0,              0, 0},
//    {11025,     I2S_FS_12K,     CODEC_1024FS,   0},
//    {16000,     I2S_FS_16K,     CODEC_768FS,    1},
//    {22050,     I2S_FS_24K,     CODEC_512FS,    0},
//    {32000,     I2S_FS_32K,     CODEC_384FS,    1},
//    {12000,     I2S_FS_12K,     CODEC_1024FS,   1},
//    {24000,     I2S_FS_24K,     CODEC_512FS,    1},
//    {352800,    I2S_FS_384K,    0xFFFFFFFF,     0},
//    {384000,    I2S_FS_384K,    0xFFFFFFFF,     1},
};
#endif

// test sin table


const int32_t i32FixSinTable[48] =
{
    0,
    70254592,
    139329536,
    205914112,
    269090816,
    327614464,
    380567552,
    426967040,
    466026496,
    497156096,
    519831552,
    533528576,
    538116096,
    533528576,
    519831552,
    497156096,
    466026496,
    426967040,
    380567552,
    327614464,
    269090816,
    205979648,
    139264000,
    70254592,
    0,
    -70254592,
    -139329536,
    -205979648,
    -269090816,
    -327614464,
    -380567552,
    -426967040,
    -466026496,
    -497156096,
    -519831552,
    -533528576,
    -538116096,
    -533528576,
    -519766016,
    -497221632,
    -466026496,
    -426967040,
    -380502016,
    -327614464,
    -269090816,
    -205914112,
    -139329536,
    -70254592,
};





/* Exported functions --------------------------------------------------------*/
//return 0, ok
//return -1, sample rate is not supported
//return -2, sample rate for codec is not supported
int32_t AppI2sClockModify(uint32_t samplerate, uint32_t codecEn)
{
#if (I2S_CLK_CHG_EN != 0)   //不进行时钟切换
    uint8_t idx = 0;
    while (1)
    {
        if (i2sSampleRateCfgTb[idx][0] == samplerate)
        {
            break;
        }
        else if (i2sSampleRateCfgTb[idx][0] == 0)
        {
            return -1;
        }
        else
        {
            idx++;
        }
    }
    if (codecEn != 0)
    {
        if (i2sSampleRateCfgTb[idx][2] != 0xFFFFFFFF)
        {
            //同时配置Codec
            SYS_CODEC_TypeDef       codecCtrl;
            codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
            codecCtrl.b.digCorePd = 0;
            codecCtrl.b.mclkMode = i2sSampleRateCfgTb[idx][2];
            HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
        }
        else
        {
            //PowerDownCodec
            SYS_CODEC_TypeDef       codecCtrl;
            codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
            codecCtrl.b.digCorePd = 1;
            HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
        }
    }
    //配置I2S
    I2S_CR_TypeDef  cr;
    cr.w = HAL_I2S_CTRL_GET();
    cr.b.lrclkDiv = i2sSampleRateCfgTb[idx][1];
    HAL_I2S_CTRL_SET(cr.w);
    HalI2sClkSelect(0, 1, i2sSampleRateCfgTb[idx][3]);
    i2sPackCurSet = i2sSampleRateCfgTb[idx][4];
#endif
    return 0;
}

/*
 * only used to set packet number(i2sPackCurSet)
 */
void AppI2sPackCurSet(uint32_t packNum)
{
    i2sPackCur = packNum;
}

uint32_t AppI2sPackCurGet(void)
{
    return i2sPackCur;
}


void AppI2sInit(void)
{
    /* 配置Codec */
    AppCodecInit();
    /* 配置使用DMIC */
#if 0   //DMIC不使用
    AppCodecDmicInit();
#endif
    /* 配置I2S，IO复用DATA线 */
#if I2S_GPIO_MCLK_USE
    HAL_GPIO_AF_SET(GPIO0, 27, GPIO_AF6_I2S_MCLK);
#endif
#if I2S_GPIO_CLK_USE
    HAL_GPIO_AF_SET(GPIO0, 18, GPIO_AF5_I2S_BCLK);
    HAL_GPIO_AF_SET(GPIO0, 26, GPIO_AF6_I2S_LRCLK);
#endif
#if I2S_GPIO_DATA_USE
    HAL_GPIO_AF_SET(GPIO0, 24, GPIO_AF5_I2S_DATA3);
    HAL_GPIO_AF_SET(GPIO0, 25, GPIO_AF5_I2S_DATA4);
#endif
    /* 配置I2S时钟 */
    HalI2sClkSelect(0, 1, 1);            //PLL1->PLL2->I2S, for 48K
    /* 配置I2S */
    HalI2sInit((I2S_InitTypeDef *)&i2sCfg1);
    return;
}


uint32_t flagUser1IrqHandler = 0;
uint32_t user1IrqEnterTimes = 0;
__RAM_CODE void USER1_IRQHandler(void)
{
    flagUser1IrqHandler = 1;
// for ANS main proc
    DrvAlgProcess();
    user1IrqEnterTimes++;
    flagUser1IrqHandler = 0;
}


__RAM_CODE void AlgAnsCheck(void)
{
    if (DrvAlgAnsTmCalcChk() == 1)
    {
        if (flagUser1IrqHandler == 0)
        {
            NVIC_SetPendingIRQ(USER1_IRQn);
        }
    }
    // 如果上次ANS处理未完成，那么此次不在触发User1IRQ
}



__RAM_CODE void USER0_IRQHandler(void)
{
#if USER0_IRQ_CLK_RECORD_EN
    HalSysCycleRecordStart(g_user0CycleCnt);
#endif
    /* <0> 进入USER0_IRQHandler做个标记，用于测试是否会出现未处理完再次进入DMA中断的情况，出现即说明速度不够 */
    flagUser0IrqHandler = 1;
    /* <1> 更新I2S的数据流缓存的IDX指针 */
    audioFifoIdx++;
    if (audioFifoIdx == I2S_PACK)
    {
        audioFifoIdx = 0;
    }
    uint32_t idx = (audioFifoIdx + I2S_PACK - 2);
    if (idx >= I2S_PACK)
    {
        idx -= I2S_PACK;
    }
    idx *= i2sPackCur;
    int32_t *pAudioSpkCh[2];
    int32_t *pAudioCh[8] ;
    pAudioSpkCh[0] = (int32_t *)&audioFifoTemp[0][0];
    pAudioSpkCh[1] = (int32_t *)&audioFifoTemp[1][0];
    DrvUac10SpkVolHandler((int32_t **)&pAudioSpkCh[0], i2sPackCur);
    /* <2> 把往DAC输出的数据流从USB-SPK通道搬过来 */
    LibDrvMemcpy(&audioFifo[0][idx], &audioFifoTemp[0][0], i2sPackCur * 4);
    LibDrvMemcpy(&audioFifo[1][idx], &audioFifoTemp[1][0], i2sPackCur * 4);
    /* <5> 进行USB的帧处理，包括检测SPK断流、音量渐变等 */
    DrvUac10FrameProcess();
//    /* <6> 进行USB各数据流的音量处理，中间可根据实际需求，增加算法处理的函数调用 */
    pAudioCh[0] = (int32_t *)&audioFifo[0][idx];        // dac-l, usb speakerL in
    pAudioCh[1] = (int32_t *)&audioFifo[1][idx];        // dac-r, usb speakerR in
    pAudioCh[2] = (int32_t *)&audioFifo[2][idx];        // adc
    pAudioCh[3] = (int32_t *)&audioFifo[3][idx];        // adc
    pAudioCh[4] = (int32_t *)&audioFifoTemp[2][0];     //Mic
    pAudioCh[5] = (int32_t *)&audioFifoTemp[3][0];
    pAudioCh[6] = (int32_t *)&audioFifoTemp[4][0];      //temp
    pAudioCh[7] = (int32_t *)&audioFifoTemp[5][0];          //temp
    DrvAlgFrameProcess((void **)pAudioCh);
    DrvUac10MicVolHandler((int32_t **)&pAudioCh[4], i2sPackCur);
//  DrvUac10SidetoneVolHandler((int32_t **)&pAudioCh[2], (int32_t **)&pAudioCh[0], i2sPackCur);
    AlgAnsCheck();
    /* <6> 退出USER0_IRQHandler清除标记 */
    flagUser0IrqHandler = 0;
#if USER0_IRQ_CLK_RECORD_EN
    HalSysCycleRecordEnd(g_user0CycleCnt);
#endif
    return;
}

/* 用来给DMA模块获取I2S缓存的首地址，让DMA重新配置下一帧的memory地址，以及一帧的样本数量 */
__RAM_CODE void AppI2sFifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum)
{
#if (I2S_CLK_CHG_EN != 0)
    if (i2sPackCurSet != i2sPackCur)
    {
        i2sPackCur = i2sPackCurSet;
    }
#endif
    *pMemAddr = (uint32_t)&audioFifo[0][audioFifoIdx * i2sPackCur];
    *pI2sPackNum = i2sPackCur;
    return;
}

extern uint8_t dbg_i2s_dma_first_enter_frame_disable;
uint32_t dbg_dma_int_enter_cnt;
/* DMA帧中断调用，进行一帧的I2S数据流处理 */
__RAM_CODE void AppI2sFrameProcess(void)
{
    if (dbg_i2s_dma_first_enter_frame_disable == 1)
    {
        //dbg_i2s_dma_first_enter_frame_disable = 0;
        return;
    }
    dbg_dma_int_enter_cnt++;
    if (flagUser0IrqHandler == 1)
    {
        /*
            进到这里说明速度不够了，导致USER0_IRQHandler还没处理完，再一次进到DMA中断
            如果是单步进行仿真的话，那么也是会进到这里
        */
        //__breakpoint(0);
        user0IrqSpdRunOutTime++;
    }
    int32_t *pUsbCh[2];
    pUsbCh[0] = (int32_t *)&audioFifoTemp[2][0];
    pUsbCh[1] = (int32_t *)&audioFifoTemp[3][0];
    DrvUac10MicDataWrite(pUsbCh, 2, i2sPackCur);    // 往USB-MIC写入上帧数据
    pUsbCh[0] = (int32_t *)&audioFifoTemp[0][0];
    pUsbCh[1] = (int32_t *)&audioFifoTemp[1][0];
    DrvUac10SpkDataRead(pUsbCh, 2, i2sPackCur);     // 从USB-SPK读取本帧数据
    if (flagUser0IrqHandler == 0)
    {
        // 如果由于debug端点导致上次处理未完成，那么此次不在触发UserIRQ
        NVIC_SetPendingIRQ(USER0_IRQn);
    }
    return;
}


__RAM_CODE void AppI2sAudioFifoClr(void)
{
    //LibDrvMemset(void * pDst, uint32_t c, uint32_t byteSize)
    LibDrvMemset((void *)audioFifo, 0x00, sizeof(audioFifo));
    LibDrvMemset((void *)audioFifoTemp, 0x00, sizeof(audioFifoTemp));
}

__RAM_CODE void AppI2sAudioFifoHalfClr(void)
{
    LibDrvMemset((void *)audioFifo, 0x00, sizeof(audioFifo) / 2);
}




/*****************************************************************************/
