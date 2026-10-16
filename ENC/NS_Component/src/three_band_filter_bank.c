/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "three_band_filter_bank.h"

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
	
	for(k=0; k<kSplitBandSize; k++)
	{
		out[k]=0.0f;
	}
	
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
	
    //Niko 移到外面清零
//	for(band=0; band<kNumBands; band++)
//	{
//		for(i=0; i<kSplitBandSize; i++)
//		{
//			out[band][i]=0.0f;
//		}
//	}
    
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
}


void ThreeBandFilterBank_Synthesis(ThreeBandFilterBank *three_band_filter_bank, float **in, float *out)
{
	int32_t i, upsampling_index, in_shift, index, filter_index, band;
	float filter[kFilterSize];
	float dct_modulation[kDctSize];
	float *state;
	float in_subsampled[kSplitBandSize];
	float out_subsampled[kSplitBandSize];
	
	for(i=0; i<kFullBandSize; i++)
	{
		out[i]=0.0f;
	}
	
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
			
			for(i=0; i<kSplitBandSize; i++)
			{
				in_subsampled[i]=0.0f;
			}
			for(band=0; band<kNumBands; band++)
			{
				for(i=0; i<kSplitBandSize; i++)
				{
					in_subsampled[i]+=dct_modulation[band]*in[band][i];
				}
			}
			
			FilterCore(filter, in_subsampled, in_shift, out_subsampled, state);
			
			for(i=0; i<kSplitBandSize; i++)
			{
				out[upsampling_index+kSubSampling*i]+=(float)kSubSampling*out_subsampled[i];
			}	
		}
	}
}





