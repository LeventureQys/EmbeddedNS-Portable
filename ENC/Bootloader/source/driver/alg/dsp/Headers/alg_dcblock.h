/**
 *******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 *******************************************************************************
 * @file    alg_dcblock.h
 * @brief   DC-Block, usually used after signal from ADC
 * @version V1.1.0
 * @date    2022/12/28
 * @author  huangzheng
 *******************************************************************************
 */

#ifndef __ALG_DCBLOCK_H__
#define __ALG_DCBLOCK_H__

#include "jat_alg.h"

#define ALG_VAR_SIZE_DCBLOCK_MONO           (2)     // byte number = size * 4
#define ALG_VAR_SIZE_DCBLOCK_STEREO         (4)     // byte number = size * 4

typedef struct
{
    int32_t     chMode;             // [0, 1], 0 is mono, 1 is stereo
    int32_t     iramAddr;           // 0, run in mcu, != 0, run in dsp
    int32_t     varAddr;            // for variable use
}ALG_PARA_DCBLOCK;

/**
 * @brief  algorithm DCBlock initialize
 * @param  pPara    points to a parameter structure
 * @retval == 0     OK
 * @retval != 0     ERROR
 * @details All instance of this algorithm should be initialized before use
 */
int32_t AlgDcblockInit(ALG_PARA_DCBLOCK *pPara);

/**
 * @brief  algorithm DCBlock process
 * @param  pPara    points to a parameter structure
 * @param  pMem     points to the memory of data
 * @retval == 0     OK
 * @retval != 0     ERROR, 1(varAddr is zero), 2(ic do not support), 3, current config is not for mcu,
 * @details usage e.g.
 *          float fBuffSample[2][ALG_FRAME_LEN];
 *          void *pMem[2] = {
 *              (void *)&fBuffSample[0][0],     // Channel Left or Channel Mono
 *              (void *)&fBuffSample[1][0],     // Channel Right
 *          };    
 *          AlgDcblockProcess(pPara, pMem);     // process
 * @note section name
 *  section("alg_dcblock_ram_code_section")
 */
int32_t AlgDcblockProcess(ALG_PARA_DCBLOCK *pPara, void **pMem);

#endif
