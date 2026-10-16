/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PRIOR_SIGNAL_MODEL_H__
#define __PRIOR_SIGNAL_MODEL_H__

#include "vendor_types.h"

/*!
 * @addtogroup prior_signal_model
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
	float lrt;
	float flatness_threshold;
	float template_diff_threshold;
	 
	float lrt_weighting;
	float flatness_weighting;
	float difference_weighting;
} PriorSignalModel;
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void PriorSignalModel_Init(PriorSignalModel *prior_signal_model, float lrt_initial_value);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __PRIOR_SIGNAL_MODEL_H__*/
