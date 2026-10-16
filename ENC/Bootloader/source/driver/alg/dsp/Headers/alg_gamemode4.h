#ifndef __ALG_GAMEMODE4_H__
#define __ALG_GAMEMODE4_H__

#include "jat_alg.h"
#include "alg_filter.h"


#define ALG_GAMEMD4_VAR_SIZE                 (607)//(678)          //byte number = size * 4
#define ALG_GAMEMD4_DELAYLINE_SIZE           (71)          //byte number = size * 4

typedef struct
{
	int32_t     iramAddr;           // 0, run in mcu, != 0, run in dsp
    int32_t     varAddr;
	int32_t     bufferAddr;
}ALG_PARA_GAMEMD4;

int32_t AlgGameMode4VarSize(void);
int32_t AlgGameMode4DelayLineVarSize(void);
void AlgGameMode4ParaSet(ALG_PARA_GAMEMD4 *pPara);
int32_t AlgGameMode4Init(ALG_PARA_GAMEMD4 *pPara);
int32_t AlgGameMode4ProcessF(ALG_PARA_GAMEMD4 *pPara, void **pMem);

#endif
