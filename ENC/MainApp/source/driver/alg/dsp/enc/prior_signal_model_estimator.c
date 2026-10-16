/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "prior_signal_model_estimator.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
void UpdateLrt(int32_t *lrt_histogram, float *prior_model_lrt, bool *low_lrt_fluctuations);
void FindFirstOfTwoLargestPeaks(float bin_size, int32_t *spectral_flatness, int32_t *peak_weight, float *peak_position);




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void UpdateLrt(int32_t *lrt_histogram, float *prior_model_lrt, bool *low_lrt_fluctuations)
{
	int32_t i, count;
	float average, average_squared, average_compl, bin_mid, kOneByFeatureUpdateWindowSize, kMaxLrt, kMinLrt;
	
	count=0;
	average=0.0f;
	average_squared=0.0f;
	average_compl=0.0f;
	
	for(i=0; i<10; i++)
	{
		bin_mid=((float)i+0.5f)*kBinSizeLrt;
		average+=(float)lrt_histogram[i]*bin_mid;
		count+=lrt_histogram[i];
	}
	
	if(count>0)
	{
		average/=(float)count;
	}
	
	for(i=0; i<kHistogramSize; i++)
	{
		bin_mid=((float)i+0.5f)*kBinSizeLrt;
		average_squared+=(float)lrt_histogram[i]*bin_mid*bin_mid;
		average_compl+=(float)lrt_histogram[i]*bin_mid;
	}
	
	kOneByFeatureUpdateWindowSize=1.0f/(float)kFeatureUpdateWindowSize;
	average_squared*=kOneByFeatureUpdateWindowSize;
	average_compl*=kOneByFeatureUpdateWindowSize;
	
	*low_lrt_fluctuations=(average_squared-average*average_compl)<0.05f;
	
	kMaxLrt=1.0f;
	kMinLrt=0.2f;
	if(*low_lrt_fluctuations)
	{
		*prior_model_lrt=kMaxLrt;
	}
	else
	{
		*prior_model_lrt=MIN(kMaxLrt, MAX(kMinLrt, 1.2f*average));
	}
}

void FindFirstOfTwoLargestPeaks(float bin_size, int32_t *spectral_flatness, int32_t *peak_weight, float *peak_position)
{
	int32_t peak_value, secondary_peak_value, secondary_peak_weight, i;
	float secondary_peak_position, bin_mid;
	
	peak_value=0;
	secondary_peak_value=0;
	*peak_weight=0;
	secondary_peak_weight=0;
	*peak_position=0.0f;
	secondary_peak_position=0.0f;
	
	for(i=0; i<kHistogramSize; i++)
	{
		bin_mid=((float)i+0.5f)*bin_size;
		if(spectral_flatness[i]>peak_value)
		{
			secondary_peak_value=peak_value;
			secondary_peak_weight=*peak_weight;
			secondary_peak_position=*peak_position;
			
			peak_value=spectral_flatness[i];
			*peak_weight=spectral_flatness[i];
			*peak_position=bin_mid;
		}
		else if(spectral_flatness[i]>secondary_peak_value)
		{
			secondary_peak_value=spectral_flatness[i];
			secondary_peak_weight=spectral_flatness[i];
			secondary_peak_position=bin_mid;
		}
	}
	
	if(ABS(secondary_peak_position-(*peak_position))<2.0f*bin_size && (float)secondary_peak_weight>(float)(*peak_weight)*0.5f)
	{
		*peak_weight+=secondary_peak_weight;
		*peak_position=0.5f*(*peak_position+secondary_peak_position);
	}
}






void PriorSignalModelEstimator_Init(PriorSignalModel *prior_signal_model, float lrt_initial_value)
{
	PriorSignalModel_Init(prior_signal_model, lrt_initial_value);
}

void PriorSignalModelEstimator_Update(PriorSignalModel *prior_signal_model, Histograms *histograms)
{
	bool low_lrt_fluctuations;
	int32_t spectral_flatness_peak_weight, spectral_diff_peak_weight, use_spec_flat, use_spec_diff;
	float spectral_flatness_peak_position, spectral_diff_peak_position, one_by_feature_sum;
	
	UpdateLrt(histograms->lrt, &prior_signal_model->lrt, &low_lrt_fluctuations);
	
	FindFirstOfTwoLargestPeaks(kBinSizeSpecFlat, histograms->spectral_flatness, &spectral_flatness_peak_weight, &spectral_flatness_peak_position);
	FindFirstOfTwoLargestPeaks(kBinSizeSpecDiff, histograms->spectral_diff, &spectral_diff_peak_weight, &spectral_diff_peak_position);
	
	use_spec_flat=(float)spectral_flatness_peak_weight<0.3f*500.0f || spectral_flatness_peak_position<0.6f ? 0 : 1;
	use_spec_diff=(float)spectral_diff_peak_weight<0.3f*500.0f || low_lrt_fluctuations ? 0 : 1;
	
	prior_signal_model->template_diff_threshold=MIN(1.0f, MAX(0.16f, 1.2f*spectral_diff_peak_position));
	
	one_by_feature_sum=1.0f/(float)(1+use_spec_flat+use_spec_diff);
	prior_signal_model->lrt_weighting=one_by_feature_sum;
	
	if(use_spec_flat==1)
	{
		prior_signal_model->flatness_threshold=MIN(0.95f, MAX(0.1f, 0.9f*spectral_flatness_peak_position));
		prior_signal_model->flatness_weighting=one_by_feature_sum;
	}
	else
	{
		prior_signal_model->flatness_weighting=0.0f;
	}
	
	if(use_spec_diff==1)
	{
		prior_signal_model->difference_weighting=one_by_feature_sum;
	}
	else
	{
		prior_signal_model->difference_weighting=0.0f;
	}
}





