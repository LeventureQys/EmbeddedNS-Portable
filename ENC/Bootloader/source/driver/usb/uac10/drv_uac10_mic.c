
#include "drv_uac10_user_config.h"

#if (UAC_IF_ID_STREAMING_MIC > 0)
#include "lib_drv_uac10.h"

extern int32_t DrvUac10SpkRunningCheck(void);
extern void DrvUac10SpkBuffIdxGet(int32_t *pWrIdx, int32_t *pRdIdx, uint32_t newBuffSize);

/* Private define ------------------------------------------------------------*/
#if (USB_SAMPLE_RATE_192K_EN)
#define USB_FIFO_TEMP_SIZE                      (1152/4)        // 32bit，192K时高带宽，最大192*3*2=1152 bytes
#else
#define USB_FIFO_TEMP_SIZE                      (1024/4)        // 32bit，常规FIFO最大1024
#endif

/* Private types -------------------------------------------------------------*/
typedef struct {
    //缓存区
    uint32_t    micBuff[USB_MIC_DATA_BUFF_CH][USB_MIC_DATA_BUFF_SIZE];  //mic数据缓存区，按32位存放
#if (USB_MIC_LPF_EN)
    float_t     tmpBuffLpf[USB_MIC_SAMPLE_PACKNUM];                     // 最大96/192
#endif
    uint32_t    tmpBuff[USB_FIFO_TEMP_SIZE];                            // 用于USB的FIFO访问进行数据过渡

    int32_t     micWrIdx;                   //0 ~ USB_MIC_DATA_BUFF_SIZE, mic循环缓存的写指针索引
    int32_t     micRdIdx;                   //0 ~ USB_MIC_DATA_BUFF_SIZE, mic循环缓存的读指针索引
    uint32_t    micBuffSize;                // <= USB_MIC_DATA_BUFF_SIZE

    //状态信息
    uint32_t    micSampleRate;              // mic当前采样率

    float_t     micPacketCntFrac;           //用于计数每包上传样本数，保留小数位
    float_t     micPacketSampleStep;        //用于记录当前MIC的Interface下AltSetting对应的每包数据步长值

    uint8_t     micFirstPackEn;
    uint8_t     micFsSupportFlag;           //0, do not support, transfer all zeros pcm data, 1, normal data transfer

} drv_uac10_para_mic_t;
/* Private and Public function declarations ---------------------------------------------*/
static void UsbClkStateReset(void);
static void UsbClkAdjust(uint32_t sampleRemained, uint32_t buffSize);

static void UsbMicClkReset(void);
static void UsbMicClkAdjust(uint32_t sampleRemained);
static void UsbMic16Bit1Ch(void);
static void UsbMic16Bit2Ch(void);
static void UsbMic24Bit1Ch(void);
static void UsbMic24Bit2Ch(void);
static void UsbMicDataWrite4X(int32_t **pSrcCh, uint32_t sampleNumber);
static void UsbMicDataWrite2X(int32_t **pSrcCh, uint32_t sampleNumber);
static void UsbMicDataWrite1X(int32_t **pSrcCh, uint32_t sampleNumber);
static void UsbMicDataWriteHalf(int32_t **pSrcCh, uint32_t sampleNumber);

/* callback函数占用时间需要小于I2S时钟下的8个样本的时间（48K采样率时，1/6ms = 166us），否则最坏情况下，dma和usb同时中断，usb进了中断会导致dma断流 */
#if (UAC_IF_ALT_SET_1_MIC_CH == 1)
#define UsbMicAltSet1Cb                 UsbMic16Bit1Ch
#define UsbMicAltSet2Cb                 UsbMic24Bit1Ch
#elif (UAC_IF_ALT_SET_1_MIC_CH == 2)
#define UsbMicAltSet1Cb                 UsbMic16Bit2Ch
#define UsbMicAltSet2Cb                 UsbMic24Bit2Ch
#endif

void DrvUac10MicSampleRateSet(uint32_t sampleRate);

/* Public variables ----------------------------------------------------------*/
#if USB_MIC_CB_CYCLE_CNT_EN
uint32_t g_usbMicCycleCnt[5];
#endif
/* Private variables ---------------------------------------------------------*/
static drv_uac10_para_mic_t drvUac10ParaMic;

/* Private constants ---------------------------------------------------------*/
#if (USB_SAMPLE_RATE_192K_EN)
static const uint16_t usbMicAltSettingMaxPacketSize[UAC_IF_ALT_SETTING_MIC_NUM + 1] =
{
    0,
    (UAC_FS_PACKET_TRANS_SIZE(UAC_IF_ALT_SET_1_MIC_PACK_SIZE)),             //最大192*2*2=768
    (UAC_FS_PACKET_TRANS_SIZE(UAC_IF_ALT_SET_2_MIC_PACK_SIZE)),             //最大192*3*2=1152/2=576,2次传输
};
#endif

static const FUNCP usbMicAltSettingCbTb[UAC_IF_ALT_SETTING_MIC_NUM+1] =
{
    NULL,
    UsbMicAltSet1Cb,
#if (UAC_IF_ALT_SETTING_MIC_NUM == 2)
    UsbMicAltSet2Cb,
#endif
};

static const float_t usbMicSampleRateInfoTb[][3] =
{// sampleRate,  packetSampleStep,  buffSampleSize
    {44100,     44.1f,   (45*USB_MIC_DELAY_SIZE)},                  // if support usb-mic 44.1, this should be uncommented
    {48000,     48.0f,   (48*USB_MIC_DELAY_SIZE)},
    {96000,     96.0f,   (96*USB_MIC_DELAY_SIZE)},
    {192000,    192.0f,  (192*USB_MIC_DELAY_SIZE)},
    {0,         0,       0},
};

/* Weak functions rewrite ----------------------------------------------------*/
/* mic functions will be called in lib_drv_uac10 */
//altSettingNum: 0, mic stop
//               1~N, mic restart and change AltSetting callback
//               other, mic only restart
//return current Alt.Setting callback function
FUNCP UsbMicCtrl(int32_t altSettingNum)
{
    FUNCP pFunMicCb = NULL;
    if (altSettingNum == 0)
    {
        drvUac10ParaMic.micFirstPackEn = 1;
        drvUac10ParaMic.micWrIdx = 0;
        drvUac10ParaMic.micRdIdx = 0;
        LibDrvMemset(drvUac10ParaMic.micBuff, 0, sizeof(drvUac10ParaMic.micBuff));
        UsbMicClkReset();
    }
    else
    {
        if ((altSettingNum > 0) && (altSettingNum <= UAC_IF_ALT_SETTING_MIC_NUM))
        {
#if (USB_SAMPLE_RATE_192K_EN)   // 出现支持高采样率的情况，需要重新设置包大小  
            uint32_t packSize = usbMicAltSettingMaxPacketSize[altSettingNum];
            OTG_DEV_EP_IN_MAX_PACKET_SET(USB_EP_NUM_AS_MIC, (packSize&0x7FF));
            if ((packSize & 0x800) == 0x800)
            {
                OTG_DEV_EP_IN_TRANS2_PER_FRAME(USB_EP_NUM_AS_MIC);
            }
#endif
            uint32_t sampleRate;
            if (LibDrvUac10SampleRateCheck(USB_PARA_IDX_MIC, &sampleRate) != 0)
            {
                DrvUac10MicSampleRateSet(sampleRate);
            }
            pFunMicCb = usbMicAltSettingCbTb[altSettingNum];
            drvUac10ParaMic.micFirstPackEn = 1;
        }
        if (DrvUac10SpkRunningCheck() != 0)    // spk opened
        {
            DrvUac10SpkBuffIdxGet(&drvUac10ParaMic.micRdIdx, &drvUac10ParaMic.micWrIdx, drvUac10ParaMic.micBuffSize);
        }
        else
        {
            drvUac10ParaMic.micWrIdx = 0;
            drvUac10ParaMic.micRdIdx = (drvUac10ParaMic.micBuffSize / 2);
        }
    }
    return pFunMicCb;
}

/* Exported functions --------------------------------------------------------*/
void DrvUac10MicBuffPtGet(int32_t **pChBuff)
{
    pChBuff[0] = (int32_t *)&drvUac10ParaMic.micBuff[0][0];
    pChBuff[1] = (int32_t *)&drvUac10ParaMic.micBuff[1][0];
    return;
}

int32_t DrvUac10MicRunningCheck(void)
{
    if ((LibDrvUac10StateGet() & BIT1) == 0)
    {
        return 0;
    }
    else if (drvUac10ParaMic.micFirstPackEn == 1)
    {
        return 0;
    }
    return 1;
}

// 返回当前的读写指针按照新缓存大小进行折算之后的索引值
void DrvUac10MicBuffIdxGet(int32_t *pWrIdx, int32_t *pRdIdx, uint32_t newBuffSize)
{
    *pWrIdx = (uint32_t)drvUac10ParaMic.micWrIdx * newBuffSize / drvUac10ParaMic.micBuffSize;
    *pRdIdx = (uint32_t)drvUac10ParaMic.micRdIdx * newBuffSize / drvUac10ParaMic.micBuffSize;
    return;
}

//MIC 采样率改变
void DrvUac10MicSampleRateSet(uint32_t sampleRate)
{
    uint32_t srInfoIdx = 0;
     if(drvUac10ParaMic.micSampleRate == sampleRate)//Niko 192K
    {
        return;
    }
    drvUac10ParaMic.micSampleRate = sampleRate;
    while (1)
    {
        if (sampleRate == (uint32_t)usbMicSampleRateInfoTb[srInfoIdx][0])
        {
            drvUac10ParaMic.micBuffSize = (uint32_t)usbMicSampleRateInfoTb[srInfoIdx][2];    //按照当前采样率调整到设定的延迟下使用的缓存长度
            drvUac10ParaMic.micPacketCntFrac = 0;
            drvUac10ParaMic.micPacketSampleStep = usbMicSampleRateInfoTb[srInfoIdx][1];
            drvUac10ParaMic.micFsSupportFlag = 1;
#if (USB_MIC_LPF_EN != 0)
            if (sampleRate == 96000)
            {
                DrvUac10MicFilterInit(0);
            }
            else if (sampleRate == 192000)
            {
                DrvUac10MicFilterInit(1);
            }
#endif
            drvUac10ParaMic.micFirstPackEn = 1;
            break;
        }
        else if (usbMicSampleRateInfoTb[srInfoIdx][0] == 0)
        {
            drvUac10ParaMic.micPacketCntFrac = 0;
            drvUac10ParaMic.micPacketSampleStep = ((float_t)sampleRate / 1000.0f);
            drvUac10ParaMic.micFsSupportFlag = 0;
            break;
        }
        srInfoIdx++;
    }
    return;
}

#if USB_SRC_FUNC_EN
extern int32_t resampleVarUsbMic[RESAMPLE_MEM_SIZE(UAC_MIC_CH_MODE)];
__RAM_CODE void DrvUac10MicDataWrite(int32_t **pSrcCh, uint8_t ch, uint32_t sampleNumber)
{
    return;//Niko Bootloader
    if (DrvUac10InitStateGet() == 0)
    {
        return;
    }
    if (DrvUac10MicRunningCheck() != 0)
    {
        if (drvUac10ParaMic.micWrIdx != drvUac10ParaMic.micRdIdx)
        {
            if (ResampleFirstPacketGet(resampleVarUsbMic) != 0)
            {
                uint32_t newWrIdx = drvUac10ParaMic.micRdIdx + (drvUac10ParaMic.micBuffSize >> 1);
                drvUac10ParaMic.micWrIdx = (newWrIdx >= drvUac10ParaMic.micBuffSize) ? (newWrIdx - drvUac10ParaMic.micBuffSize) : newWrIdx;
            }
#if (DMA_FRAME_SIZE_FIXED == 0)
            ResampleFrameSizeSet(resampleVarUsbMic, sampleNumber);
#endif
            drvUac10ParaMic.micWrIdx = ResampleHandlerWr(resampleVarUsbMic, pSrcCh, drvUac10ParaMic.micWrIdx, drvUac10ParaMic.micRdIdx, drvUac10ParaMic.micBuffSize);
        }
    }
    else
    {
        ResampleFirstPacketSet(resampleVarUsbMic);
    }
}

#else

__RAM_CODE void DrvUac10MicDataWrite(int32_t **pSrcCh, uint8_t ch, uint32_t sampleNumber)
{
    if (DrvUac10InitStateGet() == 0)
    {
        return;
    }
    if (DrvUac10MicRunningCheck() != 0)
    {
        if (drvUac10ParaMic.micWrIdx != drvUac10ParaMic.micRdIdx)
        {
            uint32_t curSampleRate = drvUac10ParaMic.micSampleRate;
            if (curSampleRate == UAC_SAMPLE_RATE_48K)
            {
                #if (USB_STREAMING_FOR_96K == 0)            //Codec按照48K采样
                UsbMicDataWrite1X(pSrcCh, sampleNumber);
                #else                                       //Codec按照96K采样
                UsbMicDataWriteHalf(pSrcCh, sampleNumber);
                #endif
            }
            
            #if (USB_MIC_SAMPLERATE_MAX >= UAC_SAMPLE_RATE_96K)
            else if (curSampleRate == UAC_SAMPLE_RATE_96K)
            {
                #if (USB_STREAMING_FOR_96K == 0)
                UsbMicDataWrite2X(pSrcCh, sampleNumber);
                #else
                UsbMicDataWrite1X(pSrcCh, sampleNumber);
                #endif
            }
            #endif

            #if (USB_MIC_SAMPLERATE_MAX >= UAC_SAMPLE_RATE_192K)
            else if (curSampleRate == UAC_SAMPLE_RATE_192K)
            {
                #if (USB_STREAMING_FOR_96K == 0)
                UsbMicDataWrite4X(pSrcCh, sampleNumber);
                #else
                UsbMicDataWrite2X(pSrcCh, sampleNumber);
                #endif
            }
            #endif
        }
        else
        {
            drvUac10ParaMic.micFirstPackEn = 1;
        }
    }
}
#endif

/* Private function prototypes -----------------------------------------------*/
static void UsbMicClkReset(void)
{
    if (DrvUac10SpkRunningCheck() == 0)
    {
        UsbClkStateReset();
    }
    return;
}

static void UsbMicClkAdjust(uint32_t sampleRemained)
{
#if (USB_CLK_REF_MIC == 0)
    if (DrvUac10SpkRunningCheck() == 0)
#endif
    {
        UsbClkAdjust(sampleRemained, drvUac10ParaMic.micBuffSize);
    }
    return;
}

// 数据流传输不带增益运算（mute/vol）
__RAM_CODE static void UsbMic16Bit1Ch(void)
{
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordStart(g_usbMicCycleCnt);
#endif
    if (drvUac10ParaMic.micFirstPackEn)
    {
        drvUac10ParaMic.micFirstPackEn = 0;
        UsbMicCtrl(-1);
    }
    uint32_t bytePerSample = 2 * 1;     //16bit, 1ch, 500us
    float_t cntFrac = drvUac10ParaMic.micPacketCntFrac + drvUac10ParaMic.micPacketSampleStep;
    uint32_t sampleNumCur = (uint32_t)cntFrac;
    drvUac10ParaMic.micPacketCntFrac = cntFrac - (float_t)sampleNumCur;
    uint32_t txByteN = bytePerSample * sampleNumCur;
    if (drvUac10ParaMic.micFsSupportFlag != 0)
    {
        int32_t sampleData;
        uint16_t *pDst = (uint16_t *)drvUac10ParaMic.tmpBuff;

        int32_t *pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][drvUac10ParaMic.micRdIdx];
        if ((drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx) >= sampleNumCur)
        {
            for (uint32_t i = 0; i < sampleNumCur; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
            }
            drvUac10ParaMic.micRdIdx += sampleNumCur;
            if (drvUac10ParaMic.micRdIdx == drvUac10ParaMic.micBuffSize)
            {
                drvUac10ParaMic.micRdIdx = 0;
            }
        }
        else
        {
            uint32_t sampleNumCur1 = drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx;
            for (uint32_t i = 0; i < sampleNumCur1; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
            }
            uint32_t sampleNumCur2 = sampleNumCur - sampleNumCur1;
            pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][0];
            for (uint32_t i = 0; i < sampleNumCur2; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
            }
            drvUac10ParaMic.micRdIdx = sampleNumCur2;
        }
    }
    else
    {
        LibDrvMemset(drvUac10ParaMic.tmpBuff, 0x00, txByteN);
    }
    HalOtgDevDxFifoWrite((uint8_t *)drvUac10ParaMic.tmpBuff, txByteN, UAC_FIFO_IDX_MIC);
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordEnd(g_usbMicCycleCnt);
#endif
    return;
}


__RAM_CODE static void UsbMic16Bit2Ch(void)
{
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordStart(g_usbMicCycleCnt);
#endif
    if (drvUac10ParaMic.micFirstPackEn)
    {
        drvUac10ParaMic.micFirstPackEn = 0;
        UsbMicCtrl(-1);
    }
    uint32_t bytePerSample = 2 * 2;     //16bit, 2ch, 500us
    float_t cntFrac = drvUac10ParaMic.micPacketCntFrac + drvUac10ParaMic.micPacketSampleStep;
    uint32_t sampleNumCur = (uint32_t)cntFrac;
    drvUac10ParaMic.micPacketCntFrac = cntFrac - (float_t)sampleNumCur;
    uint32_t txByteN = bytePerSample * sampleNumCur;
    if (drvUac10ParaMic.micFsSupportFlag != 0)
    {
        int32_t sampleData;
        uint16_t *pDst = (uint16_t *)drvUac10ParaMic.tmpBuff;

        int32_t *pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][drvUac10ParaMic.micRdIdx];
        int32_t *pSrcCh2 = (int32_t *)&drvUac10ParaMic.micBuff[1][drvUac10ParaMic.micRdIdx];
        if ((drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx) >= sampleNumCur)
        {
            for (uint32_t i = 0; i < sampleNumCur; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
                sampleData = *pSrcCh2++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
            }
            drvUac10ParaMic.micRdIdx += sampleNumCur;
            if (drvUac10ParaMic.micRdIdx == drvUac10ParaMic.micBuffSize)
            {
                drvUac10ParaMic.micRdIdx = 0;
            }
        }
        else
        {
            uint32_t sampleNumCur1 = drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx;
            for (uint32_t i = 0; i < sampleNumCur1; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
                sampleData = *pSrcCh2++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
            }
            uint32_t sampleNumCur2 = sampleNumCur - sampleNumCur1;
            pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][0];
            pSrcCh2 = (int32_t *)&drvUac10ParaMic.micBuff[1][0];
            for (uint32_t i = 0; i < sampleNumCur2; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
                sampleData = *pSrcCh2++;
                *pDst++ = *((uint16_t *)&sampleData + 1);
            }
            drvUac10ParaMic.micRdIdx = sampleNumCur2;
        }
    }
    else
    {
        LibDrvMemset(drvUac10ParaMic.tmpBuff, 0x00, txByteN);
    }
    HalOtgDevDxFifoWrite((uint8_t *)drvUac10ParaMic.tmpBuff, txByteN, UAC_FIFO_IDX_MIC);
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordEnd(g_usbMicCycleCnt);
#endif
    return;
}

// 数据流传输不带增益运算（mute/vol）
__RAM_CODE static void UsbMic24Bit1Ch(void)
{
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordStart(g_usbMicCycleCnt);
#endif
    if (drvUac10ParaMic.micFirstPackEn)
    {
        drvUac10ParaMic.micFirstPackEn = 0;
        UsbMicCtrl(-1);
    }
    uint32_t bytePerSample = 3 * 1;    //24bit, 2ch
    float_t cntFrac = drvUac10ParaMic.micPacketCntFrac + drvUac10ParaMic.micPacketSampleStep;
    uint32_t sampleNumCur = (uint32_t)cntFrac;
    drvUac10ParaMic.micPacketCntFrac = cntFrac - (float_t)sampleNumCur;
    uint32_t txByteN = bytePerSample * sampleNumCur;
    if (drvUac10ParaMic.micFsSupportFlag != 0)
    {
        int32_t sampleData;
        uint8_t *pDst = (uint8_t *)drvUac10ParaMic.tmpBuff;

        int32_t *pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][drvUac10ParaMic.micRdIdx];
        if ((drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx) >= sampleNumCur)
        {
            for (uint32_t i = 0; i < sampleNumCur; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
            }
            drvUac10ParaMic.micRdIdx += sampleNumCur;
            if (drvUac10ParaMic.micRdIdx == drvUac10ParaMic.micBuffSize)
            {
                drvUac10ParaMic.micRdIdx = 0;
            }
        }
        else
        {
            uint32_t sampleNumCur1 = drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx;
            for (uint32_t i = 0; i < sampleNumCur1; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
            }
            uint32_t sampleNumCur2 = sampleNumCur - sampleNumCur1;
            pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][0];
            for (uint32_t i = 0; i < sampleNumCur2; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
            }
            drvUac10ParaMic.micRdIdx = sampleNumCur2;
        }
    }
    else
    {
        LibDrvMemset(drvUac10ParaMic.tmpBuff, 0x00, txByteN);
    }
    HalOtgDevDxFifoWrite((uint8_t *)drvUac10ParaMic.tmpBuff, txByteN, UAC_FIFO_IDX_MIC);
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordEnd(g_usbMicCycleCnt);
#endif
    return;
}

__RAM_CODE static void UsbMic24Bit2Ch(void)
{
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordStart(g_usbMicCycleCnt);
#endif
    if (drvUac10ParaMic.micFirstPackEn)
    {
        drvUac10ParaMic.micFirstPackEn = 0;
        UsbMicCtrl(-1);
    }
    uint32_t bytePerSample = 3 * 2;    //24bit, 2ch
    float_t cntFrac = drvUac10ParaMic.micPacketCntFrac + drvUac10ParaMic.micPacketSampleStep;
    uint32_t sampleNumCur = (uint32_t)cntFrac;
    drvUac10ParaMic.micPacketCntFrac = cntFrac - (float_t)sampleNumCur;
    uint32_t txByteN = bytePerSample * sampleNumCur;
    if (drvUac10ParaMic.micFsSupportFlag != 0)
    {
        int32_t sampleData;
        uint8_t *pDst = (uint8_t *)drvUac10ParaMic.tmpBuff;

        int32_t *pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][drvUac10ParaMic.micRdIdx];
        int32_t *pSrcCh2 = (int32_t *)&drvUac10ParaMic.micBuff[1][drvUac10ParaMic.micRdIdx];
        if ((drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx) >= sampleNumCur)
        {
            for (uint32_t i = 0; i < sampleNumCur; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
                sampleData = *pSrcCh2++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
            }
            drvUac10ParaMic.micRdIdx += sampleNumCur;
            if (drvUac10ParaMic.micRdIdx == drvUac10ParaMic.micBuffSize)
            {
                drvUac10ParaMic.micRdIdx = 0;
            }
        }
        else
        {
            uint32_t sampleNumCur1 = drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micRdIdx;
            for (uint32_t i = 0; i < sampleNumCur1; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
                sampleData = *pSrcCh2++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
            }
            uint32_t sampleNumCur2 = sampleNumCur - sampleNumCur1;
            pSrcCh1 = (int32_t *)&drvUac10ParaMic.micBuff[0][0];
            pSrcCh2 = (int32_t *)&drvUac10ParaMic.micBuff[1][0];
            for (uint32_t i = 0; i < sampleNumCur2; i++)
            {
                sampleData = *pSrcCh1++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
                sampleData = *pSrcCh2++;
                *pDst++ = (uint8_t)(sampleData >> 8);
                *pDst++ = (uint8_t)(sampleData >> 16);
                *pDst++ = (uint8_t)(sampleData >> 24);
            }
            drvUac10ParaMic.micRdIdx = sampleNumCur2;
        }
    }
    else
    {
        LibDrvMemset(drvUac10ParaMic.tmpBuff, 0x00, txByteN);
    }
    HalOtgDevDxFifoWrite((uint8_t *)drvUac10ParaMic.tmpBuff, txByteN, UAC_FIFO_IDX_MIC);
#if USB_MIC_CB_CYCLE_CNT_EN
    HalSysCycleRecordEnd(g_usbMicCycleCnt);
#endif
    return;
}

#if ((USB_MIC_SAMPLERATE_MAX >= UAC_SAMPLE_RATE_192K) && \
     (USB_SRC_FUNC_EN == 0))
static void UsbMicDataWrite4X(int32_t **pSrcCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micWrIdx);
    int32_t *pSrcCur;
    sampleNumber <<= 2;
    //处理插值滤波
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            float_t *pfTempCur = &drvUac10ParaMic.tmpBuffLpf[0];
            pSrcCur = pSrcCh[i];
            for (uint32_t j = 0; j < sampleNumber; )
            {
                float_t sample = (float_t)*pSrcCur++;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
            }
            DrvUac10MicFilter(pfTempCur, pfTempCur, sampleNumber, i);
            int32_t *pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx];
            for (uint32_t j = 0; j < sampleNumber; )
            {
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
            }
        }
        drvUac10ParaMic.micWrIdx += sampleNumber;
        if (drvUac10ParaMic.micWrIdx >= drvUac10ParaMic.micBuffSize)
        {
            drvUac10ParaMic.micWrIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            float_t *pfTempCur = &drvUac10ParaMic.tmpBuffLpf[0];
            pSrcCur = pSrcCh[i];
            for (uint32_t j = 0; j < sampleNumber; )
            {
                float_t sample = (float_t)*pSrcCur++;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
            }
            //lpf
            DrvUac10MicFilter(pfTempCur, pfTempCur, sampleNumber, i);
            int32_t *pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx];
            for (uint32_t j = 0; j < num; )
            {
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
            }
            pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][0];
            for (uint32_t j = num; j < sampleNumber; )
            {
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
            }
        }
        drvUac10ParaMic.micWrIdx = numRemainded;
    }
    int32_t sampleRemained = drvUac10ParaMic.micWrIdx - drvUac10ParaMic.micRdIdx;
    if (sampleRemained < 0)
    {
        sampleRemained += drvUac10ParaMic.micBuffSize;
    }
    UsbMicClkAdjust(sampleRemained);
    return;
}
#endif

#if ((USB_MIC_SAMPLERATE_MAX >= UAC_SAMPLE_RATE_96K) && \
     (USB_SRC_FUNC_EN == 0))
static void UsbMicDataWrite2X(int32_t **pSrcCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micWrIdx);
    int32_t *pSrcCur;
    sampleNumber <<= 1;
    //处理插值滤波
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            float_t *pfTempCur = &drvUac10ParaMic.tmpBuffLpf[0];
            pSrcCur = pSrcCh[i];
            for (uint32_t j = 0; j < sampleNumber; )
            {
                float_t sample = (float_t)*pSrcCur++;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
            }
            DrvUac10MicFilter(pfTempCur, pfTempCur, sampleNumber, i);
            int32_t *pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx];
            for (uint32_t j = 0; j < sampleNumber; )
            {
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
            }
        }
        drvUac10ParaMic.micWrIdx += sampleNumber;
        if (drvUac10ParaMic.micWrIdx >= drvUac10ParaMic.micBuffSize)
        {
            drvUac10ParaMic.micWrIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            float_t *pfTempCur = &drvUac10ParaMic.tmpBuffLpf[0];
            pSrcCur = pSrcCh[i];
            for (uint32_t j = 0; j < sampleNumber; )
            {
                float_t sample = (float_t)*pSrcCur++;
                pfTempCur[j++] = sample;
                pfTempCur[j++] = sample;
            }
            //lpf
            DrvUac10MicFilter(pfTempCur, pfTempCur, sampleNumber, i);
            int32_t *pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx];
            for (uint32_t j = 0; j < num; )
            {
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
            }
            pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][0];
            for (uint32_t j = num; j < sampleNumber; )
            {
                *pMicBuff++ = (int32_t)pfTempCur[j++];
                *pMicBuff++ = (int32_t)pfTempCur[j++];
            }
        }
        drvUac10ParaMic.micWrIdx = numRemainded;
    }
    int32_t sampleRemained = drvUac10ParaMic.micWrIdx - drvUac10ParaMic.micRdIdx;
    if (sampleRemained < 0)
    {
        sampleRemained += drvUac10ParaMic.micBuffSize;
    }
    UsbMicClkAdjust(sampleRemained);
    return;
}
#endif

#if (USB_SRC_FUNC_EN == 0)
static void UsbMicDataWrite1X(int32_t **pSrcCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micWrIdx);
    int32_t *pSrcCur;
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            pSrcCur = pSrcCh[i];
            LibDrvMemcpy(&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx], pSrcCur, sampleNumber*4);
        }
        drvUac10ParaMic.micWrIdx += sampleNumber;
        if (drvUac10ParaMic.micWrIdx >= drvUac10ParaMic.micBuffSize)
        {
            drvUac10ParaMic.micWrIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            pSrcCur = pSrcCh[i];
            LibDrvMemcpy(&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx], pSrcCur, num*4);
            pSrcCur += num;
            LibDrvMemcpy(&drvUac10ParaMic.micBuff[i][0], pSrcCur, numRemainded*4);
        }
        drvUac10ParaMic.micWrIdx = numRemainded;
    }
    int32_t sampleRemained = drvUac10ParaMic.micWrIdx - drvUac10ParaMic.micRdIdx;
    if (sampleRemained < 0)
    {
        sampleRemained += drvUac10ParaMic.micBuffSize;
    }
    UsbMicClkAdjust(sampleRemained);
    return;
}

static void UsbMicDataWriteHalf(int32_t **pSrcCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaMic.micBuffSize - drvUac10ParaMic.micWrIdx);
    int32_t *pSrcCur;
    sampleNumber >>= 1;
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            int32_t *pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx];
            pSrcCur = pSrcCh[i];
            uint32_t cnt = sampleNumber;
            while(cnt--)
            {
                *pMicBuff++ = *pSrcCur;
                pSrcCur += 2;
            }
        }
        drvUac10ParaMic.micWrIdx += sampleNumber;
        if (drvUac10ParaMic.micWrIdx >= drvUac10ParaMic.micBuffSize)
        {
            drvUac10ParaMic.micWrIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_MIC_DATA_BUFF_CH; i++)
        {
            int32_t *pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][drvUac10ParaMic.micWrIdx];
            pSrcCur = pSrcCh[i];
            uint16_t cnt = num;
            while(cnt--)
            {
                *pMicBuff++ = *pSrcCur;
                pSrcCur += 2;
            }
            cnt = numRemainded;
            pMicBuff = (int32_t *)&drvUac10ParaMic.micBuff[i][0];
            while(cnt--)
            {
                *pMicBuff++ = *pSrcCur;
                pSrcCur += 2;
            }
        }
        drvUac10ParaMic.micWrIdx = numRemainded;
    }
    int32_t sampleRemained = drvUac10ParaMic.micWrIdx - drvUac10ParaMic.micRdIdx;
    if (sampleRemained < 0)
    {
        sampleRemained += drvUac10ParaMic.micBuffSize;
    }
    UsbMicClkAdjust(sampleRemained);
    return;
}
#endif

#else

int32_t DrvUac10MicRunningCheck(void)
{
    return 0;
}

//MIC 采样率改变
void DrvUac10MicSampleRateSet(uint32_t sampleRate)
{
    return;
}

__RAM_CODE void DrvUac10MicDataWrite(int32_t **pSrcCh, uint8_t ch, uint32_t sampleNumber)
{
    return;
}

void DrvUac10MicBuffPtGet(int32_t **pChBuff)
{
    return;
}


#endif



