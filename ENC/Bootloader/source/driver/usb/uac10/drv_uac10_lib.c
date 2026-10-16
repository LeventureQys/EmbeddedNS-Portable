#ifndef __DRV_UAC10_LIB_C__
#define __DRV_UAC10_LIB_C__

/* Includes -----------------------------------------------------------------*/
#include "lib_drv_uac10_config.h"
#include "lib_drv_uac10.h"
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "a0_hal_otg_device.h"

//#include "drv_uac10_hid.c"

#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "usb_device_standard.h"
#include "Headers\audio\drv_usb_device_descriptor_audio.h"
#include "Headers\hid\drv_usb_device_descriptor_hid.h"

extern void UsbHidOutIrqHandler(uint8_t* pDataMem, size_t size);
extern FUNCP UsbSpkCtrl(int32_t altSettingNum);
extern FUNCP UsbMicCtrl(int32_t altSettingNum);

/* Private variables ---------------------------------------------------------*/
uint32_t* pLibPara = NULL;
U32FUNCP TickGetFunc = NULL;//获取Tick时间
uint8_t usbIfAltSetting[3];

typedef struct {
    uint8_t Buffer[16];         //EP0 Buffer
    uint8_t OutPutBuf[64];      //HID Buffer
    uint8_t InPutBuf[64];       //HID Buffer
    FUNCP FunSpkDataRd;         //SpkCallback
    FUNCP FunMicDataWr;         //MicCallback
    uint32_t UsbConnectTime;    //USB Connect Time
    uint32_t SpkCallbackTime;   //SpkCallback Time
    uint32_t MicCallbackTime;   //MicCallback Time
    uint32_t UsbState;          //USB State
    uint8_t OutPutSize;         //HID OutPut Size
    uint8_t InPutSize;          //HID InPut Size

    uint8_t *pIfAltSetting;     //uint8_t usbIfAltSetting[3];

    uint32_t SpkSampleRateTar;  //Spk Sample Rate Target
    uint32_t SpkSampleRateCur;  //Spk Sample Rate Current
    uint32_t MicSampleRateTar;  //Mic Sample Rate Target
    uint32_t MicSampleRateCur;  //Mic Sample Rate Current

    uint8_t usbAudioMute[3][2];         //SpkMute MicMute SidetoneMute
    uint16_t usbAudioVolume[3][3][2];   //SpkVol MicVol SidetoneVol
    uint16_t usbAudioVolumeCfg[3][3];   //SpkVolRange MicVolRange SidetoneVolRange [Min, Max, Res]

    uint16_t sizeDscDev;            //size of descriptor device
    uint8_t* pDscDev;               //pointer to table of the descriptor device
    uint16_t sizeDscDevQualifier;   //size of descriptor descriptor device qualifier
    uint8_t* pDscDevQualifier;      //pointer to table of the descriptor device qualifier

    uint16_t* pSizeDscCfg;          //pointer to the size table of descriptor configration
    uint16_t* pSizeDscStr;          //pointer to the size table of descriptor string
    uint32_t* pAddrDscCfg;          //pointer to the address table of descriptor configration
    uint32_t* pAddrDscStr;          //pointer to the address table of descriptor string
    uint8_t* pAddrDscHid;           //pointer to the Hid descriptor address
    uint8_t* pDscHidReport;         //pointer to table of the descriptor hid report
    uint16_t sizeDscHidReport;      //size of descriptor hid report
    uint16_t* pSizeDscOtherSpeedCfg;
    uint32_t* pAddrDscOtherSpeedCfg;

    uint16_t PID;                   //PID码
    uint16_t VID;                   //VID码
    uint16_t deviceVer;             //版本号

    uint8_t dscCfg_bmAttributes;    //配置描述符的电流参数
    uint8_t ep0HaltState;           //端点0的halt状态
    uint8_t DevEp0RxDataSize;       //EP0 Buffe Size
    uint8_t numIf;                  //max number of interface
    uint8_t numDscCfg;              //max number of descriptor configration
    uint8_t numDscStr;              //max number of descriptor string

} usb_dev_info_t;

#define USB_STR_INDEX_MANUFACTURER                      (                         0 + USB_STR_MANUFACTURER_EN)
#define USB_STR_INDEX_PRODUCT                           (USB_STR_INDEX_MANUFACTURER + USB_STR_PRODUCT_EN)
#define USB_STR_INDEX_SERIALNUMBER                      (USB_STR_INDEX_PRODUCT      + USB_STR_SERIALNUMBER_EN)
#define USB_STR_INDEX_IF_AUDIO                          (USB_STR_INDEX_SERIALNUMBER + USB_STR_IF_AUDIO_EN)
#define USB_STR_INDEX_SIDETONE                          (USB_STR_INDEX_IF_AUDIO     + USB_STR_SIDETONE_EN)
#define USB_STR_INDEX_UCQ                               (USB_STR_INDEX_SIDETONE     + USB_STR_UCQ_EN)
#define USB_STR_INDEX_IF_MFI                            (USB_STR_INDEX_UCQ          + USB_STR_IF_MFI_EN)  
#define USB_STR_DESCRIPTORS                         	(USB_STR_INDEX_IF_MFI       + 1)//字符串描述符个数

#define USB_CFG_DESCRIPTORS                         	(1)//配置描述符个数

//设备描述符
static const usb_dsc_t usbDscDev =
{
    sizeof(usb_dsc_t),                  // bLength
    USB_DSC_DEVICE,                     // bDescriptorType
    USB_SPEC_VERSION,                   // Specification Version(bcd)
    0x00,                               // bDeviceClass
    0x00,                               // bDeviceSubClass
    0x00,                               // bDeviceProtocol
    OTG_MAX_EP0_PACK_SIZE,              // bMaxPacketSize0
    USB_DEFAULT_VID,                    // Vendor ID
    USB_DEFAULT_PID,                    // Product ID
    USB_DEVICE_VERSION,                 // bcdDevice release,Version
    USB_STR_INDEX_MANUFACTURER,         // iManufacturer
    USB_STR_INDEX_PRODUCT,              // iProduct
    USB_STR_INDEX_SERIALNUMBER,         // iSerialNumber
    USB_CFG_DESCRIPTORS,            	// bNumConfigurations
};

//设备限定描述符
const usb_dsc_qualifer_t usbDscDevQ =
{
    sizeof(usb_dsc_qualifer_t),         // bLength
    USB_DSC_DEVICE_QUALIFIER,           // bDescriptorType
    USB_SPEC_VERSION,                   // bcdUSB
    0,                                  // bDeviceClass
    0,                                  // bDeviceSubClass
    0,                                  // bDeviceProtocol
    OTG_MAX_EP0_PACK_SIZE,              // bMaxPacketSize0
    1,                                  // bNumConfigurations
    0,                                  // bReserved
};

//HID报告描述符
const uint8_t usbDscHidReport[]={
#if USB_HID_COL_PLAYER_CTRL_SPK_MUTE_EN  // 播放器HID控制支持SPK-MUTE
  0x05, 0x0C,       // Usage Page (Consumer)
  0x09, 0x01,       // Usage (Consumer Control)
  0xA1, 0x01,           // Collection (Application)
  0x15, 0x00,                   // Logical Minimum (0)
  0x25, 0x01,                   // Logical Maximum (1)
  0x75, 0x01,                   // Report Size (1)
  0x85, USB_HID_IN_PLAY_CTRL_ID,// Report ID (0x03)
  0x95, 0x08,                   // Report Count (8)
  0x09, 0xE9,                   // Usage (Volume Increment)
  0x09, 0xEA,                   // Usage (Volume Decrement)
  0x09, 0xCD,                   // Usage (Play/Pause)
  0x09, 0xB7,                   // Usage (Stop)
  0x09, 0xB5,                   // Usage (Scan Next Track)
  0x09, 0xB6,                   // Usage (Scan Previous Track)
  0x09, 0xB3,                   // Usage (Fast Forward)
  0x09, 0xB4,                   // Usage (Rewind)
  0x81, 0x02,                   // Input (Var)
  0x85, USB_HID_IN_SPK_MUTE_ID, // Report ID (0x05)
  0x95, 0x01,                   // Report Count (1)
  0x09, 0xE2,                   // Usage (Mute)
  0x81, 0x02,                   // Input (Var)
  0x95, 0x07,                   // Report Count (7)
  0x81, 0x01,                   // Input (Const)
  0xC0,                 // End Collection
#endif
#if USB_HID_COL_UC_CTRL_EN  // UC通信类HID控制
  0x05, 0x0B,       // Usage Page (Telephony)
  0x09, 0x05,       // Usage (Headset)
  0xA1, 0x01,           // Collection (Application)
  0x15, 0x00,                   // Logical Minimum (0)
  0x25, 0x01,                   // Logical Maximum (1)
  0x75, 0x01,                   // Report Size (1)
  0x85, USB_HID_IN_TEL_ID,      // Report ID (0x02)
  0x95, 0x01,                   // Report Count (1)
  0x09, 0x24,                   // Usage (Redial)
  0x81, 0x06,                   // Input (Var, Rel)
  0x95, 0x01,                   // Report Count (1)
  0x09, 0x20,                   // Usage (Hook Switch)
  0x81, 0x22,                   // Input (Var, PrefState)
  0x95, 0x03,                   // Report Count (3)
  0x09, 0x2F,                   // Usage (Phone Mute)
  0x09, 0x21,                   // Usage (Flash)
  0x09, 0x23,                   // Usage (Hold)
  0x81, 0x06,                   // Input (Var, Rel)
  0x95, 0x02,                   // Report Count (2)
  0x09, 0x27,                   // Usage (Park)
  0x09, 0x97,                   // Usage (Line Busy Tone)
  0x81, 0x02,                   // Input (Var)
  0x95, 0x01,                   // Report Count (1)
  0x09, 0x07,                   // Usage (Programmable Button)
  0x05, 0x09,                   // Usage Page (Buttons)
  0x81, 0x02,                   // Input (Var)
  0x85, USB_HID_OUT_LED_ID,     // Report ID (0x04)
  0x95, 0x04,                   // Report Count (4)
  0x05, 0x08,                   // Usage Page (LEDs)
  0x09, 0x17,                   // Usage (Off-Hook)
  0x09, 0x09,                   // Usage (Mute)
  0x09, 0x18,                   // Usage (Ring)
  0x09, 0x20,                   // Usage (Hold)
  0x91, 0x02,                   // Output (Var)
  0x95, 0x04,                   // Report Count (4)
  0x91, 0x03,                   // Output (Const, Var)
  0x85, USB_HID_TEL_VENDER_ID,  // Report ID (0x01)
  0x75, 0x08,                   // Report Size (8)
  0x95, 0x0F,                   // Report Count (15)
  0x06, 0xFF, 0x07,             // Usage Page (unk)
  0x09, 0x00,                   // Usage (unk)
  0x81, 0x06,                   // Input (Var, Rel)
  0x09, 0x00,                   // Usage (unk)
  0x91, 0x02,                   // Output (Var)
  0xC0,                 // End Collection
#endif
#if USB_HID_COL_PLAYER_CTRL_EN  // 播放器HID控制
  0x06, 0x99, 0xFF, // Usage Page (Vendor Defined)
  0x09, 0x01,       // Usage (unk)
  0xA1, 0x01,           // Collection (Application)
  0x85, USB_HID_IN_ASP_ID,      // Report ID (0x9B)
  0x15, 0x00,                   // Logical Minimum (0)
  0x25, 0x01,                   // Logical Maximum (1)
  0x75, 0x01,                   // Report Size (1)
  0x95, 0x01,                   // Report Count (1)
  0x09, 0x04,                   // Usage (unk)
  0x81, 0x06,                   // Input (Var, Rel)
  0x75, 0x01,                   // Report Size (1)
  0x95, 0x07,                   // Report Count (7)
  0x81, 0x01,                   // Input (Const)
  0x85, USB_HID_OUT_ASP_ID,     // Report ID (0x9A)
  0x15, 0x00,                   // Logical Minimum (0)
  0x26, 0xFF, 0x00,             // Logical Maximum (255)
  0x75, 0x08,                   // Report Size (8)
  0x09, 0x03,                   // Usage (unk)
  0x95, 0x0F,                   // Report Count (15)
  0xB2, 0x02, 0x01,             // Feature (Var, BuffBytes)
  0xC0,                 // End Collection
#endif
#if USB_HID_COL_USER_TRANS_EN// 自定义HID传输
  0x06, 0x01, 0xFF, // Usage Page (Vendor Defined)
  0x09, 0x01,       // Usage (unk)
  0xA1, 0x01,           // Collection (Application)
  0xA1, 0x02,               // Collection (Logical)
  0x15, 0x80,                    // Logical Minimum (-128)
  0x25, 0x7F,                    // Logical Maximum (127)
  0x75, 0x08,                    // Report Size (8)
  0x85, USB_HID_IN_ID0,          // Report ID (0x10)
  0x95, 0x0F,                    // Report Count (15)
  0x09, 0x44,                    // Usage (unk)
  0x81, 0x22,                    // Input (Var, PrefState)
  0x85, USB_HID_IN_ID1,          // Report ID (0x11)
  0x95, 0x3F,                    // Report Count (63)
  0x09, 0x45,                    // Usage (unk)
  0x81, 0x22,                    // Input (Var, PrefState)
  0x85, USB_HID_OUT_ID0,         // Report ID (0x50)
  0x95, 0x0F,                    // Report Count (15)
  0x09, 0x54,                    // Usage (unk)
  0x91, 0x22,                    // Output (Var, PrefState)
  0x85, USB_HID_OUT_ID1,         // Report ID (0x51)
  0x95, 0x3F,                    // Report Count (63)
  0x09, 0x55,                    // Usage (unk)
  0x91, 0x22,                    // Output (Var, PrefState)
  0xC0,                     // End Collection
  0xC0,                 // End Collection
#endif
};


//__packed typedef struct
//{
//	usb_dsc_audio_cs_if_header_head_t	mHead;
//    uint8_t     						baInterfaceNr[UAC_IF_ID_STREAMING_NUM];    // Interface ID value.
//}usb_dsc_audio_cs_if_header_t;


//__packed typedef struct
//{
//    usb_dsc_audio_cs_if_header_t         	dscCsIfHeader;
//#if USB_FUN_SPK_EN
//    usb_dsc_audio_cs_if_in_terminal_t    	dscCsIfInTerminal0; //IT_SPK,   
//    usb_dsc_audio_cs_if_feature_unit_t   	dscCsIfFeatureUnit0;
//    usb_dsc_audio_cs_if_out_terminal_t   	dscCsIfOutTerminal0;//MU_Mixer ->FU_Volume0 -> OT_SPK
//#endif
//#if USB_FUN_MIC_EN
//    usb_dsc_audio_cs_if_in_terminal_t    	dscCsIfInTerminal1; 
//    usb_dsc_audio_cs_if_feature_unit_t   	dscCsIfFeatureUnit1;
//    usb_dsc_audio_cs_if_out_terminal_t   	dscCsIfOutTerminal1;//IT_MIC -> FU_Volume1 -> OT_MIC
//#endif
//#if UAC_ID_SIDETONE_UNIT_ENABLE 
//    usb_dsc_audio_cs_if_in_terminal_t    	dscCsIfInTerminal2; 
//    usb_dsc_audio_cs_if_feature_unit_t   	dscCsIfFeatureUnit2;//IT_SideTone -> FU_Volume2
//    usb_dsc_audio_cs_if_mixer_unit_t     	dscCsIfMixerUnit0;  //IT_SPK + FU_Volume2 -> MU_Mixer 
//#endif
//}usb_dsc_cs_if_ac_t;

//__packed typedef struct
//{
//    usb_dsc_if_t                        	dscIf;
//    usb_dsc_cs_if_ac_t                  	dscCsIfAc;
//}usb_dsc_cs_if_audio_ctrl_t;

__packed typedef struct
{
    usb_dsc_if_t                            dscIf;
    usb_dsc_cs_if_as_general_t              dscIfAltSetAsGeneral;
    usb_dsc_cs_if_as_format_type_t          dscIfAltSetAsFormatType;
    usb_dsc_cs_if_endpoint_t                dscIfAltSetEndpoint;
    usb_dsc_cs_if_as_endpoint_t             dscIfAltSetAsEndpoint;
}usb_dsc_cs_if_as_alt_setting_all_t;

/* Class-Specific Audio Spk Interface */
__packed typedef struct
{
    usb_dsc_if_t                            dscIf;                                            /* Interface:ID(Speaker), Alt.Setting:0 */
    usb_dsc_cs_if_as_alt_setting_all_t      dscIfSpkAltSetting[UAC_IF_ALT_SETTING_SPK_NUM];   /* Interface:ID(Speaker), Alt.Setting:1~USB_AUDIO_IF_ALT_SETTING_SPK_NUM */    
}usb_dsc_cs_if_audio_spk_t;

/* Class-Specific Audio Mic Interface */
__packed typedef struct
{
    usb_dsc_if_t                            dscIf;                                            /* Interface:ID(Mic), Alt.Setting:0 */
    usb_dsc_cs_if_as_alt_setting_all_t      dscIfMicAltSetting[UAC_IF_ALT_SETTING_MIC_NUM];   /* Interface:ID(Mic), Alt.Setting:1~USB_AUDIO_IF_ALT_SETTING_MIC_NUM */    
}usb_dsc_cs_if_audio_mic_t;

//配置描述符-结构体
__packed typedef struct
{
    usb_dsc_cfg_t                       	dscCfg;
    //Audio
//    usb_dsc_cs_if_audio_ctrl_t         		dscCsIfCtrl;
#if USB_FUN_SPK_EN
    usb_dsc_cs_if_audio_spk_t           	dscCsIfSpk;
#endif
#if USB_FUN_MIC_EN
    usb_dsc_cs_if_audio_mic_t           	dscCsIfMic;
#endif
#if USB_FUN_HID_EN //HID
    usb_dsc_cs_if_hid_t                     dscCsIfHid;
#endif
}usb_dsc_cfg_audio_t;

//配置描述符
const usb_dsc_cfg_audio_t usbDscCfgAll =
{
    .dscCfg = 
    {
        .bLength                            = sizeof(usb_dsc_cfg_t),
        .bDescriptorType                    = USB_DSC_CONFIG,
        .wTotalLength                       = sizeof(usb_dsc_cfg_audio_t),
        .bNumInterfaces                     = UAC_IF_ID_NUM,
        .bConfigurationValue                = 0x01,
        .iConfiguration                     = 0x00,
        .bmAttributes                       = USB_CFG_ATTRIBUTES,
        .bMaxPower                          = (USB_CFG_MAXPOWER/2),
    },
#if 0
    .dscCsIfCtrl = 
    {
        .dscIf = 
        {
            .bLength 						= sizeof(usb_dsc_if_t),
            .bDescriptorType 				= USB_DSC_INTERFACE,
            .bInterfaceNumber 				= UAC_IF_ID_CONTROL,
            .bAlternateSetting 				= 0x00,
            .bNumEndpoints 					= 0x00,
            .bInterfaceClass 				= UAC_AUDIO,
            .bInterfaceSubClass 			= UAC_AUDIOCONTROL,
            .bInterfaceProtocol 			= UAC_PR_PROTOCOL_UNDEFINED,
            .iInterface 					= 0x00,
        },
        .dscCsIfAc = 
        {
			.dscCsIfHeader = 
            {
                .mHead.bLength 				= sizeof(usb_dsc_audio_cs_if_header_t),
                .mHead.bDescriptorType 		= UAC_CS_INTERFACE,
                .mHead.bDescriptorSubtype 	= UAC_HEADER,
                .mHead.bcdADC 				= UAC_SPEC_VERSION,
                .mHead.wTotalLength 		= sizeof(usb_dsc_cs_if_ac_t),
                .mHead.bInCollection 		= UAC_IF_ID_STREAMING_NUM,
                .baInterfaceNr = {
                    #if USB_FUN_SPK_EN
                        UAC_IF_ID_STREAMING_SPK,   
                    #endif
                    #if USB_FUN_MIC_EN
                        UAC_IF_ID_STREAMING_MIC,
                    #endif
                },
            },
#if USB_FUN_SPK_EN        
            .dscCsIfInTerminal0 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_in_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_INPUT_TERMINAL,
                .bTerminalID 				= UAC_ID_INPUT_TERMINAL_SPEAKER,
                .wTerminalType 				= UAC_TT_USB_STREAMING,
                .bAssocTerminal 			= 0x00,
                .bNrChannels 				= 0x02,
                .wChannelConfig 			= 0x0003,       //(Left, Right)
                .iChannelNames 				= 0x00,
                .iTerminal 					= 0x00,
            },
            .dscCsIfFeatureUnit0 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_feature_unit_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_FEATURE_UNIT,
                .bUnitID 					= UAC_ID_FEATURE_UNIT_SPEAKER,
                .bSourceID 					= (UAC_ID_SIDETONE_UNIT_ENABLE)?(UAC_ID_MIXER_UNIT_WITH_SIDETONE):(UAC_ID_INPUT_TERMINAL_SPEAKER),
                .bControlSize 				= 0x01,
                .bmaControls = {
                    UAC_FU_MUTE_CONTROL,
                    UAC_FU_VOLUME_CONTROL,
                    UAC_FU_VOLUME_CONTROL,                
                },
                .iFeature 					= 0x00,
            },
            .dscCsIfOutTerminal0 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_out_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_OUTPUT_TERMINAL,
                .bTerminalID 				= UAC_ID_OUTPUT_TERMINAL_SPEAKER,
                .wTerminalType 				= UAC_ID_OUTPUT_TERMINAL_SPEAKER_TYPE,//UAC_TT_OUTPUT_HEADPHONES,
                .bAssocTerminal 			= 0x00,
                .bSourceID 					= UAC_ID_FEATURE_UNIT_SPEAKER,
                .iTerminal 					= 0x00,
            },
#endif
#if USB_FUN_MIC_EN 
            .dscCsIfInTerminal1 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_in_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_INPUT_TERMINAL,
                .bTerminalID 				= UAC_ID_INPUT_TERMINAL_MICROPHONE,
                .wTerminalType 				= UAC_ID_INPUT_TERMINAL_MICROPHONE_TYPE,//UAC_TT_INPUT_MICROPHONE,
                .bAssocTerminal 			= 0x00,
                .bNrChannels 				= 0x02,
                .wChannelConfig 			= 0x0003,       //(Left, Right)
                .iChannelNames 				= 0x00,
                .iTerminal 					= 0x00,
            },
            .dscCsIfFeatureUnit1 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_feature_unit_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_FEATURE_UNIT,
                .bUnitID 					= UAC_ID_FEATURE_UNIT_MICROPHONE,
                .bSourceID 					= UAC_ID_INPUT_TERMINAL_MICROPHONE,
                .bControlSize 				= 0x01,
                .bmaControls = {
                    UAC_FU_MUTE_CONTROL,
                    UAC_FU_VOLUME_CONTROL,
                    UAC_FU_VOLUME_CONTROL,               
                },
                .iFeature 					= 0x00,
            },
            .dscCsIfOutTerminal1 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_out_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_OUTPUT_TERMINAL,
                .bTerminalID 				= UAC_ID_OUTPUT_TERMINAL_MICROPHONE,
                .wTerminalType 				= UAC_TT_USB_STREAMING,
                .bAssocTerminal 			= 0x00,
                .bSourceID 					= UAC_ID_FEATURE_UNIT_MICROPHONE,
                .iTerminal 					= 0x00,
            },
#endif
#if UAC_ID_SIDETONE_UNIT_ENABLE 
            .dscCsIfInTerminal2 =
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_in_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_INPUT_TERMINAL,
                .bTerminalID 				= UAC_ID_INPUT_TERMINAL_SIDETONE,
                .wTerminalType 				= UAC_ID_INPUT_TERMINAL_MICROPHONE_TYPE,//UAC_TT_INPUT_MICROPHONE,
                .bAssocTerminal 			= 0x00,
                .bNrChannels 				= 0x02,
                .wChannelConfig 			= 0x0003,       //(Left, Right)
                .iChannelNames 				= 0x00,
                .iTerminal 					= USB_STR_INDEX_SIDETONE,
            },
            .dscCsIfFeatureUnit2 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_feature_unit_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_FEATURE_UNIT,
                .bUnitID 					= UAC_ID_FEATURE_UNIT_SIDETONE,
                .bSourceID 					= UAC_ID_INPUT_TERMINAL_SIDETONE,
                .bControlSize 				= 0x01,
                .bmaControls = {
                    UAC_FU_MUTE_CONTROL,
                    UAC_FU_VOLUME_CONTROL,
                    UAC_FU_VOLUME_CONTROL,
                },
                .iFeature 					= 0x00,
            },
            .dscCsIfMixerUnit0 =
            {
                .bLength                    = sizeof(usb_dsc_audio_cs_if_mixer_unit_t),         
                .bDescriptorType            = UAC_CS_INTERFACE,    
                .bDescriptorSubtype         = UAC_MIXER_UNIT, 
                .bUnitID                    = UAC_ID_MIXER_UNIT_WITH_SIDETONE,            
                .bNrInPins                  = UAC_MU_IN_NUM,          
                .baSourceID = {
                    UAC_ID_FEATURE_UNIT_SIDETONE,
                    UAC_ID_INPUT_TERMINAL_SPEAKER,           
                },      
                .bNrChannels 				= 0x02,       
                .wChannelConfig 			= 0x0003,       //(Left, Right)   
                .iChannelNames              = 0x00,    
                .bmControls                 = 0x00,      
                .iMixer                     = 0x00,          
            },
#endif
        },
    },
#if USB_FUN_SPK_EN
    .dscCsIfSpk = 
    {
        .dscIf = 
        {/* Alt.Setting: 0 */
            .bLength 						= sizeof(usb_dsc_if_t),
            .bDescriptorType 				= USB_DSC_INTERFACE,
            .bInterfaceNumber 				= UAC_IF_ID_STREAMING_SPK,
            .bAlternateSetting 				= UAC_IF_ZERO_BANDWIDTH,
            .bNumEndpoints 					= 0x00,
            .bInterfaceClass 				= UAC_AUDIO,
            .bInterfaceSubClass 			= UAC_AUDIOSTREAMING,
            .bInterfaceProtocol 			= UAC_PR_PROTOCOL_UNDEFINED,
            .iInterface 					= 0x00,
        },
        .dscIfSpkAltSetting = 
        {
            {/* Alt.Setting: 1 */
                .dscIf = 
                {
                    .bLength 				= sizeof(usb_dsc_if_t),
                    .bDescriptorType 		= USB_DSC_INTERFACE,
                    .bInterfaceNumber 		= UAC_IF_ID_STREAMING_SPK,
                    .bAlternateSetting 		= 0x01,
                    .bNumEndpoints 			= 0x01,
                    .bInterfaceClass 		= UAC_AUDIO,
                    .bInterfaceSubClass 	= UAC_AUDIOSTREAMING,
                    .bInterfaceProtocol 	= UAC_PR_PROTOCOL_UNDEFINED,
                    .iInterface 			= 0x00,
                },
                .dscIfAltSetAsGeneral = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_general_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_AS_GENERAL,
                    .bTerminalLink 			= UAC_ID_INPUT_TERMINAL_SPEAKER,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
                    .bNrChannels 			= UAC_IF_ALT_SET_1_SPK_CH,
                    .bSubFrameSize 			= UAC_IF_ALT_SET_1_SPK_BYTE_SIZE,
                    .bBitResolution 		= UAC_IF_ALT_SET_1_SPK_BIT,
                    .bSamFreqType 			= UAC_IF_ALT_SET_1_SPK_FS_NUM,
                    .tSamFreq               = UAC_SPK_IF_ALT_SET_FS_ALL,
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= UAC_EP_ADDR_SPK,
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_ADAPTIVE),
                    .wMaxPacketSize 		= UAC_IF_ALT_SET_1_SPK_PACK_SIZE,
                    .bInterval 				= (USB_SPEED_SEL_FS)?(0x01):(0x04),//Full-0x01, Hight-0x04
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x01,
                    .bLockDelayUnits 		= 0x00,
                    .wLockDelay 			= 0x0000,
                },
            },
            {/* Alt.Setting: 2 */
                .dscIf = 
                {
                    .bLength 				= sizeof(usb_dsc_if_t),
                    .bDescriptorType 		= USB_DSC_INTERFACE,
                    .bInterfaceNumber 		= UAC_IF_ID_STREAMING_SPK,
                    .bAlternateSetting 		= 0x02,
                    .bNumEndpoints 			= 0x01,
                    .bInterfaceClass 		= UAC_AUDIO,
                    .bInterfaceSubClass 	= UAC_AUDIOSTREAMING,
                    .bInterfaceProtocol 	= UAC_PR_PROTOCOL_UNDEFINED,
                    .iInterface 			= 0x00,
                },
                .dscIfAltSetAsGeneral = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_general_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_AS_GENERAL,
                    .bTerminalLink 			= UAC_ID_INPUT_TERMINAL_SPEAKER,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
                    .bNrChannels 			= UAC_IF_ALT_SET_2_SPK_CH,
                    .bSubFrameSize 			= UAC_IF_ALT_SET_2_SPK_BYTE_SIZE,
                    .bBitResolution 		= UAC_IF_ALT_SET_2_SPK_BIT,
                    .bSamFreqType 			= UAC_IF_ALT_SET_2_SPK_FS_NUM,
                    .tSamFreq               = UAC_SPK_IF_ALT_SET_FS_ALL,
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= UAC_EP_ADDR_SPK,
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_ADAPTIVE),
                    .wMaxPacketSize 		= (UAC_IF_ALT_SET_2_SPK_PACK_SIZE>1024)?(0x800|(UAC_IF_ALT_SET_2_SPK_PACK_SIZE/2))/*0x0A40*/:(UAC_IF_ALT_SET_2_SPK_PACK_SIZE),
                    .bInterval 				= (USB_SPEED_SEL_FS)?(0x01):(0x04),//Full-0x01, Hight-0x04
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint 		= 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x01,
                    .bLockDelayUnits 		= 0x00,
                    .wLockDelay 			= 0x0000,
                },
            },
        }
    },
#endif
#if USB_FUN_MIC_EN
    .dscCsIfMic = 
    {
        .dscIf = 
        {/* Alt.Setting: 0 */
            .bLength 						= sizeof(usb_dsc_if_t),
            .bDescriptorType 				= USB_DSC_INTERFACE,
            .bInterfaceNumber 				= UAC_IF_ID_STREAMING_MIC,
            .bAlternateSetting 				= UAC_IF_ZERO_BANDWIDTH,
            .bNumEndpoints 					= 0x00,
            .bInterfaceClass 				= UAC_AUDIO,
            .bInterfaceSubClass 			= UAC_AUDIOSTREAMING,
            .bInterfaceProtocol 			= UAC_PR_PROTOCOL_UNDEFINED,
            .iInterface 					= 0x00,
        },
        .dscIfMicAltSetting = 
        {
            {/* Alt.Setting: 1 */
                .dscIf = 
                {
                    .bLength 				= sizeof(usb_dsc_if_t),
                    .bDescriptorType 		= USB_DSC_INTERFACE,
                    .bInterfaceNumber 		= UAC_IF_ID_STREAMING_MIC,
                    .bAlternateSetting 		= 0x01,
                    .bNumEndpoints 			= 0x01,
                    .bInterfaceClass 		= UAC_AUDIO,
                    .bInterfaceSubClass 	= UAC_AUDIOSTREAMING,
                    .bInterfaceProtocol 	= UAC_PR_PROTOCOL_UNDEFINED,
                    .iInterface 			= 0x00,
                },
                .dscIfAltSetAsGeneral = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_general_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_AS_GENERAL,
                    .bTerminalLink 			= UAC_ID_OUTPUT_TERMINAL_MICROPHONE,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
                    .bNrChannels 			= UAC_IF_ALT_SET_1_MIC_CH,
                    .bSubFrameSize 			= UAC_IF_ALT_SET_1_MIC_BYTE_SIZE,
                    .bBitResolution 		= UAC_IF_ALT_SET_1_MIC_BIT,
                    .bSamFreqType 			= UAC_IF_ALT_SET_1_MIC_FS_NUM,
                    .tSamFreq               = UAC_MIC_IF_ALT_SET_FS_ALL,
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= UAC_EP_ADDR_MIC,
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_ASYNCHRONOUS),
                    .wMaxPacketSize 		= UAC_IF_ALT_SET_1_MIC_PACK_SIZE,
                    .bInterval 				= (USB_SPEED_SEL_FS)?(0x01):(0x04),//Full-0x01, Hight-0x04
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x01,
                    .bLockDelayUnits 		= 0x00,
                    .wLockDelay 			= 0x0000,
                },
            },
            {/* Alt.Setting: 2 */
                .dscIf = 
                {
                    .bLength 				= sizeof(usb_dsc_if_t),
                    .bDescriptorType 		= USB_DSC_INTERFACE,
                    .bInterfaceNumber 		= UAC_IF_ID_STREAMING_MIC,
                    .bAlternateSetting 		= 0x02,
                    .bNumEndpoints 			= 0x01,
                    .bInterfaceClass 		= UAC_AUDIO,
                    .bInterfaceSubClass 	= UAC_AUDIOSTREAMING,
                    .bInterfaceProtocol 	= UAC_PR_PROTOCOL_UNDEFINED,
                    .iInterface 			= 0x00,
                },
                .dscIfAltSetAsGeneral = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_general_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_AS_GENERAL,
                    .bTerminalLink 			= UAC_ID_OUTPUT_TERMINAL_MICROPHONE,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
                    .bNrChannels 			= UAC_IF_ALT_SET_2_MIC_CH,
                    .bSubFrameSize 			= UAC_IF_ALT_SET_2_MIC_BYTE_SIZE,
                    .bBitResolution 		= UAC_IF_ALT_SET_2_MIC_BIT,
                    .bSamFreqType 			= UAC_IF_ALT_SET_2_MIC_FS_NUM,
                    .tSamFreq               = UAC_MIC_IF_ALT_SET_FS_ALL,
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= UAC_EP_ADDR_MIC,
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_ASYNCHRONOUS),
                    .wMaxPacketSize 		= (UAC_IF_ALT_SET_2_MIC_PACK_SIZE>1024)?(0x800|(UAC_IF_ALT_SET_2_MIC_PACK_SIZE/2))/*0x0A40*/:(UAC_IF_ALT_SET_2_MIC_PACK_SIZE),
                    .bInterval 				= (USB_SPEED_SEL_FS)?(0x01):(0x04),//Full-0x01, Hight-0x04
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x01,
                    .bLockDelayUnits 		= 0x00,
                    .wLockDelay 			= 0x0000,
                },
            },
        }
    },
#endif
#endif
#if USB_FUN_HID_EN
    .dscCsIfHid = 
    {
        .dscIf = 
        {
            .bLength 			= sizeof(usb_dsc_if_t),
            .bDescriptorType 	= USB_DSC_INTERFACE,
            .bInterfaceNumber 	= UAC_IF_ID_HID,
            .bAlternateSetting 	= 0x00,
            .bNumEndpoints 		= 0x02,
            .bInterfaceClass 	= HID_CLASS,
            .bInterfaceSubClass = HID_SUBCLASS_NONE,
            .bInterfaceProtocol = HID_PROTOCOL_NONE,
            .iInterface 		= 0x00,
        },
        .dscHid = 
        {
            .bLength 			= sizeof(usb_dsc_class_hid_t),
            .bDescriptorType 	= HID_DSC_HID,
            .bcdHID 			= HID_SPEC_VERSION,
            .bCountryCode 		= 0x00,
            .bNumDescriptors 	= HID_CLASS_DESCRIPTORS_NUM,
            .descriptor = {
                {
                    .bDescriptorType 	= HID_DSC_REPORT,
                    .wDescriptorLength 	= sizeof(usbDscHidReport),
                },
            },
        },
        .dscEpIn = 
        {
            .bLength 			= sizeof(usb_dsc_ep_t),
            .bDescriptorType 	= USB_DSC_ENDPOINT,
            .bEndpointAddress 	= UAC_EP_ADDR_HID_IN,
            .bmAttributes 		= USB_EP_TRANSFER_TYPE_INT,
            .wMaxPacketSize 	= USB_EP_PACK_SIZE_HID,
            .bInterval 			= 0x02,//USB_HID_POLL_INTERVAL
        },
        .dscEpOut = 
        {
            .bLength 			= sizeof(usb_dsc_ep_t),
            .bDescriptorType 	= USB_DSC_ENDPOINT,
            .bEndpointAddress 	= UAC_EP_ADDR_HID_OUT,
            .bmAttributes 		= USB_EP_TRANSFER_TYPE_INT,
            .wMaxPacketSize 	= USB_EP_PACK_SIZE_HID,
            .bInterval 			= 0x02,//USB_HID_POLL_INTERVAL,
        },
    },
#endif
};

static const uint16_t audioConfigDescriptorSizeTable[USB_CFG_DESCRIPTORS] = //配置描述符大小表
{
    sizeof(usbDscCfgAll),
};
static const uint32_t audioConfigDescriptorAddressTable[USB_CFG_DESCRIPTORS] = //配置描述符地址表
{
    (uint32_t)&usbDscCfgAll,
};

//语言ID描述符
#define _USB_STR_DESC_LEN_LANG  (4)
const uint8_t usbDscStrLanguage[_USB_STR_DESC_LEN_LANG] ={
    0x04,
    USB_DSC_STRING,
    0x09,
    0x04,
};
//字符串描述符
#define _USB_STR_DESC_LEN_MANUFACTURER  (sizeof(L"Maono"))
const uint16_t usbDscStrManufacturer[_USB_STR_DESC_LEN_MANUFACTURER] ={
    (sizeof(L"Maono")|(USB_DSC_STRING<<8)),
    L'M',
    L'a',
    L'o',
    L'n',
    L'o',
};

#define _USB_STR_DESC_LEN_PRODUCT   (sizeof(L"Maono PD100 Mic"))
const uint16_t usbDscStrProduct[_USB_STR_DESC_LEN_PRODUCT] ={
    (sizeof(L"Maono PD100 Mic")|(USB_DSC_STRING<<8)),
    L'M',
    L'a',
    L'o',
    L'n',
    L'o',
    L' ',
    L'P',
    L'D',
    L'1',
    L'0',
    L'0',
    L' ',
    L'M',
    L'i',
    L'c',
};

#define _USB_STR_DESC_LEN_SN    (sizeof(L"20251001"))
const uint16_t usbDscStrSerialNumber[_USB_STR_DESC_LEN_PRODUCT] ={
    (sizeof(L"20251001")|(USB_DSC_STRING<<8)),
    L'2',
    L'0',
    L'2',
    L'5',
    L'1',
    L'0',
    L'0',
    L'1',
};

#define _USB_STR_DESC_LEN_IF_AUDIO  (sizeof(L"If Audio"))
const uint16_t usbDscStrIfAudio[_USB_STR_DESC_LEN_IF_AUDIO] ={
    (sizeof(L"If Audio")|(USB_DSC_STRING<<8)),
    L'I',
    L'f',
    L' ',
    L'A',
    L'u',
    L'd',
    L'i',
    L'o',
};

#define _USB_STR_DESC_LEN_SIDETONE  (sizeof(L"Sidetone"))
const uint16_t usbDscStrSidetone[_USB_STR_DESC_LEN_SIDETONE] ={
    (sizeof(L"Sidetone")|(USB_DSC_STRING<<8)),
    L'S',
    L'i',
    L'd',
    L'e',
    L't',
    L'o',
    L'n',
    L'e',
};

#define _USB_STR_DESC_LEN_UCQ  (sizeof(L"UCQ"))
const uint16_t usbDscStrUCQ[_USB_STR_DESC_LEN_UCQ] ={
    (sizeof(L"UCQ")|(USB_DSC_STRING<<8)),
    L'U',
    L'C',
    L'Q',
};

#define _USB_STR_DESC_LEN_MFI  (sizeof(L"MFI"))
const uint16_t usbDscStrMFI[_USB_STR_DESC_LEN_MFI] ={
    (sizeof(L"MFI")|(USB_DSC_STRING<<8)),
    L'M',
    L'F',
    L'I',
};

static const uint16_t audioStringDescriptorSizeTable[USB_STR_DESCRIPTORS] = 
{
    _USB_STR_DESC_LEN_LANG,
#if(USB_STR_MANUFACTURER_EN) 
    _USB_STR_DESC_LEN_MANUFACTURER,
#endif
#if(USB_STR_PRODUCT_EN) 
    _USB_STR_DESC_LEN_PRODUCT,
#endif
#if(USB_STR_SERIALNUMBER_EN) 
    _USB_STR_DESC_LEN_SN,
#endif
#if(USB_STR_IF_AUDIO_EN) 
    _USB_STR_DESC_LEN_IF_AUDIO,
#endif
#if(USB_STR_SIDETONE_EN) 
    _USB_STR_DESC_LEN_SIDETONE,
#endif
#if(USB_STR_UCQ_EN) 
    _USB_STR_DESC_LEN_UCQ,
#endif
#if(USB_STR_IF_MFI_EN) 
    _USB_STR_DESC_LEN_MFI,
#endif
};

static const uint32_t audioStringDescriptorAddressTable[USB_STR_DESCRIPTORS] = 
{
    (uint32_t)&usbDscStrLanguage[0],
#if(USB_STR_MANUFACTURER_EN) 
    (uint32_t)&usbDscStrManufacturer[0],
#endif
#if(USB_STR_PRODUCT_EN) 
    (uint32_t)&usbDscStrProduct[0],
#endif
#if(USB_STR_SERIALNUMBER_EN) 
    (uint32_t)&usbDscStrSerialNumber[0],
#endif
#if(USB_STR_IF_AUDIO_EN) 
    (uint32_t)&usbDscStrIfAudio[0],
#endif
#if(USB_STR_SIDETONE_EN) 
    (uint32_t)&usbDscStrSidetone[0],
#endif
#if(USB_STR_UCQ_EN) 
    (uint32_t)&usbDscStrUCQ[0],
#endif
#if(USB_STR_IF_MFI_EN) 
    (uint32_t)&usbDscStrMFI[0],
#endif
};

static otg_ep_fifo_cfg_t EndpointFifoConfig[USB_EP_NUM_IN_USE] = {
#if USB_FUN_SPK_EN
    {
        .epIdx = USB_EP_NUM_AS_SPK,             /*!< endpoint index number, 1~8 */
        .transType =  1,                        /*!< transfer type, 1, iso, 2, bulk, 3, int */
        .fifo1024 = UAC_FIFO_SIZE_TYPE_SPK,     /*!< fifo size, 0, 512, 1, 1024 */
        .fifoBlockNum = UAC_FIFO_BLOCK_TYPE_SPK,/*!< fifo block num, 0, single, 1, double, 2, triple */
        .fifoInIdx = 0xF,                       /*!< fifo in index number, 0~7, 0xF means no use */
        .fifoOutIdx = UAC_FIFO_IDX_SPK,         /*!< fifo out index number, 0~7, 0xF means no use */
    },
#endif
#if USB_FUN_MIC_EN
    {
        .epIdx = USB_EP_NUM_AS_MIC,             /*!< endpoint index number, 1~8 */
        .transType =  1,                        /*!< transfer type, 1, iso, 2, bulk, 3, int */
        .fifo1024 = UAC_FIFO_SIZE_TYPE_MIC,     /*!< fifo size, 0, 512, 1, 1024 */
        .fifoBlockNum = UAC_FIFO_BLOCK_TYPE_MIC,/*!< fifo block num, 0, single, 1, double, 2, triple */
        .fifoInIdx = UAC_FIFO_IDX_MIC,          /*!< fifo in index number, 0~7, 0xF means no use */
        .fifoOutIdx = 0xF,                      /*!< fifo out index number, 0~7, 0xF means no use */
    },
#endif
#if USB_FUN_HID_EN
    {
        .epIdx = USB_EP_NUM_HID,                /*!< endpoint index number, 1~8 */
        .transType =  3,                        /*!< transfer type, 1, iso, 2, bulk, 3, int */
        .fifo1024 = UAC_FIFO_SIZE_TYPE_HID,     /*!< fifo size, 0, 512, 1, 1024 */
        .fifoBlockNum = UAC_FIFO_BLOCK_TYPE_HID,/*!< fifo block num, 0, single, 1, double, 2, triple */
        .fifoInIdx = UAC_FIFO_IDX_HID_IN,       /*!< fifo in index number, 0~7, 0xF means no use */
        .fifoOutIdx = UAC_FIFO_IDX_HID_OUT,     /*!< fifo out index number, 0~7, 0xF means no use */
    },
#endif
};

static usb_dev_info_t DeviceInformation = //pLibPara
{
    .Buffer = {0},
    .OutPutBuf = {0},
    .InPutBuf  = {0},
    .FunSpkDataRd = NULL,
    .FunMicDataWr = NULL,
    .UsbConnectTime = 0,
    .SpkCallbackTime = 0,
    .MicCallbackTime = 0,
    .UsbState = 0,
    .OutPutSize = 0,
    .InPutSize = 0,

    .pIfAltSetting =usbIfAltSetting,

    .SpkSampleRateTar = 48000,
    .SpkSampleRateCur = 48000,
    .MicSampleRateTar = 48000,
    .MicSampleRateCur  = 48000,

    .usbAudioMute = {0},
    .usbAudioVolume = {0},
    .usbAudioVolumeCfg = {{0,0,0},{0,0,0},{0,0,0}},//Min, Max, Res

    .sizeDscDev = sizeof(usbDscDev),
    .pDscDev = (uint8_t *)&usbDscDev,
    .sizeDscDevQualifier =sizeof(usbDscDevQ),
    .pDscDevQualifier = (uint8_t *)&usbDscDevQ,
    
    .pSizeDscCfg = (uint16_t *)&audioConfigDescriptorSizeTable[0],
    .pSizeDscStr = (uint16_t *)&audioStringDescriptorSizeTable[0],
    .pAddrDscCfg = (uint32_t *)&audioConfigDescriptorAddressTable[0],
    .pAddrDscStr = (uint32_t *)&audioStringDescriptorAddressTable[0],

    .pAddrDscHid = (uint8_t *)&usbDscCfgAll.dscCsIfHid,
    .pDscHidReport = (uint8_t *)usbDscHidReport,
    .sizeDscHidReport = sizeof(usbDscHidReport),

    .pSizeDscOtherSpeedCfg = 0,     //暂无
    .pAddrDscOtherSpeedCfg = NULL,  //暂无

    .PID= 0x0000,                   //PID码 暂无用
    .VID= 0x0000,                   //VID码 暂无用
    .deviceVer = 0,                 //版本号 暂无用
    
    .dscCfg_bmAttributes = 0,       //在LibDrvUac10Init函数初始化，usbDscCfgAll.dscCfg.bmAttributes,  
    .ep0HaltState = 0,
    .DevEp0RxDataSize = 0,

    .numIf = UAC_IF_ID_NUM,
    .numDscCfg = USB_CFG_DESCRIPTORS,
    .numDscStr = USB_STR_DESCRIPTORS,
};

static otg_dev_para_t OtgDevPara = {
    .isFullSpeed = USB_SPEED_SEL_FS,// 0, HS, 1, FS
    .epNumInUse = USB_EP_NUM_IN_USE,
    .pEpFifoCfg = EndpointFifoConfig,
    .pDevInfo = (void *)&DeviceInformation,//pLibPara,
    .devState = 0,
    .devStateSuspendBackup = 0,
    .devSetupPacket = 0,
    .sizeData = 0,
    .pData = NULL,
};

//fifo Spk回调函数
void UsbAudioSpkCallback()
{
#if USB_FUN_SPK_EN
    uint32_t TickNum;
    
    TickNum = TickGetFunc();
    if (0x2 < (TickNum  - DeviceInformation.SpkCallbackTime)) {
        UsbSpkCtrl(-1);
    }
    DeviceInformation.SpkCallbackTime = TickNum;
    DeviceInformation.UsbState = DeviceInformation.UsbState | 1;
    
    if (DeviceInformation.FunSpkDataRd != NULL) {
        DeviceInformation.FunSpkDataRd();
    }
#endif
    return;
}

//fifo Mic回调函数
void UsbAudioMicCallback()
{
#if USB_FUN_MIC_EN
    uint32_t TickNum;
    
    TickNum = TickGetFunc();
    if (0x2 < (TickNum  - DeviceInformation.MicCallbackTime)) {
        UsbMicCtrl(-1);
    }
    DeviceInformation.MicCallbackTime = TickNum;
    DeviceInformation.UsbState = DeviceInformation.UsbState | 2;
    
    if (DeviceInformation.FunMicDataWr != NULL) {
        DeviceInformation.FunMicDataWr();
    }
#endif
    return;
}

//fifo Hid Out回调函数
void UsbHidOutCallback()
{
    uint8_t fifoIdx = UAC_FIFO_IDX_HID_OUT;//fifo2
    uint8_t rxByteN = OTG_DEV_FIFO_OUT_BYTE_CNT(fifoIdx);
    if (rxByteN != 0) {
        HalOtgDevDxFifoRead((uint8_t *)DeviceInformation.InPutBuf, rxByteN, fifoIdx);
    }
    DeviceInformation.InPutSize = rxByteN;

    DeviceInformation.UsbState = DeviceInformation.UsbState | 4;
    UsbHidOutIrqHandler((uint8_t *)DeviceInformation.InPutBuf,DeviceInformation.InPutSize);
    return;
}

//fifo Hid In回调函数
void UsbHidInCallback()
{
    uint8_t fifoIdx = UAC_FIFO_IDX_HID_IN;//fifo3

    OTG_DEV_INT_FIFO_IN_DIS(fifoIdx);//USB_HID_IN_FIFO_INT_DIS();
    hal_otg_device_dx_fifo_write((uint8_t *)DeviceInformation.OutPutBuf, DeviceInformation.OutPutSize, fifoIdx);
    
    DeviceInformation.UsbState = DeviceInformation.UsbState & 0xfffffff7;
    return;
}


/* fifo回调函数表格 */ //需要对应fifo编号
typedef void (*UsbFifoIrqHandler)(void);
void app_fun_null(){}//空函数
static UsbFifoIrqHandler usbAudioFifoIraHandler[OTG_MAX_FIFO_NUM] = 
{
    /* fifo中断响应 */
    app_fun_null,           /* FIFO0 */
    app_fun_null,           /* FIFO1 */
    app_fun_null,           /* FIFO2 */
    app_fun_null,           /* FIFO3 */
    app_fun_null,           /* FIFO4 */
    app_fun_null,           /* FIFO5 */
    app_fun_null,           /* FIFO6 */
    app_fun_null,           /* FIFO7 */

};
// static const UsbFifoIrqHandler usbAudioFifoIraHandler[OTG_MAX_FIFO_NUM][2] = 
// {
//     /* fifo中断响应 */
//     /* IN,                   OUT */
// #if (USB_FUN_SPK_EN == 1 && USB_FUN_MIC_EN == 1)
//     {app_fun_null,           UsbAudioSpkCallback     },  /* FIFO0 */
//     {UsbAudioMicCallback,    app_fun_null            },  /* FIFO1 */
//     {app_fun_null,           UsbHidOutCallback       },  /* FIFO2 */
//     {UsbHidInCallback,       app_fun_null            },  /* FIFO3 */
//     {app_fun_null,           app_fun_null            },  /* FIFO4 */
//     {app_fun_null,           app_fun_null            },  /* FIFO5 */
//     {app_fun_null,           app_fun_null            },  /* FIFO6 */
//     {app_fun_null,           app_fun_null            },  /* FIFO7 */
// #elif (USB_FUN_SPK_EN == 0 && USB_FUN_MIC_EN == 1)
//     {UsbAudioMicCallback,    app_fun_null            },  /* FIFO0 */
//     {app_fun_null,           app_fun_null            },  /* FIFO1 */
//     {app_fun_null,           app_fun_null            },  /* FIFO2 */
//     {app_fun_null,           app_fun_null            },  /* FIFO3 */
//     {app_fun_null,           UsbHidOutCallback       },  /* FIFO4 */
//     {UsbHidInCallback,       app_fun_null            },  /* FIFO5 */
//     {app_fun_null,           app_fun_null            },  /* FIFO6 */
//     {app_fun_null,           app_fun_null            },  /* FIFO7 */
// #else
    
// #endif

// };

void *LibDrvMemcpy(void *pDst, const void *pSrc, uint32_t byteSize)
{
    uint32_t * p32Dst = (uint32_t * )pDst;
    uint32_t * p32Src = (uint32_t * )pSrc;
    uint8_t * p8Dst = NULL;
    uint8_t * p8Src = NULL;
    
    if((((uint32_t)pDst)&0x03) == 0 && (((uint32_t)pSrc)&0x03) == 0)
    {
        while(0x1f < byteSize){
            p32Dst[0] = p32Src[0];
            p32Dst[1] = p32Src[1];
            p32Dst[2] = p32Src[2];
            p32Dst[3] = p32Src[3];
            p32Dst[4] = p32Src[4];
            p32Dst[5] = p32Src[5];
            p32Dst[6] = p32Src[6];
            p32Dst[7] = p32Src[7];
            p32Dst = p32Dst + 8;
            p32Src = p32Src + 8;
            byteSize = byteSize-32;
        }
        while (3 < byteSize) {
            p32Dst[0] = p32Src[0];
            p32Dst = p32Dst + 1;
            p32Src = p32Src + 1;
            byteSize = byteSize - 4;
        }
    }
    
    p8Dst = (uint8_t *)p32Dst;
    p8Src = (uint8_t *)p32Src;
    while (byteSize != 0) {
        *p8Dst = *p8Src;
        p8Dst++;
        p8Src++;
        byteSize = byteSize-1;
    }
    
    return 0;
}


void *LibDrvMemset(void *pDst, uint32_t c, uint32_t byteSize)
{
    uint32_t * p32Dst = (uint32_t * )pDst;
    uint8_t * p8Dst = NULL;
    uint32_t u32Data = 0;
    
    c = c & 0xFF;
    u32Data = c | c<<8 | c<<16 | c<<24;
    if((((uint32_t)pDst)&0x03) == 0)
    {
        while(0x1f < byteSize){
            p32Dst[0] = u32Data;
            p32Dst[1] = u32Data;
            p32Dst[2] = u32Data;
            p32Dst[3] = u32Data;
            p32Dst[4] = u32Data;
            p32Dst[5] = u32Data;
            p32Dst[6] = u32Data;
            p32Dst[7] = u32Data;
            p32Dst = p32Dst + 8;
            byteSize = byteSize-32;
        }
        while (3 < byteSize) {
            p32Dst[0] = u32Data;
            p32Dst = p32Dst + 1;
            byteSize = byteSize - 4;
        }
        
        p8Dst = (uint8_t *)p32Dst;
        while (byteSize != 0) {
        *p8Dst = c;
        p8Dst++;
        byteSize = byteSize-1;
        }
    }
    
    return 0;
}


int32_t LibDrvUac10DscStringConfig(uint32_t addrDscStr, uint8_t strIdx)
{
    //可以直接修改组，不需要在此配置
    return 0;
}


int32_t LibDrvUac10Init(uint32_t (*pFunTickMsGet)())
{
    uint32_t FirstTime = 0;
    uint32_t CurTime = 0;
    uint32_t TimeOut = 0;

    if (pFunTickMsGet != NULL) 
    {
        TickGetFunc = pFunTickMsGet;
        FirstTime = TickGetFunc();
        TimeOut = 0x100000;
        do {
          CurTime = TickGetFunc();

          if (CurTime != FirstTime) 
          {
                if (TimeOut == 0) {
                  return -2;
                }

//fifo中断函数配置
#if (UAC_FIFO_IDX_SPK != -1)
                usbAudioFifoIraHandler[UAC_FIFO_IDX_SPK] = UsbAudioSpkCallback;
#endif
#if (UAC_FIFO_IDX_MIC != -1)
                usbAudioFifoIraHandler[UAC_FIFO_IDX_MIC] = UsbAudioMicCallback;
#endif
// #if (UAC_FIFO_IDX_MIDI_OUT != -1)
//                 usbAudioFifoIraHandler[UAC_FIFO_IDX_MIDI_OUT] = app_fun_null;
// #endif
// #if (UAC_FIFO_IDX_MIDI_IN != -1)
//                 usbAudioFifoIraHandler[UAC_FIFO_IDX_MIDI_IN] = app_fun_null;
// #endif
#if (UAC_FIFO_IDX_HID_OUT != -1)
                usbAudioFifoIraHandler[UAC_FIFO_IDX_HID_OUT] = UsbHidOutCallback;
#endif
#if (UAC_FIFO_IDX_HID_IN != -1)
                usbAudioFifoIraHandler[UAC_FIFO_IDX_HID_IN] = UsbHidInCallback;
#endif
#if (UAC_FIFO_IDX_MFI_OUT != -1)
                usbAudioFifoIraHandler[UAC_FIFO_IDX_MFI_OUT] = app_fun_null;
#endif
#if (UAC_FIFO_IDX_MFI_IN != -1)
                usbAudioFifoIraHandler[UAC_FIFO_IDX_MFI_IN] = app_fun_null;
#endif
                
                usb_dev_info_t *pDevInfo = (usb_dev_info_t *)OtgDevPara.pDevInfo;
                pDevInfo->dscCfg_bmAttributes = usbDscCfgAll.dscCfg.bmAttributes;

                HalOtgDevInit(&OtgDevPara);
                return 0;
          }
          TimeOut = TimeOut--;
        } while (TimeOut != 0);
    }
    return -2;
}


void HalOtgDevEp0RxDataCallback(void)
{
    if((DeviceInformation.DevEp0RxDataSize != 0x02)
    &&(DeviceInformation.DevEp0RxDataSize != 0x03))
    {
        return;
    }

    UsbHidOutIrqHandler(DeviceInformation.InPutBuf,DeviceInformation.InPutSize);
    DeviceInformation.DevEp0RxDataSize = 0xFF;

    return;
}


void HalOtgDevFifoCallback(uint8_t fifoIdx, uint32_t in, uint32_t out)
{
    if (in != 0)
    {
        switch (fifoIdx)
        {
            //case UAC_FIFO_IDX_MIC:
            //case UAC_FIFO_IDX_MIDI_IN:
            case UAC_FIFO_IDX_HID_IN:
            case UAC_FIFO_IDX_MFI_IN:
                usbAudioFifoIraHandler[fifoIdx]();
            break;
        
        default:
            break;
        }
    }

    if (out != 0)
    {
        switch (fifoIdx)
        {
            //case UAC_FIFO_IDX_SPK:
            //case UAC_FIFO_IDX_MIDI_OUT:
            case UAC_FIFO_IDX_HID_OUT:
            case UAC_FIFO_IDX_MFI_OUT:
                usbAudioFifoIraHandler[fifoIdx]();
            break;
        
        default:
            break;
        }
    }
    
    // if (in != 0)
    // {
    //     usbAudioFifoIraHandler[fifoIdx][0]();
    // }
    // if (out != 0)
    // {
    //     usbAudioFifoIraHandler[fifoIdx][1]();
    // }
    return;
}


static void drv_usb_device_get_state(otg_dev_para_t *pPara)
{
    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;

    switch (pPara->devSetupPacket.bmRequestType.byte)
    {
        case USB_SETUP_REQ_GET_DEVICE:
        {
            uint8_t u8Temp[2] = {0, 0};
            if ((pDevInfo->dscCfg_bmAttributes & USB_CFG_REMOTEWAKEUP) != 0)
            {
                u8Temp[0] |= BIT1;
            }
            if ((pDevInfo->dscCfg_bmAttributes & USB_CFG_SELFPOWER) != 0)
            {
                u8Temp[1] |= BIT0;
            }
            hal_otg_device_cx_fifo_write(u8Temp, 2);
            OTG_DEV_EP0_DONE_SET();
            break;
        }
        case USB_SETUP_REQ_GET_INTERFACE:
        {
            if (pPara->devState != USB_DEV_STATE_CONFIGURED)
            {
                OTG_DEV_EP0_STALL_SET();
                break;
            }
            if (pPara->devSetupPacket.wIndex >= pDevInfo->numIf)
            {
                OTG_DEV_EP0_STALL_SET();
                break;
            }
            uint8_t u8Temp[2] = {0, 0};
            hal_otg_device_cx_fifo_write(u8Temp, 2);
            OTG_DEV_EP0_DONE_SET();
            break;
        }
        case USB_SETUP_REQ_GET_ENDPOINT:
        {
            if (pPara->devState != USB_DEV_STATE_CONFIGURED)
            {
                OTG_DEV_EP0_STALL_SET();
                break;
            }
            uint8_t u8Temp[2] = {0, 0};
            if (pPara->devSetupPacket.wIndex == 0)
            {
                u8Temp[0] = pDevInfo->ep0HaltState;
            }
            else
            {
                uint8_t ep = pPara->devSetupPacket.wIndex & 0xF;
                uint8_t dir = pPara->devSetupPacket.wIndex >> 7;
                if (ep > OTG_MAX_EP_NUM)
                {
                    OTG_DEV_EP0_STALL_SET();
                    break;
                }
                else
                {
                    uint8_t fifo = OTG_DEV_EP_MAP_RD(ep);
                    fifo = (dir == 1) ? (fifo & 0xF) : (fifo >> 4);
                    if (fifo >= OTG_MAX_FIFO_NUM)
                    {
                        OTG_DEV_EP0_STALL_SET();
                        break;
                    }
                    if (OTG_DEV_FIFO_CFG_ENABLE_CHECK(fifo) == 0)
                    {
                        OTG_DEV_EP0_STALL_SET();
                        break;
                    }
                    if (dir == 1)
                    {
                        u8Temp[0] = OTG_DEV_EP_IN_STALL_CHECK(ep);
                    }
                    else
                    {
                        u8Temp[0] = OTG_DEV_EP_OUT_STALL_CHECK(ep);
                    }
                }
            }
            hal_otg_device_cx_fifo_write(u8Temp, 2);
            OTG_DEV_EP0_DONE_SET();
            break;
        }
        default:
        {
            OTG_DEV_EP0_STALL_SET();
            break;
        }
    }
    return;
}


static void drv_usb_device_clear_feature(otg_dev_para_t *pPara)
{    
    if (pPara->devState != USB_DEV_STATE_CONFIGURED)
    {
        OTG_DEV_EP0_STALL_SET();
    }
    else
    {
        usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;
        switch (pPara->devSetupPacket.bmRequestType.byte)
        {
            case USB_SETUP_REQ_SET_DEVICE:
            {
                OTG_DEV_REMOTE_WAKEUP_CLR();
                OTG_DEV_EP0_DONE_SET();
                OTG_DEV_DMA_TO_FIFO_NONE();
                break;
            }
            case USB_SETUP_REQ_SET_ENDPOINT:
            {
                if (pPara->devSetupPacket.wIndex == 0)
                {
                    pDevInfo->ep0HaltState = 0;
                }
                else
                {
                    uint8_t ep = pPara->devSetupPacket.wIndex & 0xF;
                    uint8_t dir = pPara->devSetupPacket.wIndex >> 7;
                    if (ep > OTG_MAX_EP_NUM)
                    {
                        OTG_DEV_EP0_STALL_SET();
                        break;
                    }
                    else
                    {
                        uint8_t fifo = OTG_DEV_EP_MAP_RD(ep);
                        fifo = (dir == 1) ? (fifo & 0xF) : (fifo >> 4);
                        if (fifo >= OTG_MAX_FIFO_NUM)
                        {
                            OTG_DEV_EP0_STALL_SET();
                            break;
                        }
                        if (OTG_DEV_FIFO_CFG_ENABLE_CHECK(fifo) == 0)
                        {
                            OTG_DEV_EP0_STALL_SET();
                            break;
                        }
                        if (dir == 1)
                        {
                            OTG_DEV_EP_IN_RSTG_SET(ep);
                            OTG_DEV_EP_IN_RSTG_CLR(ep);
                            OTG_DEV_EP_IN_STALL_CLR(ep);
                        }
                        else
                        {
                            OTG_DEV_EP_OUT_RSTG_SET(ep);
                            OTG_DEV_EP_OUT_RSTG_CLR(ep);
                            OTG_DEV_EP_OUT_STALL_CLR(ep);
                        }
                    }
                }
                OTG_DEV_EP0_DONE_SET();
                break;
            }
            default:
            {
                OTG_DEV_EP0_STALL_SET();
                break;
            }
        }
    }
    return;
}


static void drv_usb_device_set_feature(otg_dev_para_t *pPara)
{
    if (pPara->devState != USB_DEV_STATE_CONFIGURED)
    {
        OTG_DEV_EP0_STALL_SET();
    }
    else
    {
        usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;
        switch (pPara->devSetupPacket.bmRequestType.byte)
        {
            case USB_SETUP_REQ_SET_DEVICE:
            {
                OTG_DEV_REMOTE_WAKEUP_SET();
                OTG_DEV_EP0_DONE_SET();
                OTG_DEV_DMA_TO_FIFO_NONE();
                break;
            }
            case USB_SETUP_REQ_SET_ENDPOINT:
            {
                if (pPara->devSetupPacket.wIndex == 0)
                {
                    pDevInfo->ep0HaltState = 1;
                }
                else
                {
                    uint8_t ep = pPara->devSetupPacket.wIndex & 0xF;
                    uint8_t dir = pPara->devSetupPacket.wIndex >> 7;
                    if (ep > OTG_MAX_EP_NUM)
                    {
                        OTG_DEV_EP0_STALL_SET();
                        break;
                    }
                    else
                    {
                        uint8_t fifo = OTG_DEV_EP_MAP_RD(ep);
                        fifo = (dir == 1) ? (fifo & 0xF) : (fifo >> 4);
                        if (fifo >= OTG_MAX_FIFO_NUM)
                        {
                            OTG_DEV_EP0_STALL_SET();
                            break;
                        }
                        if (OTG_DEV_FIFO_CFG_ENABLE_CHECK(fifo) == 0)
                        {
                            OTG_DEV_EP0_STALL_SET();
                            break;
                        }
                        if (dir == 1)
                        {
                            OTG_DEV_EP_IN_STALL_SET(ep);
                        }
                        else
                        {
                            OTG_DEV_EP_OUT_STALL_SET(ep);
                        }
                    }
                }
                OTG_DEV_EP0_DONE_SET();
                break;
            }
            default:
            {
                OTG_DEV_EP0_STALL_SET();
                break;
            }
        }
    }
    return;
}


static void drv_usb_device_set_address(otg_dev_para_t *pPara)
{
    switch (pPara->devSetupPacket.bmRequestType.byte)
    {
        case USB_SETUP_REQ_SET_DEVICE:
        {
            OTG_DEV_ADDR_SET(pPara->devSetupPacket.wValue);
            OTG_DEV_EP0_DONE_SET();
            if (pPara->devSetupPacket.wValue != 0) {
                pPara->devState = USB_DEV_STATE_ADDRESSED;
            }
            if (pPara->devSetupPacket.wValue == 0) {
                pPara->devState = USB_DEV_STATE_DEFAULT;
            }
            break;
        }
        default:
        {
            OTG_DEV_EP0_STALL_SET();
            break;
        }
    }
    return;
}

uint16_t SaveCnt = 0;
static void drv_usb_device_get_descriptor(otg_dev_para_t *pPara)
{
    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;

    switch (pPara->devSetupPacket.wValue >> 8)
    {
        case USB_DSC_DEVICE:
        {    
            pPara->pData = pDevInfo->pDscDev;
            if (pPara->devSetupPacket.wLength < pDevInfo->sizeDscDev)
            {
                pPara->sizeData = pPara->devSetupPacket.wLength;
            }
            else
            {
                pPara->sizeData = pDevInfo->sizeDscDev;
            }
            hal_otg_device_ep0_tx_data(pPara);
            break;
        }
        case USB_DSC_CONFIG:
        {
            uint8_t cfgNum = (uint8_t)(pPara->devSetupPacket.wValue);
            if (cfgNum < pDevInfo->numDscCfg)
            {
                uint32_t *pAddr = pDevInfo->pAddrDscCfg;
                pAddr += cfgNum;
                pPara->pData = (uint8_t *)(*pAddr);
                uint16_t *pSize = pDevInfo->pSizeDscCfg;
                pSize += cfgNum;
                if (pPara->devSetupPacket.wLength < *pSize)
                {
                    pPara->sizeData = pPara->devSetupPacket.wLength;
                }
                else
                {
                    pPara->sizeData = *pSize;
                }
                hal_otg_device_ep0_tx_data(pPara);
            }
            else
            {
                OTG_DEV_EP0_STALL_SET();
                break;
            }
            break;
        }
		case USB_DSC_OTHER_SPEED_CONFIG:
		{
//			if(pPara->isFullSpeed || pDevInfo->pAddrDscOtherSpeedCfg == NULL || pDevInfo->pSizeDscOtherSpeedCfg == NULL){
            if(1){//by Niko 目前暂没有使用到
                OTG_DEV_EP0_STALL_SET();
                break;
            }

//            uint8_t cfgNum = (uint8_t)(pPara->devSetupPacket.wValue);
//            if (cfgNum < pDevInfo->numDscCfg)
//            {
//                uint32_t *pAddr = pDevInfo->pAddrDscOtherSpeedCfg;
//                pAddr += cfgNum;
//                pPara->pData = (uint8_t *)(*pAddr);
//                uint16_t *pSize = pDevInfo->pSizeDscOtherSpeedCfg;
//                pSize += cfgNum;
//                if (pPara->devSetupPacket.wLength < *pSize)
//                {
//                    pPara->sizeData = pPara->devSetupPacket.wLength;
//                }
//                else
//                {
//                    pPara->sizeData = *pSize;
//                }
//                hal_otg_device_ep0_tx_data(pPara);
//            }
//            else
//            {
//                OTG_DEV_EP0_STALL_SET();
//                break;
//            }
//            break;
        }
        case USB_DSC_STRING:
        {
            uint8_t strNum = (uint8_t)(pPara->devSetupPacket.wValue);
            if (strNum < pDevInfo->numDscStr)
            {
                uint32_t *pAddr = pDevInfo->pAddrDscStr;
                pAddr += strNum;
                pPara->pData = (uint8_t *)(*pAddr);
                uint16_t *pSize = pDevInfo->pSizeDscStr;

                pSize += strNum;
                if (pPara->devSetupPacket.wLength < *pSize)
                {
                    pPara->sizeData = pPara->devSetupPacket.wLength;
                }
                else
                {
                    pPara->sizeData = *pSize;
                }
                
                hal_otg_device_ep0_tx_data(pPara);
            }
            else
            {
                OTG_DEV_EP0_STALL_SET();
                break;
            }
            break;
        }
        case HID_DSC_HID:
        {
            pPara->pData = pDevInfo->pAddrDscHid;

            if (pPara->devSetupPacket.wLength < *pDevInfo->pAddrDscHid)
            {
                pPara->sizeData = pPara->devSetupPacket.wLength;
            }
            else
            {
                pPara->sizeData = *pDevInfo->pAddrDscHid;
            }
            hal_otg_device_ep0_tx_data(pPara);
            break;
        }
        case HID_DSC_REPORT:
        {
            pPara->pData = pDevInfo->pDscHidReport;
            if (pPara->devSetupPacket.wLength < pDevInfo->sizeDscHidReport)
            {
                pPara->sizeData = pPara->devSetupPacket.wLength;
            }
            else
            {
                pPara->sizeData = pDevInfo->sizeDscHidReport;
            }
            SaveCnt = 1;
            hal_otg_device_ep0_tx_data(pPara);
            break;
        }
		case USB_DSC_DEVICE_QUALIFIER:

//          if(pPara->isFullSpeed || pDevInfo->pDscDevQualifier == NULL || pDevInfo->sizeDscDevQualifier){//by Niko
//				OTG_DEV_EP0_STALL_SET();
//				break;
//			}
			
			pPara->pData = pDevInfo->pDscDevQualifier;
			if (pPara->devSetupPacket.wLength < pDevInfo->sizeDscDevQualifier)
            {
                pPara->sizeData = pPara->devSetupPacket.wLength;
            }
            else
            {
                pPara->sizeData = pDevInfo->sizeDscDevQualifier;
            }
            hal_otg_device_ep0_tx_data(pPara);
			break;
		
        default:
        {
            OTG_DEV_EP0_STALL_SET();
            break;
        }
    }
    return;
}


static void drv_usb_device_get_configuration(otg_dev_para_t *pPara)
{
    uint8_t u8Temp[1];
    if (pPara->devState == USB_DEV_STATE_CONFIGURED)
    {
        u8Temp[0] = 1;
    }
    else
    {
        u8Temp[0] = 0;
    }
    hal_otg_device_cx_fifo_write(u8Temp, 1);
    OTG_DEV_EP0_DONE_SET(); 
    return;
}


static void drv_usb_device_set_configuration(otg_dev_para_t *pPara)
{
    if ((pPara->devState == USB_DEV_STATE_ADDRESSED) ||     \
        (pPara->devState == USB_DEV_STATE_SUSPENDED) ||     \
		(pPara->devState == USB_DEV_STATE_CONFIGURED))
    {
        if (pPara->devSetupPacket.wValue == 0)
        {
            if (pPara->devState != USB_DEV_STATE_CONFIGURED) {
                OTG_DEV_EP0_STALL_SET();
                return;
            }
            else {
                OTG_DEV_CFG_CLR();
            }
            
            if (pPara->devState == USB_DEV_STATE_CONFIGURED) {
                pPara->devState = USB_DEV_STATE_ADDRESSED;
            }
        }
        else
        {
            OTG_DEV_CFG_SET();
            if (pPara->isFullSpeed != 0)
            {
                OTG_DEV_SOF_MASK_FS();
            }
            else
            {
                OTG_DEV_SOF_MASK_HS();
            }
            for (uint8_t i = 1; i < OTG_MAX_EP_NUM; i++)
            {
                OTG_DEV_EP_IN_RSTG_SET(i);
                OTG_DEV_EP_IN_RSTG_CLR(i);
                OTG_DEV_EP_OUT_RSTG_SET(i);
                OTG_DEV_EP_OUT_RSTG_CLR(i);                
            }
            pPara->devState = USB_DEV_STATE_CONFIGURED;
        }
        OTG_DEV_EP0_DONE_SET();
    }
    else
    {
        OTG_DEV_EP0_STALL_SET();
    }
    return;
}


static void drv_usb_device_get_interface(otg_dev_para_t *pPara)
{
    if (pPara->devState == USB_DEV_STATE_CONFIGURED)
    {
        usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;
        if (pPara->devSetupPacket.wIndex < pDevInfo->numIf)
        {
            uint8_t u8Temp[1];
            u8Temp[0] = *(pDevInfo->pIfAltSetting + pPara->devSetupPacket.wIndex);
            hal_otg_device_cx_fifo_write(u8Temp, 1);
            OTG_DEV_EP0_DONE_SET();
        }
        else
        {
            OTG_DEV_EP0_STALL_SET();
        }
    }
    else
    {
        OTG_DEV_EP0_STALL_SET();
    }
    return;
}


static void drv_usb_device_set_interface(otg_dev_para_t *pPara)
{
    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;

    if (pPara->devState == USB_DEV_STATE_CONFIGURED) {

      uint8_t ifId = pPara->devSetupPacket.wIndex;
      uint8_t ifAltSetting = pPara->devSetupPacket.wValue;
	  
        if (ifId == (uint8_t)UAC_IF_ID_STREAMING_SPK) {
            #if USB_FUN_SPK_EN
            OTG_DEV_EP_OUT_RSTG_CLR(USB_EP_NUM_AS_SPK);
            OTG_DEV_FIFO_RESET(UAC_FIFO_IDX_SPK);

            if (ifAltSetting == UAC_IF_ZERO_BANDWIDTH) {
                pDevInfo->UsbState = pDevInfo->UsbState & 0xfffffffe;
                UsbSpkCtrl(0);
            }
            else {
                if (2 < ifAltSetting) 
                {
                OTG_DEV_EP0_STALL_SET();
                return;
                }

                pDevInfo->SpkCallbackTime = TickGetFunc();
                pDevInfo->FunSpkDataRd = (FUNCP)UsbSpkCtrl(ifAltSetting);
            }
            #endif
        }
        else {
            if (ifId != (uint8_t)UAC_IF_ID_STREAMING_MIC)
            {
                OTG_DEV_EP0_STALL_SET();
                return;
            }
            #if USB_FUN_MIC_EN
//            OTG_DEV_EP_IN_RSTG_CLR(USB_EP_NUM_AS_MIC);
//            OTG_DEV_FIFO_RESET(UAC_FIFO_IDX_MIC);

            //192KLib 增加   
            OTG_DEV_FIFO_RESET(UAC_FIFO_IDX_HID_OUT);
            OTG_DEV_FIFO_RESET(UAC_FIFO_IDX_HID_IN);
           pDevInfo->UsbState = pDevInfo->UsbState & 0xfffffff3;


            if (ifAltSetting == 0) {
                OTG_DEV_INT_FIFO_IN_DIS(UAC_FIFO_IDX_MIC);
                pDevInfo->UsbState = pDevInfo->UsbState & 0xfffffffd;

                UsbMicCtrl(0);
            }
            else {
                if (2 < ifAltSetting)
                {
                    OTG_DEV_EP0_STALL_SET();
                    return;
                }

                pDevInfo->MicCallbackTime = TickGetFunc();
                pDevInfo->FunMicDataWr = (FUNCP)UsbMicCtrl(ifAltSetting);
                OTG_DEV_INT_FIFO_IN_EN(UAC_FIFO_IDX_MIC);
            }
            #endif
        }
        *(pDevInfo->pIfAltSetting + ifId) = ifAltSetting;
        OTG_DEV_EP0_DONE_SET();
        return;
    }
    OTG_DEV_EP0_STALL_SET();
    return;

}


static int32_t usb_hid_get_report(otg_dev_para_t *pPara)
{
    int32_t result;
    
    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;

    if ((pPara->devSetupPacket.wValue >> 8) == HID_REPORT_IN)
    {
        pDevInfo->OutPutBuf[0] = (uint8_t)pPara->devSetupPacket.wValue;
        pPara->pData = pDevInfo->OutPutBuf;
        
        pPara->sizeData = pPara->devSetupPacket.wLength;
        hal_otg_device_ep0_tx_data(pPara);
        result = -1;
    }
    else if((pPara->devSetupPacket.wValue >> 8) == HID_REPORT_FEATURE)
    {
        pDevInfo->OutPutBuf[0] = (uint8_t)pPara->devSetupPacket.wValue;
        pPara->pData = pDevInfo->OutPutBuf;
        pPara->sizeData = pPara->devSetupPacket.wLength;
        hal_otg_device_ep0_tx_data(pPara);
        
        result = -1;
    }
    else
    {
        result = 1;
    }
    return result;
}


//dir: 0, out(spk); 1, in(mic)
//return: 0, ep0 done, 1, ep0 stall, -1, do nothing
static int32_t usb_audio_feature_unit_ctrl_request_handler(otg_dev_para_t *pPara,uint8_t dir)
{
    int32_t result;

    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;
    switch (pPara->devSetupPacket.wValue >> 8)
    {
        case UAC_FU_MUTE_CONTROL:
        {
            pPara->sizeData = pPara->devSetupPacket.wLength;
            pPara->pData = (uint8_t *)&pDevInfo->usbAudioMute[dir][0];
            if (pPara->devSetupPacket.bRequest == UAC_GET_CUR)
            {
                hal_otg_device_ep0_tx_data(pPara);
            }
            result = 0;
            break;
        }
        case UAC_FU_VOLUME_CONTROL:
        {
            switch (pPara->devSetupPacket.bRequest)
            {
                case UAC_SET_CUR:
                {
					//01对应左声道，02对应右声道
					uint8_t ch = (pPara->devSetupPacket.wValue ) & 0x03;
                    pPara->pData = (uint8_t *)&pDevInfo->usbAudioVolume[dir][ch][USB_PARA_IDX_TARGET];
                    pPara->sizeData = pPara->devSetupPacket.wLength;

                    result = -1;
                    break;
                }
                case UAC_SET_RES:
                {
                    pPara->pData = (uint8_t *)&pDevInfo->usbAudioVolumeCfg[dir][2/*USB_FEATURE_UNIT_RES*/];
                    pPara->sizeData = pPara->devSetupPacket.wLength;
                    result = -1;
                    break;
                }
                case UAC_GET_CUR:
                {
                    uint8_t ch = (pPara->devSetupPacket.wValue ) & 0x03;
                    uint8_t *pCtrlData = (uint8_t *)&pDevInfo->usbAudioVolume[dir][ch][USB_PARA_IDX_TARGET];
                    hal_otg_device_cx_fifo_write(pCtrlData, 2);
                    result = 0;
                    break;
                }
                case UAC_GET_RES:
                {
                    uint8_t *pCtrlData = (uint8_t *)&pDevInfo->usbAudioVolumeCfg[dir][2/*RES*/];
                    hal_otg_device_cx_fifo_write(pCtrlData, 2);
                    result = 0;
                    break;
                }
                case UAC_GET_MIN:
                {
                    uint8_t *pCtrlData = (uint8_t *)&pDevInfo->usbAudioVolumeCfg[dir][0/*MIN*/];
                    hal_otg_device_cx_fifo_write(pCtrlData, 2);
                    result = 0;
                    break;
                }
                case UAC_GET_MAX:
                {
                    uint8_t *pCtrlData = (uint8_t *)&pDevInfo->usbAudioVolumeCfg[dir][1/*MAX*/];
                    hal_otg_device_cx_fifo_write(pCtrlData, 2);
                    result = 0;
                    break;
                }
                default:
                {
                    result = 1;
                    break;
                }
            }
            break;
        }
        default:
        {
            result = 1;
            break;
        }
    }
    return result;
}


static uint8_t reportRate = 0;
//return 0, set ep0 done
//return 1, set ep0 stall
//return -1, do nothing, prepare to receive
int32_t drv_usb_class_cmd(otg_dev_para_t *pPara)
{
    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)pPara->pDevInfo;
    int32_t result;
    if (pPara->devSetupPacket.bmRequestType.b.recipient == USB_SETUP_REQ_RECIPIENT_INTERFACE){
        //UAC
        if((pPara->devSetupPacket.wIndex >> 8) == UAC_ID_FEATURE_UNIT_SPEAKER){
            /* Spk Feature Unit */
            result = usb_audio_feature_unit_ctrl_request_handler(pPara,0);
        }else if((pPara->devSetupPacket.wIndex >> 8) == UAC_ID_FEATURE_UNIT_MICROPHONE){
            /* Mic Feature Unit */
            result = usb_audio_feature_unit_ctrl_request_handler(pPara,1);
        }else if((pPara->devSetupPacket.wIndex >> 8) == UAC_ID_FEATURE_UNIT_SIDETONE){
            /* Sidetone Feature Unit */
            result = usb_audio_feature_unit_ctrl_request_handler(pPara,2);
        }else if((pPara->devSetupPacket.wIndex >> 8) == UAC_ID_MIXER_UNIT_WITH_SIDETONE){
            /* Mixer Unit */
            result = 1;//不支持控制
            
        //HID
        }else if(pPara->devSetupPacket.bRequest == HID_GET_REPORT){
            result = usb_hid_get_report(pPara);
        }else if(pPara->devSetupPacket.bRequest == HID_SET_IDLE){
			reportRate = pPara->devSetupPacket.wValue >> 8;
			OTG_DEV_EP_IN_STALL_CLR(USB_EP_NUM_HID);
			result = 0;
		}else if(pPara->devSetupPacket.bRequest == HID_GET_IDLE){
            pPara->pData = &reportRate;
            pPara->sizeData = sizeof(reportRate);
			hal_otg_device_ep0_tx_data(pPara);
			return 0;
        }else if(pPara->devSetupPacket.bRequest == HID_SET_REPORT){

            if ((pPara->devSetupPacket.wValue >> 8) == 2){
              pDevInfo->DevEp0RxDataSize = (uint8_t)pPara->devSetupPacket.wLength;
              pPara->pData = pDevInfo->InPutBuf;
              pPara->sizeData = (uint8_t)pPara->devSetupPacket.wLength;
              return -1;
            }
            if ((pPara->devSetupPacket.wValue >> 8) == 3){
              pDevInfo->DevEp0RxDataSize = (uint8_t)pPara->devSetupPacket.wLength;
              pPara->pData = pDevInfo->InPutBuf;
              pPara->sizeData = (uint8_t)pPara->devSetupPacket.wLength;
              return -1;
            }
        
		}else{
            result = 1;
        }
    }else if(pPara->devSetupPacket.bmRequestType.b.recipient == USB_SETUP_REQ_RECIPIENT_ENDPOINT){
        if(pPara->devSetupPacket.wValue >> 8 == 0x01)
        {
                switch(pPara->devSetupPacket.bRequest/*SETUP.bRequest*/)
                {
                    case UAC_SET_CUR:
                        if ((pPara->devSetupPacket.wIndex & 0xFF) == (0x80|USB_EP_NUM_AS_MIC))
                        {//USB Mic Sample Rate control
                            pPara->pData = (UINT8*)&pDevInfo->MicSampleRateTar;
                        }
                        else
                        {//USB Speaker Sample Rate control
                            pPara->pData = (UINT8*)&pDevInfo->SpkSampleRateTar;
                        }
                        pPara->sizeData = 3;
                        return -1;

                    case UAC_SET_MIN:
                        break;
                    
                    case UAC_SET_MAX:
                        break;
                    
                    case UAC_SET_RES:
                        break;
                    
                    case UAC_GET_CUR:
                        if ((pPara->devSetupPacket.wIndex & 0xFF) == (0x80|USB_EP_NUM_AS_MIC))
                        {//USB Mic Sample Rate control
                            pPara->pData = (UINT8*)&pDevInfo->MicSampleRateCur;
                            pPara->sizeData = 3;
                        }
                        else
                        {//USB Speaker Sample Rate control
                            pPara->pData = (UINT8*)&pDevInfo->SpkSampleRateCur;
                            pPara->sizeData = 3;
                        }

                        hal_otg_device_ep0_tx_data(pPara);//UsbEp0TxDataCallback();
                        return 0;
                    
                    case UAC_GET_MIN:
                        break;
                    
                    case UAC_GET_MAX:
                        break;
                    
                    case UAC_GET_RES:
                        break;
                    
                    default:
                        break;                    
                }// end of switch(SetupPkt.bRequest)	
                
            return 1;

        }
    }else{
        result = 1;
    }
    return result;    
}


void HalOtgDevEp0Setup(otg_dev_para_t *pPara)
{
    /* confg dma to cx fifo */
    OTG_DEV_DMA_TO_FIFO_CX_SEL();
    /* read cx data */
    *(((uint32_t *)&pPara->devSetupPacket) + 0) = OTG_DEV_EP0_CMD_DATA_RD();
    *(((uint32_t *)&pPara->devSetupPacket) + 1) = OTG_DEV_EP0_CMD_DATA_RD();
    /* config dma to no fifo */
    OTG_DEV_DMA_TO_FIFO_NONE();
    
    if(SaveCnt)
    {
        SaveCnt = 1;
    }
    if (pPara->devSetupPacket.bmRequestType.b.type == USB_SETUP_REQ_TYPE_STANDARD)
    {
        /* standard command */
        switch (pPara->devSetupPacket.bRequest)
        {
            case USB_GET_STATUS:
                drv_usb_device_get_state(pPara);        
                break;
            case USB_CLEAR_FEATURE:
                drv_usb_device_clear_feature(pPara);    
                break;
            case USB_SET_FEATURE:
                drv_usb_device_set_feature(pPara);      
                break;
            case USB_SET_ADDRESS:
                drv_usb_device_set_address(pPara);      
                break;
            case USB_GET_DESCRIPTOR:
                drv_usb_device_get_descriptor(pPara);   
                break;
            case USB_GET_CONFIGURATION:
                drv_usb_device_get_configuration(pPara);
                break;
            case USB_SET_CONFIGURATION:
                drv_usb_device_set_configuration(pPara);
                break;
            case USB_GET_INTERFACE:
                drv_usb_device_get_interface(pPara);    
                break;
            case USB_SET_INTERFACE:
                drv_usb_device_set_interface(pPara);    
                break;
            default:
                OTG_DEV_EP0_STALL_SET();                
                break;
        }
    }
    else if (pPara->devSetupPacket.bmRequestType.b.type == USB_SETUP_REQ_TYPE_CLASS)
    {
        /* class command */
        int32_t resultClassCmd = drv_usb_class_cmd(pPara);
        if (resultClassCmd == 0)
        {
            OTG_DEV_EP0_DONE_SET();
        }
        else if (resultClassCmd == 1)
        {
            OTG_DEV_EP0_STALL_SET();
        }
    }
    else
    {
        /* Invalid(bad) command, Return EP0_STALL flag */
        OTG_DEV_EP0_STALL_SET();
    }
    return;
}


int32_t LibDrvUsbHidDecode(uint8_t *pHidData, uint32_t idx)
{
    uint8_t cVar1;
    uint32_t local_10;
//    uint32_t local_c;
//    uint32_t local_8;
    
    cVar1 = pHidData[1];
        
    //USB_HID_OUT_ASP_ID
    if (*pHidData == USB_HID_OUT_ASP_ID) {
//        local_10 = 0x9a;
//        local_c = 0;
//        local_8 = 0;
        if (cVar1 == '\0') {
            local_10 = 0x100009a;
//        local_c = 1;
            LibDrvMemcpy(DeviceInformation.Buffer,&local_10,5);
        }
        else {
        if (cVar1 == '\x10') {
            local_10 = 0x100109a;
//            local_c = 0x27;
            LibDrvMemcpy(DeviceInformation.Buffer,&local_10,5);
        }
        else {
            if (cVar1 == 0x40) {
            if (idx == 1) {
                local_10 = 0x100419a;
//                  local_c = 0x1010000;
                LibDrvMemcpy(DeviceInformation.Buffer,&local_10,8);
                return 0;
            }
            if (idx == 2) {
//                  local_c = 0x11030020;
                local_10 = 0x100419a;
//                  local_8 = 0x12f;
            }
            }
            LibDrvMemcpy(DeviceInformation.Buffer,&local_10,10);
        }
        }
    }
    return 0;
}


int32_t LibDrvUac10HidInSend(uint8_t *pSendData, size_t byteSize)
{
    if ((DeviceInformation.UsbState & 8) == 0) {
        DeviceInformation.UsbState = DeviceInformation.UsbState | 8;
        DeviceInformation.OutPutSize = byteSize;
        LibDrvMemcpy(DeviceInformation.OutPutBuf,pSendData,DeviceInformation.OutPutSize);

        OTG_DEV_INT_FIFO_IN_EN(UAC_FIFO_IDX_HID_IN);

        return 0;
    }
    return 1;
}


int32_t LibDrvUac10HidInSendReset(void)
{
    DeviceInformation.UsbState = DeviceInformation.UsbState & 0xfffffff7;
    return 0;
}


int32_t LibDrvUac10HidOutReceive(uint8_t *pRecvData)
{
    uint8_t bVar1;
    int32_t Ret = -1;
    
    if ((DeviceInformation.UsbState & 4) == 0) {
        if (DeviceInformation.InPutSize != 0) {
        LibDrvMemcpy(pRecvData, DeviceInformation.InPutBuf, bVar1);
        DeviceInformation.InPutSize = 0;
        Ret = bVar1;
        }
    }
    else {
        if (DeviceInformation.InPutSize != 0) {
        LibDrvMemcpy(pRecvData, DeviceInformation.InPutBuf, bVar1);
        DeviceInformation.InPutSize = 0;
        }
        DeviceInformation.UsbState = DeviceInformation.UsbState & 0xfffffffb;
        Ret = bVar1;
    }
    return Ret;
}


void LibDrvUac10Irq(void)
{
  HalOtgDevIrq((otg_dev_para_t *)&OtgDevPara);
  return;
}


void LibDrvUac10MemoryAddrInit(uint32_t *pMemAddr)
{
    //可以考虑去掉
    pLibPara = (uint32_t *)&DeviceInformation;
    return;
}


uint32_t LibDrvUac10MemorySizeGet(void)
{
    //可以考虑去掉
    return 0x67;
}


//检查mute变化
//idx = 0, spk; 1, mic, 2, sidetone
//return 0, vol no change
//return 1, vol changed
int32_t LibDrvUac10MuteCheck(uint8_t idx, int32_t *pNewVol)
{
    uint8_t MuteStateCur;
    uint8_t MuteStateTar;

    MuteStateTar = DeviceInformation.usbAudioMute[idx][0];
    MuteStateCur = DeviceInformation.usbAudioMute[idx][1];

    if (MuteStateTar != MuteStateCur) {
        DeviceInformation.usbAudioMute[idx][1] = MuteStateTar;
        *pNewVol = 1 - MuteStateTar;
        return 1;
    }
    return 0;
}


//sidetonneMuteDefault仅在UAC_ID_SIDETONE_UNIT_ENABLE配置成1的时候才有效
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10MuteDefaultSet(uint8_t spkMuteDefault, uint8_t micMuteDefault, uint8_t sidetoneMuteDefault)
{
  if (pLibPara != 0) {
    DeviceInformation.usbAudioMute[0][0] = spkMuteDefault;
    DeviceInformation.usbAudioMute[0][1] = ~spkMuteDefault & 1;
    DeviceInformation.usbAudioMute[1][0] = micMuteDefault;
    DeviceInformation.usbAudioMute[1][1] = ~micMuteDefault & 1;
    DeviceInformation.usbAudioMute[2][0] = sidetoneMuteDefault;
    DeviceInformation.usbAudioMute[2][1] = ~sidetoneMuteDefault & 1;
    return 0;
  }
  return -1;
}


//if this function do not called before lib_drv_uac20_init, default pid/vid in LIB will be used
//pid, 0, use default value in LIB (USB_DEFAULT_PID), 0x0001~0xFFFF, new pid used
//vid, 0, use default value in LIB (USB_DEFAULT_VID), 0x0001~0xFFFF, new vid used
//deviceVer, 0, use default value in LIB (USB_DEVICE_VERSION), 0x0001~0xFFFF, new value used
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10PidVidConfig(uint16_t pid, uint16_t vid, uint16_t deviceVer)
{
  if (pLibPara != 0) {
    DeviceInformation.PID = pid;
    DeviceInformation.VID = vid;
    DeviceInformation.deviceVer = deviceVer;
    return 0;
  }
  return -1;
}


//clkSrcIdx, USB_PARA_IDX_SPK, USB_PARA_IDX_MIC
//*pNewSampleRate, return new sample rate in uint32_t
//return 0, vol no change
//return 1, vol changed
int32_t LibDrvUac10SampleRateCheck(uint8_t clkSrcIdx, uint32_t *pNewSampleRate)
{
    if (0/*USB_PARA_IDX_SPK*/ == clkSrcIdx)
    {
        if (DeviceInformation.SpkSampleRateCur != DeviceInformation.SpkSampleRateTar) {
            DeviceInformation.SpkSampleRateCur = DeviceInformation.SpkSampleRateTar;
            *pNewSampleRate = DeviceInformation.SpkSampleRateCur;
            return 1;
        }
    }
    else if (1/*USB_PARA_IDX_MIC*/ == clkSrcIdx)
    {
        if (DeviceInformation.MicSampleRateCur != DeviceInformation.MicSampleRateTar) {
            DeviceInformation.MicSampleRateCur = DeviceInformation.MicSampleRateTar;
            *pNewSampleRate = DeviceInformation.MicSampleRateCur;
            return 1;
        }
    }
  
    return 0;
}


//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10SampleRateDefaultSet(uint32_t spkDefault, uint32_t micDefault)
{
    if (pLibPara != 0) {
        DeviceInformation.SpkSampleRateTar = spkDefault;
        DeviceInformation.SpkSampleRateCur  = 0;
        DeviceInformation.MicSampleRateTar = micDefault;
        DeviceInformation.MicSampleRateCur  = 0;
        return 0;
    }
    return -1;
}


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
uint32_t LibDrvUac10StateGet(void)
{
    return DeviceInformation.UsbState;
}


//在初始化之前，先配置音量范围信息，所有通道使用相同配置，SPK/MIC独立
//pSidetoneVolRange, [min, max, res, default], in dB value, 仅在UAC_ID_SIDETONE_UNIT_ENABLE配置成1的时候才有效
//pSpkVolRange, [min, max, res, default], in dB value
//pMicVolRange, [min, max, res, default], in dB value
//return 0, OK
//return -1, lib_drv_uac20_memory_addr_init() need to be called first
int32_t LibDrvUac10VolRangeSet(uint16_t *pSpkVolRange, uint16_t *pMicVolRange, uint16_t *pSidetoneVolRange)
{
    if (pLibPara != 0) {
        //DeviceInformation.usbAudioVolumeCfg[3][3]
        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_SPK][0] = pSpkVolRange[0];        //Min
        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_SPK][1] = pSpkVolRange[1];        //Max
        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_SPK][2] = pSpkVolRange[2];        //Step

        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_MIC][0] = pMicVolRange[0];        //Min
        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_MIC][1] = pMicVolRange[1];        //Max
        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_MIC][2] = pMicVolRange[2];        //Step

        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_SIDETONE][0] = pSidetoneVolRange[0];   //Min
        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_SIDETONE][1] = pSidetoneVolRange[1];   //Max
        DeviceInformation.usbAudioVolumeCfg[USB_PARA_IDX_SIDETONE][2] = pSidetoneVolRange[2];   //Step

        //DeviceInformation.usbAudioVolume[3][3][2]
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SPK][0][USB_PARA_IDX_TARGET]  = pSpkVolRange[3];            //Target 
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SPK][0][USB_PARA_IDX_CURRENT] = pSpkVolRange[3] - 1;        //Current
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SPK][1][USB_PARA_IDX_TARGET]  = pSpkVolRange[3];            //Target    //01对应左声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SPK][1][USB_PARA_IDX_CURRENT] = pSpkVolRange[3] - 1;        //Current   //01对应左声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SPK][2][USB_PARA_IDX_TARGET]  = pSpkVolRange[3];            //Target    //02对应右声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SPK][2][USB_PARA_IDX_CURRENT] = pSpkVolRange[3] - 1;        //Current   //02对应右声道

        DeviceInformation.usbAudioVolume[USB_PARA_IDX_MIC][0][USB_PARA_IDX_TARGET]  = pMicVolRange[3];            //Target 
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_MIC][0][USB_PARA_IDX_CURRENT] = pMicVolRange[3] - 1;        //Current
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_MIC][1][USB_PARA_IDX_TARGET]  = pMicVolRange[3];            //Target    //01对应左声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_MIC][1][USB_PARA_IDX_CURRENT] = pMicVolRange[3] - 1;        //Current   //01对应左声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_MIC][2][USB_PARA_IDX_TARGET]  = pMicVolRange[3];            //Target    //02对应右声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_MIC][2][USB_PARA_IDX_CURRENT] = pMicVolRange[3] - 1;        //Current   //02对应右声道

        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SIDETONE][0][USB_PARA_IDX_TARGET] = pSidetoneVolRange[3];       //Target 
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SIDETONE][0][USB_PARA_IDX_CURRENT]= pSidetoneVolRange[3] -1;    //Current
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SIDETONE][1][USB_PARA_IDX_TARGET] = pSidetoneVolRange[3];       //Target    //01对应左声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SIDETONE][1][USB_PARA_IDX_CURRENT]= pSidetoneVolRange[3] - 1;   //Current   //01对应左声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SIDETONE][2][USB_PARA_IDX_TARGET] = pSidetoneVolRange[3];       //Target    //02对应右声道
        DeviceInformation.usbAudioVolume[USB_PARA_IDX_SIDETONE][2][USB_PARA_IDX_CURRENT]= pSidetoneVolRange[3] - 1;   //Current   //02对应右声道
    
        return 0;
    }
    return -1;
}


//检查音量变化，根据dB值计算PCM数据使用的浮点数音量值
//idx = 0, spk; 1, mic, 2, sidetone
//ch = 0, LogicCh1; 1, LogicCh2; ...
//*pNewVol, return new volume in float_t
//*dBVol, return new volume in 16bit dBVol format, which is received from PC, if dBVol is NULL, this function will not write to this pointer
//return 0, vol no change
//return 1, vol changed
int32_t LibDrvUac10VolumeCheck(uint8_t idx, uint8_t ch, float_t *pNewVol, uint16_t *dBVol)
{

    uint16_t VolumeTar;
    uint16_t VolumeCur;

    VolumeTar = DeviceInformation.usbAudioVolume[idx][ch+1][USB_PARA_IDX_TARGET];
    VolumeCur  = DeviceInformation.usbAudioVolume[idx][ch+1][USB_PARA_IDX_CURRENT];
  
    if (dBVol != NULL) {
        *dBVol = VolumeTar;
    }
#if (USB_VOL_DB_CALC_EN == 1)
    if (VolumeTar != VolumeCur) {
        DeviceInformation.usbAudioVolume[idx][ch+1][USB_PARA_IDX_CURRENT] = VolumeTar;
        *pNewVol = powf(2.0f, (((float_t)(int16_t)VolumeTar * 0.00390625f) / 6.0f));

        return 1;
    }
#endif
    return 0;
}


//检查USB连接状态
//checkTime, 当检测到断开连接超过多少ms之后，更新标志位，反馈断开
//return 0, USB和主机处于断开状态
//return 1, USB和主机处于连接状态
uint32_t LibDrvUsbConnectCheck(uint32_t checkTime)
{
    uint32_t TickValue;

    if (OtgDevPara.devState != USB_DEV_STATE_DEFAULT) {
        if ((OTG_DEV_SOF_GET() & 0x3fff) == 0) {
            TickValue = TickGetFunc();
            if ((TickValue - DeviceInformation.UsbConnectTime) < checkTime) 
            {
                return ((DeviceInformation.UsbState) << 0x1b) >> 0x1f;
            }
            OtgDevPara.devState = 2;

            DeviceInformation.UsbState = (DeviceInformation.UsbState & 0xffffffe0);
        }
        else {

            DeviceInformation.UsbConnectTime = TickGetFunc();
            DeviceInformation.UsbState = DeviceInformation.UsbState | 0x10;
        }
    }

    return ((DeviceInformation.UsbState) << 0x1b) >> 0x1f;
}


uint32_t LibDrvUsbFeatureCfg(const void *pSrc, uint32_t byteSize)
{
    LibDrvMemcpy(DeviceInformation.Buffer,pSrc,byteSize);
    return 0;
}

#endif
