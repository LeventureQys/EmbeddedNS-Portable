/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __THREE_BAND_FILTER_BANK_H__
#define __THREE_BAND_FILTER_BANK_H__

#include "vendor_types.h"

#include "ns_common.h"

/*!
 * @addtogroup three_band_filter_bank
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define kNumNonZeroFilters 10
#define kZeroFilterIndex1 3
#define kZeroFilterIndex2 9
#define kMemorySize 15
 
#define kFullBandSize 480
#define kSplitBandSize 160
#define kSubSampling 3
#define kFilterSize 4
#define kDctSize 3
 
#define kStrideLog2 2
#define kStride 4

#define kFilterSize_kStride (kFilterSize*kStride)//Niko

 
typedef struct
{
	float state_analysis[kNumNonZeroFilters][kMemorySize];
	float state_synthesis[kNumNonZeroFilters][kMemorySize];
} ThreeBandFilterBank;
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void ThreeBandFilterBank_Init(ThreeBandFilterBank *three_band_filter_bank);
void ThreeBandFilterBank_Analysis(ThreeBandFilterBank *three_band_filter_bank, float *in, float **out);
void ThreeBandFilterBank_Synthesis(ThreeBandFilterBank *three_band_filter_bank, float **in, float *out);	
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __THREE_BAND_FILTER_BANK_H__*/
