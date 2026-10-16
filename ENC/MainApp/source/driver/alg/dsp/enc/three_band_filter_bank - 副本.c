/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "three_band_filter_bank.h"
#include "arm_math.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
const float kFilterCoeffs[kNumNonZeroFilters][kFilterSize]=
{
	{-0.00047749f, -0.00496888f, 0.16547118f, 0.00425496f},
	{-0.00173287f, -0.01585778f, 0.14989004f, 0.00994113f},
	{-0.00304815f, -0.02536082f, 0.12154542f, 0.01157993f},
	{-0.00346946f, -0.02587886f, 0.04760441f, 0.00607594f},
	{-0.00154717f, -0.01136076f, 0.01387458f, 0.00186353f},
	{0.00186353f, 0.01387458f, -0.01136076f, -0.00154717f},
	{0.00607594f, 0.04760441f, -0.02587886f, -0.00346946f},
	{0.00983212f, 0.08543175f, -0.02982767f, -0.00383509f},
	{0.00994113f, 0.14989004f, -0.01585778f, -0.00173287f},
	{0.00425496f, 0.16547118f, -0.00496888f, -0.00047749f}
};

const float kDctModulation[kNumNonZeroFilters][kDctSize]=
{
	{2.0f, 2.0f, 2.0f},
	{1.7320508f, 0.0f, -1.7320508f},
	{1.0f, -2.0f, 1.0f},
	{-1.0f, 2.0f, -1.0f},
	{-1.7320508f, 0.0f, 1.7320508f},
	{-2.0f, -2.0f, -2.0f},
	{-1.7320508f, 0.0f, 1.7320508f},
	{-1.0f, 2.0f, -1.0f},
	{1.0f, -2.0f, 1.0f},
	{1.7320508f, 0.0f, -1.7320508f}
};


void FilterCore(float *filter, float *in, int32_t in_shift, float *out, float *state);




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void FilterCore(float *filter, float *in, int32_t in_shift, float *out, float *state)
{
	int32_t k, j, i, shift, loop_limit;
	
	//Niko 屏蔽，后面会填数据，不需要清空
	// for(k=0; k<kSplitBandSize; k++)
	// {
	// 	out[k]=0.0f;
	// }
#if 1
	for(k=0; k<in_shift; k++)
	{
		j=kMemorySize+k-in_shift;
		for(i=0; i<kFilterSize; i++)
		{
			out[k]+=state[j]*filter[i];
			j-=kStride;
		}
	}
	
	shift=0;
	for(k=in_shift; k<kFilterSize*kStride; k++)
	{
		loop_limit=MIN(kFilterSize, 1+(shift>>kStrideLog2));
		j=shift;
		for(i=0; i<loop_limit; i++)
		{
			out[k]+=in[j]*filter[i];
			j-=kStride;
		}
		j=kMemorySize+shift-loop_limit*kStride;
		for(i=loop_limit; i<kFilterSize; i++)
		{
			out[k]+=state[j]*filter[i];
			j-=kStride;
		}
		shift++;
	}
	
	shift=kFilterSize*kStride-in_shift;
	for(k=kFilterSize*kStride; k<kSplitBandSize; k++)
	{
		j=shift;
		for(i=0; i<kFilterSize; i++)
		{
			out[k]+=in[j]*filter[i];
			j-=kStride;
		}
		shift++;
	}
	
	for(i=0; i<kMemorySize; i++)
	{
		state[i]=in[i+kSplitBandSize-kMemorySize];
	}
#endif

#if 0
    // 预计算循环边界和步长
    const int filter_end = kFilterSize * kStride;
    const int memory_end = kMemorySize;

    // 第一部分：处理历史状态
    for(k = 0; k < in_shift; k++) {
        int j = memory_end + k - in_shift;
        float acc = 0.0f;
        #pragma unroll
        for(i = 0; i < kFilterSize; i++) {
            acc += state[j] * filter[i];
            j -= kStride;
        }
        out[k] = acc;
    }

    // 第二部分：混合处理新输入和历史状态
    for(k = in_shift; k < filter_end; k++) {
        int loop_limit = MIN(kFilterSize, 1 + (shift >> kStrideLog2));
        float acc = 0.0f;
        
        // 处理新输入
        int j = shift;
        #pragma unroll 4
        for(i = 0; i < loop_limit; i++) {
            acc += in[j] * filter[i];
            j -= kStride;
        }
        
        // 处理历史状态
        j = memory_end + shift - loop_limit * kStride;
        #pragma unroll 4
        for(i = loop_limit; i < kFilterSize; i++) {
            acc += state[j] * filter[i];
            j -= kStride;
        }
        
        out[k] = acc;
        shift++;
    }

    // 第三部分：仅处理新输入
    shift = filter_end - in_shift;
    for(k = filter_end; k < kSplitBandSize; k++) {
        int j = shift;
        float acc = 0.0f;
        #pragma unroll
        for(i = 0; i < kFilterSize; i++) {
            acc += in[j] * filter[i];
            j -= kStride;
        }
        out[k] = acc;
        shift++;
    }

    // 更新状态缓冲区
    #pragma unroll 8
    for(i = 0; i < memory_end; i++) {
        state[i] = in[i + kSplitBandSize - memory_end];
    }
#endif

#if 0
    float acc;
    const int filter_end = kFilterSize * kStride;
    const int memory_end = kMemorySize;
    
	// 第一部分：处理历史状态
    for(k = 0; k < in_shift; k++) {
        j = memory_end + k - in_shift;
        arm_dot_prod_f32(&state[j], filter, kFilterSize, &acc);
        out[k] = acc;
    }

    // 第二部分：混合处理新输入和历史状态
    for(k = in_shift; k < filter_end; k++) {
        int loop_limit = MIN(kFilterSize, 1 + (shift >> kStrideLog2));
        
        // 使用ARM的向量点积函数
        arm_dot_prod_f32(&in[shift], filter, loop_limit, &acc);
        
        if (loop_limit < kFilterSize) {
            float acc2;
            j = memory_end + shift - loop_limit * kStride;
            arm_dot_prod_f32(&state[j], &filter[loop_limit], kFilterSize - loop_limit, &acc2);
            acc += acc2;
        }
        
        out[k] = acc;
        shift++;
    }

    // 第三部分：仅处理新输入
    shift = filter_end - in_shift;
    for(k = filter_end; k < kSplitBandSize; k++) {
        arm_dot_prod_f32(&in[shift], filter, kFilterSize, &acc);
        out[k] = acc;
        shift++;
    }

    // 使用ARM的向量拷贝函数更新状态
    arm_copy_f32(&in[kSplitBandSize - memory_end], state, memory_end);
#endif
}






void ThreeBandFilterBank_Init(ThreeBandFilterBank *three_band_filter_bank)
{
	int32_t i, j;
	for(i=0; i<kNumNonZeroFilters; i++)
	{
		for(j=0; j<kMemorySize; j++)
		{
			three_band_filter_bank->state_analysis[i][j]=0.0f;
			three_band_filter_bank->state_synthesis[i][j]=0.0f;
		}
	}
}


void ThreeBandFilterBank_Analysis(ThreeBandFilterBank *three_band_filter_bank, float *in, float **out)
{
	int32_t i, band, downsampling_index, in_shift, index, filter_index;
	float in_subsampled[kSplitBandSize];
	float filter[kFilterSize];
	float dct_modulation[kDctSize];
	float *state;
	float out_subsampled[kSplitBandSize];
	
    //Niko 屏蔽，后面会填数据，不需要清空
//	for(band=0; band<kNumBands; band++)
//	{
//		for(i=0; i<kSplitBandSize; i++)
//		{
//			out[band][i]=0.0f;
//		}
//	}
#if 0
	for(downsampling_index=0; downsampling_index<kSubSampling; downsampling_index++)
	{
		for(i=0; i<kSplitBandSize; i++)
		{
			in_subsampled[i]=in[kSubSampling-1-downsampling_index+kSubSampling*i];
		}
		
		for(in_shift=0; in_shift<kStride; in_shift++)
		{
			index=downsampling_index+in_shift*kSubSampling;
			if(index==kZeroFilterIndex1 || index==kZeroFilterIndex2)
			{
				continue;
			}
			filter_index=index<kZeroFilterIndex1 ? index : (index<kZeroFilterIndex2 ? index-1 : index-2);
			
			for(i=0; i<kFilterSize; i++)
			{
				filter[i]=kFilterCoeffs[filter_index][i];
			}
			for(i=0; i<kDctSize; i++)
			{
				dct_modulation[i]=kDctModulation[filter_index][i];
			}
			state=three_band_filter_bank->state_analysis[filter_index];
			
			FilterCore(filter, in_subsampled, in_shift, out_subsampled, state);
			
			for(band=0; band<kNumBands; band++)
			{
				for(i=0; i<kSplitBandSize; i++)
				{
					out[band][i]+=dct_modulation[band]*out_subsampled[i];
				}
			}
		}	
	}
#endif

#if 1
    // 预计算filter_index映射表
    static int filter_index_map[kStride * kSubSampling];
    static int initialized = 0;

    if (!initialized) {
        for (int idx = 0; idx < kStride * kSubSampling; idx++) {
            if (idx == kZeroFilterIndex1 || idx == kZeroFilterIndex2) {
                filter_index_map[idx] = -1;
            } else if (idx < kZeroFilterIndex1) {
                filter_index_map[idx] = idx;
            } else if (idx < kZeroFilterIndex2) {
                filter_index_map[idx] = idx - 1;
            } else {
                filter_index_map[idx] = idx - 2;
            }
        }
        initialized = 1;
    }

    // 主循环优化
    for (downsampling_index = 0; downsampling_index < kSubSampling; downsampling_index++) {
        // 预计算输入采样
        int base_idx = kSubSampling - 1 - downsampling_index;
        for (i = 0; i < kSplitBandSize; i++) {
            in_subsampled[i] = in[base_idx + kSubSampling * i];
        }

        // 内层循环展开
        for (in_shift = 0; in_shift < kStride; in_shift++) {
            index = downsampling_index + in_shift * kSubSampling;
            filter_index = filter_index_map[index];
            
            if (filter_index == -1) continue;

            // 预计算指针
            for(i=0; i<kFilterSize; i++)
			{
				filter[i]=kFilterCoeffs[filter_index][i];
			}
            const float* dct_ptr = kDctModulation[filter_index];
            state = three_band_filter_bank->state_analysis[filter_index];

            FilterCore(filter, in_subsampled, in_shift, out_subsampled, state);

            // 循环展开和向量化
            for (band = 0; band < kNumBands; band++) {
                float modulation = dct_ptr[band];
                for (i = 0; i < kSplitBandSize; i++) {
                    out[band][i] += modulation * out_subsampled[i];
                }
            }
        }
    }
#endif
}

#if  1
void ThreeBandFilterBank_Synthesis(ThreeBandFilterBank *three_band_filter_bank, float **in, float *out)
{
	int32_t i, upsampling_index, in_shift, index, filter_index, band;
	float filter[kFilterSize];
	float dct_modulation[kDctSize];
	float *state;
	float in_subsampled[kSplitBandSize];
	float out_subsampled[kSplitBandSize];
	
	//Niko 屏蔽，后面会填数据，不需要清空
	// for(i=0; i<kFullBandSize; i++)
	// {
	// 	out[i]=0.0f;
	// }

	for(upsampling_index=0; upsampling_index<kSubSampling; upsampling_index++)
	{
		for(in_shift=0; in_shift<kStride; in_shift++)
		{
			index=upsampling_index+in_shift*kSubSampling;
			if(index==kZeroFilterIndex1 || index==kZeroFilterIndex2)
			{
				continue;
			}
			filter_index=index<kZeroFilterIndex1 ? index : (index<kZeroFilterIndex2 ? index-1 : index-2);
			
			for(i=0; i<kFilterSize; i++)
			{
				filter[i]=kFilterCoeffs[filter_index][i];
			}
			for(i=0; i<kDctSize; i++)
			{
				dct_modulation[i]=kDctModulation[filter_index][i];
			}
			state=three_band_filter_bank->state_synthesis[filter_index];
			
			//Niko 屏蔽，后面会填数据，不需要清空
			// for(i=0; i<kSplitBandSize; i++)
			// {
			// 	in_subsampled[i]=0.0f;
			// }
			for(band=0; band<kNumBands; band++)
			{
				for(i=0; i<kSplitBandSize; i++)
				{
					in_subsampled[i]+=dct_modulation[band]*in[band][i];
				}
			}
			
			FilterCore(filter, in_subsampled, in_shift, out_subsampled, state);
			
			// #pragma unroll 8
			for(i=0; i<kSplitBandSize; i++)
			{
				out[upsampling_index+kSubSampling*i]+=(float)kSubSampling*out_subsampled[i];
			}	
		}
	}

}
#endif

#if 0
void ThreeBandFilterBank_Synthesis(ThreeBandFilterBank *three_band_filter_bank, float **in, float *out)
{
    int32_t i, upsampling_index, in_shift, index, filter_index, band;
    float filter[kFilterSize];
    float dct_modulation[kDctSize];
    float *state;
    float in_subsampled[kSplitBandSize];
    float out_subsampled[kSplitBandSize];
    
    // 预计算filter索引
    int filter_indices[kSubSampling * kStride];
    int valid_count = 0;
    int valid_indices[kSubSampling * kStride];
    
    for(upsampling_index = 0; upsampling_index < kSubSampling; upsampling_index++)
    {
        for(in_shift = 0; in_shift < kStride; in_shift++)
        {
            index = upsampling_index + in_shift * kSubSampling;
            if(index == kZeroFilterIndex1 || index == kZeroFilterIndex2)
            {
                filter_indices[upsampling_index * kStride + in_shift] = -1;
                continue;
            }
            
            filter_index = index < kZeroFilterIndex1 ? index : 
                          (index < kZeroFilterIndex2 ? index - 1 : index - 2);
            filter_indices[upsampling_index * kStride + in_shift] = filter_index;
            valid_indices[valid_count++] = upsampling_index * kStride + in_shift;
        }
    }
    
    // 只处理有效的filter索引
    for(int idx = 0; idx < valid_count; idx++)
    {
        int combined_index = valid_indices[idx];
        upsampling_index = combined_index / kStride;
        in_shift = combined_index % kStride;
        filter_index = filter_indices[combined_index];
        
        // 使用memcpy优化数组拷贝
        memcpy(filter, kFilterCoeffs[filter_index], kFilterSize * sizeof(float));
        memcpy(dct_modulation, kDctModulation[filter_index], kDctSize * sizeof(float));
        state = three_band_filter_bank->state_synthesis[filter_index];
        
        // 清零输入子采样数组
        memset(in_subsampled, 0, kSplitBandSize * sizeof(float));
        
        // 使用指针运算优化内层循环
        for(band = 0; band < kNumBands; band++)
        {
            float mod = dct_modulation[band];
            float *in_band = in[band];
            float *in_sub_ptr = in_subsampled;
            
            // 循环展开
            for(i = 0; i < kSplitBandSize; i += 4)
            {
                in_sub_ptr[i] += mod * in_band[i];
                in_sub_ptr[i+1] += mod * in_band[i+1];
                in_sub_ptr[i+2] += mod * in_band[i+2];
                in_sub_ptr[i+3] += mod * in_band[i+3];
            }
        }
        
        FilterCore(filter, in_subsampled, in_shift, out_subsampled, state);
        
        // 使用指针运算优化输出累加
        float *out_ptr = out + upsampling_index;
        float *out_sub_ptr = out_subsampled;
        
        // 循环展开
        for(i = 0; i < kSplitBandSize; i += 4)
        {
            out_ptr[kSubSampling*i] += kSubSampling * out_sub_ptr[i];
            out_ptr[kSubSampling*(i+1)] += kSubSampling * out_sub_ptr[i+1];
            out_ptr[kSubSampling*(i+2)] += kSubSampling * out_sub_ptr[i+2];
            out_ptr[kSubSampling*(i+3)] += kSubSampling * out_sub_ptr[i+3];
        }
    }
}
#endif




