/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SIGNAL_MODEL_ESTIMATOR_H__
#define __SIGNAL_MODEL_ESTIMATOR_H__

#include "prior_signal_model_estimator.h"

/*!
 * @addtogroup signal_model_estimator
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
	float diff_normalization;
	float signal_energy_sum;
	int32_t histogram_analysis_counter;
	
	PriorSignalModel prior_signal_model;
	SignalModel signal_model;
	Histograms histograms;
} SignalModelEstimator;


 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void SignalModelEstimator_Init(SignalModelEstimator *signal_model_estimator);
void SignalModelEstimator_AdjustNormalization(SignalModelEstimator *signal_model_estimator, int32_t num_analyzed_frames, float signal_energy);
void SignalModelEstimator_Update(SignalModelEstimator *signal_model_estimator, float *prior_snr, float *post_snr, float *conservative_noise_spectrum, float *signal_spectrum, float signal_spectral_sum, float signal_energy);	
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __SIGNAL_MODEL_ESTIMATOR_H__*/
