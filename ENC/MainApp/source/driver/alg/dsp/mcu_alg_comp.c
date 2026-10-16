/*
 * Copyright 2018 - 2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "A0.h"
#include "mcu_alg_comp.h"
//#include "mcu_alg_fast_approx.h"
#include "fast_math.h"
/****************************1***************************************************
* Public macro definitions
 ******************************************************************************/
#if (SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN)
    #if 0
        #include "fsl_debug_console.h"
        #define COMP_DEBUG        PRINTF
    #else
        #define COMP_DEBUG(...)
    #endif
#else
    #define COMP_DEBUG(...)
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

bool comp_bypass = 1;//false;
static float comp_sampleRate = 48000.0f; 

float comp_threshold = -10.0f;      // range: -60.0dB to 0.0dB, step: 0.1dB
float comp_ratio = 5.0f;            //range: 1.0 to 100.0, step: 0.1
float comp_kneeWidth = 5.0f;        // range: 0.0dB to 60.0dB, step: 0.1dB
float comp_attackTime = 0.005f;     // range: 0.0001s to 0.2s, step: 0.0001s
float comp_releaseTime = 0.01f;     // range: 0.005s to 2s, step: 0.0001s
float comp_makeupGain = 0.0f;       // range: 0.0dB to 60.0dB, step: 0.1dB

float comp_cvRelease = 0.0f;
float comp_cvAttack = 0.0f;
float comp_attackCoeff = 0.0f;
float comp_releaseCoeff = 0.0f;
float comp_slope = 0.0f;
float comp_halfKneeWidth = 0.0f;


static bool temp_bypass            = false;
static float temp_threshold        = -10.0f;      
static float temp_slope            = 0;
static float temp_halfKneeWidth    = 0;
static float temp_attackCoeff      = 0;
static float temp_releaseCoeff     = 0;
static float temp_makeupGain       = 0;

uint8_t comp_init_cmp_flag   = 0;
volatile uint32_t comp_param_change   = 0;
#ifdef USE_ITCM_USR
RAMFUNCTION_SECTION_CODE(void mcu_alg_comp_process(uint8_t *input, uint8_t *output, uint32_t len,uint32_t fs))
#else
#pragma push
#pragma O0
__RAM_CODE inline void mcu_alg_comp_process(uint8_t *input, uint8_t *output, uint32_t len,uint32_t fs)
#endif
{
    uint32_t i = 0;
    float *in  = (float *)input;
    float *out = (float *)output;
    
	float temp_float;
	float comp_x = 0.0f;
	float comp_xdB = 0.0f;
	float comp_overshoot = 0.0f;
	float comp_cv = 0.0f;
	float comp_gain = 0.0f;
    
    updata_comp_param_irq_callback();
    
    if((comp_bypass == 1) || (comp_slope == 9))
    {//直通、压缩比为1
        return;
    }
	
    if(comp_init_cmp_flag == 1)
    {    
    
//        for(i=0;i<len;i+=8)
        for(i=0;i<len;i+=4)//Niko
        {
//          #if 0
//          tempL = (int32_t)((*(uint32_t*)in)<<8)>>8;
//          tempR = (int32_t)((*(uint32_t*)(in+1))<<8)>>8;
//			temp_float = (float)(tempL+tempR) * 0.5f;
//          #endif
            temp_float = *in;

//            if(comp_bypass)
//            {
//				comp_cvRelease = 0.0f;
//                comp_cvAttack = 0.0f;
//            }
//            else
            {
				comp_x = temp_float / 8388608.0f;
                
                #if USE_FAST_FUNC
                //comp_xdB = 20.0f * fasterlog10(MAX(ABS(comp_x), 0.000001f));
                comp_xdB = 10.0f * fastlog10(MAX(ABS(comp_x), 0.000001f));//Niko
                #else
                comp_xdB = 20.0f * log10f(MAX(ABS(comp_x), 0.000001f));
                #endif
                
				comp_overshoot = comp_xdB - comp_threshold;

				if(comp_overshoot>-comp_halfKneeWidth && comp_overshoot<comp_halfKneeWidth)
                {
                    #if USE_FAST_FUNC
                    comp_cv = 1.0f / (2.0f*comp_kneeWidth) * (comp_overshoot+comp_halfKneeWidth)*(comp_overshoot+comp_halfKneeWidth);
                    #else
                    comp_cv = 1.0f / (2.0f*comp_kneeWidth) * powf(comp_overshoot+comp_halfKneeWidth, 2.0f);
                    #endif
                }
                else if(comp_overshoot>=comp_halfKneeWidth)
                {
                    comp_cv = comp_overshoot;
                }
                else
                {
                    comp_cv = 0.0f;
                }
                
                comp_cv *= -comp_slope;
                
                comp_cvRelease = MAX(comp_cvRelease+comp_releaseCoeff*(comp_cv-comp_cvRelease), comp_cv);
                comp_cvAttack = comp_cvAttack+comp_attackCoeff*(comp_cvRelease-comp_cvAttack);
                
                comp_cv = -comp_cvAttack;
                comp_cv += comp_makeupGain;
                
                #if USE_FAST_FUNC
                comp_gain=fastpow10(comp_cv*0.05f);
                #else
                comp_gain=powf(10.0f, comp_cv/20.0f);
                #endif

                #if 0
                temp_int = comp_x * comp_gain * 8388608.0f + 0.5f;
                #endif
//                temp_float = comp_x * comp_gain * 8388608.0f;
                temp_float = comp_x * comp_gain * 8388608.0f;

            }
            
            if(temp_float > 8388607.0f)
            {
                temp_float = 8388607.0f;
            }

            else if(temp_float < -8388608.0f)
            {
                temp_float = -8388608.0f;
            }


            *out     = temp_float;
//            *(out+1) = temp_float;//Niko
            
//            in+=2;//Niko
//            out+=2;
            
            in+=1;//Niko
            out+=1;
        }
    } 
}    
#pragma pop

void mcu_alg_comp_init(float * fs)
{
    if(comp_init_cmp_flag != 1)
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
        
        comp_sampleRate = 48000;//*fs; //Niko 固定48000
        comp_bypass = false;
        comp_threshold = -10.0f;
        comp_slope = (1.0f / comp_ratio) - 1.0f;
        comp_halfKneeWidth = comp_kneeWidth / 2.0f;
        comp_attackCoeff = 1.0f - expf(-1.0f / (comp_attackTime * comp_sampleRate));
        comp_releaseCoeff = 1.0f - expf(-1.0f / (comp_releaseTime * comp_sampleRate));
        comp_makeupGain = 0.0f;
        
        comp_init_cmp_flag = 1;
        
//        COMP_DEBUG("comp_sampleRate = %.7f;\r\n",   comp_sampleRate);
//        COMP_DEBUG("comp_bypass = %d;\r\n",         comp_bypass);
//        COMP_DEBUG("comp_threshold = %.7f;\r\n",    comp_threshold);
//        COMP_DEBUG("comp_slope = %.7f;\r\n",        comp_slope);
//        COMP_DEBUG("comp_halfKneeWidth = %.7f;\r\n",comp_halfKneeWidth);
//        COMP_DEBUG("comp_attackCoeff = %.7f;\r\n",  comp_attackCoeff);
//        COMP_DEBUG("comp_releaseCoeff = %.7f;\r\n", comp_releaseCoeff);
//        COMP_DEBUG("comp_makeupGain = %.7f;\r\n",   comp_makeupGain);
    }
}

void mcu_alg_comp_const_init(float * fs,PARAM_TBL_HANDLE paramTable)
{
    if(comp_init_cmp_flag != 1)
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
        
        //Niko 初始化 待改
//        comp_sampleRate = *fs;
//        mcu_alg_enable_comp(null,null,paramTable.Preset->MicCompBypass);
//        set_mcu_alg_comp_treshold(null,null,paramTable.Preset->MicCompTreshold);
//        set_mcu_alg_comp_ratio(null,null,paramTable.Preset->MicCompRatio);
//        set_mcu_alg_comp_kneew(null,null,paramTable.Preset->MicCompKneeWidth);
//        set_muc_alg_comp_attack(null,null,paramTable.Preset->MicCompAttack);
//        set_muc_alg_comp_release(null,null,paramTable.Preset->MicCompRelease);
//        set_muc_alg_comp_mkg(null,null,paramTable.Preset->MicCompMakeUpGain);
        
        comp_init_cmp_flag = 1;
        
        COMP_DEBUG("comp_sampleRate = %.7f;\r\n",   comp_sampleRate);
        COMP_DEBUG("comp_bypass = %d;\r\n",         comp_bypass);
        COMP_DEBUG("comp_threshold = %.7f;\r\n",    comp_threshold);
        COMP_DEBUG("comp_slope = %.7f;\r\n",        comp_slope);
        COMP_DEBUG("comp_halfKneeWidth = %.7f;\r\n",comp_halfKneeWidth);
        COMP_DEBUG("comp_attackCoeff = %.7f;\r\n",  comp_attackCoeff);
        COMP_DEBUG("comp_releaseCoeff = %.7f;\r\n", comp_releaseCoeff);
        COMP_DEBUG("comp_makeupGain = %.7f;\r\n",   comp_makeupGain);
    }
}

void mcu_alg_enable_comp(uint8_t effNum,uint8_t paramNum,uint16_t enable)
{
    if(enable > 1) return;
    temp_bypass = (bool)enable;
    comp_param_change |= 0x01;
//    COMP_DEBUG("comp_bypass = %d;\r\n",temp_bypass);
}

void set_mcu_alg_comp_treshold(uint8_t effNum,uint8_t paramNum,uint16_t Treshold)
{
    if(Treshold > 900) return;
    float treshold = (float)(Treshold-600) * 0.1f;
//    COMP_DEBUG("treshold = %.7f;\r\n",treshold);
    temp_threshold = treshold;
    comp_param_change |= 0x02;
    //COMP_DEBUG("comp_threshold = %.7f;\r\n",temp_threshold);
}

void set_mcu_alg_comp_ratio(uint8_t effNum,uint8_t paramNum,uint16_t Ratio)
{
    if(Ratio > 1000) return;
    float ratio = (float)Ratio * 0.1f;
    COMP_DEBUG("ratio = %.7f;\r\n",ratio);
    temp_slope = (1.0f / ratio) - 1.0f;
    comp_param_change |= 0x04;
    //COMP_DEBUG("comp_slope = %.7f;\r\n",temp_slope);
}

void set_mcu_alg_comp_kneew(uint8_t effNum,uint8_t paramNum,uint16_t KneeWidth)
{
    if(KneeWidth > 600) return;
    float knee_withd = (float)KneeWidth * 0.1f;
    COMP_DEBUG("knee_withd = %.7f;\r\n",knee_withd);
    temp_halfKneeWidth = knee_withd * 0.5f;
    comp_param_change |= 0x08;
    //COMP_DEBUG("comp_halfKneeWidth = %.7f;\r\n",temp_halfKneeWidth);
}

void set_muc_alg_comp_attack(uint8_t effNum,uint8_t paramNum,uint16_t Attack)
{
    if(Attack > 2000) return;
    float attack = (float)Attack * 0.0001f;
    COMP_DEBUG("attack = %.7f;\r\n",attack);
    //temp_attackCoeff = 1.0f - expf(-1.0f / (attack * comp_sampleRate));
    temp_attackCoeff = 1.0f - fastexp(-1.0f / (attack * comp_sampleRate));
    comp_param_change |= 0x10;
    //COMP_DEBUG("comp_attackCoeff = %.7f;\r\n",temp_attackCoeff);
}

void set_muc_alg_comp_release(uint8_t effNum,uint8_t paramNum,uint16_t Release)
{
    if(Release > 20000) return;
    float release = (float)Release * 0.0001f;
    COMP_DEBUG("release = %.7f;\r\n",release);
    //temp_releaseCoeff = 1.0f - expf(-1.0f / (release * comp_sampleRate));
    temp_releaseCoeff = 1.0f - fastexp(-1.0f / (release * comp_sampleRate));
    comp_param_change |= 0x20;
    //COMP_DEBUG("comp_releaseCoeff = %.7f;\r\n",temp_releaseCoeff);
} 

void set_muc_alg_comp_mkg(uint8_t effNum,uint8_t paramNum,uint16_t MakeUpGain)
{
    if(MakeUpGain > 600) return;
    float make_up_gain = (float)MakeUpGain * 0.1f;
    COMP_DEBUG("make_up_gain = %.7f;\r\n",make_up_gain);
    temp_makeupGain = make_up_gain;
    comp_param_change |= 0x40;
    //COMP_DEBUG("comp_makeupGain = %.7f;\r\n",temp_makeupGain);
}

__RAM_CODE void updata_comp_param_irq_callback(void)
{
    #if 0
    if(comp_param_change != 0)
    {
        comp_cvRelease = 0.0f;
        comp_cvAttack = 0.0f;
    }
    #endif
    if(comp_param_change & 0x01)
    {
       comp_bypass = temp_bypass;
    }
    if(comp_param_change & 0x02)
    {
       comp_threshold = temp_threshold;
    }
    if(comp_param_change & 0x04)
    {
       comp_slope = temp_slope;
    }
    if(comp_param_change & 0x08)
    {
       comp_halfKneeWidth = temp_halfKneeWidth;
    }
    if(comp_param_change & 0x10)
    {
       comp_attackCoeff = temp_attackCoeff;
    }
    if(comp_param_change & 0x20)
    {
       comp_releaseCoeff = temp_releaseCoeff;
    }
    if(comp_param_change & 0x40)
    {
       comp_makeupGain = temp_makeupGain;
    }
    comp_param_change = 0;
}
void mcu_alg_comp_disable(void)
{
    comp_cvRelease = 0.0f;
    comp_cvAttack = 0.0f;
    comp_init_cmp_flag = 0;
}
