/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ENC_H__
#define __ENC_H__

#include "three_band_filter_bank.h"
#include "noise_suppressor.h"

/*!
 * @addtogroup enc
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{	
	ThreeBandFilterBank three_band_filter_bank;
	NoiseSuppressor noise_suppressor;
} Enc;



  
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

extern Enc g_EncParam ;
extern uint8 enc_level ;
extern bool EncOnOff ;                  /* ENC开关状态：1=开，0=关，定义在drv_alg.c */

extern float enc_in[480*2] __ALIGNED(4);
extern float enc_out[480*2] __ALIGNED(4);

void Enc_Init(Enc *enc, int32_t suppression_level);
void Enc_Process(Enc *enc, float *in, float *out);
void Enc_SetOnOff(bool on);             /* ENC开关接口，供按键模块调用 */
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __ENC_H__*/
