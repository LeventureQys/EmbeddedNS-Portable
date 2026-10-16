#ifndef __ALG_FILTER_H__
#define __ALG_FILTER_H__

#include <stdint.h>
#include <math.h>

// PI is defined in "arm_math.h"
#ifndef PI
    #define PI               3.14159265358979f
#endif

typedef enum
{                       //Niko
    FILTER_PEAK = 0,    //峰值滤波器（Peak Filter）
    FILTER_LS,          //低通滤波器（Low Shelf Filter）
    FILTER_HS,          //高通滤波器（High Shelf Filter）
    FILTER_LPF,         //低通滤波器（Low Pass Filter）
    FILTER_HPF,         //高通滤波器（High Pass Filter）
    FILTER_LPF_BUTTER1, //巴特沃斯低通滤波器1（Butterworth Low Pass Filter）
    FILTER_HPF_BUTTER1, //巴特沃斯高通滤波器1（Butterworth High Pass Filter）
    FILTER_LPF_BUTTER2, //巴特沃斯低通滤波器2（Butterworth Low Pass Filter）
    FILTER_HPF_BUTTER2, //巴特沃斯高通滤波器2（Butterworth High Pass Filter）
    FILTER_LS1,         //低通滤波器1（Low Shelf Filter）
    FILTER_HS1,         //高通滤波器1（High Shelf Filter）
	FILTER_MODERNREV1,  //
	FILTER_MODERNREV2,  //
}ALG_FILTER_TYPE;

typedef struct
{
    float       gain;
    float       fc;
    float       q;
    uint32_t    type;       /* @ref ALG_FILTER_TYPE */
}ALG_PARA_FILTER;


typedef struct
{
    uint32_t    numStages;  /* number of 2-order stages in the filter */
    float       *pState;    /* points to the array of state coefficients, [numStages][x1,x2,y1,y2] */
    const float *pCoeffs;   /* points to the array of coefficients, [numStages][a0,a1,a2,-b1,-b2] */
}ALG_FILTER_F32;

/**
 * @brief combine the coeffs of two 1st-order filter to one 2nd-order filter in floating-point
 * @param[in]  coeff1   points to the coeffs of 1st-order filter 1, {a0, a1, 0, -b1, 0}
 * @param[in]  coeff2   points to the coeffs of 1st-order filter 2, {A0, A1, 0, -B1, 0}
 * @param[out] coeff    points to the coeffs of new 2nd-order filter, {a0*A0, a0*A1+a1*A0, a1*A1, -(b1+B1), -(b1*B1)}
 * @attention  the format of new coeff is still a0/a1/a2/-b1/-b2   
 */
void AlgFilterCombine(const float *coeff1, const float *coeff2, float *coeff);

/**
 * @brief calculate the coeffs according to the filter parameter
 * @param[in]  pFilterPara  points to an instance of filter parameter structure
 * @param[out] pCoeff       points to the coeffs of the result
 * @param[in]  sampleRate   sample rate of current filter
 * @attention  the format of the coeff is a0/a1/a2/-b1/-b2   
 */
void AlgFilterDesign(ALG_PARA_FILTER *pFilterPara, float *pCoeff, uint32_t sampleRate);

/**
 * @brief floating-point biquad cascade filter (direct form I) process
 * @param[in]  S          points to an instance of the floating-point biquad cascade structure
 * @param[in]  pSrc       points to the block of input data
 * @param[out] pDst       points to the block of output data
 * @param[in]  blockSize  number of samples to process
 * @note section name
 *  section("alg_filter_ram_code_section") 
 */
void AlgFilterProcess(const ALG_FILTER_F32 *S, float *pSrc, float *pDst, uint32_t blockSize);


#endif
