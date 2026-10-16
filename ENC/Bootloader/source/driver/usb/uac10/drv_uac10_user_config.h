
#ifndef __DRV_UAC10_USER_CONFIG_H
#define __DRV_UAC10_USER_CONFIG_H

#include "lib_drv_uac10_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EFFECT_DEBUG                        (0)             //菁音效果调试工具，0, 关闭，1, 打开

// 配置测试代码是否使能
#define USB_HID_SEND_TEST_EN                (0)             // 用于测试HID发码，用户使用时，一般不使能，相关函数可作为例程供用户参考
#define USB_MIDI_SEND_TEST_EN               (0)             // 用于测试MIDI收发码，用户使用可参考相关函数
#define USB_MIC_CB_CYCLE_CNT_EN             (0)             // 使能时记录MIC的数据流发送回调函数占用时间
#define USB_SPK_CB_CYCLE_CNT_EN             (0)             // 使能时记录SPK的数据流接收回调函数占用时间
// 配置SPK音量调到最小值时，是否将音量设置为0
#define USB_SPK_VOL_MIN_TO_ZERO             (1)             // 1, 当SPK音量等于USB_VOL_MIN_SPK时设置为0      0, SPK音量完全按照设定的dB值设置
#define USB_SIDETONE_VOL_MIN_TO_ZERO        (1)             // 1, 当Sidetone音量等于USB_VOL_MIN_SIDETONE时设置为0，   0，按照设定的dB值
// 修改USB枚举后，PC显示的音量范围和第一次枚举音量值等
#define USB_VOL_MIN_SPK                     (0xD000)        // (0xD000,-48.0dB), (0xA000,-96.0dB)
#define USB_VOL_MAX_SPK                     (0x0000)        // 0dB, max recommand 0dB
#define USB_VOL_RES_SPK                     (0x0010)        // 1/16dB
#define USB_VOL_DEFAULT_SPK                 (0x0000)        // 0dB
#define USB_MUTE_INIT_SPK                   (0)             // 0, unmute, 1, mute
#define USB_AGCEN_INIT_SPK                  (0)             // invalid
#define USB_VOL_MIN_MIC                     (0xE800)        // (0xE800,-24.0dB), (0xA000,-96.0dB)
#define USB_VOL_MAX_MIC                     (0x0000)        // 0dB, max recommand 0dB
#define USB_VOL_RES_MIC                     (0x0010)        // 1/16dB
#define USB_VOL_DEFAULT_MIC                 (0x0000)        // 0dB
#define USB_MUTE_INIT_MIC                   (0)             // 0, unmute, 1, mute
#define USB_AGCEN_INIT_MIC                  (0)             // 0, disable,  1, enable
#define USB_VOL_MIN_SIDETONE                (0xE800)        // (0xE800,-24.0dB), (0xA000,-96.0dB)
#define USB_VOL_MAX_SIDETONE                (0x0A00)        // 10dB, max support +18dB
#define USB_VOL_RES_SIDETONE                (0x0010)        // 1/16dB
#define USB_VOL_DEFAULT_SIDETONE            (0x0000)        // 0dB
#define USB_MUTE_INIT_SIDETONE              (0)             // 0, unmute, 1, mute
#define USB_SPK_SAMPLERATE_DEFAULT          (UAC_SAMPLE_RATE_48K)
#define USB_MIC_SAMPLERATE_DEFAULT          (UAC_SAMPLE_RATE_48K)
// 配置是否使用用户描述符更改
#define USB_DSC_USER_VIDPID_EN              (0)             // 0, use usb string descriptors in lib,     1, use user defined
#define USB_DSC_USER_MANUFACTURER_EN        (0)
#define USB_DSC_USER_PRODUCT_EN             (0)
#define USB_DSC_USER_SERIAL_NUM_EN          (0)
// 192K相关配置
#define USB_SAMPLE_RATE_192K_EN             (0)             // 当USB库支持192K立体声的时候，该选项配置成1，不支持时请配置成0
#define USB_MIC_LPF_EN                      (0)             // 当USB录音支持192或96时，需要使能，不然USB录音的时候，会产生高频混叠，使能时，192K下需要多占用40%运算资源
// 其他配置，一般情况下不用更改
#define USB_CLK_REF_MIC                     (1)             // 1, USB时钟按照MIC调节，0，USB时钟按照SPK调节
// CODEC工作模式
#define USB_STREAMING_FOR_96K               (0)             // 0, codec工作在48K/44.1K采样率下, 1, codec工作在96K/88.2K采样率下
#define DMA_FRAME_SIZE_FIXED                (1)             // 0, DMA中断I2S一帧数据包长度可变（用于单SPK或单MIC时，codec时钟跟着usb采样率动态调整后，实时调整帧长度，让中断间隔仍然保持1ms左右）
                                                            // 1, DMA中断I2S一帧数据包长度固定48（用于codec工作在固定48K时）
                                                            // 该配置大部分情况下按1配置使用，仅在某些特定项目中需要配置成0使用，并且需要DMA中断那边同步匹配修改
// 软件SRC功能配置（是否使用MCU进行SampleRateConvert处理）
#define USB_SRC_FUNC_EN                     (0)             // 使用软件重采样需要一定的MCU运算，192M全频下需要总共差不多9M左右（MIC+SPK）
// 是否支持外部Flash烧写的通信接口
#define USB_HID_EXT_FLASH_UPGRADE_EN        (0)
#if USB_HID_EXT_FLASH_UPGRADE_EN
#define USB_HID_EXT_FLASH_UPGRADE_ADDR      (0x30000000)    // 烧录外部flash的起始地址
#define USB_HID_EXT_FLASH_CACHE_ADDR        (0x10018000)    // 暂存flash数据的缓存RAM地址
#define USB_HID_EXT_FLASH_CACHE_SIZE        (32 * 1024)     // 缓存大小
#endif
#if USB_DSC_USER_VIDPID_EN
#define USER_DEV_VER                        (0x0002)
#define USER_VID                            (0x0A67)
#define USER_PID                            (0x30A2)
#endif
#if USB_DSC_USER_MANUFACTURER_EN
/* iManufacturer string descriptor, here L means get unicode value */
static const uint16_t userDscStrManufacturer[] =
{
    (sizeof(L"JinAudio")|(USB_DSC_STRING<<8)),
    L'J',L'i',L'n',L'A',L'u',L'd',L'i',L'o',
};
#endif
#if USB_DSC_USER_PRODUCT_EN
/* iProduct string descriptor, here L means get unicode value */
static const uint16_t userDscStrProduct[] =
{
    (sizeof(L"UAC1.0 Device")|(USB_DSC_STRING<<8)),
    L'U',L'A',L'C',L'1',L'.',L'0',L' ',
    L'D',L'e',L'v',L'i',L'c',L'e',
};
#endif
#if USB_DSC_USER_SERIAL_NUM_EN
/* iSerialNumber string descriptor, here L means get unicode value */
static const uint16_t userDscStrSerialNumber[] =
{
    (sizeof(L"AP5980_20240407")|(USB_DSC_STRING<<8)),
    L'A',L'P',L'5',L'9',L'8',L'0',L'_',
    L'2',L'0',L'2',L'4',L'0',L'4',L'0',L'7',
};
#endif

// 一般情况下无需更改
#define USB_SPK_DELAY                       (4)             // 4ms, 一般不建议改
#define USB_MIC_DELAY                       (4)             // 4ms, 一般不建议改
#ifdef UAC_IF_ALT_SET_2_SPK_CH
#define USB_SPK_DATA_BUFF_CH                ((UAC_IF_ALT_SET_1_SPK_CH < UAC_IF_ALT_SET_2_SPK_CH) ? UAC_IF_ALT_SET_2_SPK_CH : UAC_IF_ALT_SET_1_SPK_CH)                   // 需要和USB库匹配，一般立体声就是2
#else
#define USB_SPK_DATA_BUFF_CH                (UAC_IF_ALT_SET_1_SPK_CH)
#endif
#ifdef UAC_IF_ALT_SET_2_MIC_CH
#define USB_MIC_DATA_BUFF_CH                ((UAC_IF_ALT_SET_1_MIC_CH < UAC_IF_ALT_SET_2_MIC_CH) ? UAC_IF_ALT_SET_2_MIC_CH : UAC_IF_ALT_SET_1_MIC_CH)                   // 需要和USB库匹配，一般立体声就是2
#else
#define USB_MIC_DATA_BUFF_CH                (UAC_IF_ALT_SET_1_MIC_CH)
#endif
#define USB_SPK_DELAY_SIZE                  (USB_SPK_DELAY * 2)
#define USB_MIC_DELAY_SIZE                  (USB_MIC_DELAY * 2)
#define USB_SPK_DATA_BUFF_SIZE              (USB_SPK_SAMPLE_PACKNUM * USB_SPK_DELAY_SIZE)
#define USB_MIC_DATA_BUFF_SIZE              (USB_MIC_SAMPLE_PACKNUM * USB_MIC_DELAY_SIZE)
#ifdef UAC_IF_ALT_SET_2_SPK_FS_MAX
#define USB_SPK_SAMPLERATE_MAX              ((UAC_IF_ALT_SET_1_SPK_FS_MAX < UAC_IF_ALT_SET_2_SPK_FS_MAX) ? UAC_IF_ALT_SET_2_SPK_FS_MAX : UAC_IF_ALT_SET_1_SPK_FS_MAX)   // USB库支持的SPK最大采样率，核对lib_drv_uac10_config.h中的UAC_IF_ALT_SET_1_SPK_FS_MAX/UAC_IF_ALT_SET_2_SPK_FS_MAX中最大值
#else
#define USB_SPK_SAMPLERATE_MAX              (UAC_IF_ALT_SET_1_SPK_FS_MAX)
#endif
#ifdef UAC_IF_ALT_SET_2_MIC_FS_MAX
#define USB_MIC_SAMPLERATE_MAX              ((UAC_IF_ALT_SET_1_MIC_FS_MAX < UAC_IF_ALT_SET_2_MIC_FS_MAX) ? UAC_IF_ALT_SET_2_MIC_FS_MAX : UAC_IF_ALT_SET_1_MIC_FS_MAX)   // USB库支持的MIC最大采样率，核对lib_drv_uac10_config.h中的UAC_IF_ALT_SET_1_MIC_FS_MAX/UAC_IF_ALT_SET_2_MIC_FS_MAX中最大值
#else
#define USB_MIC_SAMPLERATE_MAX              (UAC_IF_ALT_SET_1_MIC_FS_MAX)
#endif
#define USB_SPK_SAMPLE_PACKNUM              ((USB_SPK_SAMPLERATE_MAX + 999) / 1000)
#define USB_MIC_SAMPLE_PACKNUM              ((USB_MIC_SAMPLERATE_MAX + 999) / 1000)

#if USB_SPK_SAMPLERATE_DEFAULT > USB_SPK_SAMPLERATE_MAX
    #error USB_SPK_SAMPLERATE_DEFAULT is out of range
#endif
#if USB_MIC_SAMPLERATE_DEFAULT > USB_MIC_SAMPLERATE_MAX
    #error USB_MIC_SAMPLERATE_DEFAULT is out of range
#endif

#if (((USB_VOL_MAX_SPK >> 8) > 0x00) && ((USB_VOL_MAX_SPK >> 8) < 0x80))
    #error USB_VOL_MAX_SPK max value can only be 0dB
#endif

#ifdef __cplusplus
}
#endif

#endif
