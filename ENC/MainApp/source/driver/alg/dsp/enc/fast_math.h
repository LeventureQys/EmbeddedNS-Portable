/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FAST_MATH_H__
#define __FAST_MATH_H__

//#include "usr_common.h"//Niko 屏蔽
#include "UserFunction.h"
#include "UserRetype.h"
#include "stdint.h"
#include "stdbool.h"
#include "math.h"
#include "marco.h"

/*!
 * @addtogroup fast_math
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
 
 
 
 
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define USE_FAST_FUNC       1
#define USE_MORE_THAN_48K   1

float fastsqrt(float x);
float fastlog(float x);
float fastpow(float x, float y);	
float fastexp(float x);

float fastlog10(float x);
float fastpow10(float x);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*!
 * @}
 */
 
#endif /* __FAST_MATH_H__*/
