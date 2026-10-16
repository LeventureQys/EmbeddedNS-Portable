#ifndef __USB_DEVICE_HID_H__
#define __USB_DEVICE_HID_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "USB\Device\drv_usb_device_private.h"
#include "USB\Device\drv_usb_device.h"

#ifdef DRV_OTG_DEVICE_ENABLE_HID

#include "USB\Device\hid\drv_usb_device_descriptor_hid.h"

__packed typedef struct
{
    usb_dsc_cfg_t                       dscCfg;
    usb_dsc_cs_if_hid_t                 dscCsIfHid;
}usb_dsc_cfg_hid_t;

/** @brief  Usb loop process, must be called in main loop
  * @param  None
  * @retval None
  */
void drv_usb_device_hid_process(void);
/** @brief  Usb device initial, must be called before use usb device
  * @param  None
  * @retval None
  */
void drv_usb_device_hid_init(void);
/** @brief  Irq handler, must called in irq
  * @param  None
  * @retval None
  */
void drv_usb_device_irq_handler(void);

/** @brief  Send hid in data
  * @param  pSendData: Pointer to the data need to be sent
  * @param  byteSize: byte number to be sent
  * @retval == 0, ready to send
  * @retval == 1, busy, need to call this function later
  */
int32_t drv_usb_device_hid_in_send(uint8_t *pSendData, size_t byteSize);

#if (DRV_OTG_DEVICE_HID_FUNCTION_SELECT == DRV_OTG_DEVICE_HID_FUNCTION_FOR_SIMULATION)
/** @brief  Ack when DSP stoped
  *
  */
void dspBreakPointAck(void);

USB_DEV_STATE 
     drv_usb_device_get_devstate_func_hid(void);

#endif

int  drv_usb_device_hid_register_listener(USBDeviceListener *listener);
#endif

#ifdef __cplusplus
}
#endif

#endif
