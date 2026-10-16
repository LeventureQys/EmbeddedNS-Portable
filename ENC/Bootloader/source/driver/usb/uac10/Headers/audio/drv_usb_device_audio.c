#include "USB\Device\audio\drv_usb_device_audio.h"
//#include "IIS\Drv_i2s.h"
#ifdef DRV_OTG_DEVICE_ENABLE_AUDIO

#pragma diag_suppress=191           //disable warning #191-D
#pragma diag_suppress=1296          //disable warning #1296-D

/* Private define ------------------------------------------------------------*/

#define USB_DEVICE_FUNC_AUDIO_PID			(0x30A1)

#define USB_SPEC_VERSION					USB_SPEC_VERSION1_1

//目前hal支持512或者1024，如果需要更多FIFO，需要重写hal代码
#define FIFO_MAX_SIZE_MIC                   (512*2) //max 1024 bytes, use 2 fifos
#define FIFO_MAX_SIZE_SPK                   (512*2) //max 1024 bytes, use 2 fifos

#define FIFO_USE_NUM_MIC                    (FIFO_MAX_SIZE_MIC/512)
#define FIFO_USE_NUM_SPK                    (FIFO_MAX_SIZE_SPK/512)

#define FIFO_IDX_MIC                        (0) //fifo0 for audio stream mic
#define FIFO_IDX_SPK                        (2) //fifo2 for audio stream spk

#define EP_IDX_MIC                          (USB_AS_MIC_EP_NUM) //for audio stream mic
#define EP_IDX_SPK                          (USB_AS_SPK_EP_NUM) //for audio stream spk

#define TRANS_TYPE_MIC                      (OTG_FIFO_TYPE_ISO)
#define TRANS_TYPE_SPK                      (OTG_FIFO_TYPE_ISO)

#define EP_FIFO_CFG_IDX_MIC                 (0)
#define EP_FIFO_CFG_IDX_SPK                 (1)
#define EP_FIFO_CFG_IDX_MAX                 (2)

#define USB_FEATURE_UNIT_MIN                (0)
#define USB_FEATURE_UNIT_MAX                (1)
#define USB_FEATURE_UNIT_RES                (2)
#define USB_FEATURE_UNIT_SPK                (0)
#define USB_FEATURE_UNIT_MIC                (1)

#define CHL									(0)
#define CHR									(1)

#define FUNC_SPK							0
#define FUNC_MIC							1

typedef struct {
	void (*doCopy)(float volumeLevel,int32_t *destSubframe,int32_t *srcSubframe,uint16_t frameLen);
}AudioSubFrameCopper;

typedef void (*AudioStreamProc)(void);

static const usb_dsc_t appUsbAudioDeviceDescriptor = 
{
    sizeof(usb_dsc_t),                  // bLength
    USB_DSC_DEVICE,                     // bDescriptorType
    USB_SPEC_VERSION,                   // Specification Version(bcd)
    0x00,                               // bDeviceClass
    0x00,                               // bDeviceSubClass
    0x00,                               // bDeviceProtocol
    OTG_MAX_EP0_PACK_SIZE,              // bMaxPacketSize0
    USB_DEFAULT_VID,                    // Vendor ID
    USB_DEVICE_FUNC_AUDIO_PID,          // Product ID
    USB_DEVICE_VERSION,                 // bcdDevice release,Version
    USB_STR_INDEX_MANUFACTURER,         // iManufacturer
    USB_STR_INDEX_PRODUCT,              // iProduct
    USB_STR_INDEX_SERIALNUMBER,         // iSerialNumber
    USB_CFG_DESCRIPTORS,            	// bNumConfigurations
};

static const usb_dsc_cfg_audio_t appUsbAudioConfigDescriptor = 
{
    .dscCfg = 
    {
        .bLength 							= sizeof(usb_dsc_cfg_t),
        .bDescriptorType 					= USB_DSC_CONFIG,
        .wTotalLength 						= sizeof(usb_dsc_cfg_audio_t),
        .bNumInterfaces 					= UAC_IF_NUM,
        .bConfigurationValue 				= 0x01,
        .iConfiguration 					= 0x00,
        .bmAttributes 						= USB_CFG_ATTRIBUTES,
        .bMaxPower 							= (USB_CFG_MAXPOWER/2),
    },
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
                .mHead.bInCollection 		= UAC_IF_STREAMING_NUM,
                .baInterfaceNr = {
                    UAC_IF_ID_STREAMING_MIC,
                    UAC_IF_ID_STREAMING_SPEAKER,    
                },
            },
            .dscCsIfInTerminal0 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_in_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_INPUT_TERMINAL,
                .bTerminalID 				= ID_SPEAKER_INPUT_TERMINAL,
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
                .bUnitID 					= ID_SPEAKER_FEATURE_UNIT,
                .bSourceID 					= ID_SPEAKER_INPUT_TERMINAL,
                .bControlSize 				= 0x02,
                .bmaControls = {
                    UAC_MUTE_CONTROL,
                    UAC_VOLUME_CONTROL,
                    UAC_VOLUME_CONTROL,                
                },
                .iFeature 					= 0x00,
            },
            .dscCsIfOutTerminal0 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_out_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_OUTPUT_TERMINAL,
                .bTerminalID 				= ID_SPEAKER_OUTPUT_TERMINAL,
                .wTerminalType 				= UAC_TT_OUT_HEADPHONES,
                .bAssocTerminal 			= 0x00,
                .bSourceID 					= ID_SPEAKER_FEATURE_UNIT,
                .iTerminal 					= 0x00,
            },
            .dscCsIfInTerminal1 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_in_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_INPUT_TERMINAL,
                .bTerminalID 				= ID_MICROPHONE_INPUT_TERMINAL,
                .wTerminalType 				= UAC_TT_IN_MICROPHONE,
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
                .bUnitID 					= ID_MICROPHONE_FEATURE_UNIT,
                .bSourceID 					= ID_MICROPHONE_INPUT_TERMINAL,
                .bControlSize 				= 0x02,
                .bmaControls = {
                    UAC_MUTE_CONTROL,
                    UAC_VOLUME_CONTROL,
                    UAC_VOLUME_CONTROL,                
                },
                .iFeature 					= 0x00,
            },
            .dscCsIfOutTerminal1 = 
            {
                .bLength 					= sizeof(usb_dsc_audio_cs_if_out_terminal_t),
                .bDescriptorType 			= UAC_CS_INTERFACE,
                .bDescriptorSubtype 		= UAC_OUTPUT_TERMINAL,
                .bTerminalID 				= ID_MICROPHONE_OUTPUT_TERMINAL,
                .wTerminalType 				= UAC_TT_USB_STREAMING,
                .bAssocTerminal 			= 0x00,
                .bSourceID 					= ID_MICROPHONE_FEATURE_UNIT,
                .iTerminal 					= 0x00,
            },
        },
    },
    .dscCsIfSpk = 
    {
        .dscIf = 
        {/* Alt.Setting: 0 */
            .bLength 						= sizeof(usb_dsc_if_t),
            .bDescriptorType 				= USB_DSC_INTERFACE,
            .bInterfaceNumber 				= UAC_IF_ID_STREAMING_SPEAKER,
            .bAlternateSetting 				= USB_AUDIO_IF_ZERO_BANDWIDTH,
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
                    .bInterfaceNumber 		= UAC_IF_ID_STREAMING_SPEAKER,
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
                    .bTerminalLink 			= ID_SPEAKER_INPUT_TERMINAL,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
                    .bNrChannels 			= USB_AUDIO_IF_ALT_SETTING_1_SPK_CH_NUM,
                    .bSubFrameSize 			= USB_AUDIO_IF_ALT_SETTING_1_SPK_BYTE_PERSAMPLE,
                    .bBitResolution 		= USB_AUDIO_IF_ALT_SETTING_1_SPK_BIT_PERSAMPLE,
                    .bSamFreqType 			= 0x01,
                    .tSamFreq = {
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE0,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE1,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE2,
                    },
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= (USB_AS_SPK_EP_NUM|USB_EP_HOST2DEV),
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_SYNCHRONOUS),
                    .wMaxPacketSize 		= USB_AUDIO_EP_MAX_PACKET_SIZE_32_24_STEREO,
                    .bInterval 				= 0x01,
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x00,
                    .bLockDelayUnits 		= 0x00,
                    .wLockDelay 			= 0x0000,
                },
            },
            {/* Alt.Setting: 2 */
                .dscIf = 
                {
                    .bLength 				= sizeof(usb_dsc_if_t),
                    .bDescriptorType 		= USB_DSC_INTERFACE,
                    .bInterfaceNumber 		= UAC_IF_ID_STREAMING_SPEAKER,
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
                    .bTerminalLink 			= ID_SPEAKER_INPUT_TERMINAL,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
                    .bNrChannels 			= USB_AUDIO_IF_ALT_SETTING_2_SPK_CH_NUM,
                    .bSubFrameSize 			= USB_AUDIO_IF_ALT_SETTING_2_SPK_BYTE_PERSAMPLE,
                    .bBitResolution 		= USB_AUDIO_IF_ALT_SETTING_2_SPK_BIT_PERSAMPLE,
                    .bSamFreqType 			= 0x01,
                    .tSamFreq = {
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE0,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE1,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE2,
                    },
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= (USB_AS_SPK_EP_NUM|USB_EP_HOST2DEV),
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_SYNCHRONOUS),
                    .wMaxPacketSize 		= USB_AUDIO_EP_MAX_PACKET_SIZE_32_16_STEREO,
                    .bInterval 				= 0x01,
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint 		= 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x00,
                    .bLockDelayUnits 		= 0x00,
                    .wLockDelay 			= 0x0000,
                },
            },
        }
    },
    .dscCsIfMic = 
    {
        .dscIf = 
        {/* Alt.Setting: 0 */
            .bLength 						= sizeof(usb_dsc_if_t),
            .bDescriptorType 				= USB_DSC_INTERFACE,
            .bInterfaceNumber 				= UAC_IF_ID_STREAMING_MIC,
            .bAlternateSetting 				= USB_AUDIO_IF_ZERO_BANDWIDTH,
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
                    .bTerminalLink 			= ID_MICROPHONE_OUTPUT_TERMINAL,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
					//在audio_hid修改完毕后统一修改宏定义的值，包括下边的packsize
                    .bNrChannels 			= 2,//USB_AUDIO_IF_ALT_SETTING_1_MIC_CH_NUM,
                    .bSubFrameSize 			= 2,//USB_AUDIO_IF_ALT_SETTING_1_MIC_BYTE_PERSAMPLE,
                    .bBitResolution 		= 16,//USB_AUDIO_IF_ALT_SETTING_1_MIC_BIT_PERSAMPLE,
                    .bSamFreqType 			= 0x01,
                    .tSamFreq = {
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE0,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE1,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE2,
                    },
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= (USB_AS_MIC_EP_NUM|USB_EP_DEV2HOST),
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_ASYNCHRONOUS),
                    .wMaxPacketSize 		= USB_AUDIO_EP_MAX_PACKET_SIZE_32_16_STEREO,//USB_AUDIO_IF_ALT_SETTING_1_MIC_EP_PACKET_SIZE,
                    .bInterval 				= 0x01,
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x00,
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
                    .bTerminalLink 			= ID_MICROPHONE_OUTPUT_TERMINAL,
                    .bDelay 				= 0x01,
                    .wFormatTag 			= 0x0001,       //PCM Format
                },
                .dscIfAltSetAsFormatType = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_format_type_t),
                    .bDescriptorType 		= UAC_CS_INTERFACE,
                    .bDescriptorSubtype 	= UAC_FORMAT_TYPE,
                    .bFormatType 			= 0x01,        //FORMAT_TYPE_I
                    .bNrChannels 			= USB_AUDIO_IF_ALT_SETTING_2_MIC_CH_NUM,
                    .bSubFrameSize 			= USB_AUDIO_IF_ALT_SETTING_2_MIC_BYTE_PERSAMPLE,
                    .bBitResolution 		= USB_AUDIO_IF_ALT_SETTING_2_MIC_BIT_PERSAMPLE,
                    .bSamFreqType 			= 0x01,
                    .tSamFreq = {
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE0,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE1,
                        USB_AUDIO_IF_ALT_SETTING_FS32K_BYTE2,
                    },
                },
                .dscIfAltSetEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_endpoint_t),
                    .bDescriptorType 		= USB_DSC_ENDPOINT,
                    .bEndpointAddress 		= (USB_AS_MIC_EP_NUM|USB_EP_DEV2HOST),
                    .bmAttributes 			= (USB_EP_TRANSFER_TYPE_ISO|USB_EP_ASYNCHRONOUS),
                    .wMaxPacketSize 		= USB_AUDIO_EP_MAX_PACKET_SIZE_32_24_STEREO,
                    .bInterval 				= 0x01,
                    .bRefresh 				= 0x00,
                    .bSynchAddress 			= 0x00,
                },
                .dscIfAltSetAsEndpoint = 
                {
                    .bLength 				= sizeof(usb_dsc_cs_if_as_endpoint_t),
                    .bDescriptorType 		= UAC_CS_ENDPOINT,
                    .bDescriptorSubtype 	= UAC_EP_GENERAL,
                    .bmAttributes 			= 0x00,
                    .bLockDelayUnits 		= 0x00,
                    .wLockDelay 			= 0x0000,
                },
            },
        }
    },
};

static const uint16_t audioConfigDescriptorSizeTable[USB_CFG_DESCRIPTORS] = 
{
    sizeof(appUsbAudioConfigDescriptor),
};

static const uint32_t audioConfigDescriptorAddressTable[USB_CFG_DESCRIPTORS] = 
{
    (const uint32_t)&appUsbAudioConfigDescriptor,
};

static const uint16_t audioStringDescriptorSizeTable[USB_STR_DESCRIPTORS] = 
{
    USB_STR_DESC_SIZE_LANG,
    USB_STR_DESC_SIZE_MANUFACTURER,
    USB_STR_DESC_SIZE_PRODUCT,
    USB_STR_DESC_SIZE_SN,
};

static const uint32_t audioStringDescriptorAddressTable[USB_STR_DESCRIPTORS] = 
{
    (const uint32_t)&appUsbStringDescriptorOfLanguage[0],
    (const uint32_t)&appUsbStringDescriptorOfManufacturer[0],
    (const uint32_t)&appUsbStringDescriptorOfProduct[0],
    (const uint32_t)&appUsbStringDescriptorOfSerialNumber[0],
};

static otg_ep_fifo_cfg_t audioEndpointFifoConfig[EP_FIFO_CFG_IDX_MAX] = {
    {
        .epIdx = EP_IDX_MIC,                 
        .fifoInIdx = FIFO_IDX_MIC,  
        .fifoOutIdx = 255,
        .fifo1024 = (FIFO_MAX_SIZE_MIC>>10),    
        .transType = TRANS_TYPE_MIC,
    },
    {
        .epIdx = EP_IDX_SPK,             
        .fifoInIdx = 255,
        .fifoOutIdx = FIFO_IDX_SPK,        
        .fifo1024 = (FIFO_MAX_SIZE_SPK>>10),
        .transType = TRANS_TYPE_SPK,
    },
};

/* Private macro -------------------------------------------------------------*/
#define USB_MIC_FIFO_IN_EN()                (OTG_DEV_INT_FIFO_IN_EN(FIFO_IDX_MIC))
#define USB_MIC_FIFO_IN_DIS()               (OTG_DEV_INT_FIFO_IN_DIS(FIFO_IDX_MIC))

/* Private function declarations ---------------------------------------------*/
static void usb_device_mic_cb(void);
static void usb_device_spk_cb(void);;
static void app_fun_null(void);
static int32_t usb_audio_feature_unit_ctrl_request_handler(uint8_t dir);

/* Private variables ---------------------------------------------------------*/
#define HOST_SCALE_START	0x00
#define MAX_HOST_SCALE		0x64
#define MAX_DAC_SCALE		0x2F
#define MAX_ADC0_SCALE		0x40

static uint8_t usbIfAltSetting[UAC_IF_NUM];
static uint16_t usbAudioVolume[2][2][2];            //[0][ch][]:spk, [1][ch][], mic
static uint8_t usbAudioMute[2][2];                  //[0][]:spk, [1][], mic
static float 	volumeControlLevel[2][2];
static uint16_t usbAudioVolumeCfg[2][3] = 
{//Min, Max, Res
//    {0xA000,0x0000,0x0001},                      //Speaker
//    {0xA000,0x0000,0x0001},                      //Mic
	{0x0000,0x0064,0x0001},                        //Speaker
    {0x0000,0x0064,0x0001},                        //Mic
};

static int is_spk_muted(void){
	return usbAudioMute[FUNC_SPK][0] != 0;
}

static int is_mic_muted(void){
	return usbAudioMute[FUNC_MIC][0] != 0;
}

static void updateVolumeControlLevel(void){
	int func;
	int ch;
	
	for(func = FUNC_SPK; func <= FUNC_MIC; func++){
		for(ch = CHL; ch <= CHR; ch++){
			volumeControlLevel[func][ch] = (float)(usbAudioVolume[func][ch][0] - HOST_SCALE_START) / MAX_HOST_SCALE;
		}
	}
}

//static void updateVolumeControlLevel(int func,int ch){
//	volumeControlLevel[func][ch] = (float)(usbAudioVolume[func][ch][0] - HOST_SCALE_START) / MAX_HOST_SCALE;
//}

static uint8_t testUsbSpkDataBuff[192*3*2];         //用于测试192K/24Bit/2Ch接收数据
static uint8_t testUsbMicDataBuff[192*3*2];         //用于测试192K/24Bit/2Ch发送数据

uint32_t usbSpkPacketCnt = 0;

#define AS_FORMAT_PCM_SUBSIZE_24B					(3)
#define AS_FORMAT_PCM_SUBSIZE_16B					(2)
#define AS_CHS_MONO									(1)
#define AS_CHS_STEREO								(2)

#define AS_USB_PACKET_SIZE_32K						(32)
#define AS_USB_PACKET_SIZE_48K						(48)
#define USB_SPK_CHANNELS                        	AS_CHS_STEREO
#define USB_SPK_DATA_PACKET_SIZE					AS_USB_PACKET_SIZE_32K
#define USB_SPK_DATA_BUFF_PACKETS					(4)
#define USB_SPK_DATA_BUFF_SIZE                      (USB_SPK_DATA_PACKET_SIZE * USB_SPK_DATA_BUFF_PACKETS)      //4级缓存，48K Stereo

#define USB_MIC_CHANNELS                        	AS_CHS_STEREO
#define USB_MIC_DATA_PACKET_SIZE					AS_USB_PACKET_SIZE_32K
#define USB_MIC_DATA_BUFF_PACKETS					(8)
#define USB_MIC_DATA_BUFF_SIZE                      (USB_MIC_DATA_PACKET_SIZE * USB_MIC_DATA_BUFF_PACKETS)      //4级缓存，48K Stereo

static int8_t usbClkSpeedState;                     //0, std, -1, < fs, +1, > fs

static uint8_t usbSpkState;                         //0, idle, 1, playing
static int32_t usbSpkWrIdx;
static int32_t usbSpkRdIdx;
static uint32_t usbSpkDataBuff[USB_SPK_CHANNELS][USB_SPK_DATA_BUFF_SIZE];

static uint8_t usbMicState;                         //0, idle, 1, recording
static int32_t usbMicWrIdx;
static int32_t usbMicRdIdx;
static int32_t isMicDataBufferFull;
static uint32_t usbMicDataBuff[USB_MIC_CHANNELS][USB_MIC_DATA_BUFF_SIZE];

static usb_dev_info_t audioDeviceInformation = 
{
    .numIf = UAC_IF_NUM,
    .numDscCfg = USB_CFG_DESCRIPTORS,
    .numDscStr = USB_STR_DESCRIPTORS,
    .pIfAltSetting = usbIfAltSetting,
    .sizeDscDev = sizeof(appUsbAudioDeviceDescriptor),
    .pDscDev = (uint8_t *)&appUsbAudioDeviceDescriptor,
    .pSizeDscCfg = (uint16_t *)&audioConfigDescriptorSizeTable[0],
    .pSizeDscStr = (uint16_t *)&audioStringDescriptorSizeTable[0],
    .pAddrDscCfg = (uint32_t *)&audioConfigDescriptorAddressTable[0],
    .pAddrDscStr = (uint32_t *)&audioStringDescriptorAddressTable[0],
};

static otg_dev_para_t otgAudioDeviceParameter = {
    .isFullSpeed = 1,
    .epNumInUse = 2,
    .pEpFifoCfg = audioEndpointFifoConfig,
    .pDevInfo = (void *)&audioDeviceInformation,
};

static AudioStreamProc pFunSpkDataRd;
static AudioStreamProc pFunMicDataWr;

/* Private constants ---------------------------------------------------------*/
/* fifo回调函数表格 */
static const UsbFifoIrqHandler usbAudioFifoIraHandler[OTG_MAX_FIFO_NUM][2] = 
{
    /* IN, OUT */
    {usb_device_mic_cb,     	app_fun_null                },  /* FIFO0 */
    {app_fun_null,              app_fun_null                },  /* FIFO1 */
    {app_fun_null,              usb_device_spk_cb       	},  /* FIFO2 */
    {app_fun_null,              app_fun_null                },  /* FIFO3 */
    {app_fun_null,  			app_fun_null                },  /* FIFO4 */
    {app_fun_null,              app_fun_null   				},  /* FIFO5 */
    {app_fun_null,              app_fun_null                },  /* FIFO6 */
    {app_fun_null,              app_fun_null                },  /* FIFO7 */
};

/* Private function prototypes -----------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////
//  共有静态函数
////////////////////////////////////////////////////////////////////////////////
static void app_fun_null(void)
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
//  SPK 相关静态函数
////////////////////////////////////////////////////////////////////////////////
static void usb_spk_fs_adjust(uint32_t sampleRemained)
{
#if 1
    if (usbClkSpeedState == 0)
    {
        if (sampleRemained > AS_USB_PACKET_SIZE_32K*3)
        {
            hal_i2s_clk_fine_adjust(+1);
            usbClkSpeedState = 1;
        }
        else if (sampleRemained < AS_USB_PACKET_SIZE_32K)
        {
            hal_i2s_clk_fine_adjust(-1);
            usbClkSpeedState = -1;
        }
    }
    else if (usbClkSpeedState > 0)
    {
        if (sampleRemained <= AS_USB_PACKET_SIZE_32K*2)
        {
            hal_i2s_clk_fine_adjust(-1);
            usbClkSpeedState = 0;
        }
    }
    else if (usbClkSpeedState < 0)
    {
        if (sampleRemained >= AS_USB_PACKET_SIZE_32K*2)
        {
            hal_i2s_clk_fine_adjust(+1);
            usbClkSpeedState = 0;
        }
    }
#else    
    uint32_t checkNum = sampleRemained % 48;        //因为按照48K采样率，48个点一包数据，所以这里判断数据是否是48的整数倍
    if (usbClkSpeedState == 0)
    {
        if ((checkNum < 24) && (checkNum >= 12))
        {
            hal_i2s_clk_fine_adjust(+1);
            usbClkSpeedState = 1;
        }
        else if ((checkNum >= 24) && (checkNum <= (48-12)))
        {
            hal_i2s_clk_fine_adjust(-1);
            usbClkSpeedState = -1;
        }
    }
    else if (usbClkSpeedState > 0)
    {
        if ((checkNum == 0) || (checkNum > (48-12)))
        {
            hal_i2s_clk_fine_adjust(-1);
            usbClkSpeedState = 0;
        }
    }
    else if (usbClkSpeedState < 0)
    {
        if ((checkNum == 0) || (checkNum < 12))
        {
            hal_i2s_clk_fine_adjust(+1);
            usbClkSpeedState = 0;
        }
    }
#endif
    return;
}
static void usb_spk_stop(void)
{
    usbSpkState = 0;
    usbSpkWrIdx = 0;
    usbSpkRdIdx = 0;
    usb_spk_fs_adjust(0);
    memset(usbSpkDataBuff, 0, sizeof(usbSpkDataBuff));
    return;
}
static void usb_spk_restart(void)
{
    usbSpkWrIdx = 0;
    usbSpkRdIdx = USB_SPK_DATA_BUFF_SIZE/2;
    return;
}
__RAM_CODE static void usb_spk_24bit_2ch(void)
{
    uint32_t rxByteN = OTG_DEV_FIFO_OUT_BYTE_CNT(FIFO_IDX_SPK);
    hal_otg_device_dx_fifo_read(testUsbSpkDataBuff, rxByteN, FIFO_IDX_SPK);
    /*
        默认立体声       
    */
    for (uint32_t i = 0; i < rxByteN; )
    {
        usbSpkDataBuff[0][usbSpkWrIdx] = 0;
        usbSpkDataBuff[0][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+0]) << 8;
        usbSpkDataBuff[0][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+1]) << 16;
        usbSpkDataBuff[0][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+2]) << 24;
        i += 3;
        usbSpkDataBuff[1][usbSpkWrIdx] = 0;
        usbSpkDataBuff[1][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+0]) << 8;
        usbSpkDataBuff[1][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+1]) << 16;
        usbSpkDataBuff[1][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+2]) << 24;
        i += 3;
        usbSpkWrIdx++;
        if (usbSpkWrIdx >= USB_SPK_DATA_BUFF_SIZE)
        {
            usbSpkWrIdx = 0;
        }        
    }
    return;
}
__RAM_CODE static void usb_spk_16bit_2ch(void)
{
    uint32_t rxByteN = OTG_DEV_FIFO_OUT_BYTE_CNT(FIFO_IDX_SPK);
    hal_otg_device_dx_fifo_read(testUsbSpkDataBuff, rxByteN, FIFO_IDX_SPK);
    /*
        后续再根据实际应用修改代码，将收到的数据写入到相应的缓存中去
    */
    for (uint32_t i = 0; i < rxByteN; )
    {
        usbSpkDataBuff[0][usbSpkWrIdx] = 0;
        usbSpkDataBuff[0][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+0]) << 16;
        usbSpkDataBuff[0][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+1]) << 24;
        i += 2;
        usbSpkDataBuff[1][usbSpkWrIdx] = 0;
        usbSpkDataBuff[1][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+0]) << 16;
        usbSpkDataBuff[1][usbSpkWrIdx] += ((uint32_t)testUsbSpkDataBuff[i+1]) << 24;
        i += 2;
        usbSpkWrIdx++;
        if (usbSpkWrIdx >= USB_SPK_DATA_BUFF_SIZE)
        {
            usbSpkWrIdx = 0;
        }        
    }
    return;
}
__RAM_CODE static void usb_device_spk_cb(void)
{
    usbSpkState = 1;
    pFunSpkDataRd();
    usbSpkPacketCnt++;
    return;
}
////////////////////////////////////////////////////////////////////////////////
//  MIC 相关静态函数
////////////////////////////////////////////////////////////////////////////////
//static void usb_mic_fs_adjust(uint32_t sampleRemained)
//{
//    if (usbSpkState == 0)
//    {
//        if (usbClkSpeedState == 0)
//        {
//            if (sampleRemained > AS_USB_PACKET_SIZE_32K*3)
//            {
//                hal_i2s_clk_fine_adjust(-1);
//                usbClkSpeedState = -1;
//            }
//            else if (sampleRemained < AS_USB_PACKET_SIZE_32K)
//            {
//                hal_i2s_clk_fine_adjust(+1);
//                usbClkSpeedState = +1;
//            }
//        }
//        else if (usbClkSpeedState > 0)
//        {
//            if (sampleRemained >= AS_USB_PACKET_SIZE_32K*2)
//            {
//                hal_i2s_clk_fine_adjust(-1);
//                usbClkSpeedState = 0;
//            }
//        }
//        else if (usbClkSpeedState < 0)
//        {
//            if (sampleRemained <= AS_USB_PACKET_SIZE_32K*2)
//            {
//                hal_i2s_clk_fine_adjust(+1);
//                usbClkSpeedState = 0;
//            }
//        }
//    }
//    return;
//}

typedef struct {
	uint16_t mAjustLowerBound;
	uint16_t mAjustUpperBound;
	uint16_t mBoundAjust;
}FineAjustRule;

FineAjustRule usbMicFineAjustRule = {
	.mAjustLowerBound = 2 * AS_USB_PACKET_SIZE_32K,
	.mAjustUpperBound = 6 * AS_USB_PACKET_SIZE_32K,
	.mBoundAjust = 1 * AS_USB_PACKET_SIZE_32K,
};

static void usb_fs_adjust(uint32_t mSamplesInBuffer,const FineAjustRule *rule){
	int8_t ajust = 0;
	
	if(usbClkSpeedState == 0){
		if(mSamplesInBuffer > rule->mAjustUpperBound)
			ajust = -1;
		else if(mSamplesInBuffer < rule->mAjustLowerBound)
			ajust =  1;
	}else if(usbClkSpeedState < 0){
		if(mSamplesInBuffer <= (rule->mAjustLowerBound+rule->mBoundAjust))
			ajust =  1;
	}else{
		if(mSamplesInBuffer >= (rule->mAjustUpperBound-rule->mBoundAjust))
			ajust = -1;
	}
	
	if(ajust != 0){
		usbClkSpeedState += ajust;
		hal_i2s_clk_fine_adjust(ajust);
	}
}

static void usb_mic_fs_adjust(uint32_t mSamplesInBuffer)
{
	if(usbSpkState)
		return;
  
	usb_fs_adjust(mSamplesInBuffer,&usbMicFineAjustRule);
}
static void usb_mic_stop(void)
{
    USB_MIC_FIFO_IN_DIS();
    usbMicState = 0;
    usbMicWrIdx = 0;
    usbMicRdIdx = 0;
    usb_mic_fs_adjust(0);
    memset(usbMicDataBuff, 0, sizeof(usbMicDataBuff));
	
	isMicDataBufferFull = 0;
    return;
}
static void usb_mic_restart(void)
{
    usbMicWrIdx = 0;
	usbMicRdIdx = 2;
    USB_MIC_FIFO_IN_EN();
    return;
}

#define AS_PACKET_BYTESIZE_32K_24B_MONO		(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_24B * AS_CHS_MONO)
#define AS_PACKET_BYTESIZE_32K_24B_STEREO	(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_24B * AS_CHS_STEREO)
#define AS_PACKET_BYTESIZE_32K_16B_MONO		(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_16B * AS_CHS_MONO)
#define AS_PACKET_BYTESIZE_32K_16B_STEREO	(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_16B * AS_CHS_STEREO)

#define AS_PACKET_BYTESIZE_48K_24B_MONO		(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_24B * AS_CHS_MONO)
#define AS_PACKET_BYTESIZE_48K_24B_STEREO	(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_24B * AS_CHS_STEREO)
#define AS_PACKET_BYTESIZE_48K_16B_MONO		(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_16B * AS_CHS_MONO)
#define AS_PACKET_BYTESIZE_48K_16B_STEREO	(AS_USB_PACKET_SIZE_32K * AS_FORMAT_PCM_SUBSIZE_16B * AS_CHS_STEREO)

__RAM_CODE static void usb_mic_24bit_1ch(void)
{
	if(usbMicRdIdx == usbMicWrIdx && !isMicDataBufferFull){
//		memset(testUsbMicDataBuff,0,AS_PACKET_BYTESIZE_32K_24B_MONO);
//		hal_otg_device_dx_fifo_write(testUsbMicDataBuff, AS_PACKET_BYTESIZE_32K_24B_MONO, FIFO_IDX_MIC);
		return;
	}
	
	const uint32_t *micDataL = &usbMicDataBuff[CHL][usbMicRdIdx * AS_USB_PACKET_SIZE_32K];
	
	int idx = 0;
	
	for(int i = 0; i < AS_USB_PACKET_SIZE_32K; i++){
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >>  8);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >> 16);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >> 24);
	}
	
	ASSERT(idx == AS_PACKET_BYTESIZE_32K_24B_MONO);
	
	hal_otg_device_dx_fifo_write(testUsbMicDataBuff, AS_PACKET_BYTESIZE_32K_24B_MONO, FIFO_IDX_MIC);
	
	if(++usbMicRdIdx >= USB_MIC_DATA_BUFF_PACKETS)
		usbMicRdIdx -= USB_MIC_DATA_BUFF_PACKETS;
	
	isMicDataBufferFull = 0;
	
    return;
}

__RAM_CODE static void usb_mic_16bit_2ch(void)
{
	int packetSize = AS_PACKET_BYTESIZE_32K_16B_STEREO;
	if(usbMicRdIdx == usbMicWrIdx && !isMicDataBufferFull){
		memset(testUsbMicDataBuff,0,packetSize);
//		hal_otg_device_dx_fifo_write(testUsbMicDataBuff, packetSize, FIFO_IDX_MIC);
		return;
	}
	
	const uint32_t *micDataL = &usbMicDataBuff[CHL][usbMicRdIdx * AS_USB_PACKET_SIZE_32K];
	const uint32_t *micDataR = &usbMicDataBuff[CHR][usbMicRdIdx * AS_USB_PACKET_SIZE_32K];
	
	int idx = 0;
	
	for(int i = 0; i < AS_USB_PACKET_SIZE_32K; i++){
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >> 16);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >> 24);

		testUsbMicDataBuff[idx++] = (uint8_t)(micDataR[i] >> 16);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataR[i] >> 24);
	}
	
	ASSERT(idx == packetSize);
	
	hal_otg_device_dx_fifo_write(testUsbMicDataBuff, packetSize, FIFO_IDX_MIC);
	
	if(++usbMicRdIdx >= USB_MIC_DATA_BUFF_PACKETS)
		usbMicRdIdx -= USB_MIC_DATA_BUFF_PACKETS;
	
	isMicDataBufferFull = 0;
	
    return;
}

__RAM_CODE static void usb_mic_24bit_2ch(void)
{
	if(usbMicRdIdx == usbMicWrIdx && !isMicDataBufferFull){
//		memset(testUsbMicDataBuff,0,AS_PACKET_BYTESIZE_32K_24B_STEREO);
//		hal_otg_device_dx_fifo_write(testUsbMicDataBuff, AS_PACKET_BYTESIZE_32K_24B_STEREO, FIFO_IDX_MIC);
		return;
	}
	
	const uint32_t *micDataL = &usbMicDataBuff[CHL][usbMicRdIdx * AS_USB_PACKET_SIZE_32K];
	const uint32_t *micDataR = &usbMicDataBuff[CHR][usbMicRdIdx * AS_USB_PACKET_SIZE_32K];
	
	int idx = 0;
	
	for(int i = 0; i < AS_USB_PACKET_SIZE_32K; i++){
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >>  8);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >> 16);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataL[i] >> 24);
		
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataR[i] >>  8);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataR[i] >> 16);
		testUsbMicDataBuff[idx++] = (uint8_t)(micDataR[i] >> 24);
	}
	
	ASSERT(idx == AS_PACKET_BYTESIZE_32K_24B_STEREO);
	
	hal_otg_device_dx_fifo_write(testUsbMicDataBuff,AS_PACKET_BYTESIZE_32K_24B_STEREO,FIFO_IDX_MIC);
	
	if(++usbMicRdIdx >= USB_MIC_DATA_BUFF_PACKETS)
		usbMicRdIdx -= USB_MIC_DATA_BUFF_PACKETS;
	
	isMicDataBufferFull = 0;
	
    return;
}
__RAM_CODE static void usb_device_mic_cb(void)
{
    usbMicState = 1;
    pFunMicDataWr();
    return;
}

//dir: 0, out(spk); 1, in(mic)
//return: 0, ep0 done, 1, ep0 stall, -1, do nothing
static int32_t usb_audio_feature_unit_ctrl_request_handler(uint8_t dir)
{
    int32_t result;
    otg_dev_para_t *pPara = &otgAudioDeviceParameter;
    switch (pPara->devSetupPacket.wValue >> 8)
    {
        case UAC_MUTE_CONTROL:
        {
            pPara->sizeData = pPara->devSetupPacket.wLength;
            pPara->pData = (uint8_t *)&usbAudioMute[dir];
            if (pPara->devSetupPacket.bRequest == UAC_GET_CUR)
            {
                hal_otg_device_ep0_tx_data(pPara);
            }
            result = 0;
            break;
        }
        case UAC_VOLUME_CONTROL:
        {
            switch (pPara->devSetupPacket.bRequest)
            {
                case UAC_SET_CUR:
                {
//                    uint8_t ch = pPara->devSetupPacket.wValue & 0x1;
					//01对应左声道，02对应右声道
					uint8_t ch = (pPara->devSetupPacket.wValue - 1) & 0x03;
                    pPara->pData = (uint8_t *)&usbAudioVolume[dir][ch][0];
                    pPara->sizeData = pPara->devSetupPacket.wLength;
//                    result = -1;
					result = 0;
//					updateVolumeControlLevel();
                    break;
                }
                case UAC_SET_RES:
                {
                    pPara->pData = (uint8_t *)&usbAudioVolumeCfg[dir][USB_FEATURE_UNIT_RES];
                    pPara->sizeData = pPara->devSetupPacket.wLength;
                    result = -1;
                    break;
                }
                case UAC_GET_CUR:
                {
                    uint8_t ch = pPara->devSetupPacket.wValue & 0x1;
                    uint8_t *pCtrlData = (uint8_t *)&usbAudioVolume[dir][ch][0];
                    hal_otg_device_cx_fifo_write(pCtrlData, 2);
                    result = 0;
                    break;
                }
                case UAC_GET_RES:
                {
                    uint8_t *pCtrlData = (uint8_t *)&usbAudioVolumeCfg[dir][USB_FEATURE_UNIT_RES];
                    hal_otg_device_cx_fifo_write(pCtrlData, 2);
                    result = 0;
                    break;
                }
                case UAC_GET_MIN:
                {
                    uint8_t *pCtrlData = (uint8_t *)&usbAudioVolumeCfg[dir][USB_FEATURE_UNIT_MIN];
                    hal_otg_device_cx_fifo_write(pCtrlData, 2);
                    result = 0;
                    break;
                }
                case UAC_GET_MAX:
                {
                    uint8_t *pCtrlData = (uint8_t *)&usbAudioVolumeCfg[dir][USB_FEATURE_UNIT_MAX];
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


/* WEAK function rewrite -----------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////
//  HAL内定义的WEAK函数，此处重写
////////////////////////////////////////////////////////////////////////////////
__RAM_CODE void hal_otg_device_fifo_cb(uint8_t fifoIdx, uint32_t in, uint32_t out)
{
    /* fifo中断响应 */
    if (in != 0)
    {
        usbAudioFifoIraHandler[fifoIdx][0]();
    }
    if (out != 0)
    {
        usbAudioFifoIraHandler[fifoIdx][1]();
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////
//  DRV内定义的WEAK函数，此处重写
////////////////////////////////////////////////////////////////////////////////
__RAM_CODE static __inline void subframeCopyZeroBand(float volumeLevel,int32_t *destSubframe,int32_t *srcSubframe,uint16_t frameLen){
	memset(destSubframe,0,frameLen * sizeof(int32_t));
}
__RAM_CODE static __inline void subframeCopy24B(float volumeLevel,int32_t *destSubframe,int32_t *srcSubframe,uint16_t frameLen){
	int i;
	
	int32_t data;
	
	for(i = 0; i < frameLen; i++){
		data = volumeLevel * (srcSubframe[i] >> 8);
		destSubframe[i] = data << 8;
	}
}

__RAM_CODE static __inline void subframeCopy16B(float volumeLevel,int32_t *destSubframe,int32_t *srcSubframe,uint16_t frameLen){
	int i;
	
	int32_t data;
	
	for(i = 0; i < frameLen; i++){
		data = volumeLevel * (srcSubframe[i] >> 16);
		destSubframe[i] = data << 16;
	}
}

static AudioSubFrameCopper audioSubFrameCopper16B = {
	subframeCopy16B
};
static AudioSubFrameCopper audioSubFrameCopper24B = {
	subframeCopy24B
};
static AudioSubFrameCopper audioSubFrameCopperZeroBand = {
	subframeCopyZeroBand
};

static AudioSubFrameCopper *spkSubFrameCopper = NULL;
static AudioSubFrameCopper *micSubFrameCopper = NULL;
static USBDeviceListener *interfaceListener = NULL;

int32_t drv_usb_if_alt_setting_config(uint8_t ifId, uint8_t ifAltSetting)
{
    int32_t result;
	uint8_t resolution;
    if (ifId == UAC_IF_ID_STREAMING_SPEAKER)
    {
        OTG_DEV_EP_OUT_RSTG_SET(EP_IDX_SPK); 
        OTG_DEV_EP_OUT_RSTG_CLR(EP_IDX_SPK);
        OTG_DEV_FIFO_RESET(FIFO_IDX_SPK);
        switch (ifAltSetting)
        {
            case USB_AUDIO_IF_ZERO_BANDWIDTH:
            {
                /* UsbSpeakerPlayStop */
                usb_spk_stop();
                result = 0;
				spkSubFrameCopper = &audioSubFrameCopperZeroBand;
                break;
            }
            case 1:
            {
                /* IF(Spk).AltSetting(1) */
                usb_spk_restart();
                pFunSpkDataRd = usb_spk_24bit_2ch;
				resolution = USB_AUDIO_RESOLUTION_24B;
				spkSubFrameCopper = &audioSubFrameCopper24B;
				if(interfaceListener != NULL)
					interfaceListener->onLisntnerTouched(interfaceListener->mListenerMinor,&resolution);
                result = 0;
                break;
            }
            case 2:
            {
                /* IF(Spk).AltSetting(2) */
                usb_spk_restart();
                pFunSpkDataRd = usb_spk_16bit_2ch;
				resolution = USB_AUDIO_RESOLUTION_16B;
				spkSubFrameCopper = &audioSubFrameCopper16B;
				if(interfaceListener != NULL)
					interfaceListener->onLisntnerTouched(interfaceListener->mListenerMinor,&resolution);
                result = 0;
                break;
            }
            default:
            {
                result = 1;
                break;
            }
        }
    }
    else if (ifId == UAC_IF_ID_STREAMING_MIC)
    {
        OTG_DEV_EP_IN_RSTG_SET(EP_IDX_MIC);
        OTG_DEV_EP_IN_RSTG_CLR(EP_IDX_MIC);
        OTG_DEV_FIFO_RESET(FIFO_IDX_MIC);
        switch (ifAltSetting)
        {
            case USB_AUDIO_IF_ZERO_BANDWIDTH:
            {
                /* UsbMicRecStop */
                usb_mic_stop();
                result = 0;
				micSubFrameCopper = &audioSubFrameCopperZeroBand;
                break;
            }
            case 1:
            {
                /* IF(Mic).AltSetting(1) */
//                pFunMicDataWr = usb_mic_24bit_1ch;
				pFunMicDataWr = usb_mic_16bit_2ch;
				micSubFrameCopper = &audioSubFrameCopper16B;
                usb_mic_restart();
                result = 0;
                break;
            }
            case 2:
            {
                /* IF(Mic).AltSetting(2) */
                pFunMicDataWr = usb_mic_24bit_2ch;
				micSubFrameCopper = &audioSubFrameCopper24B;
                usb_mic_restart();
                result = 0;
                break;
            }
            default:
            {
                result = 1;
                break;
            }
        }
    }
    else
    {
        result = 1;
    }
    return result;
}

//return 0, set ep0 done
//return 1, set ep0 stall
//return -1, do nothing, prepare to receive
int32_t drv_usb_class_cmd(setup_packet_t *pDevSetupPacket)
{
    int32_t result;
    if (pDevSetupPacket->bmRequestType.b.recipient == USB_SETUP_REQ_RECIPIENT_INTERFACE)
    {
        if ((pDevSetupPacket->wIndex >> 8) == ID_SPEAKER_FEATURE_UNIT)
        {
            /* Spk Feature Unit */
            result = usb_audio_feature_unit_ctrl_request_handler(0);
        }
        else if ((pDevSetupPacket->wIndex >> 8) == ID_MICROPHONE_FEATURE_UNIT)
        {
            /* Mic Feature Unit */
            result = usb_audio_feature_unit_ctrl_request_handler(1);
        }
        else
        {
                result = 1;
        }
    }
    else
    {
        result = 1;
    }
    return result;    
}

/* Exported functions --------------------------------------------------------*/
//处理音量变化控制，通过修改codec的播放增益寄存器实现
//static void usb_audio_volume_process(void){
//	uint16_t 			scale = 0;
//	float 				percent;
//	int 				ch;
//	I2sCodecFunction 	func;
//	
//	MuteSwitch mute;
//	for(func = DAC; func <= ADC0; func++){
//		for(ch = CHL; ch <= CHR; ch++){
//			ASSERT(usbAudioVolume[func][ch][0] >= HOST_SCALE_START || usbAudioVolume[func][ch][0] == 0);
//			percent	= (usbAudioVolume[func][ch][0] - HOST_SCALE_START) * 1.0 / MAX_HOST_SCALE;
//			scale = percent * (func == ADC0 ? MAX_ADC0_SCALE : MAX_DAC_SCALE) + 0.5;
//			app_i2s_codec_volume_control(func,ch,scale);
//		}
//		
//		mute = usbAudioMute[func][0] ? MUTEON : MUTEOFF;
//		app_i2s_codec_mute_control(func,CHL,mute);
//		app_i2s_codec_mute_control(func,CHR,mute);
//	}
//}

/* 主循环调用*/
void drv_usb_device_audio_process(void)
{
    /* 主循环中处理：音量变化，HID发码的判断是否开打发送FIFO中断，USBSPK的断开检测处处理 */
//	usb_audio_volume_process();
	updateVolumeControlLevel();
    return;
}

static void usb_device_volume_init(void){
	float percent;
	uint16_t scale;
	
	percent = 41.0 / MAX_DAC_SCALE;
	scale = percent * MAX_HOST_SCALE + HOST_SCALE_START;
	usbAudioVolume[0][0][0] = scale;
	usbAudioVolume[0][1][0] = scale;
	volumeControlLevel[0][0] = percent;
	volumeControlLevel[0][1] = percent;
	
	percent = 28.0 / MAX_ADC0_SCALE;
	scale = percent * MAX_HOST_SCALE + HOST_SCALE_START;
	usbAudioVolume[1][0][0] = scale;
	usbAudioVolume[1][1][0] = scale;
	volumeControlLevel[1][0] = percent;
	volumeControlLevel[1][1] = percent;
}

void drv_usb_device_audio_init(void)
{
    /* vbus config to 1 */
    HAL_SYS_USB_ID_SET();
    HAL_SYS_USB_VBUS_EN();
    
	usb_device_volume_init();
    hal_otg_device_init(&otgAudioDeviceParameter);
    
    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)otgAudioDeviceParameter.pDevInfo;
    pDevInfo->dscCfg_bmAttributes = appUsbAudioConfigDescriptor.dscCfg.bmAttributes;
	
    return;
}

__RAM_CODE void drv_usb_device_irq_handler(void)
{
    hal_otg_device_irq(&otgAudioDeviceParameter);
    return;
}

#define SPK_AS_PACKET_SIZE	AS_USB_PACKET_SIZE_32K
#define MIC_AS_PACKET_SIZE	AS_USB_PACKET_SIZE_32K

__RAM_CODE static __inline void spkSubframeCopy(int subframe,int32_t *destSubframe,uint32_t *srcSubframe,uint16_t frameLen){
	ASSERT(spkSubFrameCopper != NULL);
	float volumeLevel = is_spk_muted() ? 0 : volumeControlLevel[FUNC_SPK][subframe];
	spkSubFrameCopper->doCopy(volumeLevel,destSubframe,(int32_t*)srcSubframe,frameLen);
}
__RAM_CODE static __inline void micSubframeCopy(int subframe,uint32_t *destSubframe,int32_t *srcSubframe,uint16_t frameLen){
	ASSERT(micSubFrameCopper != NULL);
	float volumeLevel = is_mic_muted() ? 0 : volumeControlLevel[FUNC_SPK][subframe];
	micSubFrameCopper->doCopy(volumeLevel,(int32_t*)destSubframe,srcSubframe,frameLen);
}

static int32_t drv_usb_device_spk_read_samples(int32_t *frameCh1, int32_t *frameCh2, uint16_t frameLen){
	if (usbSpkWrIdx == usbSpkRdIdx)
    {
        memset(frameCh1, 0x00, frameLen*4);
        memset(frameCh2, 0x00, frameLen*4);
    }
    else
    {
        uint32_t num = (USB_SPK_DATA_BUFF_SIZE - usbSpkRdIdx);
        if (num >= frameLen)
        {
//            memcpy(frameCh1, &usbSpkDataBuff[0][usbSpkRdIdx], frameLen*4);
//            memcpy(frameCh2, &usbSpkDataBuff[1][usbSpkRdIdx], frameLen*4);
			spkSubframeCopy(CHL,frameCh1,&usbSpkDataBuff[CHL][usbSpkRdIdx],frameLen);
			spkSubframeCopy(CHR,frameCh2,&usbSpkDataBuff[CHR][usbSpkRdIdx],frameLen);
            usbSpkRdIdx += frameLen;
        }
        else
        {
//            memcpy(frameCh1, &usbSpkDataBuff[0][usbSpkRdIdx], num*4);
//            memcpy(frameCh2, &usbSpkDataBuff[1][usbSpkRdIdx], num*4);
			spkSubframeCopy(CHL,frameCh1,&usbSpkDataBuff[CHL][usbSpkRdIdx],num);
			spkSubframeCopy(CHR,frameCh2,&usbSpkDataBuff[CHR][usbSpkRdIdx],num);
            frameCh1 += num;
            frameCh2 += num;
            num = frameLen - num;
//            memcpy(frameCh1, &usbSpkDataBuff[0][0], num*4);
//            memcpy(frameCh2, &usbSpkDataBuff[1][0], num*4);
			spkSubframeCopy(CHL,frameCh1,&usbSpkDataBuff[CHL][0],num);
			spkSubframeCopy(CHR,frameCh2,&usbSpkDataBuff[CHR][0],num);
            usbSpkRdIdx = num;
        }
        int32_t sampleRemained = usbSpkWrIdx - usbSpkRdIdx;
        if (sampleRemained < 0)
        {
            sampleRemained += USB_SPK_DATA_BUFF_SIZE;
        }
        usb_spk_fs_adjust(sampleRemained);
    }
	
	return frameLen;
}
static int32_t drv_usb_device_mic_write_samples(int32_t *frameCh1, int32_t *frameCh2, uint16_t frameLen){
	ASSERT(frameLen == MIC_AS_PACKET_SIZE);

	if(usbMicWrIdx == usbMicRdIdx && isMicDataBufferFull){
		usb_mic_fs_adjust(0);
		return 0;
	}
	
	int32_t freeBuffer;
	
//	memcpy(&usbMicDataBuff[CHL][usbMicWrIdx * MIC_AS_PACKET_SIZE], frameCh1, frameLen * sizeof(int32_t));
//  memcpy(&usbMicDataBuff[CHR][usbMicWrIdx * MIC_AS_PACKET_SIZE], frameCh2, frameLen * sizeof(int32_t));
	
	micSubframeCopy(CHL,&usbMicDataBuff[CHL][usbMicWrIdx * MIC_AS_PACKET_SIZE],frameCh1,frameLen);
	micSubframeCopy(CHR,&usbMicDataBuff[CHR][usbMicWrIdx * MIC_AS_PACKET_SIZE],frameCh2,frameLen);
	
	if(++usbMicWrIdx >= USB_MIC_DATA_BUFF_PACKETS){
		usbMicWrIdx -= USB_MIC_DATA_BUFF_PACKETS;
		if(usbMicWrIdx == usbMicRdIdx)
			isMicDataBufferFull = 1;
	}
	
//	freeBuffer = usbMicWrIdx - usbMicRdIdx;
	if((freeBuffer = usbMicWrIdx - usbMicRdIdx) < 0)
		freeBuffer += USB_MIC_DATA_BUFF_PACKETS;
	
	usb_mic_fs_adjust(freeBuffer * MIC_AS_PACKET_SIZE);
	
	return 0;
}

__RAM_CODE int32_t drv_usb_device_spk_read_from_audio(int32_t *frameCh1, int32_t *frameCh2, uint16_t frameLen)
{
    ASSERT(frameLen == SPK_AS_PACKET_SIZE);
	
	if(!is_usb_device_initialized() || drv_usb_device_spk_read_samples(frameCh1,frameCh2,SPK_AS_PACKET_SIZE) <= 0){
		memset(frameCh1,0,sizeof(int32_t) * frameLen);
		memset(frameCh2,0,sizeof(int32_t) * frameLen);
		return SPK_AS_PACKET_SIZE;
	}
	
	return frameLen;
}

__RAM_CODE int32_t drv_usb_device_mic_write_to_audio(int32_t *frameCh1, int32_t *frameCh2, uint16_t frameLen)
{
	ASSERT(frameLen == MIC_AS_PACKET_SIZE);
	
	if(!is_usb_device_initialized())
		return 0;

    return drv_usb_device_mic_write_samples(frameCh1,frameCh2,MIC_AS_PACKET_SIZE);
}

void drv_usb_device_audio_clear_spk_packet(void) {
		usbSpkPacketCnt = 0;
}
int32_t drv_usb_device_audio_get_spk_state(void){
	return usbSpkState;
}

int32_t drv_usb_device_audio_get_mic_state(void){
	return usbMicState;
}
__INLINE static void usb_device_irq_save(uint32_t *status){
	*status = NVIC_GetEnableIRQ(USB_IRQn);
	NVIC_DisableIRQ(USB_IRQn);
}
__INLINE static void usb_device_irq_restore(const uint32_t newStat){
	uint32_t oldStat = NVIC_GetEnableIRQ(USB_IRQn);
	if(newStat != 0 && oldStat == 0){
		NVIC_EnableIRQ(USB_IRQn);
	}else if(newStat == 0 && oldStat != 0){
		NVIC_DisableIRQ(USB_IRQn);
	}
}
int32_t drv_usb_device_audio_register_listener(USBDeviceListener *listener){
	ASSERT(listener != NULL);
	ASSERT(listener->onLisntnerTouched != NULL);
	// 最多只能注册一个listener
	ASSERT(interfaceListener == NULL);
	
	uint32_t status = 0;
	
	usb_device_irq_save(&status);
	interfaceListener = listener;
	usb_device_irq_restore(status);
	
	return 0;
}

USB_DEV_STATE drv_usb_device_get_devstate_func_audio(void)
{
    return otgAudioDeviceParameter.devState;
}


#endif
