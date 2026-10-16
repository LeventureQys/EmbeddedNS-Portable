/*
 * Copyright 2018 - 2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "stdint.h"
#include "A0.h"
//#include "mcu_alg_fast_approx.h"
#include "fast_math.h"
/****************************1***************************************************
* Public macro definitions
 ******************************************************************************/
#if (SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN)
    #if 0
        #include "fsl_debug_console.h"
        #define FAST_APPROX_DEBUG        PRINTF
    #else
        #define FAST_APPROX_DEBUG(...)
    #endif
#else
    #define FAST_APPROX_DEBUG(...)
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
__RAM_CODE float fasterpow10(float x)
{
	union {int32_t i; float f;} v = {.i=(int32_t)(8388608.0f*(x*3.3219280f+126.9420151f))};
	return v.f;
}

__RAM_CODE float fasterexp(float x)
{
	union {int32_t i; float f;} v = {.i=(int32_t)(8388608.0f*(x*1.4426950f+126.9420151f))};
	return v.f;
}

__RAM_CODE float fasterlog10(float x)
{
	union {int32_t i; float f;} v = {.f=x};
	return ((float)(v.i)*0.00000011920929f-126.9420151f)*0.3010300f;
}

__RAM_CODE float fastsqrt(float x)
{
	union {int32_t i; float f;} v = {.f=x};
	v.i=(v.i>>1)+(int32_t)(532487669);
	return (v.f+x/v.f)*0.5f;
}
