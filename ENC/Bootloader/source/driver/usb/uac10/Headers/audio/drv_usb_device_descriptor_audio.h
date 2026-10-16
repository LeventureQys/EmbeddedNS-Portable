#ifndef __USB_DEVICE_DESCRIPTOR_AUDIO_H__
#define __USB_DEVICE_DESCRIPTOR_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Class-Specific Audio Control Interface */
/*
	这里将Audio Control接口头描述符进行了拆分，分为固定长度的头和可变长度的body，body部分
	是一个接口号数组，说明与该Audio Control对应的流接口，如AudioStreaming,MidiStreaming
	完整的Audio Control接口头因为与具体应用相关，所以应该放到具体的应用定义中取，此处为通
	用共享的部分，在需要时包含该头文件即可使用。
*/
__packed typedef struct {
	uint8_t     bLength;                                // Size of this descriptor in bytes, (=(0x08+UAC_IF_STREAMING_NUM))
    uint8_t     bDescriptorType;                        // CS_INTERFACE Descriptor Type, (=UAC_CS_INTERFACE)
    uint8_t     bDescriptorSubtype;                     // HEADER descriptor subtype, (=UAC_HEADER)
    uint16_t    bcdADC;                                 // Audio Device compliant to the USB Audio specification version 1.00, (=0x0100)
    uint16_t    wTotalLength;                           // Total number of bytes returned for the class-specific AudioControl interface descriptor.
    uint8_t     bInCollection;                          // The number of AudioStreaming and MIDIStreaming interfaces in the Audio Interface Collection to which this AudioControl interface belongs, (=UAC_IF_STREAMING_NUM)
}usb_dsc_audio_cs_if_header_head_t;

__packed typedef struct
{/* Audio Control Input Terminal Descriptor */
    uint8_t     bLength;                                // Size of the descriptor, in bytes, (=0x0C)
    uint8_t     bDescriptorType;                        // CS_INTERFACE Descriptor Type, (=UAC_CS_INTERFACE)
    uint8_t     bDescriptorSubtype;                     // INPUT_TERMINAL descriptor subtype, (=UAC_INPUT_TERMINAL)
    uint8_t     bTerminalID;                            // ID of this Input Terminal
    uint16_t    wTerminalType;                          // Terminal Type, (=UAC_TT_USB_STREAMING)
    uint8_t     bAssocTerminal;                         // No association
    uint8_t     bNrChannels;                            // Two Channel, (=0x02)
    uint16_t    wChannelConfig;                         // Left, Right, (=0x0003)
    uint8_t     iChannelNames;                          // Unused. Shall be set to 0x00.
    uint8_t     iTerminal;                              // Index of a string descriptor, describing the input terminal. If no string is provided, should be set to 0x00.
}usb_dsc_audio_cs_if_in_terminal_t;

__packed typedef struct
{/* Audio Control Output Terminal Descriptor */
    uint8_t     bLength;                                // Size of the descriptor, in bytes, (=0x09)
    uint8_t     bDescriptorType;                        // CS_INTERFACE Descriptor Type, (=UAC_CS_INTERFACE)
    uint8_t     bDescriptorSubtype;                     // INPUT_TERMINAL descriptor subtype, (=UAC_OUTPUT_TERMINAL)
    uint8_t     bTerminalID;                            // ID of this Output Terminal
    uint16_t    wTerminalType;                          // Terminal Type, (=UAC_TT_OUT_SPEAKER)
    uint8_t     bAssocTerminal;                         // No association
    uint8_t     bSourceID;                              // From Feature Unit.
    uint8_t     iTerminal;                              // Index of a string descriptor, describing the input terminal. If no string is provided, should be set to 0x00.
}usb_dsc_audio_cs_if_out_terminal_t;

__packed typedef struct
{/* Audio Control Feature Unit Descriptor */
    uint8_t     bLength;                                // Size of the descriptor, in bytes, (=0x0D)
    uint8_t     bDescriptorType;                        // CS_INTERFACE Descriptor Type, (=UAC_CS_INTERFACE)
    uint8_t     bDescriptorSubtype;                     // INPUT_TERMINAL descriptor subtype, (=UAC_FEATURE_UNIT)
    uint8_t     bUnitID;                                // ID of this Feature Unit
    uint8_t     bSourceID;                              // The ID of the Entity to which this FeatureUnit is connected.
    uint8_t     bControlSize;                           // 2 bytes for each element of the bmaControls array.
    uint8_t     bmaControls[3];                         // Mute, VolLeft, VolRight, (=0x01, 0x02, 0x02)
    uint8_t     iFeature;                               // Index of a string descriptor, describing the input terminal. If no string is provided, should be set to 0x00.
}usb_dsc_audio_cs_if_feature_unit_t;

__packed typedef struct
{/* Audio Control Mixer Unit Descriptor */
    uint8_t     bLength;                                // Size of the descriptor, in bytes, (=0x0D)
    uint8_t     bDescriptorType;                        // CS_INTERFACE Descriptor Type, (=UAC_CS_INTERFACE)
    uint8_t     bDescriptorSubtype;                     // INPUT_TERMINAL descriptor subtype, (=UAC_MIXER_UNIT)
    uint8_t     bUnitID;                                // ID of this Feature Unit
    uint8_t     bNrInPins;                              // Number of input pins on the mixer unit
    uint8_t     baSourceID[2];                          // Array of IDs of the units or terminals feeding into this mixer unit
    uint8_t     bNrChannels;                            // Two Channel, (=0x02)
    uint16_t    wChannelConfig;                         // Left, Right, (=0x0003)
    uint8_t     iChannelNames;                          // Unused. Shall be set to 0x00.
    uint8_t     bmControls;                             // Bitmask of controls for this mixer unit (e.g., volume, mute)  
    uint8_t     iMixer;                                 // Index of a string descriptor describing the mixer unit
}usb_dsc_audio_cs_if_mixer_unit_t;

__packed typedef struct
{/* Audio Streaming Interface Descriptor */
    uint8_t     bLength;                                // Size of this descriptor, in bytes, (=0x7)
    uint8_t     bDescriptorType;                        // CS_INTERFACE Descriptor Type, (=UAC_CS_INTERFACE)
    uint8_t     bDescriptorSubtype;                     // AS_GENERAL subtype, (=UAC_AS_GENERAL)    
    uint8_t     bTerminalLink;                          // Terminal ID of the Input Terminal, associated with this interface.
    uint8_t     bDelay;                                 // Total interface delay, expressed in frames. Not used and shall be set to 0x00.
    uint16_t    wFormatTag;                             // PCM Format, (=0x0001)
}usb_dsc_cs_if_as_general_t;

__packed typedef struct
{/* Audio Streaming Format Type Descriptor */
    uint8_t     bLength;                                // Size of this descriptor, in bytes, (=0xB)
    uint8_t     bDescriptorType;                        // CS_INTERFACE Descriptor Type, (=UAC_CS_INTERFACE)
    uint8_t     bDescriptorSubtype;                     // FORMAT_TYPE subtype, (=UAC_FORMAT_TYPE)    
    uint8_t     bFormatType;                            // FORMAT_TYPE_I, (=0x01)
    uint8_t     bNrChannels;                            // channel number
    uint8_t     bSubFrameSize;                          // PCM Format, (=0x0001)
    uint8_t     bBitResolution;                         // bits number per sample.
    uint8_t     bSamFreqType;                           // Number of frequencies supported
    uint8_t     tSamFreq[12];                            // samplerate (samplerate size = 3*bSamFreqType)
}usb_dsc_cs_if_as_format_type_t;

__packed typedef struct
{/* Endpoint Descriptor */
    uint8_t     bLength;                                // Size of this descriptor, in bytes, (=0x9)
    uint8_t     bDescriptorType;                        // ENDPOINT descriptor type, (=USB_DSC_ENDPOINT)
    uint8_t     bEndpointAddress;                       // ep number and direction
    uint8_t     bmAttributes;                           // Isochronous, Asynchronized, data endpoint
    uint16_t    wMaxPacketSize;                         // number of bytes per packet.
    uint8_t     bInterval;                              // number of packet per frame.
    uint8_t     bRefresh;                               // Unused. Shall be set to 0x00.
    uint8_t     bSynchAddress;                          // Unused. Shall be set to 0x00.
}usb_dsc_cs_if_endpoint_t;

__packed typedef struct
{/* Audio Streaming Isochronous Audio Data Endpoint Descriptor */
    uint8_t     bLength;                                // Size of this descriptor, in bytes, (=0x7)
    uint8_t     bDescriptorType;                        // CS_ENDPOINT Descriptor Type, (=UAC_CS_ENDPOINT)
    uint8_t     bDescriptorSubtype;                     // GENERAL subtype, (=UAC_EP_GENERAL)    
    uint8_t     bmAttributes;                           // indicates that the mentioned Control(Sampling Frequency) is supported by this endpoint.
    uint8_t     bLockDelayUnits;                        // Unused. Shall be set to 0x00.
    uint16_t    wLockDelay;                             // Unused. Shall be set to 0x00.
}usb_dsc_cs_if_as_endpoint_t;


typedef usb_dsc_audio_cs_if_header_head_t	ACInterfaceHeaderHead;
typedef usb_dsc_audio_cs_if_in_terminal_t 	ACITerminalDescriptor;
typedef usb_dsc_audio_cs_if_feature_unit_t	ACFeatureUnitDescriptor;
typedef usb_dsc_audio_cs_if_out_terminal_t	ACOterminalDescriptor;
typedef usb_dsc_cs_if_as_general_t			ASClassificInterfaceDescriptor;
typedef usb_dsc_cs_if_as_format_type_t		ASFormatTypeDescriptor;
typedef usb_dsc_cs_if_endpoint_t			ASDataEndpointStandardDescriptor;
typedef usb_dsc_cs_if_as_endpoint_t			ASDataEndpointClassificDescriptor;

#ifdef __cplusplus
}
#endif

#endif
