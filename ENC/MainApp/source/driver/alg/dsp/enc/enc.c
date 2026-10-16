/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "enc.h"
#include "arm_math.h"

//Niko GPIO测试运行周期，
#include "a0_hal_gpio.h"
#include "Gpio.h"

//GPIO0.28 已改作 RGB 灯的数据线（见 user/Ui/Ui.Led.c 的 4颗WS2812驱动），不能再当算法耗时打点脚用，
//否则会在音频中断里把 LED 数据线打成乱码、灯乱闪。原来的打点代码还留着，需要恢复时把这里改成 1
//（但要先给 LED 换一个脚，两者不能共存）。
#define ENC_GPIO_PROFILE_EN		(0)

/*******************************************************************************
 * Definitions
 ******************************************************************************/
 
// Niko
arm_rfft_fast_instance_f32 g_InstRfftF32;
uint8 enc_level = 3;//降噪等级 0-3

__RAM3 Enc g_EncParam ;
__RAM3 float enc_in[480*2] __ALIGNED(4);
__RAM3 float enc_out[480*2] __ALIGNED(4);

float g_split_data[kNumBands][kSplitBandSize] __ALIGNED(4);//Enc_Process的临时缓存
float *g_row_pointers[kNumBands];
float **split_data = NULL;


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Enc_Init(Enc *enc, int32_t suppression_level)
{
	ThreeBandFilterBank_Init(&enc->three_band_filter_bank);
	NoiseSuppressor_Init(&enc->noise_suppressor, suppression_level);
    
    if(arm_rfft_fast_init_f32(&g_InstRfftF32, 256)!=ARM_MATH_SUCCESS)
	{
        __breakpoint(0);//初始FFT失败
	}
    
    //初始化split_data双重指针
    for (int i = 0; i < kNumBands; i++) {
        g_row_pointers[i] = g_split_data[i];
    }
    split_data = g_row_pointers;
}

void Enc_SetOnOff(bool on)
{
    EncOnOff = on;
}

//#pragma push
//#pragma O0
__RAM_CODE void Enc_Process(Enc *enc, float *in, float *out)
{
#if (ENC_GPIO_PROFILE_EN)
HAL_GPIO_BIT_SET(GPIO0,GPIO_PIN_28);
#endif

//	float **split_data;//Niko 改为全局缓存。也可以改为临时数组，但需要占用stack区
    memset(g_split_data, 0, kFullBandSize * sizeof(float));//ThreeBandFilterBank_Analysis移出来清空，同时清空
	ThreeBandFilterBank_Analysis(&enc->three_band_filter_bank, in, split_data);
	NoiseSuppressor_Analyze(&enc->noise_suppressor, split_data);
	NoiseSuppressor_Process(&enc->noise_suppressor, split_data);
	ThreeBandFilterBank_Synthesis(&enc->three_band_filter_bank, split_data, out);

#if (ENC_GPIO_PROFILE_EN)
HAL_GPIO_BIT_CLR(GPIO0,GPIO_PIN_28);
#endif
}
//#pragma pop

