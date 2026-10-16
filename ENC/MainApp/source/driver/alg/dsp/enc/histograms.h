/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HISTOGRAMS_H__
#define __HISTOGRAMS_H__

#include "signal_model.h"

/*!
 * @addtogroup histograms
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define kHistogramSize 1000
 
#define kBinSizeLrt 0.1f
#define kBinSizeSpecFlat 0.05f
#define kBinSizeSpecDiff 0.1f
 
 
typedef struct
{
	int32_t lrt[kHistogramSize];
	int32_t spectral_flatness[kHistogramSize];
	int32_t spectral_diff[kHistogramSize];
} Histograms;
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void Histograms_Clear(Histograms *histograms);
void Histograms_Update(Histograms *histograms, SignalModel *signal_model);	
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __HISTOGRAMS_H__*/
