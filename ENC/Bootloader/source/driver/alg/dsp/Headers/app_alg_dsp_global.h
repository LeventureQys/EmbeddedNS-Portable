
#ifndef __APP_ALG_DSP_GLOBAL_H__
#define __APP_ALG_DSP_GLOBAL_H__

#include    "app_alg_dsp_config.h"

#define     LFO_PRECISION_BIT           (20)
#define     LFO_PRECISION               (1 << LFO_PRECISION_BIT)

/* byte address transfer to 32-bit address */
#define		DSP_VIEW_SRAM_ADDR32(mcuaddr)	((mcuaddr)>>2)
/* byte address transfer to 16-bit address */
#define		DSP_VIEW_SRAM_ADDR16(mcuaddr)	((mcuaddr)>>1)


typedef struct
{
    float32_t   preGain;
	float32_t   x[2];
    float32_t   y[2];
	
    float32_t   threshold;
	float32_t   ratio;
    float32_t   attackstep;
    float32_t   releasestep;
    
    float32_t   curGain;
   
	
    int32_t     updateFlag;
}ALG_DSP_VAR_DRC;

int32_t Dspfreq2D(int32_t fre);


void DspAlgDryWetCalcF(float32_t *dry, float32_t *wet, int32_t *dry_wet);

int32_t DspAlgMyExp(int32_t x);

int32_t DspAlgMyLog(int32_t x);


#endif


