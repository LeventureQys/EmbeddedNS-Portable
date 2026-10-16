/**
 *******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 *******************************************************************************
 * @file    jat_alg_config.h
 * @brief   Configuration of all JinAudio algorithm
 * @version V1.1.0
 * @date    2023/01/04
 * @author  huangzheng
 *******************************************************************************
 */

#ifndef __JAT_ALG_CONFIG_H
#define __JAT_ALG_CONFIG_H

/**
 * @note 
 * 1. 所有效果参数请务必按照参数结构体注释说明的参数范围进行设置，效果模块内部为了提升效率，未对输入参数进行范围有效性检查。
 * 2. 所有效果参数在调用ParaSet时，会进行所有参数计算，所以在调用ParaSet函数之前最好软件做检查，如果参数未改变请勿调用，避免造成浪费资源频繁计算参数（后续再根据实际情况看，是否再库内做检查）
 */

#define ALG_FRAME_LEN                   (48)        // fixed to 48
#define ALG_SAMPLE_BIT                  (16)        // fixed to 16
#define ALG_SAMPLE_RATE                 (48000)     // fixed to 48000

/* 
    混响采样率，目前MCU仅支持36K，DSP仅支持48K版本，所以这里确定了，就确定了混响运行的位置 
*/
#define ALG_SAMPLE_RATE_REV             (48000)
/* 
    单纯的延迟默认最大时间，ms为单位，所占用的延迟线，按照最大时间和采样率进行计算
    如果需要调整延迟线长度，可自行计算延迟线所需RAM大小，并在初始化时准备对应大小的RAM，同时配置到delay sample模块中 
*/
#define ALG_DELAY_SAMPLE_TIME_MAX       (50)
/* 
    立体声回声的延迟默认最大时间，ms为单位，所占用的延迟线，按照最大时间和采样率进行计算
    如果需要调整延迟线长度，可自行计算延迟线所需RAM大小，并在初始化时准备对应大小的RAM，同时配置到delay stereo模块中 
*/
#define ALG_DELAY_STEREO_TIME_MAX       (400)
/*
    配置是否打开DSP核进行算法处理，仅配置库的支持情况，使用库时需要注意匹配
    0，不使用DSP运行算法，
    1，使用DSP运行算法
*/
#define ALG_DSP_CORE_EN                 (0)

/*
    对于延迟线可调整长度类算法，使用下面宏计算延迟线所需memory大小
    e.g. 需要100ms延迟线长度，定义memory如下
    uint32_t delayLineMem[ALG_DELAYLINE_SIZE(100, ALG_SAMPLE_RATE)];
*/
#define ALG_DELAYLINE_SIZE(__ms, __fs)  \
(((((__ms) * (__fs) + 999) / 1000) + 1) / 2)        // 16bit delay line, byte number = size * 4


#endif
