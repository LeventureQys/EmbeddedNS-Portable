/**
 *******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 *******************************************************************************
 * @file    alg_rms.h
 * @brief   RMS calculate. Support more than one instance, can only run in MCU
 * @version V1.0.0
 * @date    2022/10/13
 * @author  huangzheng
 *******************************************************************************
 */

#ifndef __ALG_RMS_H__
#define __ALG_RMS_H__

#include "jat_alg.h"

#define ALG_VAR_SIZE_RMS_MONO               (4)     // byte number = size * 4

typedef struct
{
    int32_t     averageTime;        // [1, 1000], 1~1000ms
    int32_t     iramAddr;           // 0, run in mcu, != 0, run in dsp
    int32_t     varAddr;            // for variable use
}ALG_PARA_RMSCALC;

/**
 * @brief  algorithm RMS calculate initialize
 * @param  pPara    points to a parameter structure
 * @retval == 0     OK
 * @retval != 0     ERROR
 * @details All instance of this algorithm should be initialized before use
 */
int32_t AlgRmsCalcInit(ALG_PARA_RMSCALC *pPara);

/**
 * @brief  algorithm RMS calculate parameter set
 * @param  pPara    points to a parameter structure
 * @retval == 0     OK
 * @retval != 0     ERROR
 * @details After this function called, parameters will be auto refreshed in mcu mode. The refresh function must be called manually in dsp mode.
 */
int32_t AlgRmsCalcParaSet(ALG_PARA_RMSCALC *pPara);

/**
 * @brief  algorithm Limit process
 * @param  pPara    points to a parameter structure
 * @retval >= 0     result of rms value, normalized to 1.0f
 * @retval <  0     ERROR, -1(varAddr is zero),
 * @note section name
 *  section("alg_rms_ram_code_section")
 */
float AlgRmsCalcGet(ALG_PARA_RMSCALC *pPara);

/**
 * @brief  algorithm Limit process
 * @param  pPara    points to a parameter structure
 * @param  pMem     points to the memory of data
 * @retval >= 0     result of rms value, normalized to 1.0f
 * @retval <  0     ERROR, -1(varAddr is zero), -2(ic do not support), -3(current config is not for mcu),
 * @details usage e.g.
 *          float fBuffSample[2][ALG_FRAME_LEN];
 *          void *pMem[2] = {
 *              (void *)&fBuffSample[0][0],     // Channel Left or Channel Mono
 *              (void *)&fBuffSample[1][0],     // Channel Right
 *          };    
 *          AlgRmsCalcProcess(pPara, pMem[0]);       // process chL
 *          AlgRmsCalcProcess(pPara, pMem[1]);       // process chR
 * @note section name
 *  section("alg_rms_ram_code_section")
 *  section("alg_rms_unit_ram_code_section")
 */
float AlgRmsCalcProcess(ALG_PARA_RMSCALC *pPara, const float *pfData);

#endif
