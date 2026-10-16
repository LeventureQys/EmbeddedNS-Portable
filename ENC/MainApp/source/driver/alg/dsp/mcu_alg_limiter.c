/*
 * Copyright 2018 - 2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "A0.h"
#include "mcu_alg_limiter.h"
//#include "mcu_alg_fast_approx.h"
#include "fast_math.h"
/****************************1***************************************************
* Public macro definitions
 ******************************************************************************/
#if (SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN)
    #if 0
        #include "fsl_debug_console.h"
        #define LIMITER_DEBUG        PRINTF
    #else
        #define LIMITER_DEBUG(...)
    #endif
#else
    #define LIMITER_DEBUG(...)
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
bool limiter_bypass = 1;//false;
float limiter_threshold = -3.0f;    // range: -60.0dB to 0.0dB, step: 0.1dB
float limiter_kneeWidth = 0.0f;     // range: 0.0dB to 60.0dB, step: 0.1dB
float limiter_attackTime = 0.005f;  // range: 0.0001s to 0.2s, step: 0.0001s
float limiter_releaseTime = 0.01f;  // range: 0.005s to 2s, step: 0.0001s
float limiter_cvRelease = 0.0f;
float limiter_cvAttack = 0.0f;
float limiter_attackCoeff = 0.0f;
float limiter_releaseCoeff = 0.0f;
float limiter_slope = 0.0f;
float limiter_halfKneeWidth = 0.0f;

static bool temp_bypass =          0;
static float temp_threshold =       0;
static float temp_attackCoeff =     0;
static float temp_releaseCoeff =    0;

static float limiter_sampleRate = 48000.0f;
uint8_t limiter_init_cmp_flag = 0;
volatile uint32_t limiter_param_change = 0;

#ifdef USE_ITCM_USR
RAMFUNCTION_SECTION_CODE(void mcu_alg_limiter_process(uint8_t *input, uint8_t *output, uint32_t len,uint32_t fs))
#else
__RAM_CODE inline void mcu_alg_limiter_process(uint8_t *input, uint8_t *output, uint32_t len,uint32_t fs)
#endif
{
    uint32_t i = 0;
    float *in  = (float *)input;
    float *out = (float *)output;
	#if 0
    int32_t *in  = (int32_t *)input;
    int32_t *out = (int32_t *)output;
    int32_t temp_int;
    int32_t tempL;
    int32_t tempR;
    #endif
    
    float temp_float;
	float limiter_x = 0.0f;
	float limiter_xdB = 0.0f;
	float limiter_overshoot = 0.0f;
	float limiter_cv = 0.0f;
	float limiter_gain = 0.0f;
	
    updata_limiter_param_irq_callback();
    
    if(limiter_bypass)
    {
        return;
    }
	
    if(limiter_init_cmp_flag == 1)
    {   
    
        //for(i=0;i<len;i+=8)
        for(i=0;i<len;i+=4)//Niko
        {
            temp_float = *in;

//            if(limiter_bypass)
//            {
//				limiter_cvRelease = 0.0f;
//                limiter_cvAttack = 0.0f;
//            }
//            else
            {
				limiter_x = temp_float / 8388608.0f;	
                
                #if USE_FAST_FUNC
//                limiter_xdB = 20.0f * fasterlog10(MAX(ABS(limiter_x), 0.000001f));
                limiter_xdB = 10.0f * fastlog10(MAX(ABS(limiter_x), 0.000001f));//Niko
                #else
                limiter_xdB = 20.0f * log10f(MAX(ABS(limiter_x), 0.000001f));
                #endif

				limiter_overshoot = limiter_xdB - limiter_threshold;

				if(limiter_overshoot>-limiter_halfKneeWidth && limiter_overshoot<limiter_halfKneeWidth)
                {
                    #if USE_FAST_FUNC
                    limiter_cv = 1.0f / (2.0f*limiter_kneeWidth) * (limiter_overshoot+limiter_halfKneeWidth)*(limiter_overshoot+limiter_halfKneeWidth);
                    #else
                    limiter_cv = 1.0f / (2.0f*limiter_kneeWidth) * powf(limiter_overshoot+limiter_halfKneeWidth, 2.0f);
                    #endif
                }
                else if(limiter_overshoot>=limiter_halfKneeWidth)
                {
                    limiter_cv = limiter_overshoot;
                }
                else
                {
                    limiter_cv = 0.0f;
                }
                
                limiter_cv *= -limiter_slope;
                limiter_cvRelease = MAX(limiter_cvRelease+limiter_releaseCoeff*(limiter_cv-limiter_cvRelease), limiter_cv);
                limiter_cvAttack = limiter_cvAttack+limiter_attackCoeff*(limiter_cvRelease-limiter_cvAttack);
                
                limiter_cv = -limiter_cvAttack;
                
                #if USE_FAST_FUNC
                limiter_gain=fastpow10(limiter_cv*0.05f);
                #else
                limiter_gain=powf(10.0f, limiter_cv/20.0f);
                #endif

                #if 0
                temp_int = limiter_x * limiter_gain * 8388608.0f + 0.5f;
                #endif
                temp_float = limiter_x * limiter_gain * 8388608.0f;
            }
			
            #if 0
			if(temp_int > 8388607)
            {
                temp_int = 8388607;
            }

            else if(temp_int < -8388608)
            {
                temp_int = -8388608;
            }

            temp_int = (uint32_t)(temp_int<<8)>>8;
            
            *out     = temp_int;
            *(out+1) = temp_int;
            #endif
            if(temp_float > 8388607.0f)
            {
                temp_float = 8388607.0f;
            }

            else if(temp_float < -8388608.0f)
            {
                temp_float = -8388608.0f;
            }
            *out     = temp_float;
            //*(out+1) = temp_float;
            
            //in+=2;
            //out+=2;
            in+=1;
            out+=1;
        }
    } 
}    


void mcu_alg_limiter_init(float *fs)
{
    if(limiter_init_cmp_flag != 1)
    {
//        #if USE_MORE_THAN_48K
//        if(*fs > 192000)
//        {
//        #else
//        if(*fs > 48000)
//        {
//        #endif
//        
//            return;
//        }
        
        limiter_sampleRate = 48000;//Niko
        limiter_bypass = 0;
        limiter_threshold = -3.0f;
        limiter_attackCoeff = 1.0f - expf(-1.0f / (limiter_attackTime * limiter_sampleRate));
        limiter_releaseCoeff = 1.0f - expf(-1.0f / (limiter_releaseTime * limiter_sampleRate));
        limiter_slope = -1.0f;
        limiter_halfKneeWidth = limiter_kneeWidth / 2.0f;
        
        limiter_init_cmp_flag = 1;
        
//        LIMITER_DEBUG("limiter_sampleRate = %0.7f;\r\n",limiter_sampleRate);        
//        LIMITER_DEBUG("limiter_bypass = %d;\r\n",limiter_bypass);
//        LIMITER_DEBUG("limiter_threshold = %0.7f;\r\n",limiter_threshold);
//        LIMITER_DEBUG("limiter_attackCoeff = %0.7f;\r\n",limiter_attackCoeff);
//        LIMITER_DEBUG("limiter_releaseCoeff = %0.7f;\r\n",limiter_releaseCoeff);
//        LIMITER_DEBUG("limiter_slope = %0.7f;\r\n",limiter_slope);
//        LIMITER_DEBUG("limiter_halfKneeWidth = %0.7f;\r\n",limiter_halfKneeWidth);
//        LIMITER_DEBUG("limiter_init_cmp_flag = %d;\r\n",limiter_init_cmp_flag);
    }
}

void mcu_alg_limiter_const_init(float * fs,PARAM_TBL_HANDLE paramTable)
{
    if(limiter_init_cmp_flag != 1)
    {
        #if USE_MORE_THAN_48K
        if(*fs > 192000)
        {
        #else
        if(*fs > 48000)
        {
        #endif
        
            return;
        }
        
//        limiter_sampleRate = *fs;
//        mcu_alg_enable_limiter(null,null,paramTable.Preset->MicLimiterBypass);
//        set_mcu_alg_limiter_treshold(null,null,paramTable.Preset->MicLimiterTreshold);
//        set_muc_alg_limiter_attack(null,null,paramTable.Preset->MicLimiterAttack);
//        set_muc_alg_limiter_release(null,null,paramTable.Preset->MicLimiterRelease);
//        limiter_slope = -1.000000;
//        limiter_halfKneeWidth = limiter_kneeWidth / 2.0f;
//        
//        limiter_init_cmp_flag = 1;
        
//        LIMITER_DEBUG("limiter_sampleRate = %0.7f;\r\n",limiter_sampleRate);        
//        LIMITER_DEBUG("limiter_bypass = %d;\r\n",limiter_bypass);
//        LIMITER_DEBUG("limiter_threshold = %0.7f;\r\n",limiter_threshold);
//        LIMITER_DEBUG("limiter_attackCoeff = %0.7f;\r\n",limiter_attackCoeff);
//        LIMITER_DEBUG("limiter_releaseCoeff = %0.7f;\r\n",limiter_releaseCoeff);
//        LIMITER_DEBUG("limiter_slope = %0.7f;\r\n",limiter_slope);
//        LIMITER_DEBUG("limiter_halfKneeWidth = %0.7f;\r\n",limiter_halfKneeWidth);
//        LIMITER_DEBUG("limiter_init_cmp_flag = %d;\r\n",limiter_init_cmp_flag);
    }
}

void mcu_alg_enable_limiter(uint8_t effNum,uint8_t paramNum,uint16_t enable)
{
    if(enable > 1)  return;
    temp_bypass = (bool)enable;
    limiter_param_change |= 0x01;
    LIMITER_DEBUG("limiter_bypass = %d;\r\n",temp_bypass);
}

void set_mcu_alg_limiter_treshold(uint8_t effNum,uint8_t paramNum,uint16_t Treshold)
{
    if(Treshold > 900) return;
    float treshold = (float)(Treshold-600) * 0.1f;
    temp_threshold = treshold;
    limiter_param_change |= 0x02;
    LIMITER_DEBUG("limiter_threshold = %0.7f;\r\n",temp_threshold);
}

void set_muc_alg_limiter_attack(uint8_t effNum,uint8_t paramNum,uint16_t Attack)
{
    if(Attack > 2000) return;
    float attack = (float)Attack * 0.0001f;
    temp_attackCoeff = 1.0f - fastexp(-1.0f / (attack * limiter_sampleRate));
    limiter_param_change |= 0x04;
    LIMITER_DEBUG("limiter_attackCoeff = %0.7f;\r\n",temp_attackCoeff);
}

void set_muc_alg_limiter_release(uint8_t effNum,uint8_t paramNum,uint16_t Release)
{
    if(Release > 20000) return;
    float release = (float)Release * 0.0001f;
    temp_releaseCoeff = 1.0f - fastexp(-1.0f / (release * limiter_sampleRate));
    limiter_param_change |= 0x08;
    LIMITER_DEBUG("limiter_releaseCoeff = %0.7f;\r\n",temp_releaseCoeff);
}

__RAM_CODE void updata_limiter_param_irq_callback(void)
{
    #if 0
    if(limiter_param_change != 0)
    {
        limiter_cvRelease = 0.0f;
        limiter_cvAttack = 0.0f;
    }
    #endif
//    if(limiter_param_change & 0x01)
    {
        limiter_bypass = temp_bypass;
    }
//    if(limiter_param_change & 0x02)
    {
        limiter_threshold = temp_threshold;
    }
//    if(limiter_param_change & 0x04)
    {
        limiter_attackCoeff = temp_attackCoeff;
    }
//    if(limiter_param_change & 0x08)
    {
        limiter_releaseCoeff = temp_releaseCoeff;
    }
//    limiter_param_change = 0;
}

void mcu_alg_limiter_disable(void)
{
    limiter_cvRelease = 0.0f;
    limiter_cvAttack = 0.0f;
    limiter_init_cmp_flag = 0;
}
