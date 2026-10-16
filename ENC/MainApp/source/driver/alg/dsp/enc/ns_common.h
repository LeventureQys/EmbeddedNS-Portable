/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NS_COMMON_H__
#define __NS_COMMON_H__


/*!
 * @addtogroup ns_common
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define kNumBands 3

#define kFftSize 256
#define kFftSizeBy2Plus1 129
#define kOneByFftSizeBy2Plus1 0.0077519f
#define kNsFrameSize 160
#define kOverlapSize 96

#define kShortStartupPhaseBlocks 50
#define kOneByShortStartupPhaseBlocks 0.02f
#define kLongStartupPhaseBlocks 200




/*!
 * @}
 */
 
#endif /* __NS_COMMON_H__*/
