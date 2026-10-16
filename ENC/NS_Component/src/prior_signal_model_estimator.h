/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PRIOR_SIGNAL_MODEL_ESTIMATOR_H__
#define __PRIOR_SIGNAL_MODEL_ESTIMATOR_H__

#include "prior_signal_model.h"
#include "histograms.h"

/*!
 * @addtogroup prior_signal_model_estimator
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define kFeatureUpdateWindowSize 500
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void PriorSignalModelEstimator_Init(PriorSignalModel *prior_signal_model, float lrt_initial_value);
void PriorSignalModelEstimator_Update(PriorSignalModel *prior_signal_model, Histograms *histograms);	
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __PRIOR_SIGNAL_MODEL_ESTIMATOR_H__*/
