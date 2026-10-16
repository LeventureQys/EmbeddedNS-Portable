
#ifndef __APP_I2S_H
#define __APP_I2S_H

#include "a0_hal_conf.h"
#include "a0_hal_def.h"

#define I2S_IFX_CLOCK					(1)

// 测试无需打开I2S的IO则配置成0，如果需要输出时钟和I2S的DATA线有效，需要配置成1
// 也可跟根据define定位到所在位置，按照实际IO复用需求进行修改配置
#define I2S_GPIO_MCLK_USE               (0)                         // MCLK, P0.27
#define I2S_GPIO_CLK_USE                (1)//(0)                         // BCLK, P0.18, LRCLK, P0.26
#define I2S_GPIO_DATA_USE               (1)//(0)                         // D3, P0.24, D4, P0.25

// 下面配置一般情况下固定不变
#define I2S_PACK_NUM                    (48)                        // 采样率48K时，配置48为了让DMA中断1ms进一次
#define I2S_PACK                        (3)                         // 比乒乓缓存多1包缓存，应对某一帧数据突发情况导致断帧
#define I2S_BUFF_MAX                    (I2S_PACK_NUM*I2S_PACK)     // 单声道总缓存的样本数

// for switch to 96K usage
//#define I2S_96K_PACK_NUM                (96)                        
//#define I2S_96K_PACK                    (3)                         
//#define I2S_96K_BUFF_MAX                (I2S_96K_PACK_NUM*I2S_96K_PACK)     



void AppI2sInit(void);
/* 用来给DMA模块获取I2S缓存的首地址，让DMA重新配置下一帧的memory地址，以及一帧的样本数量 */
void AppI2sFifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum);
/* DMA帧中断调用，进行一帧的I2S数据流处理 */
void AppI2sFrameProcess(void);

int32_t AppI2sClockModify(uint32_t samplerate, uint32_t codecEn);

void AppI2sPackCurSet(uint32_t packNum);
uint32_t AppI2sPackCurGet(void);

void AppI2sAudioFifoClr(void);
void AppI2sAudioFifoHalfClr(void);

void AppI2sHwOutFifoClr(void);	

#endif
