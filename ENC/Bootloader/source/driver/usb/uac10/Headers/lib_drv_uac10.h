/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    drv_usb_uac10.h
 * @brief   USB device class definition for audio devices and hid
 * @version V1.2.1
 * @date    2023/11/28
 * @author  huangzheng
 ******************************************************************************
 */

#ifndef __LIB_DRV_UAC10_H
#define __LIB_DRV_UAC10_H

//code section of this lib which is related to isr: lib_uac10_code_section
//const section of this lib which is related to isr: lib_uac10_const_section

/* Includes -----------------------------------------------------------------*/
#include "lib_drv_uac10_config.h"
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Exported constants -------------------------------------------------------*/
#define USB_PARA_IDX_SPK                    (0)
#define USB_PARA_IDX_MIC                    (1)
#define USB_PARA_IDX_SIDETONE               (2)
#define USB_PARA_IDX_TARGET                 (0)
#define USB_PARA_IDX_CURRENT                (1)

/* Exported macros ----------------------------------------------------------*/

/* Exported functions -------------------------------------------------------*/
#if (UAC_IF_ID_STREAMING_MIDI >= 0)
/**
 * @brief  Send midi in data
 * @param  pSendData: Pointer to the data need to be sent
 * @param  byteSize: byte number to be sent
 * @retval == 0, ready to send
 * @retval == 1, busy, need to call this function later
 */
int32_t LibDrvUac10MidiInSend(uint8_t *pSendData, size_t byteSize);

/** 
 * @brief  Send midi in data reset, if hid in busy for a long time, use this function to reset midi in send
 * @param  None
 * @retval == 0, reset ok, ready to send
 * @retval == 1, midi is not supported
 */
int32_t LibDrvUac10MidiInSendReset(void);

/**
 * @brief  Receive midi out data
 * @param  pRecvData: Pointer to the data need to be written
 * @retval > 0, byte number of the pack received
 * @retval == 0, empty pack received
 * @retval == -1, no data received
 */
int32_t LibDrvUac10MidiOutReceive(uint8_t *pRecvData);
#endif

#if (UAC_IF_ID_HID >= 0)
/**
 * @brief  Send hid in data
 * @param  pSendData: Pointer to the data need to be sent
 * @param  byteSize: byte number to be sent
 * @retval == 0, ready to send
 * @retval == 1, busy, need to call this function later
 */
int32_t LibDrvUac10HidInSend(uint8_t *pSendData, size_t byteSize);

/** 
 * @brief  Send hid in data reset, if hid in busy for a long time, use this function to reset hid in send
 * @param  None
 * @retval == 0, reset ok, ready to send
 * @retval == 1, hid is not supported
 */
int32_t LibDrvUac10HidInSendReset(void);

/**
 * @brief  Receive hid out data
 * @param  pRecvData: Pointer to the data need to be written
 * @retval > 0, byte number of the pack received
 * @retval == 0, empty pack received
 * @retval == -1, no data received
 */
int32_t LibDrvUac10HidOutReceive(uint8_t *pRecvData);
#endif

#if (UAC_IF_ID_MFI >= 0)
/**
 * @brief  Send mfi in data
 * @param  pSendData: Pointer to the data need to be sent
 * @param  byteSize: byte number to be sent
 * @retval == 0, ready to send
 * @retval == 1, busy, need to call this function later
 */
int32_t LibDrvUac10MfiInSend(uint8_t *dat, size_t byteSize);

/**
 * @brief  Receive mfi out data
 * @param  pRecvData: Pointer to the data need to be written
 * @retval > 0, byte number of the pack received
 * @retval == 0, empty pack received
 * @retval == -1, no data received
 */
int32_t LibDrvUac10MfiOutReceive(uint8_t *pRecvData);
#endif

/**
 * @brief  USB State of Spk/Mic/HidOut/HidIn
 * @param  None
 * @return State
 * @details
 * BIT0: spkState,     0, idle,                1, playing
 * BIT1: micState,     0, idle,                1, recording
 * BIT2: hidOutState,  0, no hid out data,     1, got data, need to decode
 * BIT3: hidInState,   0, idle, ready to send, 1, busy, can't send
 * BIT4: connectState, 0, usb disconnected     1, usb connected
 * BIT5: midiOutState, 0, no midi out data,    1, got data, need to decodec
 * BIT6: midiInState,  0, idle, ready to send, 1, busy, can't send
 */
uint32_t LibDrvUac10StateGet(void);

//检查USB连接状态
//checkTime, 当检测到断开连接超过多少ms之后，更新标志位，反馈断开
//return 0, USB和主机处于断开状态
//return 1, USB和主机处于连接状态
uint32_t LibDrvUsbConnectCheck(uint32_t checkTime);

#if ((UAC_IF_ID_STREAMING_SPK > 0) || (UAC_IF_ID_STREAMING_MIC > 0))
//clkSrcIdx, USB_PARA_IDX_SPK, USB_PARA_IDX_MIC
//*pNewSampleRate, return new sample rate in uint32_t
//return 0, vol no change
//return 1, vol changed
int32_t LibDrvUac10SampleRateCheck(uint8_t clkSrcIdx, uint32_t *pNewSampleRate);

//检查音量变化，根据dB值计算PCM数据使用的浮点数音量值
//idx = 0, spk; 1, mic, 2, sidetone
//ch = 0, LogicCh1; 1, LogicCh2; ...
//*pNewVol, return new volume in float_t
//*dBVol, return new volume in 16bit dBVol format, which is received from PC, if dBVol is NULL, this function will not write to this pointer
//return 0, vol no change
//return 1, vol changed
int32_t LibDrvUac10VolumeCheck(uint8_t idx, uint8_t ch, float_t *pNewVol, uint16_t *dBVol);

//检查mute变化
//idx = 0, spk; 1, mic, 2, sidetone
//return 0, vol no change
//return 1, vol changed
int32_t LibDrvUac10MuteCheck(uint8_t idx, int32_t *pNewVol);

#if UAC_FU_AGC_CTRL_EN
//检查agc变化
//idx = 0, spk(invalid); 1, mic,
//return 0, vol no change
//return 1, vol changed
int32_t LibDrvUac10AgcCheck(uint8_t idx, int32_t *pNewVol);
#endif

//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10SampleRateDefaultSet(uint32_t spkDefault, uint32_t micDefault);

//在初始化之前，先配置音量范围信息，所有通道使用相同配置，SPK/MIC独立
//pSidetoneVolRange, [min, max, res, default], in dB value, 仅在UAC_ID_SIDETONE_UNIT_ENABLE配置成1的时候才有效
//pSpkVolRange, [min, max, res, default], in dB value
//pMicVolRange, [min, max, res, default], in dB value
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10VolRangeSet(uint16_t *pSpkVolRange, uint16_t *pMicVolRange, uint16_t *pSidetoneVolRange);

//sidetonneMuteDefault仅在UAC_ID_SIDETONE_UNIT_ENABLE配置成1的时候才有效
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10MuteDefaultSet(uint8_t spkMuteDefault, uint8_t micMuteDefault, uint8_t sidetoneMuteDefault);

#if UAC_FU_AGC_CTRL_EN
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10AgcDefaultSet(uint8_t spkAgcDefault, uint8_t micAgcDefault);
#endif
#endif



//获取当前库需要的变量区大小，按照32位计数
uint32_t LibDrvUac10MemorySizeGet(void);
//需要在库初始化之前，先分配内存指针
void LibDrvUac10MemoryAddrInit(uint32_t *pMemAddr);

//if this function do not called before lib_drv_uac20_init, default string descriptor in LIB will be used
//addrDscStr, string descriptor start address
//strIdx, USB_STR_MANUFACTURER_INDEX/USB_STR_PRODUCT_INDEX/USB_STR_SERIALNUMBER_INDEX
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10DscStringConfig(uint32_t addrDscStr, uint8_t strIdx);

//if this function do not called before lib_drv_uac20_init, default pid/vid in LIB will be used
//pid, 0, use default value in LIB (USB_DEFAULT_PID), 0x0001~0xFFFF, new pid used
//vid, 0, use default value in LIB (USB_DEFAULT_VID), 0x0001~0xFFFF, new vid used
//deviceVer, 0, use default value in LIB (USB_DEVICE_VERSION), 0x0001~0xFFFF, new value used
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10PidVidConfig(uint16_t pid, uint16_t vid, uint16_t deviceVer);

/*
 * audio参数初始化调用函数
 * pFunTickMsGet, 传入毫秒时钟获取函数，如：HAL_TickGet, LibDrvUac10Init(HAL_TickGet);
 * return 0, OK
 * return -1, lib_drv_uac20_memory_addr_init() need to be called first
 * return -2, pInit is wrong
 */
int32_t LibDrvUac10Init(uint32_t (*pFunTickMsGet)());
//需要加在IRQ中断响应中调用，即USB_IRQHandler()
void LibDrvUac10Irq(void);

// 用于支持Teams唤出，需要在HID解码时判断，如果ID==0x9A时，调用此函数
// 如果需要激活Teams窗口，idx=1，其他时候idx=0
int32_t LibDrvUsbHidDecode(uint8_t *pHidData, uint32_t idx);

void *LibDrvMemcpy(void *pDst, const void *pSrc, uint32_t byteSize);

void *LibDrvMemset(void *pDst, uint32_t c, uint32_t byteSize);


#ifdef __cplusplus
}
#endif

#endif
