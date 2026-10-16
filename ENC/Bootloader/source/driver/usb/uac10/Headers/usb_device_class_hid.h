/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    usb_device_class_hid.h
 * @brief   USB device class definition for hid device
    | Version | Data       | Author     | Info                                |
    | ------- | ---------- | ---------- | ----------------------------------- |
    | 1.0.0   | 2021/10/08 | huangzheng | First version                       |
    | 1.0.1   | 2022/04/08 | huangzheng | Add copyright info                  |
 * @version V1.0.1
 * @date    2022/04/08
 * @author  huangzheng
 ******************************************************************************
 */

#ifndef __USB_DEVICE_CLASS_HID_H
#define __USB_DEVICE_CLASS_HID_H

#ifdef __cplusplus
extern "C" {
#endif

/* HID Interface Class Code */
#define HID_CLASS                                       0x03
/* HID Interface Subclass Codes */
#define HID_SUBCLASS_NONE                               0x00
#define HID_SUBCLASS_BOOT_INTERFACE                     0x01
/* HID Interface Protocol Codes */
#define HID_PROTOCOL_NONE                               0x00
#define HID_PROTOCOL_KEYBOARD                           0x01
#define HID_PROTOCOL_MOUSE                              0x02
/* HID Class-Specific Descriptor Types */
#define HID_DSC_HID                                     0x21
#define HID_DSC_REPORT                                  0x22
#define HID_DSC_PHYSICAL                                0x23
/* HID Class-Specific Request Codes */
#define HID_GET_REPORT                                  0x01            // must be supported by all device
#define HID_GET_IDLE                                    0x02
#define HID_GET_PROTOCOL                                0x03            // required only for boot device
#define HID_SET_REPORT                                  0x09
#define HID_SET_IDLE                                    0x0A
#define HID_SET_PROTOCOL                                0x0B            // required only for boot device
/* HID Class-Specific Report Type */
#define HID_REPORT_IN                                   0x01
#define HID_REPORT_OUT                                  0x02
#define HID_REPORT_FEATURE                              0x03

#ifdef __cplusplus
}
#endif

#endif

