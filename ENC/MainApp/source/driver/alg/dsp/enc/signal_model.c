/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "signal_model.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void SignalModel_Init(SignalModel *signal_model)
{
	int32_t i;
	float kSfFeatureThr=0.5f;
	
	signal_model->lrt=kLrtFeatureThr;
	signal_model->spectral_flatness=kSfFeatureThr;
	signal_model->spectral_diff=kSfFeatureThr;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		signal_model->avg_log_lrt[i]=kLrtFeatureThr;
	}
}





