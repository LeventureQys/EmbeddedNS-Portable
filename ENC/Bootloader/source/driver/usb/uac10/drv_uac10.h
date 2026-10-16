
#ifndef __DRV_UAC10_H
#define __DRV_UAC10_H

#include "jat_typedef.h"
#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief  Usb loop process, must be called in main loop
  * @param  None
  * @return None
  */
void DrvUac10Process(void);
/** @brief  Usb device initial, must be called before use usb device
  * @param  None
  * @return None
  */
void DrvUac10Init(void);
/** @brief  Irq handler, must called in irq
  * @param  None
  * @return None
  */
void DrvUac10IrqHandler(void);

/* functions below is for dma interrupt to process frame data */
// drv_uac10
uint32_t DrvUac10InitStateGet(void);

void DrvUac10FrameProcess(void);

void DrvUac10SidetoneVolHandler(int32_t **pIn, int32_t **pOut, uint32_t sampleNumber);
// 首先对从ADC来的数据进行 USB MIC VOLUME/MUTE 的处理
void DrvUac10MicVolHandler(int32_t **pAdcCh, uint32_t sampleNumber);
// 最后对往DAC送的数据进行 USB SPK VOLUME/MUTE 的处理，并做剪切处理
void DrvUac10SpkVolHandler(int32_t **pDacCh, uint32_t sampleNumber);
// 获取MIC的MUTE状态，返回1表示mute，返回0表示unmute
uint32_t DrvUac10MicMuteGet(void);
// 软件设置MIC的MUTE状态，在UC通信耳机功能中，部分情况下，为了实现和PC同步，需要用到此函数
void DrvUac10MicMuteSet(uint32_t micMuteState);
// 获取SPK的MUTE状态，返回1表示mute，返回0表示unmute
uint32_t DrvUac10SpkMuteGet(void);
// 软件设置SPK的MUTE状态，在UC通信耳机功能中，部分情况下，为了实现和PC同步，需要用到此函数
void DrvUac10SpkMuteSet(uint32_t spkMuteState);

// drv_uac10_mic
void DrvUac10MicDataWrite(int32_t **pSrcCh, uint8_t ch, uint32_t sampleNumber); // 帧中断中调用往USB-MIC发送数据
// drv_uac10_spk
void DrvUac10SpkDataRead(int32_t **pDstCh, uint8_t ch, uint32_t sampleNumber);  // 帧中断中调用从USB-SPK获取数据
void DrvUac10SpkStreamingCheck(void);                                           // 必须在DMA帧中断中调用，用于检测USB-SPK数据流是否停止传输
// drv_uac10_hid
// 用于UC通信、播放器控制等进行HID-IN的控制码发送
// ctrlData: 待发送数据，如：0x0206，则表示发送 ID=0x02，数据=0x06
// return:   0, 发送成功
//           1, 发送失败
int32_t DrvUsbHidSend2Byte(uint32_t ctrlData);
int32_t DrvUsbHidSend2ByteForHeadset(void *pCtrlData);
uint32_t DrvUsbHidLedStateGet(void);
void DrvUac10HidHandler(void);                                                  // HID主循环处理，已经在usb主循环处理中同步调用
void DrvUsbHidUserParaHandlerCallbackInstall(FUNCPP cb);                        // 加载HID64字节处理的回调函数
void DrvUsbHidUserParaHandlerCallbackUnInstall(void);                           // 卸载HID64字节处理的回调函数
void DrvUsbHidVenderTelephonyCallbackInstall(FUNCPP cb);                        // 加载HID16字节处理的回调函数
void DrvUsbHidVenderTelephonyCallbackUnInstall(void);                           // 卸载HID16字节处理的回调函数
// drv_uac10_midi
void DrvUac10MidiHandler(void);                                                   // MIDI主循环处理，已经在usb主循环处理中同步调用
// 针对1821额外增加的接口
void DrvUac10VolRangeConfig(uint32_t idx, uint16_t dBMin, uint16_t dbMax, uint16_t dbDefault);
void DrvUac10Init0(void);
#ifdef __cplusplus
}
#endif

#endif
