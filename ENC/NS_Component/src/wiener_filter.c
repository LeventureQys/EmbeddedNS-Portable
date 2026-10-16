/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "wiener_filter.h"
#include "fast_math.h"
#include "ns_patch.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void WienerFilter_Init(WienerFilter *wiener_filter)
{
	int32_t i;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		wiener_filter->spectrum_prev_process[i]=0.0f;
		wiener_filter->initial_spectral_estimate[i]=0.0f;
		wiener_filter->filter[i]=1.0f;
	}
}

void WienerFilter_Update(WienerFilter *wiener_filter, SuppressionParams *suppression_params, int32_t num_analyzed_frames, float *noise_spectrum, float *prev_noise_spectrum, float *parametric_noise_spectrum, float *signal_spectrum)
{
	int32_t i;
	float prev_tsa, current_tsa, snr_prior, filter_initial;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		prev_tsa=wiener_filter->spectrum_prev_process[i]/(prev_noise_spectrum[i]+0.0001f)*wiener_filter->filter[i];
		
		if(signal_spectrum[i]>noise_spectrum[i])
		{
			current_tsa=signal_spectrum[i]/(noise_spectrum[i]+0.0001f)-1.0f;
		}
		else
		{
			current_tsa=0.0f;
		}
		
		snr_prior=0.98f*prev_tsa+(1.0f-0.98f)*current_tsa;
		wiener_filter->filter[i]=snr_prior/(suppression_params->over_subtraction_factor+snr_prior);
		wiener_filter->filter[i]=MAX(MIN(wiener_filter->filter[i], 1.0f), suppression_params->minimum_attenuating_gain);		
	}
	
	if(num_analyzed_frames<kShortStartupPhaseBlocks)
	{
		for(i=0; i<kFftSizeBy2Plus1; i++)
		{
			wiener_filter->initial_spectral_estimate[i]+=signal_spectrum[i];
			filter_initial=wiener_filter->initial_spectral_estimate[i]-suppression_params->over_subtraction_factor*parametric_noise_spectrum[i];
			filter_initial/=wiener_filter->initial_spectral_estimate[i]+0.0001f;
			filter_initial=MAX(MIN(filter_initial, 1.0f), suppression_params->minimum_attenuating_gain);
			
			filter_initial*=(float)(kShortStartupPhaseBlocks-num_analyzed_frames);
			wiener_filter->filter[i]*=(float)num_analyzed_frames;
			wiener_filter->filter[i]+=filter_initial;
			wiener_filter->filter[i]*=kOneByShortStartupPhaseBlocks;
		}
	}
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		wiener_filter->spectrum_prev_process[i]=signal_spectrum[i];
	}
}

float WienerFilter_ComputeOverallScalingFactor(SuppressionParams *suppression_params, int32_t num_analyzed_frames, float prior_speech_prob, float energy_before_filtering, float energy_after_filtering)
{
	float gain, kBLim, scale_factor1, scale_factor2;
	
	if(!suppression_params->use_attenuation_adjustment || num_analyzed_frames<=kLongStartupPhaseBlocks)
	{
		return 1.0f;
	}
	
	gain=NS_SQRT(energy_after_filtering/(energy_before_filtering+1.0f));
	
	kBLim=0.5f;
	scale_factor1=1.0f;
	if(gain>kBLim)
	{
		scale_factor1=1.0f+1.3f*(gain-kBLim);
		if(gain*scale_factor1>1.0f)
		{
			scale_factor1=1.0f/gain;
		}
	}
	
	scale_factor2=1.0f;
	if(gain<kBLim)
	{
		gain=MAX(gain, suppression_params->minimum_attenuating_gain);
		scale_factor2=1.0f-0.3f*(kBLim-gain);
	}
	
	return prior_speech_prob*scale_factor1+(1.0f-prior_speech_prob)*scale_factor2;
}





