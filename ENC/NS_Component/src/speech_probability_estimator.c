/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "speech_probability_estimator.h"
#include "fast_math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void SpeechProbabilityEstimator_Init(SpeechProbabilityEstimator *speech_probability_estimator)
{
	int32_t i;
	
	speech_probability_estimator->prior_speech_prob=0.5f;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		speech_probability_estimator->speech_probability[i]=0.0f;
	}
	
	SignalModelEstimator_Init(&speech_probability_estimator->signal_model_estimator);
}

#pragma push
#pragma O0
void SpeechProbabilityEstimator_Update(SpeechProbabilityEstimator *speech_probability_estimator, int32_t num_analyzed_frames, float *prior_snr, float *post_snr, float *conservative_noise_spectrum, float *signal_spectrum, float signal_spectral_sum, float signal_energy)
{
	int32_t i;
	float kWidthPrior0, kWidthPrior1, width_prior, indicator0, indicator1, indicator2, ind_prior, gain_prior, inv_lrt;
	SignalModel *model;
	PriorSignalModel *prior_model;
	
	if(num_analyzed_frames<kLongStartupPhaseBlocks)
	{
		SignalModelEstimator_AdjustNormalization(&speech_probability_estimator->signal_model_estimator, num_analyzed_frames, signal_energy);
	}
	
	SignalModelEstimator_Update(&speech_probability_estimator->signal_model_estimator, prior_snr, post_snr, conservative_noise_spectrum, signal_spectrum, signal_spectral_sum, signal_energy);
	
	model=&speech_probability_estimator->signal_model_estimator.signal_model;
	prior_model=&speech_probability_estimator->signal_model_estimator.prior_signal_model;
	
	kWidthPrior0=4.0f;
	kWidthPrior1=2.0f*kWidthPrior0;
	
	width_prior=model->lrt<prior_model->lrt ? kWidthPrior1 : kWidthPrior0;
	indicator0=0.5f*(tanhf(width_prior*(model->lrt-prior_model->lrt))+1.0f);
	
	width_prior=model->spectral_flatness>prior_model->flatness_threshold ? kWidthPrior1 : kWidthPrior0;
	indicator1=0.5f*(tanhf(width_prior*(prior_model->flatness_threshold-model->spectral_flatness))+1.0f);
	
	width_prior=model->spectral_diff<prior_model->template_diff_threshold ? kWidthPrior1 : kWidthPrior0;
	indicator2=0.5f*(tanhf(width_prior*(model->spectral_diff-prior_model->template_diff_threshold))+1.0f);
	
	ind_prior=prior_model->lrt_weighting*indicator0+prior_model->flatness_weighting*indicator1+prior_model->difference_weighting*indicator2;
	speech_probability_estimator->prior_speech_prob+=0.1f*(ind_prior-speech_probability_estimator->prior_speech_prob);
	speech_probability_estimator->prior_speech_prob=MAX(MIN(speech_probability_estimator->prior_speech_prob, 1.0f), 0.01f);
	
	gain_prior=(1.0f-speech_probability_estimator->prior_speech_prob)/(speech_probability_estimator->prior_speech_prob+0.0001f);
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		inv_lrt=fastexp(-model->avg_log_lrt[i]);
		speech_probability_estimator->speech_probability[i]=1.0f/(1.0f+gain_prior*inv_lrt);
	}
}

#pragma pop



