/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "prior_signal_model.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void PriorSignalModel_Init(PriorSignalModel *prior_signal_model, float lrt_initial_value)
{
	prior_signal_model->lrt=lrt_initial_value;
	prior_signal_model->flatness_threshold=0.5f;
	prior_signal_model->template_diff_threshold=0.5f;
	
	prior_signal_model->lrt_weighting=1.0f;
	prior_signal_model->flatness_weighting=0.0f;
	prior_signal_model->difference_weighting=0.0f;
}





