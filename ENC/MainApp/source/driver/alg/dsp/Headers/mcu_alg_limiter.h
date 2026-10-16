/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MCU_ALG_LIMITER_H__
#define __MCU_ALG_LIMITER_H__

#include "stdint.h"
#include "stdbool.h"
#include "math.h"
#include "marco.h"
#include "UserRetype.h"
#include "ParamTable.h"
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

void mcu_alg_limiter_disable(void);
void mcu_alg_limiter_process(uint8_t *input, uint8_t *output, uint32_t len,uint32_t fs);
void mcu_alg_limiter_init(float * fs);
void mcu_alg_limiter_const_init(float * fs,PARAM_TBL_HANDLE paramTable);
void mcu_alg_enable_limiter(uint8_t effNum,uint8_t paramNum,uint16_t enable);
void set_mcu_alg_limiter_treshold(uint8_t effNum,uint8_t paramNum,uint16_t Treshold);
void set_muc_alg_limiter_attack(uint8_t effNum,uint8_t paramNum,uint16_t Attack);
void set_muc_alg_limiter_release(uint8_t effNum,uint8_t paramNum,uint16_t Release);
void updata_limiter_param_irq_callback(void);
#if defined(__cplusplus)
}
#endif
/*! @}*/
#endif /* __MCU_ALG_LIMITER_H__ */
