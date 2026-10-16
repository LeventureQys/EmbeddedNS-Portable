/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SIGNAL_MODEL_H__
#define __SIGNAL_MODEL_H__

//#include "usr_common.h"//Niko 屏蔽
#include "UserFunction.h"
#include "UserRetype.h"
#include "stdint.h"
#include "stdbool.h"
#include "math.h"
#include "marco.h"

#include "ns_common.h"

/*!
 * @addtogroup signal_model
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define kLrtFeatureThr 0.5f
 
 
typedef struct
{
	float lrt;
	float spectral_flatness;
	float spectral_diff;
	 
	float avg_log_lrt[kFftSizeBy2Plus1];
} SignalModel;
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void SignalModel_Init(SignalModel *signal_model);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __SIGNAL_MODEL_H__*/
