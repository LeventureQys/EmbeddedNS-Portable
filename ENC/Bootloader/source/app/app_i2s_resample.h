
#ifndef __APP_I2S_RESAMPLE_H
#define __APP_I2S_RESAMPLE_H

#include "a0_hal_def.h"

#define I2S_SALVE_TEMP_GROUP_NUM           (4)                 // 临时缓存组数，用于DMA帧中断中
#define I2S_S1_PARA_IDX_IN                 (0)
#define I2S_S1_PARA_IDX_OUT                (1)
//slave
#define I2S_S1_PACK_NUM                    (48) 
#define I2S_S1_PACK                        (8)  
#define I2S_BUFF_S1_MAX                    (I2S_S1_PACK_NUM*I2S_S1_PACK)

#define         I2S_S1_OUT_CH           (2)
#define         I2S_S1_IN_CH            (2)

void DmaS1RestartI2s(void);
void AppI2sSlaveInFifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum);
void AppI2sSlaveOutFifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum);
void AppI2sS1FrameProcess(void);
void DrvI2sSampleRateInit(void);
void DrvI2sSampleRateCheck(uint8_t idx, uint32_t sampleRate);
void DrvI2sInDataRead(int32_t **pDstCh, uint8_t ch, uint32_t sampleNumber);
void DrvI2sOutDataWrite(int32_t **pSrcCh, uint8_t ch, uint32_t sampleNumber);


void AppI2sSlave1FifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum);
void AppI2sSlave2FifoInfoGet(uint32_t *pMemAddr, uint32_t *pI2sPackNum);

#endif
