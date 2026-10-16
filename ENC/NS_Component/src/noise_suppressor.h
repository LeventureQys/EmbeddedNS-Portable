/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NOISE_SUPPRESSOR_H__
#define __NOISE_SUPPRESSOR_H__

#include "noise_estimator.h"
#include "speech_probability_estimator.h"
#include "wiener_filter.h"

/*!
 * @addtogroup noise_suppressor
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
	int32_t num_analyzed_frames;
	float extended_frame[kFftSize];
	
	float prev_analysis_signal_spectrum[kFftSizeBy2Plus1];
	float analyze_analysis_memory[kOverlapSize];
	float process_analysis_memory[kOverlapSize];
	float process_synthesis_memory[kOverlapSize];
	float process_delay_memory[kNumBands-1][kOverlapSize];
	
	SuppressionParams suppression_params;
	NoiseEstimator noise_estimator;
	SpeechProbabilityEstimator speech_probability_estimator;
	WienerFilter wiener_filter;
	
	/* --- observation instrumentation (T2 diagnostics; not part of any computation) --- */
	float last_signal_energy;
	float last_signal_spectral_sum;
	float last_gain_adjustment;
	int32_t clamp_hits;
	int32_t zero_frame_skips;
} NoiseSuppressor;


 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void NoiseSuppressor_Init(NoiseSuppressor *noise_suppressor, int32_t suppression_level);
void NoiseSuppressor_Analyze(NoiseSuppressor *noise_suppressor, float **split_data);
void NoiseSuppressor_Process(NoiseSuppressor *noise_suppressor, float **split_data);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __NOISE_SUPPRESSOR_H__*/
