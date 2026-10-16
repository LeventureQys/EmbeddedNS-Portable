/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SPEECH_PROBABILITY_ESTIMATOR_H__
#define __SPEECH_PROBABILITY_ESTIMATOR_H__

#include "signal_model_estimator.h"

/*!
 * @addtogroup speech_probability_estimator
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
	float prior_speech_prob;
	float speech_probability[kFftSizeBy2Plus1];
	
	SignalModelEstimator signal_model_estimator;
} SpeechProbabilityEstimator;


 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void SpeechProbabilityEstimator_Init(SpeechProbabilityEstimator *speech_probability_estimator);
void SpeechProbabilityEstimator_Update(SpeechProbabilityEstimator *speech_probability_estimator, int32_t num_analyzed_frames, float *prior_snr, float *post_snr, float *conservative_noise_spectrum, float *signal_spectrum, float signal_spectral_sum, float signal_energy);	
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __SPEECH_PROBABILITY_ESTIMATOR_H__*/
