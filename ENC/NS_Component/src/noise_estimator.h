/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NOISE_ESTIMATOR_H__
#define __NOISE_ESTIMATOR_H__

#include "ns_common.h"
#include "quantile_noise_estimator.h"
#include "suppression_params.h"

/*!
 * @addtogroup noise_estimator
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
	QuantileNoiseEstimator quantile_noise_estimator;
	 
	float white_noise_level;
	float pink_noise_numerator;
	float pink_noise_exp;
	 
	float prev_noise_spectrum[kFftSizeBy2Plus1];
	float conservative_noise_spectrum[kFftSizeBy2Plus1];
	float parametric_noise_spectrum[kFftSizeBy2Plus1];
	float noise_spectrum[kFftSizeBy2Plus1];
} NoiseEstimator;
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void NoiseEstimator_Init(NoiseEstimator *noise_estimator);
void NoiseEstimator_PrepareAnalysis(NoiseEstimator *noise_estimator);
void NoiseEstimator_PreUpdate(NoiseEstimator *noise_estimator, SuppressionParams *suppression_params, int32_t num_analyzed_frames, float *signal_spectrum, float signal_spectral_sum);
void NoiseEstimator_PostUpdate(NoiseEstimator *noise_estimator, float *speech_probability, float *signal_spectrum);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __NOISE_ESTIMATOR_H__*/
