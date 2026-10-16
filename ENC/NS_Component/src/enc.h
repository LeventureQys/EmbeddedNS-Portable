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



void Enc_Init(Enc *enc, int32_t suppression_level);
void Enc_Process(Enc *enc, float *in, float *out);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __ENC_H__*/
