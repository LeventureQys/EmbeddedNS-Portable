#ifndef __APP_ALG_DSP_CONFIG_H__
#define __APP_ALG_DSP_CONFIG_H__

#include	"a0.h"

#include    "stdint.h"
#include    "stdbool.h"

#include    "arm_math.h"
	

#ifndef     ALG_SAMPLE_RATE
#define     ALG_SAMPLE_RATE             (48000)
#endif

#ifndef     ALG_DATA_BIT
#define     ALG_DATA_BIT                (24)//(16)
#endif

#ifndef     ALG_DATA_NORM_ONE
#define     ALG_DATA_NORM_ONE           ((float32_t)(1 << (ALG_DATA_BIT-1)))
#endif


#define     CLIP_16BIT(in)              (((in) > 32767) ? 32767 : (((in) < -32767) ? -32767 : (in)))
#define     DISTRICT(a, MINM, MAXM)     (a = ((a > MAXM) ? MAXM : ((a < MINM) ? MINM : a)))
#define     GET_MIN(a, b)               ((a) < (b) ? (a) : (b))
#define     GET_MAX(a, b)               ((a) < (b) ? (b) : (a))


typedef enum
{
    D_FILTER_PEAK = 0,
    D_FILTER_LS,
    D_FILTER_HS,
    D_FILTER_LPF,
    D_FILTER_HPF,
    D_FILTER_LPF_BUTTER1,
    D_FILTER_HPF_BUTTER1,
    D_FILTER_LPF_BUTTER2,
    D_FILTER_HPF_BUTTER2,
	
}ALG_DSP_FILTER_TYPE;

typedef struct
{
	
    float       gain;
    float       fc;
    float       q;
	//uint32_t    type; 	//ALG_FILTER_TYPE
	float		type;
              
}ALG_DSP_PARA_FILTER; //ALG_PARA_FILTER;


#endif
