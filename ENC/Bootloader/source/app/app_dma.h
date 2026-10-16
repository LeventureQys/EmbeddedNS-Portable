
#ifndef __APP_DMA_H
#define __APP_DMA_H

#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_dma.h"


/* 下列需要根据I2S的DMA通道使用情况进行配置 */
#define DMA_CH_I2S_OUT                  (2)                             // DAC-L/R, I2SOut-L/R
#define DMA_CH_I2S_IN                   (2)                             // ADC-L/R, I2SIn-L/R
#define DMA_CH_I2S_USE                  (DMA_CH_I2S_OUT+DMA_CH_I2S_IN)  // I2S使用的DMA通道总数
#define DMA_CH_I2S_START                (8)                             // 0~15, 0~7 in DMA0, 8~15 in DMA1
static const uint32_t DMA_CH_I2S_REG_ADDR_OUT[DMA_CH_I2S_OUT] = {
    I2S_FIFOOUT_BASE,   (I2S_FIFOOUT_BASE+4),                           // DAC-L/R
};
static const uint32_t DMA_CH_I2S_REG_ADDR_IN[DMA_CH_I2S_IN] = {
    I2S_FIFOIN_BASE,    (I2S_FIFOIN_BASE+4),                            // ADC-L/R
};




void AppDmaInit(void);

/* defination check */
#if ((DMA_CH_I2S_START + DMA_CH_I2S_USE) > DMA_CH_MAX)
    #error DMA channel defination for i2s is out of range, please check DMA_CH_I2S_START and DMA_CH_I2S_USE
#endif

#endif
