#include "jat_resample.h"
#include "app_i2s_resample.h"
#include "a0_hal_i2s.h"
#include "a0_hal_dma.h"
#include "app_i2s.h"
#include "app_dma.h"
#include "lib_drv_uac10.h"

#if (I2S_IFX_CLOCK == 0)
//slave
// audioFifoS1[0], audioFifoS1[1]   d3 out
// audioFifoS1[2], audioFifoS1[3]   d4 in


__RAM1  int32_t audioFifoS1[DMAS1_CH_I2S_USE][I2S_BUFF_S1_MAX];
__RAM1 int32_t audioFifoTempS1[I2S_SALVE_TEMP_GROUP_NUM][I2S_S1_PACK_NUM];           // 存放即将写入S1的当前帧数据和从S1读取到的当前帧数据
static uint32_t i2sS1PackCur = I2S_S1_PACK_NUM;

uint32_t    i2sSlaveRestartFlag;
/* Private types -------------------------------------------------------------*/
typedef struct {

    int32_t     i2sS1DInRdIdx;
    int32_t     i2sS1DInWrIdx;
    uint32_t    i2sBuffSize;
    uint8_t     i2sSlaveInitFlag;
    
    uint32_t    i2sFrameCnt;
    uint32_t    i2sFrameCntLast;
    uint32_t    i2sDisConnectCnt;

} drv_i2s_in_para;

typedef struct {

    int32_t     i2sS1DOutRdIdx;
    int32_t     i2sS1DOutWrIdx;
    uint32_t    i2sBuffSize;
    uint8_t     i2sSlaveInitFlag;
    


} drv_i2s_out_para;

/* Private variables ---------------------------------------------------------*/
static drv_i2s_in_para drvI2sInPara;
static drv_i2s_out_para drvI2sOutPara;

__RAM1 int32_t resampleVarI2sDIn[RESAMPLE_MEM_SIZE(I2S_S1_IN_CH)];
__RAM1 int32_t resampleVarI2sDOut[RESAMPLE_MEM_SIZE(I2S_S1_OUT_CH)];

__RAM1 resample_init_t resampleVarParaRdI2s = {

    .pParaMem = resampleVarI2sDIn,
    .memSize = (sizeof(resampleVarI2sDIn) / 4),
    .chNum = I2S_S1_IN_CH,
    .frameSize = 48,
    .sampleRateRef = 48000,
};
resample_init_t resampleVarParaWrI2s = {
    
    .pParaMem = resampleVarI2sDOut,
    .memSize = (sizeof(resampleVarI2sDOut)/4),
    .chNum = I2S_S1_OUT_CH,
    .frameSize = 48,
    .sampleRateRef = 48000,
};


void DrvI2sDInBuffPtGet(int32_t **pChBuff)
{
    pChBuff[0] = (int32_t *)&audioFifoS1[2][0];
    pChBuff[1] = (int32_t *)&audioFifoS1[3][0];

    return;
}
void DrvI2sDOutBuffPtGet(int32_t **pChBuff)
{
    pChBuff[0] = (int32_t *)&audioFifoS1[0][0];
    pChBuff[1] = (int32_t *)&audioFifoS1[1][0];
    return;
}

void I2sSlaveInInit(void)
{
    //I2s输入读写指针初始化
    drvI2sInPara.i2sS1DInRdIdx = 0;
    drvI2sInPara.i2sS1DInWrIdx = 0;
    drvI2sInPara.i2sBuffSize = I2S_BUFF_S1_MAX;
    //I2s输出读写指针初始化
    drvI2sOutPara.i2sS1DOutRdIdx = 0;
    drvI2sOutPara.i2sS1DOutWrIdx= 0;
    drvI2sOutPara.i2sBuffSize = I2S_BUFF_S1_MAX;   
    i2sSlaveRestartFlag = 1;
    
}

void DrvI2sSampleRateInit(void)
{
    /* 初始化重采样部分 */
    I2sSlaveInInit();
    int32_t *pSampleRdSrc[I2S_S1_IN_CH];  
    DrvI2sDInBuffPtGet(pSampleRdSrc);
    int32_t *pSampleWrDst[I2S_S1_OUT_CH];
    DrvI2sDOutBuffPtGet(pSampleWrDst);
    resampleVarParaRdI2s.pRWSample = pSampleRdSrc;
    resampleVarParaWrI2s.pRWSample = pSampleWrDst;
    if (ResampleInit(&resampleVarParaRdI2s) != 0)
    {
        __breakpoint(0);    // memory size error
    }
    if (ResampleInit(&resampleVarParaWrI2s) != 0)
    {
        __breakpoint(0);    // memory size error
    }
    drvI2sInPara.i2sSlaveInitFlag = 1;
    drvI2sOutPara.i2sSlaveInitFlag = 1;
}

__RAM_CODE uint32_t DrvI2sDInStreamingCheck(void)
{
    if (ResampleFirstPacketGet(resampleVarI2sDIn) == 0)
    {//不是第一包（可能未接收到任何数据包）
        if(drvI2sInPara.i2sFrameCntLast == drvI2sInPara.i2sFrameCnt)
        {   //如果上一帧计数等于当前帧计数，即连续两次帧计数相同
            // 表示未接收到新的数据帧  
            drvI2sInPara.i2sDisConnectCnt++;
            if(drvI2sInPara.i2sDisConnectCnt == 4)
            {
                // 连续4帧未收到新数据，认为I2S数据流已经停止传输传输了
                // 将第一个包使能，将i2s缓冲区清零，重置断开连接计数器
                ResampleFirstPacketSet(RESAMPLE_IDX_RD);
                LibDrvMemset(audioFifoS1, 0, sizeof(audioFifoS1));
                drvI2sInPara.i2sDisConnectCnt = 0;
                i2sSlaveRestartFlag = 0;
            }
            else
            {
                 i2sSlaveRestartFlag = 1;
            }
        }
        else
        {
            // 如果当前帧计数不等于上一帧计数，即有新的数据帧传输
            // 则更新上一帧计数，重置断开连接计数器
            drvI2sInPara.i2sFrameCntLast = drvI2sInPara.i2sFrameCnt;
            drvI2sInPara.i2sDisConnectCnt = 0;
            i2sSlaveRestartFlag = 1;
        }
    }
    return i2sSlaveRestartFlag;
}


__RAM_CODE void DrvI2sInDataRead(int32_t **pDstCh, uint8_t ch, uint32_t sampleNumber)
{
    int32_t *pDstCur;
    //检查slave初始化是否完成
    if (drvI2sInPara.i2sSlaveInitFlag == 0)
    {
        return;
    }
    //检查缓存是否工作正常
//    if((DrvI2sDInStreamingCheck()) != 0) 
    {
        if (drvI2sInPara.i2sS1DInWrIdx == drvI2sInPara.i2sS1DInRdIdx)
        {
            for (uint32_t i = 0; i < ch; i++)
            {
                pDstCur = pDstCh[i];
                LibDrvMemset(pDstCur, 0x00, sampleNumber * 4);
            }
        }
        else
        {
            if (ResampleFirstPacketGet(resampleVarI2sDIn) != 0)
            {
                uint32_t newRdIdx = drvI2sInPara.i2sS1DInWrIdx + (drvI2sInPara.i2sBuffSize >> 1);
                drvI2sInPara.i2sS1DInRdIdx = (newRdIdx >= drvI2sInPara.i2sBuffSize) ? (newRdIdx - drvI2sInPara.i2sBuffSize) : newRdIdx;
            }
            drvI2sInPara.i2sS1DInRdIdx = ResampleHandlerRd(resampleVarI2sDIn, pDstCh, drvI2sInPara.i2sS1DInWrIdx, drvI2sInPara.i2sS1DInRdIdx, drvI2sInPara.i2sBuffSize);
        }
    }
}

__RAM_CODE void DrvI2sOutDataWrite(int32_t **pSrcCh, uint8_t ch, uint32_t sampleNumber)
{
    int32_t *pDstCur;
    //检查slave初始化是否完成
    if (drvI2sOutPara.i2sSlaveInitFlag == 0)
    {
        return;
    }
    //检查缓存是否工作正常
//    DrvI2sDInStreamingCheck();   
    {
        if (drvI2sOutPara.i2sS1DOutWrIdx != drvI2sOutPara.i2sS1DOutRdIdx)
        {
            if (ResampleFirstPacketGet(resampleVarI2sDOut) != 0)
            {
                uint32_t newWrIdx = drvI2sOutPara.i2sS1DOutRdIdx + (drvI2sOutPara.i2sBuffSize >> 1);
                drvI2sOutPara.i2sS1DOutWrIdx = (newWrIdx >= drvI2sOutPara.i2sBuffSize) ? (newWrIdx - drvI2sOutPara.i2sBuffSize) : newWrIdx;
            }
            drvI2sOutPara.i2sS1DOutWrIdx = ResampleHandlerWr(resampleVarI2sDOut, pSrcCh, drvI2sOutPara.i2sS1DOutWrIdx, drvI2sOutPara.i2sS1DOutRdIdx, drvI2sOutPara.i2sBuffSize);
        }
    }
}

#define I2S_RESAMPLE_RATE_ADJ   (8)

void DrvI2sSampleRateCheck(uint8_t idx, uint32_t sampleRate)
{
    if (idx == I2S_S1_PARA_IDX_IN)
    {
        //i2s 输入 采样率改变
        ResampleFSCfg(resampleVarI2sDIn, RESAMPLE_IDX_RD, sampleRate, I2S_RESAMPLE_RATE_ADJ);         // 配置采样率
    }
    else
    {
        //i2s 输出 采样率改变
        ResampleFSCfg(resampleVarI2sDOut, RESAMPLE_IDX_WR, sampleRate, I2S_RESAMPLE_RATE_ADJ);         // 配置采样率
    }
    return;
}



/* DMA帧中断调用，进行一帧的I2S数据流处理 */
__RAM_CODE void AppI2sS1FrameProcess(void)
{
    drvI2sInPara.i2sS1DInWrIdx += 48;
    if (drvI2sInPara.i2sS1DInWrIdx == I2S_BUFF_S1_MAX)
    {
        drvI2sInPara.i2sS1DInWrIdx = 0;
    }
    drvI2sOutPara.i2sS1DOutRdIdx += 48;
    if ( drvI2sOutPara.i2sS1DOutRdIdx == I2S_BUFF_S1_MAX)
    {
         drvI2sOutPara.i2sS1DOutRdIdx= 0;
    }
//    drvI2sInPara.i2sFrameCnt++;
    return;
}


__RAM_CODE void AppI2sSlaveInFifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum)
{
#if (I2S_CLK_CHG_EN != 0)
    if (i2sPackCurSet != i2sPackCur)
    {
        i2sPackCur = i2sPackCurSet;
    }
#endif
    *pMemAddr = (uint32_t)&audioFifoS1[2][drvI2sInPara.i2sS1DInWrIdx]; //输入
    *pI2sPackNum = i2sS1PackCur;
    return;
}

__RAM_CODE void AppI2sSlaveOutFifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum)
{
#if (I2S_CLK_CHG_EN != 0)
    if (i2sPackCurSet != i2sPackCur)
    {
        i2sPackCur = i2sPackCurSet;
    }
#endif
    *pMemAddr = (uint32_t)&audioFifoS1[0][drvI2sOutPara.i2sS1DOutRdIdx]; //输出
    *pI2sPackNum = i2sS1PackCur;
    return;
}


#endif