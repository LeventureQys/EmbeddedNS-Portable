/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "histograms.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Histograms_Clear(Histograms *histograms)
{
	int32_t i;
	
	for(i=0; i<kHistogramSize; i++)
	{
		histograms->lrt[i]=0;
		histograms->spectral_flatness[i]=0;
		histograms->spectral_diff[i]=0;
	}
}

void Histograms_Update(Histograms *histograms, SignalModel *signal_model)
{
	int32_t idx;
	float kOneByBinSizeLrt, kOneByBinSizeSpecFlat, kOneByBinSizeSpecDiff;
	
	kOneByBinSizeLrt=1.0f/kBinSizeLrt;
	if(signal_model->lrt>=0.0f && signal_model->lrt<(float)kHistogramSize*kBinSizeLrt)
	{
		idx=kOneByBinSizeLrt*signal_model->lrt;
		histograms->lrt[idx]++;
	}
	
	kOneByBinSizeSpecFlat=1.0f/kBinSizeSpecFlat;
	if(signal_model->spectral_flatness>=0.0f && signal_model->spectral_flatness<(float)kHistogramSize*kBinSizeSpecFlat)
	{
		idx=kOneByBinSizeSpecFlat*signal_model->spectral_flatness;
		histograms->spectral_flatness[idx]++;
	}
	
	kOneByBinSizeSpecDiff=1.0f/kBinSizeSpecDiff;
	if(signal_model->spectral_diff>=0.0f && signal_model->spectral_diff<(float)kHistogramSize*kBinSizeSpecDiff)
	{
		idx=kOneByBinSizeSpecDiff*signal_model->spectral_diff;
		histograms->spectral_diff[idx]++;
	}
}





