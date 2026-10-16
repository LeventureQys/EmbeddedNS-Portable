/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    usb_device_class_audio.h
 * @brief   USB device class definition for audio1.0 devices
    | Version | Data       | Author     | Info                                |
    | ------- | ---------- | ---------- | ----------------------------------- |
    | 1.0.0   | 2021/09/28 | huangzheng | First version                       |
    | 1.0.1   | 2022/04/08 | huangzheng | Add copyright info                  |
 * @version V1.0.1
 * @date    2022/04/08
 * @author  huangzheng
 ******************************************************************************
 */

#ifndef __USB_DEVICE_CLASS_AUDIO_H
#define __USB_DEVICE_CLASS_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Audio Interface Class Code */
#define UAC_AUDIO                                       0x01
/* Audio Interface Subclass Codes */
#define UAC_SUBCLASS_UNDEFINED                          0x00
#define UAC_AUDIOCONTROL                                0x01
#define UAC_AUDIOSTREAMING                              0x02
#define UAC_MIDISTREAMING                               0x03
/* Audio Interface Protocol Codes */
#define UAC_PR_PROTOCOL_UNDEFINED                       0x00
/* Audio Class-Specific Descriptor Types */
#define UAC_CS_UNDEFINED                                0x20
#define UAC_CS_DEVICE                                   0x21
#define UAC_CS_CONFIGURATION                            0x22
#define UAC_CS_STRING                                   0x23
#define UAC_CS_INTERFACE                                0x24
#define UAC_CS_ENDPOINT                                 0x25
/* Audio Class-Specific AC Interface Descriptor Subtypes, AC: AudioControl */
#define UAC_AC_DESCRIPTOR_UNDEFINED                     0x00
#define UAC_HEADER                                      0x01
#define UAC_INPUT_TERMINAL                              0x02
#define UAC_OUTPUT_TERMINAL                             0x03
#define UAC_MIXER_UNIT                                  0x04
#define UAC_SELECTOR_UNIT                               0x05
#define UAC_FEATURE_UNIT                                0x06
#define UAC_PROCESSING_UNIT                             0x07
#define UAC_EXTENSION_UNIT                              0x08
/* Audio Class-Specific AS Interface Descriptor Subtypes, AS: AudioStreaming */
#define UAC_AS_DESCRIPTOR_UNDEFINED                     0x00
#define UAC_AS_GENERAL                                  0x01
#define UAC_FORMAT_TYPE                                 0x02
#define UAC_FORMAT_SPECIFIC                             0x03
/* Processing Unit Process Types */
#define UAC_PROCESS_UNDEFINED                           0x00
#define UAC_UPDOWNMIX_PROCESS                           0x01
#define UAC_DOLBY_PROLOGIC_PROCESS                      0x02
#define UAC_3D_STEREO_EXTENDER_PROCESS                  0x03
#define UAC_REVERBERATION_PROCESS                       0x04
#define UAC_CHORUS_PROCESS                              0x05
#define UAC_DYN_RANGE_COMP_PROCESS                      0x06
/* Audio Class-Specific Endpoint Descriptor Subtypes */
#define UAC_DESCRIPTOR_UNDEFINED                        0x00
#define UAC_EP_GENERAL                                  0x01
/* Audio Class-Specific Request Codes */
#define UAC_REQUEST_CODE_UNDEFINED                      0x00
#define UAC_SET_CUR                                     0x01
#define UAC_GET_CUR                                     0x81
#define UAC_SET_MIN                                     0x02
#define UAC_GET_MIN                                     0x82
#define UAC_SET_MAX                                     0x03
#define UAC_GET_MAX                                     0x83
#define UAC_SET_RES                                     0x04
#define UAC_GET_RES                                     0x84
#define UAC_SET_MEM                                     0x05
#define UAC_GET_MEM                                     0x85
#define UAC_GET_STAT                                    0xFF
/* Control Selector Codes */
/* Terminal Control Selectors */
#define UAC_TE_CONTROL_UNDEFINED                        0x00
#define UAC_COPY_PROTECT_CONTROL                        0x01
/* Feature Unit Control Selectors */
#define UAC_FU_CONTROL_UNDEFINED                        0x00
#define UAC_FU_MUTE_CONTROL                             0x01
#define UAC_FU_VOLUME_CONTROL                           0x02
#define UAC_FU_BASS_CONTROL                             0x03
#define UAC_FU_MID_CONTROL                              0x04
#define UAC_FU_TREBLE_CONTROL                           0x05
#define UAC_FU_GRAPHIC_EQUALIZER_CONTROL                0x06
#define UAC_FU_AUTOMATIC_GAIN_CONTROL                   0x07
#define UAC_FU_DELAY_CONTROL                            0x08
#define UAC_FU_BASS_BOOST_CONTROL                       0x09
#define UAC_FU_LOUDNESS_CONTROL                         0x0A
/* Processing Unit Control Selectors */
/* Extension Unit Control Selectors */
/* Endpoint Control Selectors */
#define UAC_EP_CONTROL_UNDEFINED                        0x00
#define UAC_SAMPLING_FREQ_CONTROL                       0x01
#define UAC_PITCH_CONTROL                               0x02
/* USB Terminal Types, I/O */
#define UAC_TT_USB_UNDEFINED                            (0x0100)        // USB Terminal, undefined Type.
#define UAC_TT_USB_STREAMING                            (0x0101)        // A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface. The AudioStreaming interface descriptor points to the associated Terminal through the bTerminalLink field.
#define UAC_TT_USB_VENDOR_SPECIFIC                      (0x01FF)        // A Terminal dealing with a signal carried over a vendor-specific interface. The vendor-specific interface descriptor must contain a field that references the Terminal.
/* Input Terminal Types, I */
#define UAC_TT_INPUT_UNDEFINED                          (0x0200)        // Input Terminal, undefined Type.
#define UAC_TT_INPUT_MICROPHONE                         (0x0201)        // A generic microphone that does not fit under any of the other classifications.
#define UAC_TT_INPUT_DESKTOP_MICROPHONE                 (0x0202)        // A microphone normally placed on the desktop or integrated into the monitor.
#define UAC_TT_INPUT_PERSONAL_MICROPHONE                (0x0203)        // A head-mounted or clip-on microphone.
#define UAC_TT_INPUT_OMNI_DIRECTIONAL_MICROPHONE        (0x0204)        // A microphone designed to pick up voice from more than one speaker at relatively long ranges.
#define UAC_TT_INPUT_MICROPHONE_ARRAY                   (0x0205)        // An array of microphones designed for directional processing using host-based signal processing algorithms.
#define UAC_TT_INPUT_PROCESSING_MICROPHONE_ARRAY        (0x0206)        // An array of microphones with an embedded signal processor.
/* Output Terminal Types, O */
#define UAC_TT_OUTPUT_UNDEFINED                         (0x0300)        // Output Terminal, undefined Type.
#define UAC_TT_OUTPUT_SPEAKER                           (0x0301)        // A generic speaker or set of speakers that does not fit under any of the other classifications.
#define UAC_TT_OUTPUT_HEADPHONES                        (0x0302)        // A head-mounted audio output device.
#define UAC_TT_OUTPUT_HEAD_MOUNTED_DISPLAY_AUDIO        (0x0303)        // The audio part of a VR head mounted display. The Associated Interfaces descriptor can be used to reference the HID interface used to report the position and orientation of the HMD.
#define UAC_TT_OUTPUT_DESKTOP_SPEAKER                   (0x0304)        // Relatively small speaker or set of speakers normally placed on the desktop or integrated into the monitor. These speakers are close to the user and have limited stereo separation.
#define UAC_TT_OUTPUT_ROOM_SPEAKER                      (0x0305)        // Larger speaker or set of speakers that are heard well anywhere in the room.
#define UAC_TT_OUTPUT_COMMUNICATION_SPEAKER             (0x0306)        // Speaker or set of speakers designed for voice communication.
#define UAC_TT_OUTPUT_LOW_FREQUENCY_EFFECTS_SPEAKER     (0x0307)        // Speaker designed for low frequencies (subwoofer). Not capable of reproducing speech or music.
/* Bi-directional Terminal Types, I/O */
#define UAC_TT_BI_DIR_UNDEFINED                         (0x0400)        // Bi-directional Terminal, undefined Type.
#define UAC_TT_HANDSET                                  (0x0401)        // Hand-held bi-directional audio device.
#define UAC_TT_HEADSET                                  (0x0402)        // Head-mounted bi-directional audio device.
#define UAC_TT_SPEAKERPHONE_NO_ECHO_REDUCTION           (0x0403)        // A hands-free audio device designed for host-based echo cancellation.
#define UAC_TT_SPEAKERPHONE_ECHO_SUPPRESSING            (0x0404)        // A hands-free audio device with echo suppression capable of half-duplex operation
#define UAC_TT_SPEAKERPHONE_ECHO_CANCELING              (0x0405)        // A hands-free audio device with echo cancellation capable of full-duplex operation
/* Telephony Terminal Types, I/O */
#define UAC_TT_TELEPHONY_UNDEFINED                      (0x0500)        // Telephony Terminal, undefined Type.
#define UAC_TT_PHONE_LINE                               (0x0501)        // May be an analog telephone line jack, an ISDN line, a proprietary PBX interface, or a wireless link.
#define UAC_TT_TELEPHONE                                (0x0502)        // Device can be used as a telephone. When not in use as a telephone, handset is used as a bi-directional audio device.
#define UAC_TT_DOWN_LINE_PHONE                          (0x0503)        // A standard telephone set connected to the device. When not in use as a telephone, it can be used as a bidirectional audio device.
/* External Terminal Types, I/O */
#define UAC_TT_EXTERNAL_UNDEFINED                       (0x0600)        // External Terminal, undefined Type.
#define UAC_TT_ANALOG_CONNECTOR                         (0x0601)        // A generic analog connector.
#define UAC_TT_DIGITAL_AUDIO_INITERFACE                 (0x0602)        // A generic digital audio interface.
#define UAC_TT_LINE_CONNECTOR                           (0x0603)        // An analog connector at standard line levels. Usually uses 3.5mm.
#define UAC_TT_LEGACY_AUDIO_CONNECTOR                   (0x0604)        // An input connector assumed to be connected to the lineout of the legacy audio system of the host computer. Used for backward compatibility.
#define UAC_TT_SPDIF_INTERFACE                          (0x0605)        // An S/PDIF digital audio interface. The Associated Interface descriptor can be used to reference an interface used for controlling special functions of this interface.
#define UAC_TT_1394_DA_STREAM                           (0x0606)        // An interface to audio streams on a 1394 bus.
#define UAC_TT_1394_DV_STREAM_SOUNDTRACK                (0x0607)        // An interface to soundtrack of A/V stream on a 1394 bus.
/* Audio Class-Specific MS Interface Descriptor SubTypes, MS: MidiStreaming */
#define UAC_MS_DESCRIPTOR_UNDEFINED                     0x00
#define UAC_MS_HEADER                                   0x01
#define UAC_MIDI_IN_JACK                                0x02
#define UAC_MIDI_OUT_JACK                               0x03
#define UAC_ELEMENT                                     0x04
/* Audio Class-Specific MS MIDI IN and OUT Jack types */
#define UAC_MS_JACK_TYPE_UNDEFINED                      0x00
#define UAC_MS_EMBEDDED                                 0x01
#define UAC_MS_EXTERNAL                                 0x02

/* UAC sample rate */
#define UAC_SAMPLE_RATE_11K                             (11025)
#define UAC_SAMPLE_RATE_16K                             (16000)
#define UAC_SAMPLE_RATE_22K                             (22050)
#define UAC_SAMPLE_RATE_32K                             (32000)
#define UAC_SAMPLE_RATE_44K                             (44100)
#define UAC_SAMPLE_RATE_48K                             (48000)
#define UAC_SAMPLE_RATE_88K                             (88200)
#define UAC_SAMPLE_RATE_96K                             (96000)
#define UAC_SAMPLE_RATE_176K                            (176400)
#define UAC_SAMPLE_RATE_192K                            (192000)
#define UAC_SAMPLE_RATE_352K                            (352800)
#define UAC_SAMPLE_RATE_384K                            (384000)

#ifdef __cplusplus
}
#endif

#endif
