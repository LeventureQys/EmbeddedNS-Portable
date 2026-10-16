/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MCU_ALG_COMP_H__
#define __MCU_ALG_COMP_H__

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

void mcu_alg_comp_disable(void);
void mcu_alg_comp_process(uint8_t *input, uint8_t *output, uint32_t len,uint32_t fs);
void mcu_alg_comp_init(float * fs);
void mcu_alg_comp_const_init(float * fs,PARAM_TBL_HANDLE paramTable);
void mcu_alg_enable_comp(uint8_t effNum,uint8_t paramNum,uint16_t enable);
void set_mcu_alg_comp_treshold(uint8_t effNum,uint8_t paramNum,uint16_t Treshold);
void set_mcu_alg_comp_ratio(uint8_t effNum,uint8_t paramNum,uint16_t Ratio);
void set_mcu_alg_comp_kneew(uint8_t effNum,uint8_t paramNum,uint16_t KneeWidth);
void set_muc_alg_comp_attack(uint8_t effNum,uint8_t paramNum,uint16_t Attack);
void set_muc_alg_comp_mkg(uint8_t effNum,uint8_t paramNum,uint16_t MakeUpGain);
void set_muc_alg_comp_release(uint8_t effNum,uint8_t paramNum,uint16_t Release);
void updata_comp_param_irq_callback(void);
#if defined(__cplusplus)
}
#endif
/*! @}*/
#endif /* __MCU_ALG_COMP_H__ */
