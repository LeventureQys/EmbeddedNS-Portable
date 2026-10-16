
#ifndef __APP_CODEC_H
#define __APP_CODEC_H

#include "a0_hal_conf.h"
#include "a0_hal_def.h"

void AppCodecDmicInit(void);
// dBVolL, -27~36, -127 means no change
// dBVolR, -27~36, -127 means no change
void AppCodecMicVolCfg(int8_t dBVolL, int8_t dBVolR);
void AppCodecMicBoostCfg(uint8_t boostEn);
void AppCodecMicPD(void);
void AppCodecOutPD(void);
void AppCodecInit(void);

#endif
