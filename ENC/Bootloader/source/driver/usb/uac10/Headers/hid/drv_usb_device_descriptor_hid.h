#ifndef __USB_DEVICE_DESCRIPTOR_HID_H__
#define __USB_DEVICE_DESCRIPTOR_HID_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "usb_device_standard.h"
//#include "USB\usb_std.h"
//#include "USB\Device\common\drv_usb_device_descriptor_common.h"

/*********************************************************************************************************************/
/* User Config */														 

// /* Current usb hid version, 1.11 */
// #define HID_SPEC_VERSION                                0x0111
// #define USB_CFG_DESCRIPTORS                         	(1)

// /* Current usb interface number */
// #define HID_IF_ID                                   	(0)
// #define HID_IF_NUM                                      (1)

// /* Current usb endpoint number definitions */
// #define USB_HID_EP_NUM                                  (1)

 #define HID_CLASS_DESCRIPTORS_NUM                       (1) /* Number of HID class descriptors */

/*************************************************************************************************/
/* Class-Specific HID Interface */
__packed typedef struct
{
    uint8_t                 bLength;                    // Size of this descriptor, in bytes.
    uint8_t                 bDescriptorType;            // Class-specific HID descriptor type. (=HID_DCS_HID)
    uint16_t                bcdHID;                     // HID Class Specification release ¨C 1.11. (=0x0111)
    uint8_t                 bCountryCode;               // Hardware target country ¨C None.
    uint8_t                 bNumDescriptors;            // Number of HID class descriptors to follow. Shall be at least 0x01 for the mandatory Report descriptor. (=HID_CLASS_DESCRIPTORS_NUM)
    __packed struct {
        uint8_t                 bDescriptorType;            // Descriptor type.
        uint16_t                wDescriptorLength;          // Total length of the Report descriptor.
    }descriptor[HID_CLASS_DESCRIPTORS_NUM];
}usb_dsc_class_hid_t;

__packed typedef struct
{
    usb_dsc_if_t                        dscIf;
    usb_dsc_class_hid_t                 dscHid;
    usb_dsc_ep_t                        dscEpIn;
    usb_dsc_ep_t                        dscEpOut;
}usb_dsc_cs_if_hid_t;

#ifdef __cplusplus
}
#endif

#endif
