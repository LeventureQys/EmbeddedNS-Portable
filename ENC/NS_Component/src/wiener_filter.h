/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __WIENER_FILTER_H__
#define __WIENER_FILTER_H__

#include "ns_common.h"
#include "suppression_params.h"

/*!
 * @addtogroup wiener_filter
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
	float spectrum_prev_process[kFftSizeBy2Plus1];
	float initial_spectral_estimate[kFftSizeBy2Plus1];
	float filter[kFftSizeBy2Plus1];
} WienerFilter;


 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void WienerFilter_Init(WienerFilter *wiener_filter);
void WienerFilter_Update(WienerFilter *wiener_filter, SuppressionParams *suppression_params, int32_t num_analyzed_frames, float *noise_spectrum, float *prev_noise_spectrum, float *parametric_noise_spectrum, float *signal_spectrum);
float WienerFilter_ComputeOverallScalingFactor(SuppressionParams *suppression_params, int32_t num_analyzed_frames, float prior_speech_prob, float energy_before_filtering, float energy_after_filtering);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __WIENER_FILTER_H__*/
