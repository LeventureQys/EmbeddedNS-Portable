
#ifndef __ALG_FREQSHIFT_H__
#define __ALG_FREQSHIFT_H__

#include "jat_alg.h"
#include "alg_common.h"
#define ALG_VAR_SIZE_FREQSHIFT		(50)  //(47)



typedef struct
{
	int32_t	freqOffset;		// -10~10
	int32_t	freqOnOff;		// 0 or 1

	int32_t iramAddr;
	int32_t varAddr;
	// address passed via AlgFreqShiftProcess() function
	//int32_t bufAddr;	
}ALG_PARA_FREQ;

int32_t AlgFreqShiftParaRefresh(ALG_PARA_FREQ *pPara);

int32_t AlgFreqShiftParaSet(ALG_PARA_FREQ *pPara);

int32_t AlgFreqShiftInit(ALG_PARA_FREQ *pPara);

int32_t AlgFreqShiftProcess(ALG_PARA_FREQ *pPara, void **pMem);



#endif

