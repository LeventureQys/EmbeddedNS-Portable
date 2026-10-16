/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "quantile_noise_estimator.h"
#include "fast_math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void QuantileNoiseEstimator_Init(QuantileNoiseEstimator *quantile_noise_estimator)
{
	int32_t i;
	for(i=0; i<kSimult*kFftSizeBy2Plus1; i++)
	{
		quantile_noise_estimator->density[i]=0.3f;
		quantile_noise_estimator->log_quantile[i]=8.0f;
	}
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		quantile_noise_estimator->quantile[i]=0.0f;
	}
	for(i=0; i<kSimult; i++)
	{
		quantile_noise_estimator->counter[i]=(float)kLongStartupPhaseBlocks*(float)(i+1)*kOneBySimult;
	}
	quantile_noise_estimator->num_updates=1;
}

void QuantileNoiseEstimator_Estimate(QuantileNoiseEstimator *quantile_noise_estimator, float *signal_spectrum, float *noise_spectrum)
{
	int32_t quantile_index_to_return, i, k, s, j;
	float one_by_counter_plus_1, delta, multiplier, kWidth, kOneByWidthPlus2;
	float log_spectrum[kFftSizeBy2Plus1];
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		log_spectrum[i]=fastlog(signal_spectrum[i]);
	}
	
	quantile_index_to_return=-1;
	
	k=0;
	for(s=0; s<kSimult; s++)
	{
		one_by_counter_plus_1=1.0f/(float)(quantile_noise_estimator->counter[s]+1);
		j=k;
		for(i=0; i<kFftSizeBy2Plus1; i++)
		{
			delta=quantile_noise_estimator->density[j]>1.0f ? 40.0f/quantile_noise_estimator->density[j] : 40.0f;
			multiplier=delta*one_by_counter_plus_1;
			
			if(log_spectrum[i]>quantile_noise_estimator->log_quantile[j])
			{
				quantile_noise_estimator->log_quantile[j]+=0.25f*multiplier;
			}
			else
			{
				quantile_noise_estimator->log_quantile[j]-=0.75f*multiplier;
			}
			
			kWidth=0.01f;
			kOneByWidthPlus2=1.0f/(2.0f*kWidth);
			if(ABS(log_spectrum[i]-quantile_noise_estimator->log_quantile[j])<kWidth)
			{
				quantile_noise_estimator->density[j]=((float)quantile_noise_estimator->counter[s]*quantile_noise_estimator->density[j]+kOneByWidthPlus2)*one_by_counter_plus_1;
			}
			
			j++;	
		}
		
		if(quantile_noise_estimator->counter[s]>=kLongStartupPhaseBlocks)
		{
			quantile_noise_estimator->counter[s]=0;
			if(quantile_noise_estimator->num_updates>=kLongStartupPhaseBlocks)
			{
				quantile_index_to_return=k;
			}
		}
		
		quantile_noise_estimator->counter[s]++;
		k+=kFftSizeBy2Plus1;
	}
	
	if(quantile_noise_estimator->num_updates<kLongStartupPhaseBlocks)
	{
		quantile_index_to_return=kFftSizeBy2Plus1*(kSimult-1);
		quantile_noise_estimator->num_updates++;
	}
	
	if(quantile_index_to_return>=0)
	{
		for(i=0; i<kFftSizeBy2Plus1; i++)
		{
			quantile_noise_estimator->quantile[i]=fastexp(quantile_noise_estimator->log_quantile[quantile_index_to_return+i]);
		}
	}
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		noise_spectrum[i]=quantile_noise_estimator->quantile[i];
	}
}





