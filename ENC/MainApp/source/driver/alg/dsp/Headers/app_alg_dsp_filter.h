#ifndef __APP_ALG_DSP_FILTER_H__
#define __APP_ALG_DSP_FILTER_H__

#include "app_alg_dsp_config.h"


void DspAlgFilterDesign(ALG_DSP_PARA_FILTER *pFilterPara, float32_t *pCoff);

int DspOctaveFilterSetPara(int order, float fc, uint8_t isLp, float *coeff);


void DspAlgFilterDesignLow(float gain, float freq, float fs, int type, float *filterPara);
void DspAlgFilterDesignHigh(float gain, float freq, float fs, int type, float *filterPara);
void DspAlgFilterCombine(float *filter1, float *filter2, float *filterPara);


#endif


