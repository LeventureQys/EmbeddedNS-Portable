/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "fast_math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
__RAM_CODE float fastlog2(float x);
__RAM_CODE float fastpow2(float x);




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
__RAM_CODE float fastlog2(float x)
{
	union {int32_t i; float f;} v = {.f=x};
	return (float)(v.i)*0.00000011920929f-126.9420151f;
}

__RAM_CODE float fastpow2(float x)
{
	union {int32_t i; float f;} v = {.i=(int32_t)(8388608.0f*(x+126.9420151f))};
	return v.f;
}






__RAM_CODE float fastsqrt(float x)
{
	union {int32_t i; float f;} v = {.f=x};
	v.i=(v.i>>1)+(int32_t)(532487669);
	return (v.f+x/v.f)*0.5f;
}

__RAM_CODE float fastlog(float x)
{
	return fastlog2(x)*0.6931472f;
}

__RAM_CODE float fastpow(float x, float y)
{
	return fastpow2(fastlog2(x)*y);
}

__RAM_CODE float fastexp(float x)
{
	return fastpow(10.0f, x*0.4342945f);
}

__RAM_CODE float fastlog10(float x)
{
	union {int32_t i; float f;} v = {.f=x};
	return ((float)(v.i)*0.00000011920929f-126.9420151f)*0.3010300f;
}

__RAM_CODE float fastpow10(float x)
{
	union {int32_t i; float f;} v = {.i=(int32_t)(8388608.0f*(x*3.3219280f+126.9420151f))};
	return v.f;
}


