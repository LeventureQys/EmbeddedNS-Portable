#ifndef __APP_ALG_DSP_MBD_H__
#define __APP_ALG_DSP_MBD_H__
#include "app_alg_dsp_global.h"
#include "app_alg_dsp_dynamic.h"

#define	ALG_VAR_SIZE_MBD	(43)

typedef struct
{
	// low pass cut freq
	float fc1;
	// high pass cut freq
	float fc2;
	ALG_DSP_PARA_DRC algParaDrc[3];
	// for debug usage
	float dbgG1;
	float dbgG2;
	float dbgG3;

	int32_t varAddr;
}ALG_DSP_PARA_MBD;

void DspAlgMBDParaSet(ALG_DSP_PARA_MBD *pPara);
void DspAlgMBDVarUpdate(ALG_DSP_PARA_MBD *pPara, int32_t *dspAddr);
void DspAlgMBDRefresh(ALG_DSP_PARA_MBD *pPara);



#endif

