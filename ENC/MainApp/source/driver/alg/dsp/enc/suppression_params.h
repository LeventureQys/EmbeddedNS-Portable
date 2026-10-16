/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SUPPRESSION_PARAMS_H__
#define __SUPPRESSION_PARAMS_H__

//#include "usr_common.h"//Niko 屏蔽
#include "UserFunction.h"
#include "UserRetype.h"
#include "stdint.h"
#include "stdbool.h"
#include "math.h"
#include "marco.h"

/*!
 * @addtogroup suppression_params
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
	float over_subtraction_factor;
	float minimum_attenuating_gain;
	bool use_attenuation_adjustment;
} SuppressionParams;
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void SuppressionParams_Init(SuppressionParams *suppression_params, int32_t suppression_level);
	

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __SUPPRESSION_PARAMS_H__*/
