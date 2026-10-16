/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    jat_resample.h
 * @brief   The configuration of JAT SRC(SampleRateConvert) LIB
 * @version V1.0.5
 * @date    2024/04/24
 * @author  huangzheng
 ******************************************************************************
 */

/*
    此采样率转换，仅针对固定采样率（初始化配置），例如48k转换到44.1k/48k写入，从44.1k/48k转换到48k读出的操作
    写入：从固定采样率的信号源处，通过重采样处理，变换到设定采样率，使用每帧长度固定48点（初始化配置）的原始数据，写入长度不固定
    读取：从设定采样率的信号源处，通过重采样处理，变换到固定采样率，得到每帧长度固定48点（初始化配置）的结果数据，读取长度不固定
    
    eg：针对本地i2s/codec固定再48k时，usb-spk和usb-mic可选44.1k/48k，读取对应从usb-spk缓存读到codec缓存，写入对应从codec缓存写入usb-mic缓存
        codec缓存传入的指针每次更新，且指针指向的内存连续（即函数中不对指针的是否越界做判定
        app-spk/app-mic缓存需要给缓存的最大值，函数内会对环形缓存是否越界做判定
*/

#ifndef __JAT_RESAMPLE_H
#define __JAT_RESAMPLE_H

#include "jat_typedef.h"

/*
    资源消耗依次递增，资源不够的时候选0，一般情况下选1就够用了（96K录音下存在少许谐波），2/3基本没有谐波但高频会有衰减
    0, 2点线性插值, 2Ch-RD需要 5.2M(96->48,48->48,44.1->48), 2Ch-WR需要 7.8M(48->96),  4.4M(48->48,48->44.1), 1Ch-WR: 7.4M(48->192),  4.0M(48->96), 2.3M(48->48, 48->44.1)
    1, 3点样条插值, 2Ch-RD需要12.6M(96->48,48->48,44.1->48), 2Ch-WR需要16.7M(48->96), 10.8M(48->48,48->44.1), 1Ch-WR:15.0M(48->192),  8.4M(48->96), 5.1M(48->48, 48->44.1)
    2, 4点样条插值, 2Ch-RD需要16.0M(96->48,48->48,44.1->48), 2Ch-WR需要23.2M(48->96), 16.0M(48->48,48->44.1), 1Ch-WR:18.9M(48->192), 11.2M(48->96), 7.3M(48->48, 48->44.1)
    3, 6点样条插值, 2Ch-RD需要20.6M(96->48,48->48,44.1->48), 2Ch-WR需要30.0M(48->96), 21.8M(48->48,48->44.1), 1Ch-WR:23.9M(48->192), 14.6M(48->96), 9.9M(48->48, 48->44.1)
*/
#define RESAMPLE_MODE_RD            (1)
#define RESAMPLE_MODE_WR            (1)

// 宏定义，不可更改
#define RESAMPLE_IDX_RD             (0)
#define RESAMPLE_IDX_WR             (1)
#define RESAMPLE_IDX_MAX            (2)

#define RESAMPLE_MEM_SIZE(CH)       (11 + (CH) * ((RESAMPLE_MODE_WR < 3) ? (RESAMPLE_MODE_WR + 3) : (RESAMPLE_MODE_WR + 4)))

typedef struct {
    int32_t *pParaMem;              // 当前重采样处理模块指向的内存，如需多个独立重采样处理，需要多个内存并独立初始化
    int32_t memSize;                // 内存大小，用于给库做判断，11 + ch * (3/4/5/7,RESAMPLE_MODE_WR决定)
    int32_t chNum;                  // 当前重采样通道数
    int32_t **pRWSample;            // 配置读缓存地址指针（循环缓存），配置写缓存的目标地址（循环缓存）
    int32_t frameSize;              // 一般情况下，设置为48，跟I2S的DMA中断一帧数据长度一致
    int32_t sampleRateRef;          // 采样率转换的参考采样率，从该采样率转换到不同采样率写入，从不同采样率转换到该采样率读出
}resample_init_t;

void ResampleFirstPacketSet(int32_t *pParaMem);
void ResampleFirstPacketClr(int32_t *pParaMem);
uint32_t ResampleFirstPacketGet(int32_t *pParaMem);
// 对于支持多采样率，且需要动态调整帧长度时，进行frameSize的刷新设置
void ResampleFrameSizeSet(int32_t *pParaMem, int32_t frameSize);

int32_t ResampleInit(resample_init_t *pResampleInit);

// pParaMem:        当前重采样处理模块的内存指针
// idx:             RESAMPLE_IDX_RD / RESAMPLE_IDX_WR
// newSampleRate:   44100 / 48000
// rateAdj:         1~41，设置成41时，偏差1%（偏差=rateAdj/4096），一般建议20，0.5%，如果时钟偏差本来就比较小（内部i2s和外部i2s从时钟偏差小，可适当减小到8）
// return:          0, ok, 1, rateAdj is out of range
uint32_t ResampleFSCfg(int32_t *pParaMem, uint32_t idx, uint32_t newSampleRate, uint32_t rateAdj);

int32_t ResampleHandlerRd(int32_t *pParaMem, int32_t **pSample, int32_t sampleIdxWr, int32_t sampleIdxRd, int32_t sampleIdxMax);
int32_t ResampleHandlerWr(int32_t *pParaMem, int32_t **pSample, int32_t sampleIdxWr, int32_t sampleIdxRd, int32_t sampleIdxMax);

#endif
