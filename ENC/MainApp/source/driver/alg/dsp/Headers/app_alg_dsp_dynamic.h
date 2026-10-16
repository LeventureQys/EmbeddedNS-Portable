#ifndef __APP_ALG_DSP_DYNAMIC_H__
#define __APP_ALG_DSP_DYNAMIC_H__

#include "app_alg_dsp_config.h"

#include "app_alg_dsp_global.h"


typedef struct
{
    int32_t     preGain;            //0~128
    int32_t     threshold;          //dB
    int32_t     attackTime;         //ms
    int32_t     releaseTime;        //ms
    //float32_t   ratio;              //1.0f~20.0f
    uint32_t	ratio;				// 100 ~2000 , expand 100 in comparison with float type		
    int32_t     varAddr;            
}ALG_DSP_PARA_DRC;

void DspAlgDrcUnitParaSet(ALG_DSP_PARA_DRC *pPara, ALG_DSP_VAR_DRC *pVar);


typedef struct
{
    int32_t     preGain;            //0-128
    int32_t     preGainEnergy;      //0-128
    int32_t     threshold;          //dB
    int32_t     attackTime;         //ms
    int32_t     releaseTime;        //ms
    int32_t     level;              //0~32768
    int32_t     varAddr;
	
}ALG_DSP_PARA_GATE;



void DspAlgGateUnitParaSet(ALG_DSP_PARA_GATE *pPara, float32_t *pVar);

void DspAlgGateUnitUpdate(ALG_DSP_PARA_GATE *pPara, int32_t *dspAddr);

void DspAlgGateUnitRefresh(ALG_DSP_PARA_GATE *pPara);

#endif


