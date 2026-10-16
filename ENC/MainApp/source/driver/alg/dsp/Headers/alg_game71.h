/**
 *******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 *******************************************************************************
 * @file    alg_gamemode3.h
 * @brief   Game mode type3.
 * @version V1.1.0
 * @date    2023/01/03
 * @author  huangzheng
 *******************************************************************************
 */
 
#ifndef __APP_ALG_GAME71_H__
#define __APP_ALG_GAME71_H__

#include "jat_alg.h"
#define ALG_GAME71_VAR_SIZE                 (109)//(204)
#define	ALG_GAME71_DELAY_SIZE				(48 *2)

typedef struct
{
	// this gain control both left and right
	// 4096 corresponds to 1.0f
    int32_t     gain;
	int32_t		gainOut;
//	int32_t     delayLineLen;       // default set to ALG_DELAYLINE_SIZE_DLY_SPL
	int32_t     iramAddr;           // 0, run in mcu, != 0, run in dsp
    int32_t     varAddr;            // for variable use
	int32_t     bufferAddr;         // for delay line use
}ALG_PARA_GAME71;


int32_t AlgGame71VarSize(void);
int32_t AlgGame71Init(ALG_PARA_GAME71 *pPara);
int32_t AlgGame71ProcessF(ALG_PARA_GAME71 *pPara, void **pMem);

int32_t AlgGame71ParaSet(ALG_PARA_GAME71 *pPara);
int32_t AlgGame71ParaRefresh(ALG_PARA_GAME71 *pPara);

#endif
