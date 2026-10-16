/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "suppression_params.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void SuppressionParams_Init(SuppressionParams *suppression_params, int32_t suppression_level)
{
	switch(suppression_level)
	{
		case 0:
			suppression_params->over_subtraction_factor=1.0f;
			suppression_params->minimum_attenuating_gain=0.5f;
			suppression_params->use_attenuation_adjustment=false;
			break;
		case 1:
			suppression_params->over_subtraction_factor=1.0f;
			suppression_params->minimum_attenuating_gain=0.25f;
			suppression_params->use_attenuation_adjustment=true;
			break;
		case 2:
			suppression_params->over_subtraction_factor=1.1f;
			suppression_params->minimum_attenuating_gain=0.125f;
			suppression_params->use_attenuation_adjustment=true;
			break;
		case 3:
			suppression_params->over_subtraction_factor=1.25f;
			suppression_params->minimum_attenuating_gain=0.09f;
			suppression_params->use_attenuation_adjustment=true;
			break;
		default:
			suppression_params->over_subtraction_factor=1.0f;
			suppression_params->minimum_attenuating_gain=0.25f;
			suppression_params->use_attenuation_adjustment=true;
	}
}





