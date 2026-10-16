/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __QUANTILE_NOISE_ESTIMATOR_H__
#define __QUANTILE_NOISE_ESTIMATOR_H__

//#include "usr_common.h"//Niko 屏蔽
#include "UserFunction.h"
#include "UserRetype.h"
#include "stdint.h"
#include "stdbool.h"
#include "math.h"
#include "marco.h"

#include "ns_common.h"

/*!
 * @addtogroup quantile_noise_estimator
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define kSimult 3
#define kOneBySimult 0.3333333f

 
typedef struct
{
	float density[kSimult*kFftSizeBy2Plus1];
	float log_quantile[kSimult*kFftSizeBy2Plus1];
	float quantile[kFftSizeBy2Plus1];
	int32_t counter[kSimult];
	int32_t num_updates;
} QuantileNoiseEstimator;
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void QuantileNoiseEstimator_Init(QuantileNoiseEstimator *quantile_noise_estimator);
void QuantileNoiseEstimator_Estimate(QuantileNoiseEstimator *quantile_noise_estimator, float *signal_spectrum, float *noise_spectrum);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __QUANTILE_NOISE_ESTIMATOR_H__*/
