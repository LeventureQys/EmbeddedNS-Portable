
#include "drv_uac10_user_config.h"

#if (UAC_IF_ID_STREAMING_SPK > 0)
#include "lib_drv_uac10.h"

extern int32_t DrvUac10MicRunningCheck(void);
extern void DrvUac10MicBuffIdxGet(int32_t *pWrIdx, int32_t *pRdIdx, uint32_t newBuffSize);

/* Private types -------------------------------------------------------------*/
typedef struct {
    //缓存区
    uint32_t    spkBuff[USB_SPK_DATA_BUFF_CH][USB_SPK_DATA_BUFF_SIZE];  // spk数据缓存区，按32位存放
    uint32_t    tmpBuff[1024/4];                                        // 用于USB的FIFO访问进行数据过渡

    int32_t     spkWrIdx;                   //0 ~ USB_SPK_DATA_BUFF_SIZE, spk循环缓存的写指针索引
    int32_t     spkRdIdx;                   //0 ~ USB_SPK_DATA_BUFF_SIZE, spk循环缓存的读指针索引
    uint32_t    spkBuffSize;                // <= USB_SPK_DATA_BUFF_SIZE

    //状态信息
    uint32_t    spkSampleRate;              // spk当前采样率

    uint32_t    spkFrameCnt;
    uint32_t    spkFrameCntLast;
    uint32_t    spkDisConnectCnt;

    uint8_t     spkFirstPackEn;

} drv_uac10_para_spk_t;

/* Private function declarations ---------------------------------------------*/
static void UsbClkStateReset(void);
static void UsbClkAdjust(uint32_t sampleRemained, uint32_t buffSize);

static void UsbSpkClkReset(void);
static void UsbSpkClkAdjust(uint32_t sampleRemained);
static void UsbSpk24Bit2Ch(void);
static void UsbSpk16Bit2Ch(void);
static void UsbSpkDataRead4X(int32_t **pDstCh, uint32_t sampleNumber);
static void UsbSpkDataRead2X(int32_t **pDstCh, uint32_t sampleNumber);
static void UsbSpkDataRead1X(int32_t **pDstCh, uint32_t sampleNumber);
static void UsbSpkDataReadHalf(int32_t **pDstCh, uint32_t sampleNumber);

/* callback函数占用时间需要小于I2S时钟下的8个样本的时间（48K采样率时，1/6ms = 166us），否则最坏情况下，dma和usb同时中断，usb进了中断会导致dma断流 */
#define UsbSpkAltSet1Cb                 UsbSpk16Bit2Ch
#define UsbSpkAltSet2Cb                 UsbSpk24Bit2Ch

void DrvUac10SpkSampleRateSet(uint32_t sampleRate);

/* Public variables ----------------------------------------------------------*/
#if USB_SPK_CB_CYCLE_CNT_EN
uint32_t g_usbSpkCycleCnt[5];
#endif
/* Private variables ---------------------------------------------------------*/
static drv_uac10_para_spk_t drvUac10ParaSpk;

/* Private constants ---------------------------------------------------------*/
#if (USB_SAMPLE_RATE_192K_EN)
static const uint16_t usbSpkAltSettingMaxPacketSize[UAC_IF_ALT_SETTING_SPK_NUM + 1] =
{
    0,
    (UAC_FS_PACKET_TRANS_SIZE(UAC_IF_ALT_SET_1_SPK_PACK_SIZE)&0x7FF),       //最大192*2*2=768
    (UAC_FS_PACKET_TRANS_SIZE(UAC_IF_ALT_SET_2_SPK_PACK_SIZE)&0x7FF),       //最大192*3*2=1152/2=576,2次传输
};
#endif

static const FUNCP usbSpkAltSettingCbTb[UAC_IF_ALT_SETTING_SPK_NUM+1] =
{
    NULL,
    UsbSpkAltSet1Cb,
#if (UAC_IF_ALT_SETTING_SPK_NUM == 2)
    UsbSpkAltSet2Cb,
#endif
};

/* Weak functions rewrite ----------------------------------------------------*/
/* spk functions will be called in lib_drv_uac10 */
//altSettingNum: 0, spk stop
//               1~N, spk restart and change AltSetting callback
//               other, spk only restart
//return current Alt.Setting callback function
FUNCP UsbSpkCtrl(int32_t altSettingNum)
{
    FUNCP pFunSpkCb = NULL;
    if (altSettingNum == 0)
    {
        drvUac10ParaSpk.spkFirstPackEn = 1;
        drvUac10ParaSpk.spkWrIdx = 0;
        drvUac10ParaSpk.spkRdIdx = 0;
        LibDrvMemset(drvUac10ParaSpk.spkBuff, 0, sizeof(drvUac10ParaSpk.spkBuff));
        UsbSpkClkReset();
    }
    else
    {
        if ((altSettingNum > 0) && (altSettingNum <= UAC_IF_ALT_SETTING_SPK_NUM))
        {
#if (USB_SAMPLE_RATE_192K_EN)   // 出现支持高采样率的情况，需要重新设置包大小  
            OTG_DEV_EP_OUT_MAX_PACKET_SET(USB_EP_NUM_AS_SPK, usbSpkAltSettingMaxPacketSize[altSettingNum]);
#endif

            uint32_t sampleRate;
            if (LibDrvUac10SampleRateCheck(USB_PARA_IDX_SPK, &sampleRate) != 0)
            {
                DrvUac10SpkSampleRateSet(sampleRate);
            }
            pFunSpkCb = usbSpkAltSettingCbTb[altSettingNum];
            drvUac10ParaSpk.spkFirstPackEn = 1;
        }
        if (DrvUac10MicRunningCheck() != 0)         // mic opened
        {
            DrvUac10MicBuffIdxGet(&drvUac10ParaSpk.spkRdIdx, &drvUac10ParaSpk.spkWrIdx, drvUac10ParaSpk.spkBuffSize);
        }
        else
        {
            drvUac10ParaSpk.spkWrIdx = 0;
            drvUac10ParaSpk.spkRdIdx = (drvUac10ParaSpk.spkBuffSize / 2);
        }
    }
    return pFunSpkCb;
}

/* Exported functions --------------------------------------------------------*/
void DrvUac10SpkBuffPtGet(int32_t **pChBuff)
{
    pChBuff[0] = (int32_t *)&drvUac10ParaSpk.spkBuff[0][0];
    pChBuff[1] = (int32_t *)&drvUac10ParaSpk.spkBuff[1][0];
    return;
}

int32_t DrvUac10SpkRunningCheck(void)
{
    if ((LibDrvUac10StateGet() & BIT0) == 0)
    {
        return 0;
    }
    else if (drvUac10ParaSpk.spkFirstPackEn == 1)
    {
        return 0;
    }
    return 1;
}

// 返回当前的读写指针按照新缓存大小进行折算之后的索引值
void DrvUac10SpkBuffIdxGet(int32_t *pWrIdx, int32_t *pRdIdx, uint32_t newBuffSize)
{
    *pWrIdx = (uint32_t)drvUac10ParaSpk.spkWrIdx * newBuffSize / drvUac10ParaSpk.spkBuffSize;
    *pRdIdx = (uint32_t)drvUac10ParaSpk.spkRdIdx * newBuffSize / drvUac10ParaSpk.spkBuffSize;
    return;
}

//MIC 采样率改变
void DrvUac10SpkSampleRateSet(uint32_t sampleRate)
{
    drvUac10ParaSpk.spkSampleRate = sampleRate;
    drvUac10ParaSpk.spkBuffSize = (sampleRate + 999) / 1000 * USB_SPK_DELAY_SIZE;    //按照当前采样率调整到设定的延迟下使用的缓存长度
    drvUac10ParaSpk.spkFirstPackEn = 1;
    return;
}

// 主要针对像windows下录音机这种，打开spk数据流端口，但是不送数据流的情况的处理
__RAM_CODE void DrvUac10SpkStreamingCheck(void)
{
    if(drvUac10ParaSpk.spkFirstPackEn == 0)
    {
        if(drvUac10ParaSpk.spkFrameCntLast == drvUac10ParaSpk.spkFrameCnt)
        {
            drvUac10ParaSpk.spkDisConnectCnt++;
            if(drvUac10ParaSpk.spkDisConnectCnt == 4)
            {
                // 连续4帧未进过USB的ISO中断，认为SPK数据流已经停止传输传输了
                drvUac10ParaSpk.spkFirstPackEn = 1;
                LibDrvMemset(drvUac10ParaSpk.spkBuff, 0, sizeof(drvUac10ParaSpk.spkBuff));
                drvUac10ParaSpk.spkDisConnectCnt = 0;
                UsbSpkClkReset();
            }
        }
        else
        {
            drvUac10ParaSpk.spkFrameCntLast = drvUac10ParaSpk.spkFrameCnt;
            drvUac10ParaSpk.spkDisConnectCnt = 0;
        }
    }
    return;
}

#if USB_SRC_FUNC_EN
extern int32_t resampleVarUsbSpk[RESAMPLE_MEM_SIZE(UAC_SPK_CH_MODE)];
__RAM_CODE void DrvUac10SpkDataRead(int32_t **pDstCh, uint8_t ch, uint32_t sampleNumber)
{
    return;//Niko Bootloader
    if (DrvUac10InitStateGet() == 0)
    {
        return;
    }
    int32_t *pDstCur;
    if (DrvUac10SpkRunningCheck() != 0)
    {
        if (drvUac10ParaSpk.spkWrIdx != drvUac10ParaSpk.spkRdIdx)
        {
            if (ResampleFirstPacketGet(resampleVarUsbSpk) != 0)
            {
                uint32_t newRdIdx = drvUac10ParaSpk.spkWrIdx + (drvUac10ParaSpk.spkBuffSize >> 1);
                drvUac10ParaSpk.spkRdIdx = (newRdIdx >= drvUac10ParaSpk.spkBuffSize) ? (newRdIdx - drvUac10ParaSpk.spkBuffSize) : newRdIdx;
            }
#if (DMA_FRAME_SIZE_FIXED == 0)
            ResampleFrameSizeSet(resampleVarUsbSpk, sampleNumber);
#endif
            drvUac10ParaSpk.spkRdIdx = ResampleHandlerRd(resampleVarUsbSpk, pDstCh, drvUac10ParaSpk.spkWrIdx, drvUac10ParaSpk.spkRdIdx, drvUac10ParaSpk.spkBuffSize);
        }
        else
        {
            for (uint32_t i = 0; i < ch; i++)
            {
                pDstCur = pDstCh[i];
                LibDrvMemset(pDstCur, 0x00, sampleNumber*4);
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < ch; i++)
        {
            pDstCur = pDstCh[i];
            LibDrvMemset(pDstCur, 0x00, sampleNumber*4);
        }
        ResampleFirstPacketSet(RESAMPLE_IDX_RD);
    }
}

#else

__RAM_CODE void DrvUac10SpkDataRead(int32_t **pDstCh, uint8_t ch, uint32_t sampleNumber)
{
    if (DrvUac10InitStateGet() == 0)
    {
        return;
    }
    int32_t *pDstCur;
    if (DrvUac10SpkRunningCheck() != 0)
    {
        if (drvUac10ParaSpk.spkWrIdx != drvUac10ParaSpk.spkRdIdx)
        {
            uint32_t curSampleRate = drvUac10ParaSpk.spkSampleRate;
            if (curSampleRate == 48000)
            {
                #if (USB_STREAMING_FOR_96K == 0)                //Codec按照48K采样
                UsbSpkDataRead1X(pDstCh, sampleNumber);
                #else                                           //Codec按照96K采样
                UsbSpkDataReadHalf(pDstCh, sampleNumber);
                #endif
            }

            #if (USB_SPK_SAMPLERATE_MAX >= UAC_SAMPLE_RATE_96K)
            else if (curSampleRate == 96000)
            {
                #if (USB_STREAMING_FOR_96K == 0)
                UsbSpkDataRead2X(pDstCh, sampleNumber);
                #else
                UsbSpkDataRead1X(pDstCh, sampleNumber);
                #endif
            }
            #endif
            
            #if (USB_SPK_SAMPLERATE_MAX >= UAC_SAMPLE_RATE_192K)
            else if (curSampleRate == 192000)
            {
                #if (USB_STREAMING_FOR_96K == 0)
                UsbSpkDataRead4X(pDstCh, sampleNumber);
                #else
                UsbSpkDataRead2X(pDstCh, sampleNumber);
                #endif
            }
            #endif
        }
        else
        {
            for (uint32_t i = 0; i < ch; i++)
            {
                pDstCur = pDstCh[i];
                LibDrvMemset(pDstCur, 0x00, sampleNumber*4);
            }
            drvUac10ParaSpk.spkFirstPackEn = 1;
        }
    }
    else
    {
        for (uint32_t i = 0; i < ch; i++)
        {
            pDstCur = pDstCh[i];
            LibDrvMemset(pDstCur, 0x00, sampleNumber*4);
        }
    }
}
#endif

/* Private function prototypes -----------------------------------------------*/

static void UsbSpkClkReset(void)
{
    if (DrvUac10MicRunningCheck() == 0)
    {
        UsbClkStateReset();
    }
    return;
}

static void UsbSpkClkAdjust(uint32_t sampleRemained)
{
#if USB_CLK_REF_MIC
    uint32_t micState = (LibDrvUac10StateGet() & BIT1);
    if (micState == 0)
#endif
    {
        UsbClkAdjust((drvUac10ParaSpk.spkBuffSize - sampleRemained), drvUac10ParaSpk.spkBuffSize);
    }
    return;
}

__RAM_CODE static void UsbSpk16Bit2Ch(void)
{
#if USB_SPK_CB_CYCLE_CNT_EN
    HalSysCycleRecordStart(g_usbSpkCycleCnt);
#endif
    drvUac10ParaSpk.spkFrameCnt++;
    if (drvUac10ParaSpk.spkFirstPackEn)
    {
        drvUac10ParaSpk.spkFirstPackEn = 0;
        UsbSpkCtrl(-1);
    }
    uint32_t rxByteN = OTG_DEV_FIFO_OUT_BYTE_CNT(UAC_FIFO_IDX_SPK);
    HalOtgDevDxFifoRead((uint8_t *)drvUac10ParaSpk.tmpBuff, rxByteN, UAC_FIFO_IDX_SPK);

    int16_t *pi16DataRd = (int16_t *)drvUac10ParaSpk.tmpBuff;
    uint16_t sampleNumCur = rxByteN >> 2;
    int32_t *pDstCh1 = (int32_t *)&drvUac10ParaSpk.spkBuff[0][drvUac10ParaSpk.spkWrIdx];
    int32_t *pDstCh2 = (int32_t *)&drvUac10ParaSpk.spkBuff[1][drvUac10ParaSpk.spkWrIdx];

    if ((drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkWrIdx) >= sampleNumCur)
    {
        for (uint32_t i = 0; i < sampleNumCur; i++)
        {
            *pDstCh1++ = ((int32_t)(*pi16DataRd++) << 16);
            *pDstCh2++ = ((int32_t)(*pi16DataRd++) << 16);
        }
        drvUac10ParaSpk.spkWrIdx += sampleNumCur;
        if (drvUac10ParaSpk.spkWrIdx == drvUac10ParaSpk.spkBuffSize)
        {
            drvUac10ParaSpk.spkWrIdx = 0;
        }
    }
    else
    {
        uint32_t sampleNumCur1 = drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkWrIdx;
        for (uint32_t i = 0; i < sampleNumCur1; i++)
        {
            *pDstCh1++ = ((int32_t)(*pi16DataRd++) << 16);
            *pDstCh2++ = ((int32_t)(*pi16DataRd++) << 16);
        }
        uint32_t sampleNumCur2 = sampleNumCur - sampleNumCur1;
        pDstCh1 = (int32_t *)&drvUac10ParaSpk.spkBuff[0][0];
        pDstCh2 = (int32_t *)&drvUac10ParaSpk.spkBuff[1][0];
        for (uint32_t i = 0; i < sampleNumCur2; i++)
        {
            *pDstCh1++ = ((int32_t)(*pi16DataRd++) << 16);
            *pDstCh2++ = ((int32_t)(*pi16DataRd++) << 16);
        }
        drvUac10ParaSpk.spkWrIdx = sampleNumCur2;
    }
#if USB_SPK_CB_CYCLE_CNT_EN
    HalSysCycleRecordEnd(g_usbSpkCycleCnt);
#endif
    return;
}

__RAM_CODE static void UsbSpk24Bit2Ch(void)
{
#if USB_SPK_CB_CYCLE_CNT_EN
    HalSysCycleRecordStart(g_usbSpkCycleCnt);
#endif
    drvUac10ParaSpk.spkFrameCnt++;
    if (drvUac10ParaSpk.spkFirstPackEn)
    {
        drvUac10ParaSpk.spkFirstPackEn = 0;
        UsbSpkCtrl(-1);
    }
    uint32_t rxByteN = OTG_DEV_FIFO_OUT_BYTE_CNT(UAC_FIFO_IDX_SPK);
    HalOtgDevDxFifoRead((uint8_t *)drvUac10ParaSpk.tmpBuff, rxByteN, UAC_FIFO_IDX_SPK);

    uint8_t *pDataRd = (uint8_t *)drvUac10ParaSpk.tmpBuff;
    int32_t *pDstCh1 = (int32_t *)&drvUac10ParaSpk.spkBuff[0][drvUac10ParaSpk.spkWrIdx];
    int32_t *pDstCh2 = (int32_t *)&drvUac10ParaSpk.spkBuff[1][drvUac10ParaSpk.spkWrIdx];

    if (((drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkWrIdx) * 6) >= rxByteN)
    {
        for (uint32_t i = 0; i < rxByteN; )
        {
            *pDstCh1++ = (int32_t)(PACKED_U32(&pDataRd[i]) << 8);
            i += 3;
            *pDstCh2++ = (int32_t)(PACKED_U32(&pDataRd[i]) << 8);
            i += 3;
        }
        drvUac10ParaSpk.spkWrIdx += (rxByteN / 6);
        if (drvUac10ParaSpk.spkWrIdx == drvUac10ParaSpk.spkBuffSize)
        {
            drvUac10ParaSpk.spkWrIdx = 0;
        }
    }
    else
    {
        uint32_t rxByteN1 = (drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkWrIdx) * 6;
        uint32_t i;
        for (i = 0; i < rxByteN1; )
        {
            *pDstCh1++ = (int32_t)(PACKED_U32(&pDataRd[i]) << 8);
            i += 3;
            *pDstCh2++ = (int32_t)(PACKED_U32(&pDataRd[i]) << 8);
            i += 3;
        }
        uint32_t rxByteN2 = rxByteN - rxByteN1;
        pDstCh1 = (int32_t *)&drvUac10ParaSpk.spkBuff[0][0];
        pDstCh2 = (int32_t *)&drvUac10ParaSpk.spkBuff[1][0];
        for (; i < rxByteN; )    // 注意，次处i是承接上面的循环计数，继续计数，所以判断条件还是要以总字节数判断
        {
            *pDstCh1++ = (int32_t)(PACKED_U32(&pDataRd[i]) << 8);
            i += 3;
            *pDstCh2++ = (int32_t)(PACKED_U32(&pDataRd[i]) << 8);
            i += 3;
        }
        drvUac10ParaSpk.spkWrIdx = (rxByteN2 / 6);
    }
#if USB_SPK_CB_CYCLE_CNT_EN
    HalSysCycleRecordEnd(g_usbSpkCycleCnt);
#endif
    return;
}

static void UsbSpkDataRead4X(int32_t **pDstCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkRdIdx);
    int32_t *pDstCur;
    sampleNumber <<= 2;                 //4个点当1个点用
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            int32_t *pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx];
            uint32_t cnt = sampleNumber >> 2;
            while(cnt--)
            {
                *pDstCur++ = *pSrcChCur;
                pSrcChCur += 4;
            }
        }
        drvUac10ParaSpk.spkRdIdx += sampleNumber;
        if (drvUac10ParaSpk.spkRdIdx >= drvUac10ParaSpk.spkBuffSize)
        {
            drvUac10ParaSpk.spkRdIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            int32_t *pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx];
            uint32_t cnt = num >> 2;
            while(cnt--)
            {
                *pDstCur++ = *pSrcChCur;
                pSrcChCur += 4;
            }
            pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][0];
            cnt = numRemainded >> 2;
            while(cnt--)
            {
                *pDstCur++ = *pSrcChCur;
                pSrcChCur += 4;
            }
        }
        drvUac10ParaSpk.spkRdIdx = numRemainded;
    }
    int32_t sampleRemained = drvUac10ParaSpk.spkWrIdx - drvUac10ParaSpk.spkRdIdx;
    if (sampleRemained < 0)
    {
        sampleRemained += drvUac10ParaSpk.spkBuffSize;
    }
    UsbSpkClkAdjust(sampleRemained);
    return;
}

static void UsbSpkDataRead2X(int32_t **pDstCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkRdIdx);
    int32_t *pDstCur;
    sampleNumber <<= 1;                 //2个点当1个点用
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            int32_t *pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx];
            uint32_t cnt = sampleNumber >> 1;
            while(cnt--)
            {
                *pDstCur++ = *pSrcChCur;
                pSrcChCur += 2;
            }
        }
        drvUac10ParaSpk.spkRdIdx += sampleNumber;
        if (drvUac10ParaSpk.spkRdIdx >= drvUac10ParaSpk.spkBuffSize)
        {
            drvUac10ParaSpk.spkRdIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            int32_t *pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx];
            uint32_t cnt = num >> 1;
            while(cnt--)
            {
                *pDstCur++ = *pSrcChCur;
                pSrcChCur += 2;
            }
            pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][0];
            cnt = numRemainded >> 1;
            while(cnt--)
            {
                *pDstCur++ = *pSrcChCur;
                pSrcChCur += 2;
            }
        }
        drvUac10ParaSpk.spkRdIdx = numRemainded;
    }
    int32_t sampleRemained = drvUac10ParaSpk.spkWrIdx - drvUac10ParaSpk.spkRdIdx;
    if (sampleRemained < 0)
    {
        sampleRemained += drvUac10ParaSpk.spkBuffSize;
    }
    UsbSpkClkAdjust(sampleRemained);
    return;
}

static void UsbSpkDataRead1X(int32_t **pDstCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkRdIdx);
    int32_t *pDstCur;
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            LibDrvMemcpy(pDstCur, &drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx], sampleNumber*4);
        }
        drvUac10ParaSpk.spkRdIdx += sampleNumber;
        if (drvUac10ParaSpk.spkRdIdx >= drvUac10ParaSpk.spkBuffSize)
        {
            drvUac10ParaSpk.spkRdIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            LibDrvMemcpy(pDstCur, &drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx], num*4);
            pDstCur += num;
            LibDrvMemcpy(pDstCur, &drvUac10ParaSpk.spkBuff[i][0], numRemainded*4);
        }
        drvUac10ParaSpk.spkRdIdx = numRemainded;
    }
    int32_t sampleRemained = drvUac10ParaSpk.spkWrIdx - drvUac10ParaSpk.spkRdIdx;
    if (sampleRemained < 0)
    {
        sampleRemained += drvUac10ParaSpk.spkBuffSize;
    }
    UsbSpkClkAdjust(sampleRemained);
    return;
}

static void UsbSpkDataReadHalf(int32_t **pDstCh, uint32_t sampleNumber)
{
    uint32_t num = (drvUac10ParaSpk.spkBuffSize - drvUac10ParaSpk.spkRdIdx);
    int32_t *pDstCur;
    sampleNumber >>= 1;                 //1个点当2个点用
    if (num >= sampleNumber)
    {
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            int32_t *pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx];
            uint32_t cnt = sampleNumber;
            while(cnt--)
            {
                int32_t sample = *pSrcChCur++;
                *pDstCur++ = sample;
                *pDstCur++ = sample;
            }
        }
        drvUac10ParaSpk.spkRdIdx += sampleNumber;
        if (drvUac10ParaSpk.spkRdIdx >= drvUac10ParaSpk.spkBuffSize)
        {
            drvUac10ParaSpk.spkRdIdx = 0;
        }
    }
    else
    {
        uint32_t numRemainded = sampleNumber - num;
        for (uint32_t i = 0; i < USB_SPK_DATA_BUFF_CH; i++)
        {
            pDstCur = pDstCh[i];
            int32_t *pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][drvUac10ParaSpk.spkRdIdx];
            uint32_t cnt = num;
            while(cnt--)
            {
                int32_t sample = *pSrcChCur++;
                *pDstCur++ = sample;
                *pDstCur++ = sample;
            }
            pSrcChCur = (int32_t *)&drvUac10ParaSpk.spkBuff[i][0];
            cnt = numRemainded;
            while(cnt--)
            {
                int32_t sample = *pSrcChCur++;
                *pDstCur++ = sample;
                *pDstCur++ = sample;
            }
        }
        drvUac10ParaSpk.spkRdIdx = numRemainded;
    }
}


#else

// 当前LIB库不包含SPK时，下列接口函数为空
int32_t DrvUac10SpkRunningCheck(void)
{
    return 0;
}

void DrvUac10SpkBuffIdxGet(int32_t *pWrIdx, int32_t *pRdIdx, uint32_t newBuffSize)
{
    return;
}

void DrvUac10SpkSampleRateSet(uint32_t sampleRate)
{
    return;
}

void DrvUac10SpkStreamingCheck(void)
{
    return;
}

__RAM_CODE void DrvUac10SpkDataRead(int32_t **pDstCh, uint8_t ch, uint32_t sampleNumber)
{
    return;
}


#endif




