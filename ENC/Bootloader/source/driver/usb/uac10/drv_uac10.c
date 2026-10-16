
#include "a0_hal_conf.h"

#include "drv_uac10_user_config.h"
#include "drv_uac10.h"
#include "lib_drv_uac10.h"

#include "jat_resample.h"

#include "a0_hal_otg_device.h"
#include "a0_hal_otg.h"
#include "a0_hal_sys_ctrl.h"
#include "a0_hal_i2s.h"
#include "a0_hal_timer.h"
#include "a0_hal_def.h"
#include "UsbUSerInfoSet.h"
// 暂时做个简单的函数define，后续把codec部分拆分成drv函数的时候，再做调整
extern void AppCodecMicVolCfg(int8_t dBVolL, int8_t dBVolR);
//extern void AppI2sClockModify(uint32_t samplerate, uint32_t codecEn);
#define DrvCodecMicVolCfg                   AppCodecMicVolCfg
//#define DrvI2sClkFsCfg                      AppI2sClockModify

static void UsbGainStepChangeF(float_t *gain, float_t step);
static void UsbGainStepChange(int32_t *gain, int32_t step);

#if ((UAC_IF_ID_STREAMING_SPK > 0) || (UAC_IF_ID_STREAMING_MIC > 0))
static void UsbClkStateReset(void);
static void UsbClkAdjust(uint32_t sampleRemained, uint32_t buffSize);
static void UsbSampleRateCheck(uint8_t idx);
#endif

#include "drv_uac10_filter.c"
#include "drv_uac10_mic.c"
#include "drv_uac10_spk.c"
#include "drv_uac10_midi.c"
#include "drv_uac10_hid.c"

/* Private define ------------------------------------------------------------*/
#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
#define USB_VOL_NUM                         (3)
#else
#define USB_VOL_NUM                         (2)
#endif
// 注意：当前代码codec是按照48K配置的，如果codec需要按照96K配置，那么AppUsbDevSpkDataRead，AppUsbDevMicDataWrite需要对应修改选编译
#define LIB_DRV_UAC10_MEM_SIZE              (0x81)          // USB库需要的Memory大小，按照32bit计算

/* Private types -------------------------------------------------------------*/
typedef struct {
    //控制信息
    int32_t     mute[USB_VOL_NUM];                    // [0(spk), 1{mic)],     0(mute), 1(unmute), 计算音量的之后，直接乘，此处未作渐变处理，如需要，可以做成音量一样
    float_t     volume[USB_VOL_NUM][2][2];            // [0(spk)][chL,chR][target, cur],   [1(mic)][chL,chR][target, cur],     1.0f = 0dB
    float_t     volumeWithMute[USB_VOL_NUM][2][2];    // 为了让Mute也能响应渐变，将mute和volume合在一起作为目标和当前值处理

    //状态信息
    uint32_t    micMuteSendFirst;
    uint32_t    micMuteSendCntEn;
    uint32_t    micMuteSendCnt;

    uint8_t     micAgcEn;
    uint8_t     flagInitialized;            // 0, not initialized,  1, initialized
    uint8_t     flagSuspend;                // record suspend state, 1, enter suspend state, 0, normal
    uint8_t     ledState;
    int8_t      clkState;                   //0, std,   -1, < fs,   +1, > fs
} drv_uac10_para_t;

/* Private function declarations ---------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint32_t libDrvUac10Memory[LIB_DRV_UAC10_MEM_SIZE];
static drv_uac10_para_t   drvUac10Para;

/* Private constants ---------------------------------------------------------*/
#if ((UAC_IF_ID_STREAMING_SPK > 0) || (UAC_IF_ID_STREAMING_MIC > 0))
static const uint16_t usbVolRangeTb[USB_VOL_NUM][4] =
{
    {USB_VOL_MIN_SPK, USB_VOL_MAX_SPK, USB_VOL_RES_SPK, USB_VOL_DEFAULT_SPK},    //spk
    {USB_VOL_MIN_MIC, USB_VOL_MAX_MIC, USB_VOL_RES_MIC, USB_VOL_DEFAULT_MIC},    //mic
#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
    {USB_VOL_MIN_SIDETONE, USB_VOL_MAX_SIDETONE, USB_VOL_RES_SIDETONE, USB_VOL_DEFAULT_SIDETONE},   //sidetone
#endif
};
#endif
/* Weak functions rewrite ----------------------------------------------------*/
// Hal层的Weak函数实现，用户无需修改
void HalOtgDevResume(void)
{
    drvUac10Para.flagSuspend = 0;
    if ((LibDrvUac10StateGet() & BIT3) != 0)
    {
        // 上次suspend之前没有发送完hid需要发送的信息
        OTG_DEV_INT_FIFO_IN_EN(UAC_FIFO_IDX_HID_IN);
    }
}

void HalOtgDevSuspend(void)
{
    drvUac10Para.flagSuspend = 1;
}

void HalOtgDevReset(void)
{
    drvUac10Para.flagSuspend = 0;
}

/* Exported functions --------------------------------------------------------*/
uint32_t DrvUac10InitStateGet(void)
{
    return (drvUac10Para.flagInitialized);
}

/* 主循环调用 */
void DrvUac10Process(void)
{
#if ((UAC_IF_ID_STREAMING_SPK > 0) || (UAC_IF_ID_STREAMING_MIC > 0))
    uint8_t idx;
    float_t muteCur;
#if (UAC_IF_ID_STREAMING_SPK > 0)
    /* SPK音量控制、静音控制、采样率变化响应 */
    idx = USB_PARA_IDX_SPK;
    LibDrvUac10MuteCheck(idx, &drvUac10Para.mute[idx]);
    muteCur = (float_t)drvUac10Para.mute[idx];
    for (uint32_t ch = 0; ch < UAC_FU_CTRL_SPK_CH; ch++)
    {
        int16_t dBVol;
        LibDrvUac10VolumeCheck(idx, ch, &drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET], (uint16_t *)&dBVol);
#if (USB_VOL_DB_CALC_EN == 0)
        #warning 这里默认使用powf计算，可以按照此公式，根据实际音量范围，制作针对性的表格来提升此处音量计算的速度
        drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] = powf(2.0f, (((float_t)dBVol / 256.0f) / 6.0f));
#endif
#if USB_SPK_VOL_MIN_TO_ZERO
        if (dBVol == (int16_t)usbVolRangeTb[idx][0])
        {
            drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] = 0;
        }
#endif
        drvUac10Para.volumeWithMute[idx][ch][USB_PARA_IDX_TARGET] = drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] * muteCur;
    }
    UsbSampleRateCheck(idx);
#endif

#if (UAC_IF_ID_STREAMING_MIC > 0)
    /* MIC音量控制、静音控制、采样率变化响应 */
    idx = USB_PARA_IDX_MIC;
    if (LibDrvUac10MuteCheck(idx, &drvUac10Para.mute[idx]) != 0)
    {
        // mute changed, 用于支持usb-mic的绝对值，和windows进行同步操作
#if USB_HID_COL_UC_MIC_MUTE_ABS
        uint32_t hookSwitch = (drvUac10Para.ledState & BIT0) << 1;
        uint32_t ctrlData = (((uint32_t)USB_HID_IN_TEL_ID << 8) | ((drvUac10Para.mute[idx] == 0) ? (0x04 | hookSwitch) : (0x00 | hookSwitch)));
        DrvUsbHidSend2Byte(ctrlData);
#endif
    }
    muteCur = (float_t)drvUac10Para.mute[idx];
    for (uint32_t ch = 0; ch < UAC_FU_CTRL_MIC_CH; ch++)
    {
#if (USB_VOL_DB_CALC_EN == 0)
        int16_t dBVol;
        LibDrvUac10VolumeCheck(idx, ch, &drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET], (uint16_t *)&dBVol);
        #warning 这里默认使用powf计算，可以按照此公式，根据实际音量范围，制作针对性的表格来提升此处音量计算的速度
        drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] = powf(2.0f, (((float_t)dBVol / 256.0f) / 6.0f));
#else        
        LibDrvUac10VolumeCheck(idx, ch, &drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET], NULL);
#endif
        drvUac10Para.volumeWithMute[idx][ch][USB_PARA_IDX_TARGET] = drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] * muteCur;
    }
    UsbSampleRateCheck(idx);
    
#if (UAC_FU_AGC_CTRL_EN != 0)
    idx = USB_PARA_IDX_MIC;
    int32_t agcEn;
    if (LibDrvUac10AgcCheck(idx, &agcEn) != 0)
    {
        drvUac10Para.micAgcEn = (uint8_t)agcEn;
        #warning 此处添加函数调用来响应AGC的开关变化后，可以将此行注释掉，来关闭此warning  
    }
#endif
    
#endif

#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
    /* Sidetone音量控制、静音控制变化响应 */
    idx = USB_PARA_IDX_SIDETONE;
    LibDrvUac10MuteCheck(idx, &drvUac10Para.mute[idx]);
    muteCur = (float_t)drvUac10Para.mute[idx];
    int16_t dBVol;
    LibDrvUac10VolumeCheck(idx, 0, &drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET], (uint16_t *)&dBVol);
#if (USB_VOL_DB_CALC_EN == 0)
    #warning 这里默认使用powf计算，可以按照此公式，根据实际音量范围，制作针对性的表格来提升此处音量计算的速度
    drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET] = powf(2.0f, (((float_t)dBVol / 256.0f) / 6.0f));
#endif        
#if USB_SIDETONE_VOL_MIN_TO_ZERO
    if (dBVol == (int16_t)usbVolRangeTb[idx][0])
    {
        drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET] = 0;
    }
#endif
    drvUac10Para.volumeWithMute[idx][0][USB_PARA_IDX_TARGET] = drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET] * muteCur;
#endif
#endif

    /* USB设备连接状态检查 */
    if (LibDrvUsbConnectCheck(100) == 0)
    {
        drvUac10Para.micMuteSendFirst = 0;
    }


#if (UAC_IF_ID_STREAMING_MIDI >= 0)
    DrvUac10MidiHandler();
#endif

#if (UAC_IF_ID_HID >= 0)
    DrvUac10HidHandler();
#endif
    return;
}

#if (USB_SRC_FUNC_EN)
int32_t resampleVarUsbSpk[RESAMPLE_MEM_SIZE(UAC_SPK_CH_MODE)];
int32_t resampleVarUsbMic[RESAMPLE_MEM_SIZE(UAC_MIC_CH_MODE)];
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
#endif

void DrvUac10Init(void)
{
#if USB_SRC_FUNC_EN
    /* <0> 初始化重采样部分 */
    int32_t *pSampleRdSrc[2];
#if USB_FUN_SPK_EN
    DrvUac10SpkBuffPtGet(pSampleRdSrc);
#endif
    int32_t *pSampleWrDst[2];
    DrvUac10MicBuffPtGet(pSampleWrDst);
    resampleVarParaRd.pRWSample = pSampleRdSrc;
    resampleVarParaWr.pRWSample = pSampleWrDst;
//    if (ResampleInit(&resampleVarParaRd) != 0)
//    {
//        __breakpoint(0);    // memory size error
//    }
//    if (ResampleInit(&resampleVarParaWr) != 0)
//    {
//        __breakpoint(0);    // memory size error
//    }
#endif
    /* <1> USB的IRQ中断优先级配置成0 */
    NVIC_SetPriority(USB_IRQn, 0);
    /* <2> OTG config to device, vbus config to enable */
    HAL_SYS_USB_ID_SET();
    HAL_SYS_USB_VBUS_EN();
    /* <3> 分配变量内存给lib_drv_uac10 */
    uint32_t memSize = LibDrvUac10MemorySizeGet();
    if (memSize > LIB_DRV_UAC10_MEM_SIZE)
    {
        __breakpoint(0);
    }
    LibDrvUac10MemoryAddrInit((uint32_t *)libDrvUac10Memory);
    /* <4> 配置支持的采样率、音量范围和默认值、静音状态默认值 */
#if ((UAC_IF_ID_STREAMING_SPK > 0) || (UAC_IF_ID_STREAMING_MIC > 0))
#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
    LibDrvUac10VolRangeSet((uint16_t *)usbVolRangeTb[0], (uint16_t *)usbVolRangeTb[1], (uint16_t *)usbVolRangeTb[2]);
    LibDrvUac10MuteDefaultSet(USB_MUTE_INIT_SPK, USB_MUTE_INIT_MIC, USB_MUTE_INIT_SIDETONE);
#else
    LibDrvUac10VolRangeSet((uint16_t *)usbVolRangeTb[0], (uint16_t *)usbVolRangeTb[1], NULL);
    LibDrvUac10MuteDefaultSet(USB_MUTE_INIT_SPK, USB_MUTE_INIT_MIC, 0);
#endif
#if (UAC_FU_AGC_CTRL_EN != 0)
    LibDrvUac10AgcDefaultSet(USB_AGCEN_INIT_SPK, USB_AGCEN_INIT_MIC);
#endif  
    LibDrvUac10SampleRateDefaultSet(USB_SPK_SAMPLERATE_DEFAULT, USB_MIC_SAMPLERATE_DEFAULT);
    /* <5> SPK/MIC/SIDETONE 音量控制、静音控制变化响应 */
    uint8_t idx;
    float_t muteCur;
#if (UAC_IF_ID_STREAMING_SPK > 0)
    idx = USB_PARA_IDX_SPK;
    LibDrvUac10MuteCheck(idx, &drvUac10Para.mute[idx]);
    muteCur = (float_t)drvUac10Para.mute[idx];
    for (uint32_t ch = 0; ch < UAC_FU_CTRL_SPK_CH; ch++)
    {
        int16_t dBVol;
        LibDrvUac10VolumeCheck(idx, ch, &drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET], (uint16_t *)&dBVol);
#if (USB_VOL_DB_CALC_EN == 0)
        #warning 这里默认使用powf计算，可以按照此公式，根据实际音量范围，制作针对性的表格来提升此处音量计算的速度
        drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] = powf(2.0f, (((float_t)dBVol / 256.0f) / 6.0f));
#endif
#if USB_SPK_VOL_MIN_TO_ZERO
        if (dBVol == (int16_t)usbVolRangeTb[idx][0])
        {
            drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] = 0;
        }
#endif
        drvUac10Para.volumeWithMute[idx][ch][USB_PARA_IDX_TARGET] = drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] * muteCur;
    }
    UsbSampleRateCheck(idx);
#endif
#if (UAC_IF_ID_STREAMING_MIC > 0)
    idx = USB_PARA_IDX_MIC;
    LibDrvUac10MuteCheck(idx, &drvUac10Para.mute[idx]);
    muteCur = (float_t)drvUac10Para.mute[idx];
    for (uint32_t ch = 0; ch < UAC_FU_CTRL_MIC_CH; ch++)
    {
#if (USB_VOL_DB_CALC_EN == 0)
        int16_t dBVol;
        LibDrvUac10VolumeCheck(idx, ch, &drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET], (uint16_t *)&dBVol);
        #warning 这里默认使用powf计算，可以按照此公式，根据实际音量范围，制作针对性的表格来提升此处音量计算的速度
        drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] = powf(2.0f, (((float_t)dBVol / 256.0f) / 6.0f));
#else        
        LibDrvUac10VolumeCheck(idx, ch, &drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET], NULL);
#endif
        drvUac10Para.volumeWithMute[idx][ch][USB_PARA_IDX_TARGET] = drvUac10Para.volume[idx][ch][USB_PARA_IDX_TARGET] * muteCur;
    }
    UsbSampleRateCheck(idx);
#if (UAC_FU_AGC_CTRL_EN != 0)
    idx = USB_PARA_IDX_MIC;
    int32_t agcEn;
    LibDrvUac10AgcCheck(idx, &agcEn);
    drvUac10Para.micAgcEn = (uint8_t)agcEn;
    #warning 此处添加函数调用来响应AGC的开关变化后，可以将此行注释掉，来关闭此warning  
#endif
#endif
#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
    idx = USB_PARA_IDX_SIDETONE;
    LibDrvUac10MuteCheck(idx, &drvUac10Para.mute[idx]);
    muteCur = (float_t)drvUac10Para.mute[idx];
    int16_t dBVol;
    LibDrvUac10VolumeCheck(idx, 0, &drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET], (uint16_t *)&dBVol);
#if (USB_VOL_DB_CALC_EN == 0)
    #warning 这里默认使用powf计算，可以按照此公式，根据实际音量范围，制作针对性的表格来提升此处音量计算的速度
    drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET] = powf(2.0f, (((float_t)dBVol / 256.0f) / 6.0f));
#endif        
#if USB_SIDETONE_VOL_MIN_TO_ZERO
    if (dBVol == (int16_t)usbVolRangeTb[idx][0])
    {
        drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET] = 0;
    }
#endif
    drvUac10Para.volumeWithMute[idx][0][USB_PARA_IDX_TARGET] = drvUac10Para.volume[idx][0][USB_PARA_IDX_TARGET] * muteCur;
#endif
#endif
    /* <6> 描述符重配置 */
#if USB_DSC_USER_MANUFACTURER_EN
    LibDrvUac10DscStringConfig((uint32_t)&userDscStrManufacturer, USB_STR_MANUFACTURER_INDEX);
#endif
#if USB_DSC_USER_PRODUCT_EN
    LibDrvUac10DscStringConfig((uint32_t)&userDscStrProduct, USB_STR_PRODUCT_INDEX);
#endif
#if USB_DSC_USER_SERIAL_NUM_EN
    LibDrvUac10DscStringConfig((uint32_t)&userDscStrSerialNumber, USB_STR_SERIALNUMBER_INDEX);
#endif
#if USB_DSC_USER_VIDPID_EN
    LibDrvUac10PidVidConfig(USER_PID, USER_VID, USER_DEV_VER);
#endif
    /* <7> 为了适配降频，对OTG模块进行时钟配置 */
    if (SystemHclk < 30000000)
    {
        U32USB(OTG_DEV_CTL) |= ( BIT1);
    }
    /* <8> lib_drv_uac10初始化 */
    if (LibDrvUac10Init(HAL_TickGet) != 0)
    {
        __breakpoint(0);
    }

    drvUac10Para.flagInitialized = 0xA5;
    return;
}

__RAM_CODE void DrvUac10IrqHandler(void)
{
    LibDrvUac10Irq();
    return;
}

__RAM_CODE void DrvUac10FrameProcess(void)
{
    if (drvUac10Para.flagInitialized == 0)
    {
        return;
    }

    DrvUac10SpkStreamingCheck();

#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
    for (uint32_t ch = 0; ch < UAC_FU_CTRL_SIDETONE_CH; ch++)
    {
        UsbGainStepChangeF(drvUac10Para.volumeWithMute[USB_PARA_IDX_SIDETONE][ch], 0.01f);
    }
#endif
    return;
}

// 首先对从ADC来的数据进行 USB MIC VOLUME/MUTE 的处理
__RAM_CODE void DrvUac10MicVolHandler(int32_t **pAdcCh, uint32_t sampleNumber)
{
#if (UAC_IF_ID_STREAMING_MIC > 0)
    if (drvUac10Para.flagInitialized == 0)
    {
        return;
    }
    for (uint32_t ch = 0; ch < UAC_FU_CTRL_MIC_CH; ch++)
    {
        int32_t *pIn = pAdcCh[ch];
        int32_t *pOut = pAdcCh[ch];
        for (uint32_t i = 0; i < sampleNumber; i++)
        {
            UsbGainStepChangeF(drvUac10Para.volumeWithMute[USB_PARA_IDX_MIC][ch], 0.001f);
            float_t micVol = drvUac10Para.volumeWithMute[USB_PARA_IDX_MIC][ch][USB_PARA_IDX_CURRENT];
            float_t input = (float_t)*pIn++ / 2147483647.0f * micVol;
#if (((USB_VOL_MAX_MIC >> 8) > 0x00) && ((USB_VOL_MAX_MIC >> 8) <= 0x7F))       // if gain > 0dB, signal should be clipped
            if (input > 1.0f)
            {
                input = 1.0f;
            }
            else if (input < -1.0f)
            {
                input = -1.0f;
            }
#endif
            *pOut++ = (int32_t)(input * 2147483647.0f);
        }
    }
#endif
    return;
}

// 最后对往DAC送的数据进行 USB SPK VOLUME/MUTE 的处理，并做剪切处理
__RAM_CODE void DrvUac10SpkVolHandler(int32_t **pDacCh, uint32_t sampleNumber)
{
#if (UAC_IF_ID_STREAMING_SPK > 0)
    if (drvUac10Para.flagInitialized == 0)
    {
        return;
    }
    for (uint32_t ch = 0; ch < UAC_FU_CTRL_SPK_CH; ch++)
    {
        int32_t *pIn = pDacCh[ch];
        int32_t *pOut = pDacCh[ch];
        for (uint32_t i = 0; i < sampleNumber; i++)
        {
            UsbGainStepChangeF(drvUac10Para.volumeWithMute[USB_PARA_IDX_SPK][ch], 0.001f);
            float_t spkVol = drvUac10Para.volumeWithMute[USB_PARA_IDX_SPK][ch][USB_PARA_IDX_CURRENT];
            float_t input = (float_t)*pIn++ / 2147483647.0f * spkVol;
            *pOut++ = (int32_t)(input * 2147483647.0f);                         // digital spkVol max is 0dB, do not need to clip
        }
    }
#endif
    return;
}

__RAM_CODE void DrvUac10SidetoneVolHandler(int32_t **pIn, int32_t **pOut, uint32_t sampleNumber)
{
#if UAC_ID_SIDETONE_UNIT_ENABLE
    if (drvUac10Para.flagInitialized == 0)
    {
        return;
    }
    int32_t *pInL = pIn[0];
#if (UAC_FU_CTRL_SIDETONE_CH == 2)
    int32_t *pInR = pIn[1];
#endif    
    int32_t *pOutL = pOut[0];
    int32_t *pOutR = pOut[1];
    // 由于sidetone音量变化不频繁，暂时不做逐点渐变，节省资源
    // 如果后续有特殊需求，要求sidetone音量变化更丝滑，那就需要参考spk、mic做逐点渐变处理，DrvUac10FrameProcess中的帧变化去掉
    float_t sidetoneVolL = drvUac10Para.volumeWithMute[USB_PARA_IDX_SIDETONE][0][USB_PARA_IDX_CURRENT];
#if (UAC_FU_CTRL_SIDETONE_CH == 2)
    float_t sidetoneVolR = drvUac10Para.volumeWithMute[USB_PARA_IDX_SIDETONE][1][USB_PARA_IDX_CURRENT];
#endif
    float_t inputL, inputR;
    if (DrvUac10MicRunningCheck() != 0)
    {
        for (uint32_t i = 0; i < sampleNumber; i++)
        {
            inputL = (float_t)*pInL++ / 2147483647.0f * sidetoneVolL;
#if (UAC_FU_CTRL_SIDETONE_CH == 1)
            inputR = inputL;
#elif (UAC_FU_CTRL_SIDETONE_CH == 2)
            inputR = (float_t)*pInR++ / 2147483647.0f * sidetoneVolR;
#else
            #error UAC_FU_CTRL_SIDETONE_CH must be set to 1 or 2
#endif
            inputL += ((float_t)*pOutL / 2147483647.0f);
            if (inputL > 1.0f)
            {
                inputL = 1.0f;
            }
            else if (inputL < -1.0f)
            {
                inputL = -1.0f;
            }
            *pOutL++ = (int32_t)(inputL * 2147483647.0f);
            
            inputR += ((float_t)*pOutR / 2147483647.0f);
            if (inputR > 1.0f)
            {
                inputR = 1.0f;
            }
            else if (inputR < -1.0f)
            {
                inputR = -1.0f;
            }
            *pOutR++ = (int32_t)(inputR * 2147483647.0f);
        }
    }
#endif
    return;
}

uint32_t DrvUac10MicMuteGet(void)
{
    return ((drvUac10Para.mute[USB_PARA_IDX_MIC] != 0) ? 0 : 1);
}

void DrvUac10MicMuteSet(uint32_t micMuteState)
{
    drvUac10Para.mute[USB_PARA_IDX_MIC] = ((micMuteState != 0) ? 0 : 1);
    return;
}

uint32_t DrvUac10SpkMuteGet(void)
{
    return ((drvUac10Para.mute[USB_PARA_IDX_SPK] != 0) ? 0 : 1);
}

void DrvUac10SpkMuteSet(uint32_t spkMuteState)
{
    drvUac10Para.mute[USB_PARA_IDX_SPK] = ((spkMuteState != 0) ? 0 : 1);
    return;
}

/* Private function prototypes -----------------------------------------------*/
__RAM_CODE static void UsbGainStepChangeF(float_t *gain, float_t step)
{
    if (gain[USB_PARA_IDX_CURRENT] <= gain[USB_PARA_IDX_TARGET])
    {
        gain[USB_PARA_IDX_CURRENT] += step;
        if (gain[USB_PARA_IDX_CURRENT] > gain[USB_PARA_IDX_TARGET])
        {
            gain[USB_PARA_IDX_CURRENT] = gain[USB_PARA_IDX_TARGET];
        }
    }
    else
    {
        gain[USB_PARA_IDX_CURRENT] -= step;
        if (gain[USB_PARA_IDX_CURRENT] < gain[USB_PARA_IDX_TARGET])
        {
            gain[USB_PARA_IDX_CURRENT] = gain[USB_PARA_IDX_TARGET];
        }
    }
    return;
}

__RAM_CODE static void UsbGainStepChange(int32_t *gain, int32_t step)
{
    if (gain[USB_PARA_IDX_CURRENT] <= gain[USB_PARA_IDX_TARGET])
    {
        gain[USB_PARA_IDX_CURRENT] += step;
        if (gain[USB_PARA_IDX_CURRENT] > gain[USB_PARA_IDX_TARGET])
        {
            gain[USB_PARA_IDX_CURRENT] = gain[USB_PARA_IDX_TARGET];
        }
    }
    else
    {
        gain[USB_PARA_IDX_CURRENT] -= step;
        if (gain[USB_PARA_IDX_CURRENT] < gain[USB_PARA_IDX_TARGET])
        {
            gain[USB_PARA_IDX_CURRENT] = gain[USB_PARA_IDX_TARGET];
        }
    }
    return;
}

#if ((UAC_IF_ID_STREAMING_SPK > 0) || (UAC_IF_ID_STREAMING_MIC > 0))
static void UsbClkStateReset(void)
{
    if(drvUac10Para.clkState > 0)
    {
        hal_i2s_clk_fine_adjust(-1);
        drvUac10Para.clkState = 0;
    }
    else if (drvUac10Para.clkState < 0)
    {
        hal_i2s_clk_fine_adjust(+1);
        drvUac10Para.clkState = 0;
    }
    return;
}

static void UsbClkAdjust(uint32_t sampleRemained, uint32_t buffSize)
{
    if (drvUac10Para.clkState == 0)
    {
        uint32_t buffSizeLow = buffSize >> 2;                   // 1/4
        uint32_t buffSizeHigh = buffSize - buffSizeLow;         // 3/4
        if (sampleRemained < buffSizeLow)
        {
            hal_i2s_clk_fine_adjust(+1);
            drvUac10Para.clkState = +1;
        }
        else if (sampleRemained > buffSizeHigh)
        {
            HalI2sClkFineAdjust(-1);
            drvUac10Para.clkState = -1;
        }
    }
    else if (drvUac10Para.clkState > 0)
    {
        uint32_t buffSizeHalf = buffSize >> 1;
        if (sampleRemained >= buffSizeHalf)
        {
            HalI2sClkFineAdjust(-1);
            drvUac10Para.clkState = 0;
        }
    }
    else if (drvUac10Para.clkState < 0)
    {
        uint32_t buffSizeHalf = buffSize >> 1;
        if (sampleRemained <= buffSizeHalf)
        {
            HalI2sClkFineAdjust(+1);
            drvUac10Para.clkState = 0;
        }
    }
    return;
}

//检查采样率变化，调整I2S时钟
//每次USB的FIFO中断回调函数都会查询一次采样率，所以作为RAM代码
//idx, USB_PARA_IDX_SPK/USB_PARA_IDX_MIC, MIC/SPK的采样率响应
__RAM_CODE static void UsbSampleRateCheck(uint8_t idx)
{
    return;//Niko Bootloader
    uint32_t sampleRate;
    if (LibDrvUac10SampleRateCheck(idx, &sampleRate) != 0)
    {
        if (idx == USB_PARA_IDX_SPK)
        {
            DrvUac10SpkSampleRateSet(sampleRate);
            DrvI2sClkFsCfg(sampleRate, ((sampleRate == 192000) ? 0 : 1));
#if USB_SRC_FUNC_EN
    #if (DMA_FRAME_SIZE_FIXED == 0)
            sampleRate = 48000;     //这里因为是调了I2S时钟，所以重采样仅仅是匹配几个点的偏差，仍然按照48K到48K进行
    #endif
            ResampleFSCfg(resampleVarUsbSpk, RESAMPLE_IDX_RD, sampleRate,8);
#endif
        }
        else
        {
            DrvUac10MicSampleRateSet(sampleRate);
#if USB_SRC_FUNC_EN
            ResampleFSCfg(resampleVarUsbMic, RESAMPLE_IDX_WR, sampleRate,8);
#endif
        }
    }
    return;
}
#endif


void DrvUac10Init0(void)
{
    /* 提前给USB模块分配内存，方便配置USB的用户更改信息 */
    uint32_t memSize = LibDrvUac10MemorySizeGet();
    if (memSize > LIB_DRV_UAC10_MEM_SIZE)
    {
        __breakpoint(0);
    }
    LibDrvUac10MemoryAddrInit((uint32_t *)libDrvUac10Memory);
    UsbUserInfoInit();
}


/*****************************************************************************/
/* 额外针对1821添加的USB相关代码 */
void DrvUac10VolRangeConfig(uint32_t idx, uint16_t dBMin, uint16_t dbMax, uint16_t dbDefault)
{
#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
    if (idx > 2)
    {
        return;
    }
#else
    if (idx > 1)
    {
        return;
    }
#endif
//    usbVolRangeTb[idx][0] = dBMin;
//    usbVolRangeTb[idx][1] = dbMax;
//    usbVolRangeTb[idx][3] = dbDefault;
    return;
}



