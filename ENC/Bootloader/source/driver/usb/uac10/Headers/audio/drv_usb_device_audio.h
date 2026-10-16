#ifndef __USB_DEVICE_AUDIO_H__
#define __USB_DEVICE_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "USB\Device\drv_usb_device_private.h"
#include "USB\Device\drv_usb_device.h"

#ifdef DRV_OTG_DEVICE_ENABLE_AUDIO
#include "USB\Device\audio\drv_usb_device_descriptor_audio.h"

#define USB_CFG_DESCRIPTORS                         	(1)

/* Current usb interface number */
#define UAC_IF_ID_CONTROL                               (0)
#define UAC_IF_ID_STREAMING_SPEAKER                     (1)
#define UAC_IF_ID_STREAMING_MIC                         (2)
#define UAC_IF_NUM                                      (3)
#define UAC_IF_STREAMING_NUM                            (2)     //StreamingSpeaker, StreamingMic, if include midi, this number should add 1.

/* Current usb endpoint number definitions */
#define USB_AS_MIC_EP_NUM                               1
#define USB_AS_SPK_EP_NUM                               2

/* Current usb interface alternate setting definitions */
#define USB_AUDIO_IF_ZERO_BANDWIDTH                     (0)
#define USB_AUDIO_IF_FULL_BANDWIDTH                     (1)
#define USB_AUDIO_IF_ALT_SETTING_SPK_NUM                (2)
#define USB_AUDIO_IF_ALT_SETTING_MIC_NUM                (2)

/* configuration of spk/mic interface, Alt.Setting 1~4 */
#define USB_AUDIO_IF_ALT_SETTING_1_SPK_CH_NUM           (2)             //mono = 1, stereo = 2,
#define USB_AUDIO_IF_ALT_SETTING_1_SPK_BYTE_PERSAMPLE   (3)             //16bit = 2, 24bit = 3,
#define USB_AUDIO_IF_ALT_SETTING_1_SPK_BIT_PERSAMPLE    (24)            //16bit = 16, 24bit = 24,
#define USB_AUDIO_IF_ALT_SETTING_1_SPK_FS_BYTE0         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE0)      //use fs 48 definition
#define USB_AUDIO_IF_ALT_SETTING_1_SPK_FS_BYTE1         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE1)
#define USB_AUDIO_IF_ALT_SETTING_1_SPK_FS_BYTE2         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE2)
#define USB_AUDIO_IF_ALT_SETTING_1_SPK_EP_PACKET_SIZE   (USB_AUDIO_EP_MAX_PACKET_SIZE_48_24_STEREO)   //use packet size of 48fs/16bit/mono

#define USB_AUDIO_IF_ALT_SETTING_2_SPK_CH_NUM           (2)             //mono = 1, stereo = 2,
#define USB_AUDIO_IF_ALT_SETTING_2_SPK_BYTE_PERSAMPLE   (2)             //16bit = 2, 24bit = 3,
#define USB_AUDIO_IF_ALT_SETTING_2_SPK_BIT_PERSAMPLE    (16)            //16bit = 16, 24bit = 24,
#define USB_AUDIO_IF_ALT_SETTING_2_SPK_FS_BYTE0         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE0)      //use fs 48 definition
#define USB_AUDIO_IF_ALT_SETTING_2_SPK_FS_BYTE1         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE1)
#define USB_AUDIO_IF_ALT_SETTING_2_SPK_FS_BYTE2         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE2)
#define USB_AUDIO_IF_ALT_SETTING_2_SPK_EP_PACKET_SIZE   (USB_AUDIO_EP_MAX_PACKET_SIZE_48_16_STEREO)   //use packet size of 48fs/16bit/mono

#define USB_AUDIO_IF_ALT_SETTING_1_MIC_CH_NUM           (1)             //mono = 1, stereo = 2,
#define USB_AUDIO_IF_ALT_SETTING_1_MIC_BYTE_PERSAMPLE   (3)             //16bit = 2, 24bit = 3,
#define USB_AUDIO_IF_ALT_SETTING_1_MIC_BIT_PERSAMPLE    (24)            //16bit = 16, 24bit = 24,
#define USB_AUDIO_IF_ALT_SETTING_1_MIC_FS_BYTE0         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE0)
#define USB_AUDIO_IF_ALT_SETTING_1_MIC_FS_BYTE1         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE1)
#define USB_AUDIO_IF_ALT_SETTING_1_MIC_FS_BYTE2         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE2)
#define USB_AUDIO_IF_ALT_SETTING_1_MIC_EP_PACKET_SIZE   (USB_AUDIO_EP_MAX_PACKET_SIZE_48_24_MONO)

#define USB_AUDIO_IF_ALT_SETTING_2_MIC_CH_NUM           (2)             //mono = 1, stereo = 2,
#define USB_AUDIO_IF_ALT_SETTING_2_MIC_BYTE_PERSAMPLE   (3)             //16bit = 2, 24bit = 3,
#define USB_AUDIO_IF_ALT_SETTING_2_MIC_BIT_PERSAMPLE    (24)            //16bit = 16, 24bit = 24,
#define USB_AUDIO_IF_ALT_SETTING_2_MIC_FS_BYTE0         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE0)
#define USB_AUDIO_IF_ALT_SETTING_2_MIC_FS_BYTE1         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE1)
#define USB_AUDIO_IF_ALT_SETTING_2_MIC_FS_BYTE2         (USB_AUDIO_IF_ALT_SETTING_FS48K_BYTE2)
#define USB_AUDIO_IF_ALT_SETTING_2_MIC_EP_PACKET_SIZE   (USB_AUDIO_EP_MAX_PACKET_SIZE_48_24_STEREO)

/* Current usb interface audio control IDs of terminal/unit */
#define ID_SPEAKER_INPUT_TERMINAL                       (0x01)
#define ID_SPEAKER_OUTPUT_TERMINAL                      (0x02)
#define ID_MICROPHONE_INPUT_TERMINAL                    (0x03)
#define ID_MICROPHONE_OUTPUT_TERMINAL                   (0x04)
#define ID_SPEAKER_FEATURE_UNIT                         (0x10)
#define ID_MICROPHONE_FEATURE_UNIT                      (0x11)

/* Audio Control Interface Header Descriptor */
/*
	这里将Audio Control接口头描述符进行了拆分，分为固定长度的头和可变长度的body，body部分
	是一个接口号数组，说明与该Audio Control对应的流接口，如AudioStreaming,MidiStreaming
*/
__packed typedef struct
{
	usb_dsc_audio_cs_if_header_head_t	mHead;
    uint8_t     						baInterfaceNr[UAC_IF_STREAMING_NUM];    // Interface ID value.
}usb_dsc_audio_cs_if_header_t;

__packed typedef struct
{
    usb_dsc_audio_cs_if_header_t         	dscCsIfHeader;
    usb_dsc_audio_cs_if_in_terminal_t    	dscCsIfInTerminal0;
    usb_dsc_audio_cs_if_feature_unit_t   	dscCsIfFeatureUnit0;
    usb_dsc_audio_cs_if_out_terminal_t   	dscCsIfOutTerminal0;
    usb_dsc_audio_cs_if_in_terminal_t    	dscCsIfInTerminal1;
    usb_dsc_audio_cs_if_feature_unit_t   	dscCsIfFeatureUnit1;
    usb_dsc_audio_cs_if_out_terminal_t   	dscCsIfOutTerminal1;
}usb_dsc_cs_if_ac_t;

__packed typedef struct
{
    usb_dsc_if_t                        	dscIf;
    usb_dsc_cs_if_ac_t                  	dscCsIfAc;
}usb_dsc_cs_if_audio_ctrl_t;

__packed typedef struct
{
    usb_dsc_if_t                            dscIf;
    usb_dsc_cs_if_as_general_t              dscIfAltSetAsGeneral;
    usb_dsc_cs_if_as_format_type_t          dscIfAltSetAsFormatType;
    usb_dsc_cs_if_endpoint_t                dscIfAltSetEndpoint;
    usb_dsc_cs_if_as_endpoint_t             dscIfAltSetAsEndpoint;
}usb_dsc_cs_if_as_alt_setting_all_t;

__packed typedef struct
{
    usb_dsc_if_t                            dscIf;                                               	/* Interface:ID(Speaker), Alt.Setting:0 */
    usb_dsc_cs_if_as_alt_setting_all_t      dscIfSpkAltSetting[USB_AUDIO_IF_ALT_SETTING_SPK_NUM];   /* Interface:ID(Speaker), Alt.Setting:1~USB_AUDIO_IF_ALT_SETTING_SPK_NUM */    
}usb_dsc_cs_if_audio_spk_t;

/* Class-Specific Audio Mic Interface */
__packed typedef struct
{
    usb_dsc_if_t                            dscIf;                                               	/* Interface:ID(Mic), Alt.Setting:0 */
    usb_dsc_cs_if_as_alt_setting_all_t      dscIfMicAltSetting[USB_AUDIO_IF_ALT_SETTING_MIC_NUM];   /* Interface:ID(Mic), Alt.Setting:1~USB_AUDIO_IF_ALT_SETTING_MIC_NUM */    
}usb_dsc_cs_if_audio_mic_t;

__packed typedef struct
{
    usb_dsc_cfg_t                       	dscCfg;
    usb_dsc_cs_if_audio_ctrl_t         		dscCsIfCtrl;
    usb_dsc_cs_if_audio_spk_t           	dscCsIfSpk;
    usb_dsc_cs_if_audio_mic_t           	dscCsIfMic;
}usb_dsc_cfg_audio_t;

#include <stdint.h>

#define USB_AUDIO_LISTENER_MINOR_RES		(0)

/** @brief  Usb loop process, must be called in main loop
  * @param  None
  * @retval None
  */
void 	drv_usb_device_audio_process(void);
/** @brief  Usb device initial, must be called before use usb device
  * @param  None
  * @retval None
  */
void 	drv_usb_device_audio_init(void);
/** @brief  Irq handler, must called in irq
  * @param  None
  * @retval None
  */
void 	drv_usb_device_irq_handler(void);

int32_t drv_usb_device_spk_read_from_audio(int32_t *pDstCh1, int32_t *pDstCh2, uint16_t nSamples);

int32_t drv_usb_device_mic_write_to_audio(int32_t *pSrcCh1, int32_t *pSrcCh2, uint16_t nSamples);

void 	drv_usb_device_audio_clear_spk_packet(void);

int 	drv_usb_device_audio_register_listener(USBDeviceListener *listener);

int32_t drv_usb_device_audio_get_spk_state(void);
int32_t drv_usb_device_audio_get_mic_state(void);

USB_DEV_STATE 
        drv_usb_device_get_devstate_func_audio(void);


#ifdef __cplusplus
}
#endif


#endif

#endif
