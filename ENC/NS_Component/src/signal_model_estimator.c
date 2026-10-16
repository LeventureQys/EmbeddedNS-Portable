/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "signal_model_estimator.h"
#include "fast_math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
void UpdateSpectralFlatness(float *signal_spectrum, float signal_spectral_sum, float *spectral_flatness);
float ComputeSpectralDiff(float *conservative_noise_spectrum, float *signal_spectrum, float signal_spectral_sum, float diff_normalization);
void UpdateSpectralLrt(float *prior_snr, float *post_snr, float *avg_log_lrt, float *lrt);




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void UpdateSpectralFlatness(float *signal_spectrum, float signal_spectral_sum, float *spectral_flatness)
{
	int32_t i;
	float kAveraging, avg_spect_flatness_num, avg_spect_flatness_denom, spectral_tmp;
	
	kAveraging=0.3f;
	avg_spect_flatness_num=0.0f;
	
	for(i=1; i<kFftSizeBy2Plus1; i++)
	{
		if(signal_spectrum[i]==0.0f)
		{
			*spectral_flatness-=kAveraging*(*spectral_flatness);
			return;
		}
	}
	
	for(i=1; i<kFftSizeBy2Plus1; i++)
	{
		avg_spect_flatness_num+=fastlog(signal_spectrum[i]);
	}
	
	avg_spect_flatness_denom=(signal_spectral_sum-signal_spectrum[0])*kOneByFftSizeBy2Plus1;
	avg_spect_flatness_num*=kOneByFftSizeBy2Plus1;
	
	spectral_tmp=fastexp(avg_spect_flatness_num)/(avg_spect_flatness_denom+0.0001f);
	
	*spectral_flatness+=kAveraging*(spectral_tmp-(*spectral_flatness));
}

float ComputeSpectralDiff(float *conservative_noise_spectrum, float *signal_spectrum, float signal_spectral_sum, float diff_normalization)
{
	int32_t i;
	float noise_average, signal_average, covariance, noise_variance, signal_variance, signal_diff, noise_diff, spectral_diff;
	
	noise_average=0.0f;
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		noise_average+=conservative_noise_spectrum[i];
	}
	
	noise_average*=kOneByFftSizeBy2Plus1;
	signal_average=signal_spectral_sum*kOneByFftSizeBy2Plus1;
	
	covariance=0.0f;
	noise_variance=0.0f;
	signal_variance=0.0f;
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		signal_diff=signal_spectrum[i]-signal_average;
		noise_diff=conservative_noise_spectrum[i]-noise_average;
		covariance+=signal_diff*noise_diff;
		noise_variance+=noise_diff*noise_diff;
		signal_variance+=signal_diff*signal_diff;
	}
	
	covariance*=kOneByFftSizeBy2Plus1;
	noise_variance*=kOneByFftSizeBy2Plus1;
	signal_variance*=kOneByFftSizeBy2Plus1;
	
	spectral_diff=signal_variance-(covariance*covariance)/(noise_variance+0.0001f);
	return spectral_diff/(diff_normalization+0.0001f);
}

void UpdateSpectralLrt(float *prior_snr, float *post_snr, float *avg_log_lrt, float *lrt)
{
	int32_t i;
	float tmp1, tmp2, bessel_tmp, log_lrt_time_avg_k_sum;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		tmp1=1.0f+2.0f*prior_snr[i];
		tmp2=2.0f*prior_snr[i]/(tmp1+0.0001f);
		bessel_tmp=(post_snr[i]+1.0f)*tmp2;
		avg_log_lrt[i]+=0.5f*(bessel_tmp-fastlog(tmp1)-avg_log_lrt[i]);
	}
	
	log_lrt_time_avg_k_sum=0.0f;
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		log_lrt_time_avg_k_sum+=avg_log_lrt[i];
	}
	
	*lrt=log_lrt_time_avg_k_sum*kOneByFftSizeBy2Plus1;
}






void SignalModelEstimator_Init(SignalModelEstimator *signal_model_estimator)
{
	signal_model_estimator->diff_normalization=0.0f;
	signal_model_estimator->signal_energy_sum=0.0f;
	signal_model_estimator->histogram_analysis_counter=500;
	
	Histograms_Clear(&signal_model_estimator->histograms);
	SignalModel_Init(&signal_model_estimator->signal_model);
	PriorSignalModelEstimator_Init(&signal_model_estimator->prior_signal_model, kLrtFeatureThr);
}

void SignalModelEstimator_AdjustNormalization(SignalModelEstimator *signal_model_estimator, int32_t num_analyzed_frames, float signal_energy)
{
	signal_model_estimator->diff_normalization*=(float)num_analyzed_frames;
	signal_model_estimator->diff_normalization+=signal_energy;
	signal_model_estimator->diff_normalization/=(float)(num_analyzed_frames+1);
}

void SignalModelEstimator_Update(SignalModelEstimator *signal_model_estimator, float *prior_snr, float *post_snr, float *conservative_noise_spectrum, float *signal_spectrum, float signal_spectral_sum, float signal_energy)
{
	float spectral_diff;
	
	UpdateSpectralFlatness(signal_spectrum, signal_spectral_sum, &signal_model_estimator->signal_model.spectral_flatness);
	
	spectral_diff=ComputeSpectralDiff(conservative_noise_spectrum, signal_spectrum, signal_spectral_sum, signal_model_estimator->diff_normalization);
	
	signal_model_estimator->signal_model.spectral_diff+=0.3f*(spectral_diff-signal_model_estimator->signal_model.spectral_diff);
	
	signal_model_estimator->signal_energy_sum+=signal_energy;
	
	signal_model_estimator->histogram_analysis_counter--;
	if(signal_model_estimator->histogram_analysis_counter>0)
	{
		Histograms_Update(&signal_model_estimator->histograms, &signal_model_estimator->signal_model);	
	}
	else
	{
		PriorSignalModelEstimator_Update(&signal_model_estimator->prior_signal_model, &signal_model_estimator->histograms);
		
		Histograms_Clear(&signal_model_estimator->histograms);
		signal_model_estimator->histogram_analysis_counter=kFeatureUpdateWindowSize;
		
		signal_model_estimator->signal_energy_sum/=(float)kFeatureUpdateWindowSize;
		signal_model_estimator->diff_normalization=0.5f*(signal_model_estimator->signal_energy_sum+signal_model_estimator->diff_normalization);
		signal_model_estimator->signal_energy_sum=0.0f;
	}
	
	UpdateSpectralLrt(prior_snr, post_snr, signal_model_estimator->signal_model.avg_log_lrt, &signal_model_estimator->signal_model.lrt);
}





