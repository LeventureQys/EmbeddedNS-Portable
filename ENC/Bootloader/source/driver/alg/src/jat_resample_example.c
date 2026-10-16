
#include "jat_resample.h"

int32_t resampleVarUsbSpk[RESAMPLE_MEM_SIZE(2)];
int32_t resampleVarUsbMic[RESAMPLE_MEM_SIZE(2)];
resample_init_t resampleVarParaRd = {
    .pParaMem = resampleVarUsbSpk,
    .memSize = (sizeof(resampleVarUsbSpk)/4),
    .chNum = UAC_SPK_CH_MODE,
    .frameSize = 48,
    .sampleRateRef = 48000,
};
resample_init_t resampleVarParaWr = {
    .pParaMem = resampleVarUsbMic,
    .memSize = (sizeof(resampleVarUsbMic)/4),
    .chNum = UAC_MIC_CH_MODE,
    .frameSize = 48,
    .sampleRateRef = 48000,
};

// DMA帧中断中，从USB缓存读取数据，并从目标采样率重采样到48K的参考函数
void AppUsbDevSpkDataReadResample(int32_t **pDstCh, uint8_t ch, uint32_t sampleNumber)
{
    uint32_t spkState = (LibDrvUac10StateGet() & BIT0);
    int32_t *pDstCur;
    if ((spkState == 0)||(appUsbPara.spkFirstPackEn == 1))
    {
        for (uint32_t i = 0; i < ch; i++)
        {
            pDstCur = pDstCh[i];
            memset(pDstCur, 0x00, sampleNumber*4);
        }
        ResampleFirstPacketSet(resampleVarUsbSpk);
    }
    else
    {
        if (appUsbPara.spkWrIdx == appUsbPara.spkRdIdx)
        {
            for (uint32_t i = 0; i < ch; i++)
            {
                pDstCur = pDstCh[i];
                memset(pDstCur, 0x00, sampleNumber*4);
            }
        }
        else
        {
            if (ResampleFirstPacketGet(resampleVarUsbSpk) != 0)
            {
                uint32_t newRdIdx = appUsbPara.spkWrIdx + (appUsbPara.spkBuffSize >> 1);
                appUsbPara.spkRdIdx = (newRdIdx >= appUsbPara.spkBuffSize) ? (newRdIdx - appUsbPara.spkBuffSize) : newRdIdx;
            }
            appUsbPara.spkRdIdx = ResampleHandlerRd(resampleVarUsbSpk, pDstCh, appUsbPara.spkWrIdx, appUsbPara.spkRdIdx, appUsbPara.spkBuffSize);
        }
    }   
}

// DMA帧中断中，往USB缓存写入数据，并从48K重采样到目标采样率的参考函数
void AppUsbDevMicDataWriteResample(int32_t **pSrcCh, uint8_t ch, uint32_t sampleNumber)
{
    uint32_t micState = (LibDrvUac10StateGet() & BIT1);
    if (micState != 0)
    {
        if (appUsbPara.micWrIdx != appUsbPara.micRdIdx)
        {
            if (ResampleFirstPacketGet(resampleVarUsbMic) != 0)
            {
                uint32_t newWrIdx = appUsbPara.micRdIdx + (appUsbPara.micBuffSize >> 1);
                appUsbPara.micWrIdx = (newWrIdx >= appUsbPara.micBuffSize) ? (newWrIdx - appUsbPara.micBuffSize) : newWrIdx;
            }
            appUsbPara.micWrIdx = ResampleHandlerWr(resampleVarUsbMic, pSrcCh, appUsbPara.micWrIdx, appUsbPara.micRdIdx, appUsbPara.micBuffSize);
        }
    }
    else
    {
        ResampleFirstPacketSet(resampleVarUsbMic);
    }
}

// USB采样率改变时，配置重采样函数的读取源采样率/写入目标采样率的参考函数
void UsbSampleRateCheck(uint8_t idx)
{
    uint32_t sampleRate;
    if (LibDrvUac10SampleRateCheck(idx, &sampleRate) != 0)
    {
        appUsbPara.audioSampleRate[idx] = sampleRate;
        if (idx == USB_PARA_IDX_SPK)
        {
            //SPK 采样率改变
            /*
                其他处理
            */
            ResampleFSCfg(resampleVarUsbSpk, RESAMPLE_IDX_RD, sampleRate);         // 配置采样率
        }
        else
        {
            //MIC 采样率改变
            /*
                其他处理
            */
            ResampleFSCfg(resampleVarUsbMic, RESAMPLE_IDX_WR, sampleRate);         // 配置采样率
        }
    }
    return;
}

// USB数据流需要匹配到codec的采样率，进行初始化配置的参考函数
void UsbSrcInit(void)
{
	/* <0> 初始化重采样部分 */
    int32_t *pSampleRdSrc[2];
    DrvUac10SpkBuffPtGet(pSampleRdSrc);
    int32_t *pSampleWrDst[2];
    DrvUac10MicBuffPtGet(pSampleWrDst);
    resampleVarParaRd.pRWSample = pSampleRdSrc;
    resampleVarParaWr.pRWSample = pSampleWrDst;
    if (ResampleInit(&resampleVarParaRd) != 0)
    {
        __breakpoint(0);    // memory size error
    }
    if (ResampleInit(&resampleVarParaWr) != 0)
    {
        __breakpoint(0);    // memory size error
    }

    
}