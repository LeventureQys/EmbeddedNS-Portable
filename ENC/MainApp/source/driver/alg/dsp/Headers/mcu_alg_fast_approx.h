/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MCU_ALG_FAST_APPROX_H__
#define __MCU_ALG_FAST_APPROX_H__

//#include "fsl_common.h"

/*!
 * @addtogroup mcu_alg_fast_approx
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

#define USE_FAST_FUNC       1
#define USE_MORE_THAN_48K   1

float fasterpow10(float x);
float fasterexp(float x);
float fasterlog10(float x);
float fastsqrt(float x);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
 
#endif /* __MCU_ALG_FAST_APPROX_H__*/
