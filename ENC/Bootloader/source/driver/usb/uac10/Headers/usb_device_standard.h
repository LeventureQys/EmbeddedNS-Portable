/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    usb_device_standard.h
 * @brief   USB standard request codes and other definitions
    | Version | Data       | Author     | Info                                |
    | ------- | ---------- | ---------- | ----------------------------------- |
    | 1.0.0   | 2021/09/28 | huangzheng | First version                       |
    | 1.0.1   | 2022/04/08 | huangzheng | Add copyright info                  |
 * @version V1.0.1
 * @date    2022/04/08
 * @author  huangzheng
 ******************************************************************************
 */

#ifndef __USB_DEVICE_STANDARD_H
#define __USB_DEVICE_STANDARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard Device Descriptor */
__packed typedef struct
{
    uint8_t     bLength;                    /*!< Size of this Descriptor in Bytes */
    uint8_t     bDescriptorType;            /*!< Descriptor Type (=USB_DSC_DEVICE) */
    uint16_t    bcdUSB;                     /*!< USB Spec Release Number in BCD */
    uint8_t     bDeviceClass;               /*!< Device Class Code */
    uint8_t     bDeviceSubClass;            /*!< Device Subclass Code */
    uint8_t     bDeviceProtocol;            /*!< Device Protocol Code */
    uint8_t     bMaxPacketSize0;            /*!< Maximum Packet Size for EP0 */
    uint16_t    idVendor;                   /*!< Vendor ID */
    uint16_t    idProduct;                  /*!< Product ID */
    uint16_t    bcdDevice;                  /*!< Device Release Number in BCD */
    uint8_t     iManufacturer;              /*!< Index of String Desc for Manufacturer */
    uint8_t     iProduct;                   /*!< Index of String Desc for Product */
    uint8_t     iSerialNumber;              /*!< Index of String Desc for SerNo */
    uint8_t     bNumConfigurations;         /*!< Number of possible Configurations */
}usb_dsc_t;

/* Standard Device_Qualifier Descriptor, added in usb2.0, if device support HS, this will be use */
__packed typedef struct
{
    uint8_t     bLength;                    /*!< Size of this Descriptor in Bytes */
    uint8_t     bDescriptorType;            /*!< Descriptor Type (=USB_DSC_DEVICE_QUALIFIER) */
    uint16_t    bcdUSB;                     /*!< USB Spec Release Number in BCD */
    uint8_t     bDeviceClass;               /*!< Device Class Code */
    uint8_t     bDeviceSubClass;            /*!< Device Subclass Code */
    uint8_t     bDeviceProtocol;            /*!< Device Protocol Code */
    uint8_t     bMaxPacketSize0;            /*!< Maximum Packet Size for EP0 */
    uint8_t     bNumConfigurations;         /*!< Number of possible Configurations */
    uint8_t     bReserved;                  /*!< Must be zero */
}usb_dsc_qualifer_t;

/* Standard Configuration Descriptor */
__packed typedef struct
{
    uint8_t     bLength;                    /*!< Size of this Descriptor in Bytes */
    uint8_t     bDescriptorType;            /*!< Descriptor Type (=USB_DSC_CONFIG) */
    uint16_t    wTotalLength;               /*!< Total Length of Data for this Conf */
    uint8_t     bNumInterfaces;             /*!< No of Interfaces supported by this configuration */
    uint8_t     bConfigurationValue;        /*!< Designator Value for this configuration */
    uint8_t     iConfiguration;             /*!< Index of String Desc for this Conf */
    uint8_t     bmAttributes;               /*!< Configuration Characteristics (see below) */
    uint8_t     bMaxPower;                  /*!< Max. Power Consumption in this configuration (*2mA) */
}usb_dsc_cfg_t;

/* Standard Other Speed Configuration Descriptor, added in usb2.0, a configuration of a highspeed capable device if it were operating at its other possible speed */
__packed typedef struct
{
    uint8_t     bLength;                    /*!< Size of this Descriptor in Bytes */
    uint8_t     bDescriptorType;            /*!< Descriptor Type (=USB_DSC_OTHER_SPEED_CONFIG) */
    uint16_t    wTotalLength;               /*!< Total Length of Data for this Conf */
    uint8_t     bNumInterfaces;             /*!< No of Interfaces supported by this configuration */
    uint8_t     bConfigurationValue;        /*!< Designator Value for this configuration */
    uint8_t     iConfiguration;             /*!< Index of String Desc for this Conf */
    uint8_t     bmAttributes;               /*!< Configuration Characteristics (see below) */
    uint8_t     bMaxPower;                  /*!< Max. Power Consumption in this configuration (*2mA) */
}usb_dsc_cfg_other_speed_t;

/* Standard Interface Descriptor */
__packed typedef struct
{
    uint8_t     bLength;                    /*!< Size of this Descriptor in Bytes: 9 */
    uint8_t     bDescriptorType;            /*!< Descriptor Type (=USB_DSC_INTERFACE) */
    uint8_t     bInterfaceNumber;           /*!< Number of *this* Interface (0..) */
    uint8_t     bAlternateSetting;          /*!< Alternative for this Interface (if any) */
    uint8_t     bNumEndpoints;              /*!< No of EPs used by this IF (excl. EP0) */
    uint8_t     bInterfaceClass;            /*!< Interface Class Code */
    uint8_t     bInterfaceSubClass;         /*!< Interface Subclass Code */
    uint8_t     bInterfaceProtocol;         /*!< Interface Protocol Code */
    uint8_t     iInterface;                 /*!< Index of String Desc for this Interface */
}usb_dsc_if_t;

/* Standard Endpoint Descriptor */
__packed typedef struct
{
    uint8_t     bLength;                    /*!< Size of this Descriptor in Bytes: 7 */
    uint8_t     bDescriptorType;            /*!< Descriptor Type (=USB_DSC_ENDPOINT) */
    uint8_t     bEndpointAddress;           /*!< The address of the endpoint on the USB device described by this descriptor. The address is encoded as follows: Bit 3...0: The endpoint number, Bit 6...4: Reserved, reset to zero, Bit 7: Direction, ignored for control endpoints, 0 = OUT endpoint, 1 = IN endpoint */
    uint8_t     bmAttributes;               /*!< This field describes the endpoint¡¯s attributes when it is configured using the bConfigurationValue. Transfer Type: 00 = Control, 01 = Isochronous, 10 = Bulk, 11 = Interrupt */
    uint16_t    wMaxPacketSize;             /*!< Maximum packet size this endpoint is capable of sending or receiving when this configuration is selected. */
    uint8_t     bInterval;                  /*!< Interval for polling endpoint for data transfers. Expressed in milliseconds. This field is ignored for bulk and control endpoints. For isochronous endpoints this field must be set to 1. For interrupt endpoints, this field may range from 1 to 255.  */
}usb_dsc_ep_t;

/* Standard Interface Association Descriptor, InterfaceAssociationDescriptor_ecn.pdf -> Table 9-Z */
__packed typedef struct
{
    uint8_t     bLength;                    /*!< Size of this descruptor in bytes: 8 */
    uint8_t     bDescriptorType;            /*!< INTERFACE ASSOCIATION Descriptor (USB_DSC_INTERFACE_ASSOCIATION) */
    uint8_t     bFirstInterface;            /*!< Interface number of the first interface that is associated with this function */
    uint8_t     bInterfaceCount;            /*!< Number of contiguous interfaces that are associated with this function */
    uint8_t     bFunctionClass;             /*!< Class code, 0x00 is not allowed, 0xFF means vendor-specific */
    uint8_t     bFunctionSubClass;          /*!< Subclass code */
    uint8_t     bFunctionProtocol;          /*!< Protocol code */
    uint8_t     iFunction;                  /*!< Index of string describing this function */
}usb_dsc_if_assoc_t;

/* USB Interface Class */
#define USB_IF_CLASS_AUDIO                          (1)
#define USB_IF_CLASS_AUDIO_SUBCLASS_AC              (1)     //AudioCtrl
#define USB_IF_CLASS_AUDIO_SUBCLASS_AS              (2)     //AudioStream
#define USB_IF_CLASS_HID                            (3)     //Human Interface Device (HID)
#define USB_IF_CLASS_MSD                            (8)     //MassStorageDevice
#define USB_IF_CLASS_APP                            (0xFE)  //Application specific

/* Standard Request Codes */
#define USB_GET_STATUS                              0x00    // wValue:Zero, wIndex:Zero/Interface/Endpoint, wLength:Two, Data: Device/Interface/Endpoint Status
#define USB_CLEAR_FEATURE                           0x01    // wValue:FeatureSelector, wIndex:Zero/Interface/Endpoint, wLength:Zero, Data: None
#define USB_SET_FEATURE                             0x03    // wValue:FeatureSelector, wIndex:Zero/Interface/Endpoint, wLength:Zero, Data: None
#define USB_SET_ADDRESS                             0x05    // wValue:DeviceAddress, wIndex:Zero, wLength:Zero, Data: None
#define USB_GET_DESCRIPTOR                          0x06    // wValue:DescriptorType and DescriptorIndex, wIndex:Zero or LanguageID, wLength:DescriptorLength, Data: Descriptor
#define USB_SET_DESCRIPTOR                          0x07    // wValue:DescriptorType and DescriptorIndex, wIndex:Zero or LanguageID, wLength:DescriptorLength, Data: Descriptor
#define USB_GET_CONFIGURATION                       0x08    // wValue:Zero, wIndex:Zero, wLength:One, Data: ConfigurationValue
#define USB_SET_CONFIGURATION                       0x09    // wValue:ConfigurationValue, wIndex:Zero, wLength:Zero, Data: None
#define USB_GET_INTERFACE                           0x0A    // wValue:Zero, wIndex:Interface, wLength:One, Data: AlternateInterface
#define USB_SET_INTERFACE                           0x0B    // wValue:AlternateSetting, wIndex:Interface, wLength:Zero, Data: None
#define USB_SYNCH_FRAME                             0x0C    // wValue:Zero, wIndex:Endpoint, wLength:Two, Data: FrameNumber
/* Standard Descriptor Types */
#define USB_DSC_DEVICE                              0x01    // Device Descriptor
#define USB_DSC_CONFIG                              0x02    // Configuration Descriptor
#define USB_DSC_STRING                              0x03    // String Descriptor
#define USB_DSC_INTERFACE                           0x04    // Interface Descriptor
#define USB_DSC_ENDPOINT                            0x05    // Endpoint Descriptor
#define USB_DSC_DEVICE_QUALIFIER                    0x06    // Device Qualifer Descriptor, only need when device support FS and HS, added in usb2.0
#define USB_DSC_OTHER_SPEED_CONFIG                  0x07    // Other Speed Configuration, only need when device support FS and HS, added in usb2.0
#define USB_DSC_INTERFACE_POWER                     0x08    // Other Speed Configuration, only need when device support FS and HS, added in usb2.0
#define USB_DSC_OTG                                 0x09
#define USB_DSC_DEBUG                               0x0A
#define USB_DSC_INTERFACE_ASSOCIATION               0x0B    // Interface Association Descriptor, added in usb2.0
/* Standard Feature Selectors */
#define USB_DEVICE_REMOTE_WAKEUP                    0x01    // Remote wakeup feature(not used), Recipient: Device
#define USB_ENDPOINT_HALT                           0x00    // Endpoint_Halt feature selector, Recipient: Endpoint

/* Bitmap definition of bmRequestType */
#define USB_SETUP_REQ_TYPE_STANDARD                 (0)     // Standard
#define USB_SETUP_REQ_TYPE_CLASS                    (1)     // Class
#define USB_SETUP_REQ_TYPE_VENDOR                   (2)     // Vendor
#define USB_SETUP_REQ_TYPE_BIT_OFFSET               (5)
#define USB_SETUP_REQ_DIRECTION_HOST2DEV            (0x00<<7)    // Data transfer direction, Host-to-device
#define USB_SETUP_REQ_DIRECTION_DEV2HOST            (0x01<<7)    // Data transfer direction, Device-to-host
#define USB_SETUP_REQ_RECIPIENT_DEVICE              (0x00<<0)    // Device
#define USB_SETUP_REQ_RECIPIENT_INTERFACE           (0x01<<0)    // Interface
#define USB_SETUP_REQ_RECIPIENT_ENDPOINT            (0x02<<0)    // Endpoint
#define USB_SETUP_REQ_RECIPIENT_OTHER               (0x03<<0)    // Other

#define USB_SETUP_REQ_SET_DEVICE                    (USB_SETUP_REQ_DIRECTION_HOST2DEV|(USB_SETUP_REQ_TYPE_STANDARD<<USB_SETUP_REQ_TYPE_BIT_OFFSET)|USB_SETUP_REQ_RECIPIENT_DEVICE)
#define USB_SETUP_REQ_GET_DEVICE                    (USB_SETUP_REQ_DIRECTION_DEV2HOST|(USB_SETUP_REQ_TYPE_STANDARD<<USB_SETUP_REQ_TYPE_BIT_OFFSET)|USB_SETUP_REQ_RECIPIENT_DEVICE)
#define USB_SETUP_REQ_SET_INTERFACE                 (USB_SETUP_REQ_DIRECTION_HOST2DEV|(USB_SETUP_REQ_TYPE_STANDARD<<USB_SETUP_REQ_TYPE_BIT_OFFSET)|USB_SETUP_REQ_RECIPIENT_INTERFACE)
#define USB_SETUP_REQ_GET_INTERFACE                 (USB_SETUP_REQ_DIRECTION_DEV2HOST|(USB_SETUP_REQ_TYPE_STANDARD<<USB_SETUP_REQ_TYPE_BIT_OFFSET)|USB_SETUP_REQ_RECIPIENT_INTERFACE)
#define USB_SETUP_REQ_SET_ENDPOINT                  (USB_SETUP_REQ_DIRECTION_HOST2DEV|(USB_SETUP_REQ_TYPE_STANDARD<<USB_SETUP_REQ_TYPE_BIT_OFFSET)|USB_SETUP_REQ_RECIPIENT_ENDPOINT)
#define USB_SETUP_REQ_GET_ENDPOINT                  (USB_SETUP_REQ_DIRECTION_DEV2HOST|(USB_SETUP_REQ_TYPE_STANDARD<<USB_SETUP_REQ_TYPE_BIT_OFFSET)|USB_SETUP_REQ_RECIPIENT_ENDPOINT)

/* Configuration characteristics of Standard Configuration Descriptor */
#define USB_CFG_DEFAULT                             (0x01<<7)   // Default Value (BIT7 set to one)
#define USB_CFG_SELFPOWER                           (0x01<<6)   // Self-powered (Supports if set)
#define USB_CFG_REMOTEWAKEUP                        (0x01<<5)   // Remote Wakeup (Supports if set)

/* Bitmap definition of endpoint direction */
#define USB_EP_DEV2HOST                             (0x80)      // Device to host
#define USB_EP_HOST2DEV                             (0x00)      // Host to device

/* Bitmap definnition of endpoint transfer type */
#define USB_EP_TRANSFER_TYPE_CONTROL                (0x00)
#define USB_EP_TRANSFER_TYPE_ISO                    (0x01)
#define USB_EP_TRANSFER_TYPE_BULK                   (0x02)
#define USB_EP_TRANSFER_TYPE_INT                    (0x03)

/* Bitmap definition of endpoint synchronization type */
#define USB_EP_ASYNCHRONOUS                         (0x01<<2)      // for spk HS, mic HS and FS
#define USB_EP_ADAPTIVE                             (0x02<<2)      // for spk HS and FS
#define USB_EP_SYNCHRONOUS                          (0x03<<2)      // for old device

/* Bitmap definition of endpoint usage type */
#define USB_EP_DATA                                 (0x00<<4)
#define USB_EP_FEEDBACK                             (0x01<<4)
#define USB_IMPLICIT_FEEDBACK_DATA                  (0x02<<4)

#ifdef __cplusplus
}
#endif

#endif
