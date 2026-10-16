/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    lib_drv_uac10_config.h
 * @brief   The configuration of USB device class definition for audio1.0 devices and hid
 * @version V1.3.2
 * @date    2024/05/09
 * @author  huangzheng
 ******************************************************************************
 */

#ifndef __LIB_DRV_UAC10_CONFIG_H
#define __LIB_DRV_UAC10_CONFIG_H

/*
    // 此处信息需要使用者更改配置之后，将该库相关信息更新，方便使用者快速了解该库支持项
    // SPK：48K，16/24Bit，2Ch
    // MIC：48K，16/24Bit，2Ch
    // HID-COL0：标准线控播放支持
    //           IN,    ReportID = USB_HID_IN_PLAY_CTRL_ID, 音量+(0x01)/音量-(0x02)/播放暂停(0x04)/停止(0x08)/下一首(0x10)（需要播放器支持）/上一首(0x20)（需要播放器支持）/快进(0x40)（需要播放器支持）/快退(0x80)（需要播放器支持）
    //           IN,    ReportID = USB_HID_IN_SPK_MUTE_ID, 播放静音(0x01)
    // HID-COL1: UC通信相关支持
    //           IN,    ReportID = USB_HID_IN_TEL_ID, Redial(0x01)/HookSwitch(0x02)/PhoneMute(0x04)/Flash(0x08)/Hold(0x10)/Park(0x20)/LineBusyTone(0x40)/Reject(0x80)
    //           OUT,   ReportID = USB_HID_OUT_LED_ID, OffHook(0x01)/Mute(0x02)/Ring(0x04)/Hold(0x08)
    // HID-COL2: IN,    ReportID = USB_HID_IN_ASP_ID, 0x01
    // HID-COL3：自定义传输，支持16字节/64字节包收发
    // 如需其他配置，请联系本库提供者，进行重新修改配置编译新库，自行修改此文件无效
*/

/* Includes -----------------------------------------------------------------*/
#include "usb_device_class_audio.h"
#include "usb_device_class_hid.h"
#include "usb_device_standard.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
;-------- <<< Use Configuration Wizard in Context Menu >>> -------------------
*/
/* User Config --------------------------------------------------------------*/
/*****************************************************************************/
//<o> USB电流配置 (mA) <1-500:1>
//<i> 配置USB电流属性，根据实际需求进行修改，最大500mA
#define USB_CFG_MAXPOWER                                (100)   // 1~500
//<o> USB速度选择 <0=>High Speed <1=>Full Speed
//<i> 仅在FullSpeed带宽不够情况下选择HighSpeed
#define USB_SPEED_SEL_FS                                (1)     // 0, HS, 1, FS       //by Niko 192K需要开启HS
//<q> USB音量对数数值计算
//<i> 选择库内是否直接支持USB音量的对数计算
//<i> 支持：库内需使用powf函数进行计算，疯狂拖动音量条可能造成运算速度的瓶颈
//<i> 不支持：库内不进行计算，返回的对数数值，用户根据实际对数值的范围和精度要求来通过查表实现，可以提升速度
#define USB_VOL_DB_CALC_EN                              (1)
//<h> 字符串信息配置
//<q0> string of manufacturer enable
//<q1> string of product enable
//<q2> string of serial number enable
//<q3> string of audio interface
//<q4> string of sidetone
//<q5> string of UCQ
//<q6> string of mfi interface
#define USB_STR_MANUFACTURER_EN                         (1)
#define USB_STR_PRODUCT_EN                              (1)
#define USB_STR_SERIALNUMBER_EN                         (1)
#define USB_STR_IF_AUDIO_EN                             (0)
#define USB_STR_SIDETONE_EN                             (0)
#define USB_STR_UCQ_EN                                  (0)
#define USB_STR_IF_MFI_EN                               (0)
//</h>
//<h> USB功能选择
//<q0> USB FUN SPK
//<q1> USB FUN MIC
//<q2> USB FUN MIDI
//<q3> USB FUN HID
//<q4> USB FUN MFI
#define USB_FUN_SPK_EN                                  (0)
#define USB_FUN_MIC_EN                                  (0)
#define USB_FUN_MIDI_EN                                 (0)     //by Niko 暂无实现
#define USB_FUN_HID_EN                                  (1)
#define USB_FUN_MFI_EN                                  (0)     //by Niko 暂无实现
//<q> FeatureUnit MIC AGC Enable
#define UAC_FU_AGC_CTRL_EN                              (0)     //by Niko 暂无实现
//<q> FeatureUnit Sidetone Enable 
#define UAC_ID_SIDETONE_UNIT_ENABLE                     (0)             // 1, enable sidetone unit and mixer unit (only mic and spk are enabled), 0, disable
/* HID报表描述符配置，根据需要支持的HID报表进行选择，每支持一个项，HID多一个Collection */
//<q0> HID Collection User Transport Enable
//<q1> HID Collection Player Control Enable
//<q2> HID Collection Player Control for SPK-MUTE Enable
//<q3> HID Collection UC Control Enable
//<q4> HID Collection UC Control MIC-MUTE abs Enable
//<q5> HID Collection Keyboard Enable
#define USB_HID_COL_USER_TRANS_EN                       (1)             // 自定义HID传输
#define USB_HID_COL_PLAYER_CTRL_EN                      (0)             // 播放器HID控制
#define USB_HID_COL_PLAYER_CTRL_SPK_MUTE_EN             (0)             // 播放器HID控制支持SPK-MUTE
#define USB_HID_COL_UC_CTRL_EN                          (0)             // UC通信类HID控制
#define USB_HID_COL_UC_MIC_MUTE_ABS                     (0)             // 1, MIC-MUTE使用绝对值, 0, MIC-MUTE使用相对值
#define USB_HID_COL_KEYBOARD_EN                         (0)             // 1, 支持键盘输入到PC和键盘LED灯状态, 0, 不支持
//</h>
/* 如果对Windows下的SPK图标有要求，可以更改此项，可选择 UAC_TT_OUTPUT_SPEAKER，显示喇叭图案 */
//<h> USB终点类型选择（影响windows界面显示图标）
//<o0> Output Terminal Type <0x0402=>HEADSET <0x0301=>SPEAKER <0x0302=>HEADPHONES
//<o1> Input Terminal Type <0x0402=>HEADSET <0x0201=>MICROPHONE
#define UAC_ID_OUTPUT_TERMINAL_SPEAKER_TYPE             (0x0302)        //UAC_TT_HEADSET,UAC_TT_OUTPUT_SPEAKER,UAC_TT_OUTPUT_HEADPHONES
#define UAC_ID_INPUT_TERMINAL_MICROPHONE_TYPE           (0x0201)        //UAC_TT_INPUT_MICROPHONE,UAC_TT_HEADSET
//</h>
//<h> SPK属性配置
//<o0> 通道支持模式 <2=>2Ch
//<o1> 位深支持模式 <0=>16Bit <1=>24Bit <2=>16/24Bit
//<o2> 采样率支持模式 <3=>48K <5=>48/96K <7=>48/96/192K <4=>44.1/48K <6=>44.1/48/96K <8=>44.1/48/96/192K 
#define UAC_SPK_CH_MODE                                 (2)
#define UAC_SPK_BIT_MODE                                (2)
#define UAC_SPK_FS_MODE                                 (4)
//</h>
//<h> MIC属性配置
//<o0> 通道支持模式 <1=>1Ch <2=>2Ch
//<o1> 位深支持模式 <0=>16Bit <1=>24Bit <2=>16/24Bit
//<o2> 采样率支持模式 <3=>48K <5=>48/96K <7=>48/96/192K <4=>44.1/48K <6=>44.1/48/96K <8=>44.1/48/96/192K 
#define UAC_MIC_CH_MODE                                 (2)
#define UAC_MIC_BIT_MODE                                (2)
#define UAC_MIC_FS_MODE                                 (4)
//</h>
/*
;------------- <<< end of configuration section >>> ---------------------------
*/
/*****************************************************************************/
/* PID/VID定义，无需修改，库提供接口函数进行配置 */
#define USB_DEFAULT_VID                                 0x352F//by Niko 可以更改
#define USB_DEFAULT_PID                                 0x0111//by Niko 可以更改
/* USB版本号，无需修改，默认USB2.0的版本号 */
#define USB_SPEC_VERSION                                0x0200
/* USB设备版本号，无需修改，库提供接口函数进行配置 */
#define USB_DEVICE_VERSION                              0x0001
/* UAC版本号，不可更改，目前是audio1.0 */
#define UAC_SPEC_VERSION                                0x0100
/* HID版本号，不可更改，目前是hid1.11 */
#define HID_SPEC_VERSION                                0x0111
/* 配置描述符数量，目前仅支持1组 */
#define USB_CFG_DESCRIPTORS_NUM                         (1)
/* HID报表描述符数量，目前仅支持1组 */
#define HID_CLASS_DESCRIPTORS_NUM                       (1)     // Number of HID class descriptors
/* 配置属性 attributes，无需修改，如需支持RemoteWakeup，可添加属性(|USB_CFG_REMOTEWAKEUP)，对应代码可能需要做修改以支持RemoteWakeup，用到时再进行测试确认 */
#define USB_CFG_ATTRIBUTES                              (USB_CFG_DEFAULT)//|USB_CFG_REMOTEWAKEUP)
/* 定义MIDI的端点一包最大字节数，一般都是64，没有特别需求无需改动，如更改需要确认代码正确性 */
#define USB_EP_PACK_SIZE_MIDI                           (64)
/* 定义HID的端点一包最大字节数，一般都是64，没有特别需求无需改动，如更改需要确认代码正确性 */
#define USB_EP_PACK_SIZE_HID                            (64)
/* 定义MFI的端点一包最大字节数，一般都是64，没有特别需求无需改动，如更改需要确认代码正确性 */
#define USB_EP_PACK_SIZE_MFI                            (64)

/* 下列配置根据功能选择自动完成配置，不可更改 */
/* 各字符串描述符的索引号，无需修改，如果需要增加额外一些描述符字符串，需要对应修改描述符相关代码 */
#define USB_STR_MANUFACTURER_INDEX                      ((USB_STR_MANUFACTURER_EN != 0) ? 1 : 0)
#define USB_STR_MANUFACTURER_INDEX_NEXT                 ((USB_STR_MANUFACTURER_EN != 0) ? (USB_STR_MANUFACTURER_INDEX + 1) : 1)
#define USB_STR_PRODUCT_INDEX                           ((USB_STR_PRODUCT_EN != 0) ? (USB_STR_MANUFACTURER_INDEX_NEXT) : 0)
#define USB_STR_PRODUCT_INDEX_NEXT                      ((USB_STR_PRODUCT_EN != 0) ? (USB_STR_PRODUCT_INDEX + 1) : USB_STR_MANUFACTURER_INDEX_NEXT)
#define USB_STR_SERIALNUMBER_INDEX                      ((USB_STR_SERIALNUMBER_EN != 0) ? (USB_STR_PRODUCT_INDEX_NEXT) : 0)
#define USB_STR_SERIALNUMBER_INDEX_NEXT                 ((USB_STR_SERIALNUMBER_EN != 0) ? (USB_STR_SERIALNUMBER_INDEX + 1) : USB_STR_PRODUCT_INDEX_NEXT)
#define USB_STR_IF_AUDIO_INDEX                          ((USB_STR_IF_AUDIO_EN != 0) ? (USB_STR_SERIALNUMBER_INDEX_NEXT) : 0)
#define USB_STR_IF_AUDIO_INDEX_NEXT                     ((USB_STR_IF_AUDIO_EN != 0) ? (USB_STR_IF_AUDIO_INDEX + 1) : USB_STR_SERIALNUMBER_INDEX_NEXT)
#define USB_STR_SIDETONE_INDEX                          ((USB_STR_SIDETONE_EN != 0) ? (USB_STR_IF_AUDIO_INDEX_NEXT) : 0)
#define USB_STR_SIDETONE_INDEX_NEXT                     ((USB_STR_SIDETONE_EN != 0) ? (USB_STR_SIDETONE_INDEX + 1) : USB_STR_IF_AUDIO_INDEX_NEXT)
#define USB_STR_UCQ_INDEX                               ((USB_STR_UCQ_EN != 0) ? (USB_STR_SIDETONE_INDEX_NEXT) : 0)
#define USB_STR_UCQ_INDEX_NEXT                          ((USB_STR_UCQ_EN != 0) ? (USB_STR_UCQ_INDEX + 1) : USB_STR_SIDETONE_INDEX_NEXT)
#define USB_STR_IF_MFI_INDEX                            ((USB_STR_IF_MFI_EN != 0) ? (USB_STR_UCQ_INDEX_NEXT) : 0)
#define USB_STR_IF_MFI_INDEX_NEXT                       ((USB_STR_IF_MFI_EN != 0) ? (USB_STR_IF_MFI_INDEX + 1) : USB_STR_UCQ_INDEX_NEXT)
#define USB_STR_DESCRIPTORS_NUM                         (USB_STR_IF_MFI_INDEX_NEXT) // 上述不需要的配置成0，其他按照顺序排列
#define USB_STR_IF_AUDIO_FU_INDEX                       (0)     // 只能在0~最大值内选择
#define USB_STR_IF_AUDIO_MIDI_INDEX                     (0)     // 只能在0~最大值内选择
/* 端点号配置，可根据实际需求调整，序号范围：1~8，总数量必须正确，为了便于阅读，将不使用的EP配置成-1 */
/* 接口数量和ID号配置，不使用的必须配置成-1，所有有效的ID必须从0开始排列 */
/* AudioSpk/AudioMic/AudioMidi, 每个占用一个流ID号，如：仅SPK或者仅MIC的话，那就配置成1，如果SPK/MIC都存在，那就配置成2，如果在SPK/MIC都存在的基础上还有MIDI，那就配置成3 */
#define USB_EP_NUM_START                                (1)
#define USB_IF_ID_START                                 (0)
#if (USB_FUN_SPK_EN) || (USB_FUN_MIC_EN) || (USB_FUN_MIDI_EN)
    #define UAC_IF_ID_CONTROL                           (USB_IF_ID_START)               // Interface, AudioCtrl的ID号
    #if USB_FUN_SPK_EN
        #define USB_EP_NUM_AS_SPK                       (USB_EP_NUM_START)              // Endpoint, SPK端点号
        #define USB_EP_NUM_AS_SPK_NEXT                  (USB_EP_NUM_AS_SPK + 1)
        #define UAC_IF_ID_STREAMING_SPK                 (UAC_IF_ID_CONTROL + 1)         // Interface, AudioStreamSpk的ID号
        #define UAC_IF_ID_STREAMING_SPK_NEXT            (UAC_IF_ID_STREAMING_SPK + 1)
    #else
        #define USB_EP_NUM_AS_SPK                       (-1)
        #define USB_EP_NUM_AS_SPK_NEXT                  (USB_EP_NUM_START)
        #define UAC_IF_ID_STREAMING_SPK                 (-1)
        #define UAC_IF_ID_STREAMING_SPK_NEXT            (UAC_IF_ID_CONTROL + 1)
    #endif
    #if USB_FUN_MIC_EN
        #define USB_EP_NUM_AS_MIC                       (USB_EP_NUM_AS_SPK_NEXT)        // Endpoint, MIC端点号
        #define USB_EP_NUM_AS_MIC_NEXT                  (USB_EP_NUM_AS_MIC + 1)
        #define UAC_IF_ID_STREAMING_MIC                 (UAC_IF_ID_STREAMING_SPK_NEXT)  // Interface, AudioStreamMic的ID号
        #define UAC_IF_ID_STREAMING_MIC_NEXT            (UAC_IF_ID_STREAMING_MIC + 1)
    #else
        #define USB_EP_NUM_AS_MIC                       (-1)
        #define USB_EP_NUM_AS_MIC_NEXT                  (USB_EP_NUM_AS_SPK_NEXT)
        #define UAC_IF_ID_STREAMING_MIC                 (-1)
        #define UAC_IF_ID_STREAMING_MIC_NEXT            (UAC_IF_ID_STREAMING_SPK_NEXT)
    #endif
    #if USB_FUN_MIDI_EN
        #define USB_EP_NUM_AS_MIDI                      (USB_EP_NUM_AS_MIC_NEXT)        // Endpoint, MIDI端点号
        #define USB_EP_NUM_AS_MIDI_NEXT                 (USB_EP_NUM_AS_MIDI + 1)
        #define UAC_IF_ID_STREAMING_MIDI                (UAC_IF_ID_STREAMING_MIC_NEXT)  // Interface, AudioStreamMidi的ID号
        #define UAC_IF_ID_STREAMING_MIDI_NEXT           (UAC_IF_ID_STREAMING_MIDI + 1)
    #else
        #define USB_EP_NUM_AS_MIDI                      (-1)
        #define USB_EP_NUM_AS_MIDI_NEXT                 (USB_EP_NUM_AS_MIC_NEXT)
        #define UAC_IF_ID_STREAMING_MIDI                (-1)
        #define UAC_IF_ID_STREAMING_MIDI_NEXT           (UAC_IF_ID_STREAMING_MIC_NEXT)
    #endif
    #define UAC_IF_ID_STREAMING_NUM                     (UAC_IF_ID_STREAMING_MIDI_NEXT - 1)     // Interface, 流数量
    /* 必须和UAC_IF_ID_STREAMING_NUM值匹配，对花括号内定义的内容进行增减调整 */
    #if !(USB_FUN_SPK_EN)
        #if !(USB_FUN_MIC_EN)
            #define UAC_IF_ID_STREAMING_ALL             {UAC_IF_ID_STREAMING_MIDI}
        #elif !(USB_FUN_MIDI_EN)
            #define UAC_IF_ID_STREAMING_ALL             {UAC_IF_ID_STREAMING_MIC}
        #else
            #define UAC_IF_ID_STREAMING_ALL             {UAC_IF_ID_STREAMING_MIC, UAC_IF_ID_STREAMING_MIDI}
        #endif
    #elif !(USB_FUN_MIC_EN)
        #if !(USB_FUN_MIDI_EN)
            #define UAC_IF_ID_STREAMING_ALL             {UAC_IF_ID_STREAMING_SPK}
        #else
            #define UAC_IF_ID_STREAMING_ALL             {UAC_IF_ID_STREAMING_SPK, UAC_IF_ID_STREAMING_MIDI}
        #endif
    #else
        #if !(USB_FUN_MIDI_EN)
            #define UAC_IF_ID_STREAMING_ALL             {UAC_IF_ID_STREAMING_SPK, UAC_IF_ID_STREAMING_MIC}
        #else
            #define UAC_IF_ID_STREAMING_ALL             {UAC_IF_ID_STREAMING_SPK, UAC_IF_ID_STREAMING_MIC, UAC_IF_ID_STREAMING_MIDI}
        #endif
    #endif
#else
    #define USB_EP_NUM_AS_SPK                           (-1)
    #define USB_EP_NUM_AS_MIC                           (-1)
    #define USB_EP_NUM_AS_MIDI                          (-1)
    #define USB_EP_NUM_AS_MIDI_NEXT                     (USB_EP_NUM_START)
    #define UAC_IF_ID_CONTROL                           (-1)
    #define UAC_IF_ID_STREAMING_SPK                     (-1)
    #define UAC_IF_ID_STREAMING_MIC                     (-1)
    #define UAC_IF_ID_STREAMING_MIDI                    (-1)
    #define UAC_IF_ID_STREAMING_MIDI_NEXT               (0)
    #define UAC_IF_ID_STREAMING_NUM                     (0)
#endif
#if USB_FUN_HID_EN
    #define USB_EP_NUM_HID                              (USB_EP_NUM_AS_MIDI_NEXT)
    #define USB_EP_NUM_HID_NEXT                         (USB_EP_NUM_HID + 1)            // Endpoint, HID端点号
    #define UAC_IF_ID_HID                               (UAC_IF_ID_STREAMING_MIDI_NEXT) // Interface, HID的ID号
    #define UAC_IF_ID_HID_NEXT                          (UAC_IF_ID_HID + 1)
#else
    #define USB_EP_NUM_HID                              (-1)
    #define USB_EP_NUM_HID_NEXT                         (USB_EP_NUM_AS_MIDI_NEXT)
    #define UAC_IF_ID_HID                               (-1)
    #define UAC_IF_ID_HID_NEXT                          (UAC_IF_ID_STREAMING_MIDI_NEXT)
#endif

#if USB_FUN_MFI_EN
    #define USB_EP_NUM_MFI                              (USB_EP_NUM_HID_NEXT)
    #define USB_EP_NUM_MFI_NEXT                         (USB_EP_NUM_MFI + 1)            // Endpoint, MFI端点号
    #define UAC_IF_ID_MFI                               (UAC_IF_ID_HID_NEXT)            // Interface, MFI认证的ID号
    #define UAC_IF_ID_MFI_NEXT                          (UAC_IF_ID_MFI + 1)
#else
    #define USB_EP_NUM_MFI                              (-1)
    #define USB_EP_NUM_MFI_NEXT                         (USB_EP_NUM_HID_NEXT)
    #define UAC_IF_ID_MFI                               (-1)
    #define UAC_IF_ID_MFI_NEXT                          (UAC_IF_ID_HID_NEXT)
#endif
#define USB_EP_NUM_IN_USE                               (USB_EP_NUM_MFI_NEXT - 1)       // Endpoint, 总数量
#define UAC_IF_ID_NUM                                   (UAC_IF_ID_HID_NEXT)            // Interface, 总数量
/* 定义用于描述符内的内容，不可更改 */
#define UAC_EP_ADDR_SPK                                 (USB_EP_NUM_AS_SPK|USB_EP_HOST2DEV)
#define UAC_EP_ADDR_MIC                                 (USB_EP_NUM_AS_MIC|USB_EP_DEV2HOST)
#define UAC_EP_ADDR_MIDI_OUT                            (USB_EP_NUM_AS_MIDI|USB_EP_HOST2DEV)
#define UAC_EP_ADDR_MIDI_IN                             (USB_EP_NUM_AS_MIDI|USB_EP_DEV2HOST)
#define UAC_EP_ADDR_HID_OUT                             (USB_EP_NUM_HID|USB_EP_HOST2DEV)
#define UAC_EP_ADDR_HID_IN                              (USB_EP_NUM_HID|USB_EP_DEV2HOST)
#define UAC_EP_ADDR_MFI_OUT                             (USB_EP_NUM_MFI|USB_EP_HOST2DEV)
#define UAC_EP_ADDR_MFI_IN                              (USB_EP_NUM_MFI|USB_EP_DEV2HOST)
/* Current hid out report ID，自定义协议都基于此ID号，一版情况下不更改 */
#define USB_HID_OUT_ID0                                 (0x50)  // for 16 Bytes
#define USB_HID_OUT_ID1                                 (0x51)  // for 64 Bytes
#define USB_HID_OUT_LED_ID                              (0x04)  // 2 bytes
#define USB_HID_OUT_ASP_ID                              (0x9A)  // 64 bytes
#define USB_HID_OUT_LED_KEYBOARD_ID                     (0x09)  // 1 byte
#define USB_HID_IN_ID0                                  (0x10)  // for 16 Bytes
#define USB_HID_IN_ID1                                  (0x11)  // for 64 Bytes
#define USB_HID_IN_TEL_ID                               (0x02)  // 2 bytes
#define USB_HID_IN_PLAY_CTRL_ID                         (0x03)  // 2 bytes
#define USB_HID_IN_SPK_MUTE_ID                          (0x05)  // 2 bytes                    
#define USB_HID_IN_KEYBOARD_ID                          (0x08)  // 2 byte
#define USB_HID_IN_ASP_ID                               (0x9B)  // 64 bytes
#define USB_HID_TEL_VENDER_ID                           (0x01)  // 16 bytes, IN/OUT
/* FeatureUnit控制字节数，一个通道占用1个字节，一般立体声需要支持3通道：MasterCh0/LogicCh1/LogicCh2，MIC和SPK相同，如果需要不一样，需要写2个独立的FU结构体，并对应修改相关代码 */
#define UAC_FU_CTRL_SPK_CH                              (UAC_SPK_CH_MODE)     // FeatureUnit的控制通道数
#define UAC_FU_CTRL_MIC_CH                              (UAC_MIC_CH_MODE)     //
#define UAC_FU_CTRL_SIDETONE_CH                         (UAC_MIC_CH_MODE)     // 仅可配置1或2
/* MixerUnit输入数量 */
#define UAC_MU_IN_NUM                                   (2)     // MixerUnit的输入模块数量，当使能Sidetone时，固定为2
/* 接口 Alternate Setting 定义 */
#define UAC_IF_ZERO_BANDWIDTH                           (0)     // Alt.Setting=0
/* 当前库由于采样率切换使用的是切换采样率命令，所以如果位深/通道没有差异的话，1个AltSetting就够了，此处配置成2，因为需要同时支持16/24Bit位深 */
#if (UAC_SPK_BIT_MODE == 2)
    #define UAC_IF_ALT_SETTING_SPK_NUM                  (2)     // Alt.Setting max number for spk
#else
    #define UAC_IF_ALT_SETTING_SPK_NUM                  (1)
#endif
#if (UAC_MIC_BIT_MODE == 2)
    #define UAC_IF_ALT_SETTING_MIC_NUM                  (2)     // Alt.Setting max number for mic
#else
    #define UAC_IF_ALT_SETTING_MIC_NUM                  (1)
#endif
/* 根据SPK/MIC配置计算最大的AltSetting值，为了统一定义结构体，此处不可更改 */
#define UAC_IF_ALT_SETTING_MAX                          ((USB_AUDIO_IF_ALT_SETTING_SPK_NUM > USB_AUDIO_IF_ALT_SETTING_MIC_NUM) ? USB_AUDIO_IF_ALT_SETTING_SPK_NUM : USB_AUDIO_IF_ALT_SETTING_MIC_NUM)
/* Terminal/Unit ID号定义，一般情况下，无需更改 */
#define UAC_ID_INPUT_TERMINAL_SPEAKER                   (0x01)
#define UAC_ID_INPUT_TERMINAL_MICROPHONE                (0x02)
#define UAC_ID_INPUT_TERMINAL_SIDETONE                  (0x03)
#define UAC_ID_OUTPUT_TERMINAL_SPEAKER                  (0x11)
#define UAC_ID_OUTPUT_TERMINAL_MICROPHONE               (0x12)
#define UAC_ID_FEATURE_UNIT_SPEAKER                     (0x21)
#define UAC_ID_FEATURE_UNIT_MICROPHONE                  (0x22)
#define UAC_ID_FEATURE_UNIT_SIDETONE                    (0x23)
#define UAC_ID_MIXER_UNIT_WITH_SIDETONE                 (0x24)
#define UAC_ID_INPUT_TERMINAL_SIDETONE_TYPE             (UAC_TT_INPUT_MICROPHONE)
#if (UAC_ID_OUTPUT_TERMINAL_SPEAKER_TYPE != UAC_TT_HEADSET)
    #define UAC_ID_OUTPUT_TERMINAL_SPEAKER_ASSOC        (0x00)  // 如果非双向类型，设置成0x00
#else
    #define UAC_ID_OUTPUT_TERMINAL_SPEAKER_ASSOC        (UAC_ID_INPUT_TERMINAL_MICROPHONE)
#endif
#if (UAC_ID_INPUT_TERMINAL_MICROPHONE_TYPE != UAC_TT_HEADSET)
    #define UAC_ID_INPUT_TERMINAL_MICROPHONE_ASSOC      (0x00)  // 如果非双向类型，设置成0x00
#else
    #define UAC_ID_INPUT_TERMINAL_MICROPHONE_ASSOC      (UAC_ID_OUTPUT_TERMINAL_SPEAKER)
#endif
/* Midistreaming IDs of midi jack，一般情况下，无需更改 */
#define UAC_ID_MIDI_IN_JACK_EMBEDDED                    (0x01)
#define UAC_ID_MIDI_IN_JACK_EXTERNAL                    (0x02)
#define UAC_ID_MIDI_OUT_JACK_EMBEDDED                   (0x03)
#define UAC_ID_MIDI_OUT_JACK_EXTERNAL                   (0x04)
/*****************************************************************************/
/* 通过 Configuration Wizard 无法实现时，可直接修改此处进行自定义配置        */
/* 如果可以通过修改 Configuration Wizard 实现的，可修改后并提交更新          */
#define UAC_SPK_FS_NUM                                  (UAC_SPK_FS_MODE/2)
#define UAC_MIC_FS_NUM                                  (UAC_MIC_FS_MODE/2)
#define UAC_SAMPLE_RATE_NUM                             ((UAC_SPK_FS_NUM > UAC_MIC_FS_NUM) ? (UAC_SPK_FS_NUM) : UAC_MIC_FS_NUM) //支持的最大采样率数量
#define UAC_SPK_FS_MAX                                  ((UAC_SPK_FS_MODE > 6) ? UAC_SAMPLE_RATE_192K : ((UAC_SPK_FS_MODE > 4) ? UAC_SAMPLE_RATE_96K : UAC_SAMPLE_RATE_48K))
#define UAC_MIC_FS_MAX                                  ((UAC_MIC_FS_MODE > 6) ? UAC_SAMPLE_RATE_192K : ((UAC_MIC_FS_MODE > 4) ? UAC_SAMPLE_RATE_96K : UAC_SAMPLE_RATE_48K))
#if (UAC_SPK_FS_MODE == 3)
#define UAC_SPK_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), }
#elif (UAC_SPK_FS_MODE == 4)
#define UAC_SPK_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_44K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), }
#elif (UAC_SPK_FS_MODE == 5)
#define UAC_SPK_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), }
#elif (UAC_SPK_FS_MODE == 6)
#define UAC_SPK_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_44K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), }
#elif (UAC_SPK_FS_MODE == 7)
#define UAC_SPK_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_192K), }
#elif (UAC_SPK_FS_MODE == 8)
#define UAC_SPK_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_44K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_192K), }
#endif
#if (UAC_MIC_FS_MODE == 3)
#define UAC_MIC_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), }
#elif (UAC_MIC_FS_MODE == 4)
#define UAC_MIC_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_44K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), }
#elif (UAC_MIC_FS_MODE == 5)
#define UAC_MIC_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), }
#elif (UAC_MIC_FS_MODE == 6)
#define UAC_MIC_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_44K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), }
#elif (UAC_MIC_FS_MODE == 7)
#define UAC_MIC_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_192K), }
#elif (UAC_MIC_FS_MODE == 8)
#define UAC_MIC_IF_ALT_SET_FS_ALL                       \
{   UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_44K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_44K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_48K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_48K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_96K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_96K), \
    UAC_FS_BYTE0_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE1_GET(UAC_SAMPLE_RATE_192K), UAC_FS_BYTE2_GET(UAC_SAMPLE_RATE_192K), }
#endif
//SPK接口信息自动配置
#if (UAC_SPK_BIT_MODE == 2)     //2个AltSetting，同时支持16bit和24bit
    /* IF(SPK), AltSetting(1) */
    #define UAC_IF_ALT_SET_1_SPK_FS_MAX                 (UAC_SPK_FS_MAX)        // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_1_SPK_FS_NUM                 (UAC_SPK_FS_NUM)        // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_1_SPK_FS_ALL                 UAC_SPK_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_1_SPK_CH                     (UAC_SPK_CH_MODE)                     // 1, mono; 2, stereo;
    #define UAC_IF_ALT_SET_1_SPK_BIT                    (16)                    // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_1_SPK_BYTE_SIZE              (2)                     // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_1_SPK_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_1_SPK_FS_MAX, UAC_IF_ALT_SET_1_SPK_CH, UAC_IF_ALT_SET_1_SPK_BYTE_SIZE))
    /* IF(SPK), AltSetting(2) */
    #define UAC_IF_ALT_SET_2_SPK_FS_MAX                 (UAC_SPK_FS_MAX)        // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_2_SPK_FS_NUM                 (UAC_SPK_FS_NUM)        // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_2_SPK_FS_ALL                 UAC_SPK_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_2_SPK_CH                     (UAC_SPK_CH_MODE)                     // 1, mono; 2, stereo;
    #define UAC_IF_ALT_SET_2_SPK_BIT                    (24)                    // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_2_SPK_BYTE_SIZE              (3)                     // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_2_SPK_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_2_SPK_FS_MAX, UAC_IF_ALT_SET_2_SPK_CH, UAC_IF_ALT_SET_2_SPK_BYTE_SIZE))
    /* 最大包大小用于安排FIFO大小 */
    #define UAC_IF_SPK_PACK_SIZE_MAX                    ((UAC_IF_ALT_SET_1_SPK_PACK_SIZE > UAC_IF_ALT_SET_2_SPK_PACK_SIZE) ? UAC_IF_ALT_SET_1_SPK_PACK_SIZE : UAC_IF_ALT_SET_2_SPK_PACK_SIZE)
#elif (UAC_SPK_BIT_MODE == 0)   //1个AltSetting，仅支持16bit
    #define UAC_IF_ALT_SET_1_SPK_FS_MAX                 (UAC_SPK_FS_MAX)        // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_1_SPK_FS_NUM                 (UAC_SPK_FS_NUM)        // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_1_SPK_FS_ALL                 UAC_SPK_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_1_SPK_CH                     (UAC_SPK_CH_MODE)                     // 1, mono; 2, stereo;
    #define UAC_IF_ALT_SET_1_SPK_BIT                    (16)                    // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_1_SPK_BYTE_SIZE              (2)                     // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_1_SPK_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_1_SPK_FS_MAX, UAC_IF_ALT_SET_1_SPK_CH, UAC_IF_ALT_SET_1_SPK_BYTE_SIZE))
    /* 最大包大小用于安排FIFO大小 */
    #define UAC_IF_SPK_PACK_SIZE_MAX                    UAC_IF_ALT_SET_1_SPK_PACK_SIZE
#elif (UAC_SPK_BIT_MODE == 1)   //1个AltSetting，仅支持24bit
    #define UAC_IF_ALT_SET_1_SPK_FS_MAX                 (UAC_SPK_FS_MAX)        // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_1_SPK_FS_NUM                 (UAC_SPK_FS_NUM)        // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_1_SPK_FS_ALL                 UAC_SPK_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_1_SPK_CH                     (UAC_SPK_CH_MODE)                     // 1, mono; 2, stereo;
    #define UAC_IF_ALT_SET_1_SPK_BIT                    (24)                    // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_1_SPK_BYTE_SIZE              (3)                     // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_1_SPK_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_1_SPK_FS_MAX, UAC_IF_ALT_SET_1_SPK_CH, UAC_IF_ALT_SET_1_SPK_BYTE_SIZE))
    /* 最大包大小用于安排FIFO大小 */
    #define UAC_IF_SPK_PACK_SIZE_MAX                    UAC_IF_ALT_SET_1_SPK_PACK_SIZE
#endif
//MIC接口信息自动配置
#if (UAC_MIC_BIT_MODE == 2)     //2个AltSetting，同时支持16bit和24bit
    /* IF(MIC), AltSetting(1) */
    #define UAC_IF_ALT_SET_1_MIC_FS_MAX                 (UAC_MIC_FS_MAX)    // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_1_MIC_FS_NUM                 (UAC_MIC_FS_NUM)    // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_1_MIC_FS_ALL                 UAC_MIC_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_1_MIC_CH                     (UAC_MIC_CH_MODE)                 // 1, mono; 2, strreo;
    #define UAC_IF_ALT_SET_1_MIC_BIT                    (16)                // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_1_MIC_BYTE_SIZE              (2)                 // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_1_MIC_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_1_MIC_FS_MAX, UAC_IF_ALT_SET_1_MIC_CH, UAC_IF_ALT_SET_1_MIC_BYTE_SIZE))
    /* If(MIC), AltSetting(2) */
    #define UAC_IF_ALT_SET_2_MIC_FS_MAX                 (UAC_MIC_FS_MAX)    // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_2_MIC_FS_NUM                 (UAC_MIC_FS_NUM)    // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_2_MIC_FS_ALL                 UAC_MIC_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_2_MIC_CH                     (UAC_MIC_CH_MODE)                 // 1, mono; 2, strreo;
    #define UAC_IF_ALT_SET_2_MIC_BIT                    (24)                // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_2_MIC_BYTE_SIZE              (3)                 // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_2_MIC_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_2_MIC_FS_MAX, UAC_IF_ALT_SET_2_MIC_CH, UAC_IF_ALT_SET_2_MIC_BYTE_SIZE))
    /* 最大包大小用于安排FIFO大小 */
    #define UAC_IF_MIC_PACK_SIZE_MAX                    ((UAC_IF_ALT_SET_1_MIC_PACK_SIZE > UAC_IF_ALT_SET_2_MIC_PACK_SIZE) ? UAC_IF_ALT_SET_1_MIC_PACK_SIZE : UAC_IF_ALT_SET_2_MIC_PACK_SIZE)
#elif (UAC_MIC_BIT_MODE == 0)   //1个AltSetting，仅支持16bit
    #define UAC_IF_ALT_SET_1_MIC_FS_MAX                 (UAC_MIC_FS_MAX)    // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_1_MIC_FS_NUM                 (UAC_MIC_FS_NUM)    // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_1_MIC_FS_ALL                 UAC_MIC_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_1_MIC_CH                     (UAC_MIC_CH_MODE)                 // 1, mono; 2, strreo;
    #define UAC_IF_ALT_SET_1_MIC_BIT                    (16)                // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_1_MIC_BYTE_SIZE              (2)                 // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_1_MIC_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_1_MIC_FS_MAX, UAC_IF_ALT_SET_1_MIC_CH, UAC_IF_ALT_SET_1_MIC_BYTE_SIZE))
    /* 最大包大小用于安排FIFO大小 */
    #define UAC_IF_MIC_PACK_SIZE_MAX                    UAC_IF_ALT_SET_1_MIC_PACK_SIZE
#elif (UAC_MIC_BIT_MODE == 1)   //1个AltSetting，仅支持24bit
    #define UAC_IF_ALT_SET_1_MIC_FS_MAX                 (UAC_MIC_FS_MAX)    // 该Alt.Setting支持的最大采样率
    #define UAC_IF_ALT_SET_1_MIC_FS_NUM                 (UAC_MIC_FS_NUM)    // 该Alt.Setting支持的采样率数量
    /* 所有采样率的字节表格数，每个采样率需要3个字节表示，当前支持1个采样率，如需支持更多，此项需要更改 */
    #define UAC_IF_ALT_SET_1_MIC_FS_ALL                 UAC_MIC_IF_ALT_SET_FS_ALL
    #define UAC_IF_ALT_SET_1_MIC_CH                     (UAC_MIC_CH_MODE)                 // 1, mono; 2, strreo;
    #define UAC_IF_ALT_SET_1_MIC_BIT                    (24)                // 16, 16bit, 24, 24bit
    #define UAC_IF_ALT_SET_1_MIC_BYTE_SIZE              (3)                 // bytes per sample
    /* 自动根据通道，最大采样率，字节数，计算每包数据字节数 */
    #define UAC_IF_ALT_SET_1_MIC_PACK_SIZE              (UAC_FS_PACKET_SIZE_GET(UAC_IF_ALT_SET_1_MIC_FS_MAX, UAC_IF_ALT_SET_1_MIC_CH, UAC_IF_ALT_SET_1_MIC_BYTE_SIZE))
    /* 最大包大小用于安排FIFO大小 */
    #define UAC_IF_MIC_PACK_SIZE_MAX                    UAC_IF_ALT_SET_1_MIC_PACK_SIZE
#endif

/* OTG寄存器的FIFO配置相关 */
/* 其中，size为对应的AltSetting最大包字节数
   微帧数据包大小   | SIZE_TYPE | BLOCK_TYPE | 说明
       size <= 512  |    0      |     0      | 使用1个512的FIFO
 512 < size <= 1024 |    1      |     0      | 使用2个512的FIFO拼成1024的FIFO
1024 < size <= 2048 |    1      |     1      | 使用4个512的FIFO拼成2个1024的FIFO，用于接收每个微帧中的2次传输
2048 < size <= 3076 |    1      |     2      | 使用6个512的FIFO拼成3个1024的FIFO，用于接收每个微帧中的3次传输
*/
#if USB_FUN_SPK_EN
    #if (UAC_SPK_FS_MAX >= UAC_SAMPLE_RATE_96K)
    #define UAC_FIFO_SIZE_TYPE_SPK                      (1) // 0, 512 bytes, 1, 1024 bytes, -1, no use
    #else
    #define UAC_FIFO_SIZE_TYPE_SPK                      (0)
    #endif
#else
    #define UAC_FIFO_SIZE_TYPE_SPK                      (-1)
#endif
#if USB_FUN_MIC_EN
    #if (UAC_MIC_FS_MAX >= UAC_SAMPLE_RATE_96K)
    #define UAC_FIFO_SIZE_TYPE_MIC                      (1) // 0, 512 bytes, 1, 1024 bytes, -1, no use
    #else
    #define UAC_FIFO_SIZE_TYPE_MIC                      (0)
    #endif
#else
    #define UAC_FIFO_SIZE_TYPE_MIC                      (-1)
#endif
#if USB_FUN_MIDI_EN
    #define UAC_FIFO_SIZE_TYPE_MIDI                     (0)
#else
    #define UAC_FIFO_SIZE_TYPE_MIDI                     (-1)
#endif
#if USB_FUN_HID_EN
    #define UAC_FIFO_SIZE_TYPE_HID                      (0)
#else
    #define UAC_FIFO_SIZE_TYPE_HID                      (-1)
#endif
#if USB_FUN_MFI_EN
    #define UAC_FIFO_SIZE_TYPE_MFI                      (0)
#else
    #define UAC_FIFO_SIZE_TYPE_MFI                      (-1)
#endif

/* 除非UAC10做192K，才需要用到DoubleBlock，其他时候都使用默认的SingleBlock即可，无需更改 */
#if (UAC_SPK_FS_MAX == UAC_SAMPLE_RATE_192K)
#define UAC_FIFO_BLOCK_TYPE_SPK                         (OTG_FIFO_BLK_DOUBLE)
#else
#define UAC_FIFO_BLOCK_TYPE_SPK                         (OTG_FIFO_BLK_SINGLE)   // 0, single, 1, double, 2, triple
#endif
#if (UAC_MIC_FS_MAX == UAC_SAMPLE_RATE_192K)
#define UAC_FIFO_BLOCK_TYPE_MIC                         (OTG_FIFO_BLK_DOUBLE)
#else
#define UAC_FIFO_BLOCK_TYPE_MIC                         (OTG_FIFO_BLK_SINGLE)   // 0, single, 1, double, 2, triple
#endif
#define UAC_FIFO_BLOCK_TYPE_MIDI                        (OTG_FIFO_BLK_SINGLE)
#define UAC_FIFO_BLOCK_TYPE_HID                         (OTG_FIFO_BLK_SINGLE)
#define UAC_FIFO_BLOCK_TYPE_MFI                         (OTG_FIFO_BLK_SINGLE)

/* 下列配置自动计算，无需修改 */
#define UAC_FIFO_IDX_START                              (0)
#define UAC_FIFO_USE_NUM_SPK                            ((UAC_FIFO_SIZE_TYPE_SPK+1)*(UAC_FIFO_BLOCK_TYPE_SPK+1))
#define UAC_FIFO_USE_NUM_MIC                            ((UAC_FIFO_SIZE_TYPE_MIC+1)*(UAC_FIFO_BLOCK_TYPE_MIC+1))
#define UAC_FIFO_USE_NUM_MIDI                           ((UAC_FIFO_SIZE_TYPE_MIDI+1)*(UAC_FIFO_BLOCK_TYPE_MIDI+1))
#define UAC_FIFO_USE_NUM_HID                            ((UAC_FIFO_SIZE_TYPE_HID+1)*(UAC_FIFO_BLOCK_TYPE_HID+1))
#define UAC_FIFO_USE_NUM_MFI                            ((UAC_FIFO_SIZE_TYPE_MFI+1)*(UAC_FIFO_BLOCK_TYPE_MFI+1))

#define UAC_FIFO_IDX_SPK                                (UAC_FIFO_IDX_START)                            // fifo idx for audio stream spk
#define UAC_FIFO_IDX_MIC                                (UAC_FIFO_IDX_SPK+UAC_FIFO_USE_NUM_SPK)         // fifo idx for audio stream spk
#define UAC_FIFO_IDX_MIDI_OUT                           (UAC_FIFO_IDX_MIC+UAC_FIFO_USE_NUM_MIC)         // fifo idx for audio stream midi out
#define UAC_FIFO_IDX_MIDI_IN                            (UAC_FIFO_IDX_MIDI_OUT+UAC_FIFO_USE_NUM_MIDI)   // fifo idx for audio stream midi in
#define UAC_FIFO_IDX_HID_OUT                            (UAC_FIFO_IDX_MIDI_IN+UAC_FIFO_USE_NUM_MIDI)    // fifo idx for hid out
#define UAC_FIFO_IDX_HID_IN                             (UAC_FIFO_IDX_HID_OUT+UAC_FIFO_USE_NUM_HID)     // fifo idx for hid in
#define UAC_FIFO_IDX_MFI_OUT                            (UAC_FIFO_IDX_HID_IN+UAC_FIFO_USE_NUM_HID)
#define UAC_FIFO_IDX_MFI_IN                             (UAC_FIFO_IDX_MFI_OUT+UAC_FIFO_USE_NUM_MFI)
#define UAC_FIFO_IDX_END                                (UAC_FIFO_IDX_MFI_IN+UAC_FIFO_USE_NUM_MFI)      // fifo idx end number, for check use

#if (UAC_FIFO_IDX_END > 8)
    #error USB fifo assigened is out of range, please check!
#endif
/* Exported constants -------------------------------------------------------*/
/*****************************************************************************/

/* Exported macros ----------------------------------------------------------*/
/*****************************************************************************/
#define UAC_FS_BYTE0_GET(fs)                            ((uint8_t)(((uint32_t)(fs) >> 0) & 0xFF))
#define UAC_FS_BYTE1_GET(fs)                            ((uint8_t)(((uint32_t)(fs) >> 8) & 0xFF))
#define UAC_FS_BYTE2_GET(fs)                            ((uint8_t)(((uint32_t)(fs) >> 16) & 0xFF))
#define UAC_FS_PACKET_SIZE_GET(fs, ch, byte)            ((((uint32_t)(fs) + 999) / 1000) * (ch) * (byte))
#define UAC_FS_PACKET_TRANS_SIZE(byte)                  ((byte > 1024) ? (0x800 | (byte >> 1)) : byte)

/* Config value check -------------------------------------------------------*/
/*****************************************************************************/
/* 用于检查当前配置的采样率数量是否超出最大值，如超过编译会直接报错，无需更改 */
#if (USB_STR_IF_AUDIO_FU_INDEX >= USB_STR_DESCRIPTORS_NUM)
    #error USB_STR_IF_AUDIO_FU_INDEX should be smaller than USB_STR_DESCRIPTORS_NUM
#endif
#if (USB_STR_IF_AUDIO_MIDI_INDEX >= USB_STR_DESCRIPTORS_NUM)
    #error USB_STR_IF_AUDIO_MIDI_INDEX should be smaller than USB_STR_DESCRIPTORS_NUM
#endif
#if (UAC_ID_SIDETONE_UNIT_ENABLE != 0)
    #if (USB_STR_SIDETONE_INDEX == 0)
        #error If select "FeatureUnit Sidetone Enable", "string of sidetone" should be selected
    #endif
    #if ((UAC_IF_ID_STREAMING_SPK > 0) && (UAC_IF_ID_STREAMING_MIC > 0))
    #else
        #error If select "FeatureUnit Sidetone Enable", "USB_FUN_SPK_EN" or "USB_FUN_MIC_EN" should be selected
    #endif
#else
    #if (USB_STR_SIDETONE_INDEX != 0)
        #error If "FeatureUnit Sidetone Enable" is not selected, "string of sidetone" should not be selected
    #endif
#endif
#if (UAC_IF_ALT_SET_1_SPK_FS_NUM > UAC_SAMPLE_RATE_NUM)
    #error UAC_SAMPLE_RATE_NUM is too small, UAC_IF_ALT_SET_1_SPK_FS_NUM is bigger than UAC_SAMPLE_RATE_NUM, please check!
#endif
#if (UAC_IF_ALT_SET_2_SPK_FS_NUM > UAC_SAMPLE_RATE_NUM)
    #error UAC_SAMPLE_RATE_NUM is too small, UAC_IF_ALT_SET_2_SPK_FS_NUM is bigger than UAC_SAMPLE_RATE_NUM, please check!
#endif
#if (UAC_IF_ALT_SET_1_MIC_FS_NUM > UAC_SAMPLE_RATE_NUM)
    #error UAC_SAMPLE_RATE_NUM is too small, UAC_IF_ALT_SET_1_MIC_FS_NUM is bigger than UAC_SAMPLE_RATE_NUM, please check!
#endif
#if (UAC_IF_ALT_SET_2_MIC_FS_NUM > UAC_SAMPLE_RATE_NUM)
    #error UAC_SAMPLE_RATE_NUM is too small, UAC_IF_ALT_SET_2_MIC_FS_NUM is bigger than UAC_SAMPLE_RATE_NUM, please check!
#endif
#if (UAC_FIFO_IDX_END > 8)
    #error USB fifo assigened is out of range, please check!
#endif
#ifdef __cplusplus
}
#endif

#endif
