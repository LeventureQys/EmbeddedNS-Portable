/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "noise_suppressor.h"
#include "fast_math.h"
#include "arm_math_compat.h"
#include "ns_patch.h"
extern arm_rfft_fast_instance_f32 g_InstRfftF32;
/*******************************************************************************
 * Definitions
 ******************************************************************************/
const float kBlocks160w256FirstHalf[96]=
{
	0.00000000f, 0.01636173f, 0.03271908f, 0.04906767f, 0.06540313f,
	0.08172107f, 0.09801714f, 0.11428696f, 0.13052619f, 0.14673047f,
	0.16289547f, 0.17901686f, 0.19509032f, 0.21111155f, 0.22707626f,
	0.24298018f, 0.25881905f, 0.27458862f, 0.29028468f, 0.30590302f,
	0.32143947f, 0.33688985f, 0.35225005f, 0.36751594f, 0.38268343f,
	0.39774847f, 0.41270703f, 0.42755509f, 0.44228869f, 0.45690388f,
	0.47139674f, 0.48576339f, 0.50000000f, 0.51410274f, 0.52806785f,
	0.54189158f, 0.55557023f, 0.56910015f, 0.58247770f, 0.59569930f,
	0.60876143f, 0.62166057f, 0.63439328f, 0.64695615f, 0.65934582f,
	0.67155895f, 0.68359230f, 0.69544264f, 0.70710678f, 0.71858162f,
	0.72986407f, 0.74095113f, 0.75183981f, 0.76252720f, 0.77301045f,
	0.78328675f, 0.79335334f, 0.80320753f, 0.81284668f, 0.82226822f,
	0.83146961f, 0.84044840f, 0.84920218f, 0.85772861f, 0.86602540f,
	0.87409034f, 0.88192126f, 0.88951608f, 0.89687274f, 0.90398929f,
	0.91086382f, 0.91749450f, 0.92387953f, 0.93001722f, 0.93590593f,
	0.94154407f, 0.94693013f, 0.95206268f, 0.95694034f, 0.96156180f,
	0.96592583f, 0.97003125f, 0.97387698f, 0.97746197f, 0.98078528f,
	0.98384601f, 0.98664333f, 0.98917651f, 0.99144486f, 0.99344778f,
	0.99518473f, 0.99665524f, 0.99785892f, 0.99879546f, 0.99946459f,
	0.99986614f
};


float ComputeEnergyOfExtendedFrame_two(float *frame, int32_t frame_len, float *old_data, int32_t old_data_len);
float ComputeEnergyOfExtendedFrame(float *frame, int32_t frame_len);
void FormExtendedFrame(float *frame, float *old_data, float *extended_frame);
void ApplyFilterBankWindow(float *x);
void ComputeMagnitudeSpectrum(float *y, float *signal_spectrum);
void ComputeSnr(float *filter, float *prev_signal_spectrum, float *signal_spectrum, float *prev_noise_spectrum, float *noise_spectrum, float *post_snr, float *prior_snr);
float ComputeUpperBandsGain(float minimum_attenuating_gain, float *filter, float *speech_probability, float *prev_analysis_signal_spectrum, float *signal_spectrum);
void OverlapAndAdd(float *extended_frame, float *overlap_memory, float *output_frame);
void DelaySignal(float *frame, float *delay_buffer, float *delayed_frame);




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
float ComputeEnergyOfExtendedFrame_two(float *frame, int32_t frame_len, float *old_data, int32_t old_data_len)
{
	int32_t i;
	float energy=0.0f;
	
	for(i=0; i<frame_len; i++)
	{
		energy+=frame[i]*frame[i];
	}
	
	for(i=0; i<old_data_len; i++)
	{
		energy+=old_data[i]*old_data[i];
	}
	
	return energy;
}

float ComputeEnergyOfExtendedFrame(float *frame, int32_t frame_len)
{
	int32_t i;
	float energy=0.0f;
	
	for(i=0; i<frame_len; i++)
	{
		energy+=frame[i]*frame[i];
	}
	
	return energy;
}

void FormExtendedFrame(float *frame, float *old_data, float *extended_frame)
{
	int32_t i;
	
	for(i=0; i<kFftSize-kNsFrameSize; i++)
	{
		extended_frame[i]=old_data[i];
	}
	
	for(i=0; i<kNsFrameSize; i++)
	{
		extended_frame[i+kFftSize-kNsFrameSize]=frame[i];
	}
	
	for(i=0; i<kFftSize-kNsFrameSize; i++)
	{
		old_data[i]=extended_frame[i+kNsFrameSize];
	}
}

void ApplyFilterBankWindow(float *x)
{
	int32_t i, k;
	
	for(i=0; i<96; i++)
	{
		x[i]*=kBlocks160w256FirstHalf[i];
	}
	
	k=95;
	for(i=161; i<kFftSize; i++)
	{
		x[i]*=kBlocks160w256FirstHalf[k];
		k--;
	}
}

void ComputeMagnitudeSpectrum(float *y, float *signal_spectrum)
{
	int32_t i;
	
	signal_spectrum[0]=ABS(y[0])+1.0f;
	signal_spectrum[kFftSizeBy2Plus1-1]=ABS(y[kFftSize])+1.0f;
	
	for(i=1; i<kFftSizeBy2Plus1-1; i++)
	{
		signal_spectrum[i]=NS_SQRT(y[2*i]*y[2*i]+y[2*i+1]*y[2*i+1])+1.0f;
	}
}

void ComputeSnr(float *filter, float *prev_signal_spectrum, float *signal_spectrum, float *prev_noise_spectrum, float *noise_spectrum, float *post_snr, float *prior_snr)
{
	int32_t i;
	float prev_estimate;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		prev_estimate=prev_signal_spectrum[i]/(prev_noise_spectrum[i]+0.0001f)*filter[i];
		
		if(signal_spectrum[i]>noise_spectrum[i])
		{
			post_snr[i]=signal_spectrum[i]/(noise_spectrum[i]+0.0001f)-1.0f;
		}
		else
		{
			post_snr[i]=0.0f;
		}
		
		prior_snr[i]=0.98f*prev_estimate+(1.0f-0.98f)*post_snr[i];
	}
}

//#pragma push
//#pragma O0
float ComputeUpperBandsGain(float minimum_attenuating_gain, float *filter, float *speech_probability, float *prev_analysis_signal_spectrum, float *signal_spectrum)
{
	int32_t i, kNumAvgBins;
	float kOneByNumAvgBins, avg_prob_speech, avg_filter_gain, sum_analysis_spectrum, sum_processing_spectrum, gain;
	
	kNumAvgBins=32;
	kOneByNumAvgBins=1.0f/(float)kNumAvgBins;
	
	avg_prob_speech=0.0f;
	avg_filter_gain=0.0f;
	for(i=kFftSizeBy2Plus1-kNumAvgBins-1; i<kFftSizeBy2Plus1-1; i++)
	{
		avg_prob_speech+=speech_probability[i];
		avg_filter_gain+=filter[i];
	}
	avg_prob_speech*=kOneByNumAvgBins;
	avg_filter_gain*=kOneByNumAvgBins;
	
	sum_analysis_spectrum=0.0f;
	sum_processing_spectrum=0.0f;
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		sum_analysis_spectrum+=prev_analysis_signal_spectrum[i];
		sum_processing_spectrum+=signal_spectrum[i];
	}
	
	avg_prob_speech*=sum_processing_spectrum/(sum_analysis_spectrum+0.0001f);
	
	gain=0.5f*(1.0f+tanhf(2.0f*avg_prob_speech-1.0f));
	
	if(avg_prob_speech>=0.5f)
	{
		gain=0.25f*gain+0.75f*avg_filter_gain;
	}
	else
	{
		gain=0.5f*gain+0.5f*avg_filter_gain;
	}
	
	return MIN(MAX(gain, minimum_attenuating_gain), 1.0f);	
}
//#pragma pop

void OverlapAndAdd(float *extended_frame, float *overlap_memory, float *output_frame)
{
	int32_t i;
	
	for(i=0; i<kOverlapSize; i++)
	{
		output_frame[i]=overlap_memory[i]+extended_frame[i];
	}
	
	for(i=0; i<kNsFrameSize-kOverlapSize; i++)
	{
		output_frame[i+kOverlapSize]=extended_frame[i+kOverlapSize];
	}
	
	for(i=0; i<kOverlapSize; i++)
	{
		overlap_memory[i]=extended_frame[i+kNsFrameSize];
	}
}

void DelaySignal(float *frame, float *delay_buffer, float *delayed_frame)
{
	int32_t i, kSampleFromFrame;
	
	kSampleFromFrame=kNsFrameSize-(kFftSize-kNsFrameSize);
	
	for(i=0; i<kFftSize-kNsFrameSize; i++)
	{
		delayed_frame[i]=delay_buffer[i];
	}
	
	for(i=0; i<kSampleFromFrame; i++)
	{
		delayed_frame[i+kFftSize-kNsFrameSize]=frame[i];
	}
	
	for(i=0; i<kFftSize-kNsFrameSize; i++)
	{
		delay_buffer[i]=frame[i+kSampleFromFrame];
	}
}






void NoiseSuppressor_Init(NoiseSuppressor *noise_suppressor, int32_t suppression_level)
{
	int32_t i, band;
	
	noise_suppressor->num_analyzed_frames=-1;
	
	for(i=0; i<kFftSize; i++)
	{
		noise_suppressor->extended_frame[i]=0.0f;
	}
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		noise_suppressor->prev_analysis_signal_spectrum[i]=1.0f;
	}
	
	for(i=0; i<kOverlapSize; i++)
	{
		noise_suppressor->analyze_analysis_memory[i]=0.0f;
		noise_suppressor->process_analysis_memory[i]=0.0f;
		noise_suppressor->process_synthesis_memory[i]=0.0f;
	}
	
	for(band=0; band<kNumBands-1; band++)
	{
		for(i=0; i<kOverlapSize; i++)
		{
			noise_suppressor->process_delay_memory[band][i]=0.0f;
		}
	}
	
	SuppressionParams_Init(&noise_suppressor->suppression_params, suppression_level);
	NoiseEstimator_Init(&noise_suppressor->noise_estimator);
	SpeechProbabilityEstimator_Init(&noise_suppressor->speech_probability_estimator);
	WienerFilter_Init(&noise_suppressor->wiener_filter);
}

//#pragma push
//#pragma O0
void NoiseSuppressor_Analyze(NoiseSuppressor *noise_suppressor, float **split_data)
{
	bool zero_frame=true;
	int32_t i;
	float energy, signal_energy, signal_spectral_sum;
	float extended_frame[kFftSize];
	float Y[2*kFftSizeBy2Plus1];
	float signal_spectrum[kFftSizeBy2Plus1];
	float post_snr[kFftSizeBy2Plus1];
	float prior_snr[kFftSizeBy2Plus1];
	
	arm_status status=ARM_MATH_SUCCESS;
	arm_rfft_fast_instance_f32 InstRfftF32;
	
	NoiseEstimator_PrepareAnalysis(&noise_suppressor->noise_estimator);
	
	energy=ComputeEnergyOfExtendedFrame_two(split_data[0], kNsFrameSize, noise_suppressor->analyze_analysis_memory, kOverlapSize);
	if(energy>0.0f)
	{
		zero_frame=false;
	}
	if(zero_frame)
	{
		noise_suppressor->zero_frame_skips++;
		return;
	}
	
	noise_suppressor->num_analyzed_frames++;
	if(noise_suppressor->num_analyzed_frames<0)
	{
		noise_suppressor->num_analyzed_frames=0;
	}

	FormExtendedFrame(split_data[0], noise_suppressor->analyze_analysis_memory, extended_frame);
	ApplyFilterBankWindow(extended_frame);
	
//	status=arm_rfft_fast_init_256_f32(&InstRfftF32);//Niko 替换为arm_rfft_fast_init_f32 ，InstRfftF32改为全局变量

//	if(status==ARM_MATH_SUCCESS)
	{
        arm_rfft_fast_f32(&g_InstRfftF32, extended_frame, Y, 0);
		Y[kFftSize]=Y[1];
		Y[1]=0.0f;
		NS_FIX_D1_AFTER_FFT(Y);
	}	
	ComputeMagnitudeSpectrum(Y, signal_spectrum);
	
	signal_energy=0.0f;
	signal_spectral_sum=0.0f;
    
    #pragma unroll
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		signal_energy+=Y[2*i]*Y[2*i]+Y[2*i+1]*Y[2*i+1];
		signal_spectral_sum+=signal_spectrum[i];
	}
	signal_energy*=kOneByFftSizeBy2Plus1;
	noise_suppressor->last_signal_energy=signal_energy;
	noise_suppressor->last_signal_spectral_sum=signal_spectral_sum;
	
	NoiseEstimator_PreUpdate(&noise_suppressor->noise_estimator, &noise_suppressor->suppression_params, noise_suppressor->num_analyzed_frames, signal_spectrum, signal_spectral_sum);
	
	ComputeSnr(noise_suppressor->wiener_filter.filter, noise_suppressor->prev_analysis_signal_spectrum, signal_spectrum, noise_suppressor->noise_estimator.prev_noise_spectrum, noise_suppressor->noise_estimator.noise_spectrum, post_snr, prior_snr);
	
	SpeechProbabilityEstimator_Update(&noise_suppressor->speech_probability_estimator, noise_suppressor->num_analyzed_frames, prior_snr, post_snr, noise_suppressor->noise_estimator.conservative_noise_spectrum, signal_spectrum, signal_spectral_sum, signal_energy);
	
	NoiseEstimator_PostUpdate(&noise_suppressor->noise_estimator, noise_suppressor->speech_probability_estimator.speech_probability, signal_spectrum);
	
    #pragma unroll
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		noise_suppressor->prev_analysis_signal_spectrum[i]=signal_spectrum[i];
	}
}
//#pragma pop


//#pragma push
//#pragma O0
void NoiseSuppressor_Process(NoiseSuppressor *noise_suppressor, float **split_data)
{
	int32_t i, band;
	float energy_before_filtering, upper_band_gain, energy_after_filtering, gain_adjustment;
	float Y[2*kFftSizeBy2Plus1];
	float signal_spectrum[kFftSizeBy2Plus1];
	float X[2*kFftSizeBy2Plus1];
	float delayed_frame[kNsFrameSize];
	
	arm_status status=ARM_MATH_SUCCESS;
	arm_rfft_fast_instance_f32 InstRfftF32;
	
	FormExtendedFrame(split_data[0], noise_suppressor->process_analysis_memory, noise_suppressor->extended_frame);
	ApplyFilterBankWindow(noise_suppressor->extended_frame);
	
	energy_before_filtering=ComputeEnergyOfExtendedFrame(noise_suppressor->extended_frame, kFftSize);
	
//	status=arm_rfft_fast_init_256_f32(&InstRfftF32);//Niko 替换为arm_rfft_fast_init_f32 ，InstRfftF32改为全局变量

//	if(status==ARM_MATH_SUCCESS)
	{
        arm_rfft_fast_f32(&g_InstRfftF32, noise_suppressor->extended_frame, Y, 0);
		Y[kFftSize]=Y[1];
		Y[1]=0.0f;
		NS_FIX_D1_AFTER_FFT(Y);
	}	
	ComputeMagnitudeSpectrum(Y, signal_spectrum);
	
	WienerFilter_Update(&noise_suppressor->wiener_filter, &noise_suppressor->suppression_params, noise_suppressor->num_analyzed_frames, noise_suppressor->noise_estimator.noise_spectrum, noise_suppressor->noise_estimator.prev_noise_spectrum, noise_suppressor->noise_estimator.parametric_noise_spectrum, signal_spectrum);
	
	upper_band_gain=ComputeUpperBandsGain(noise_suppressor->suppression_params.minimum_attenuating_gain, noise_suppressor->wiener_filter.filter, noise_suppressor->speech_probability_estimator.speech_probability, noise_suppressor->prev_analysis_signal_spectrum, signal_spectrum);
	
    #pragma unroll
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		X[2*i]=Y[2*i]*noise_suppressor->wiener_filter.filter[i];
		X[2*i+1]=Y[2*i+1]*noise_suppressor->wiener_filter.filter[i];
	}
	X[1]=X[kFftSize];
//	status=arm_rfft_fast_init_256_f32(&InstRfftF32);//Niko 替换为arm_rfft_fast_init_f32 ，InstRfftF32改为全局变量

//	if(status==ARM_MATH_SUCCESS)
	{
        arm_rfft_fast_f32(&g_InstRfftF32, X, noise_suppressor->extended_frame, 1);
	}
	
	energy_after_filtering=ComputeEnergyOfExtendedFrame(noise_suppressor->extended_frame, kFftSize);
	
	ApplyFilterBankWindow(noise_suppressor->extended_frame);
	
	gain_adjustment=WienerFilter_ComputeOverallScalingFactor(&noise_suppressor->suppression_params, noise_suppressor->num_analyzed_frames, noise_suppressor->speech_probability_estimator.prior_speech_prob, energy_before_filtering, energy_after_filtering);
	noise_suppressor->last_gain_adjustment=gain_adjustment;
	
    #pragma unroll
	for(i=0; i<kFftSize; i++)
	{
		noise_suppressor->extended_frame[i]*=gain_adjustment;
	}
	
	OverlapAndAdd(noise_suppressor->extended_frame, noise_suppressor->process_synthesis_memory, split_data[0]);
	
	for(band=1; band<kNumBands; band++)
	{
		DelaySignal(split_data[band], noise_suppressor->process_delay_memory[band-1], delayed_frame);
		for(i=0; i<kNsFrameSize; i++)
		{
			split_data[band][i]=upper_band_gain*delayed_frame[i];
		}
	}
	
	noise_suppressor->clamp_hits=0;
	for(band=0; band<kNumBands; band++)
	{
		for(i=0; i<kNsFrameSize; i++)
		{
//			split_data[band][i]=MIN(MAX(split_data[band][i], -32768.0f), 32767.0f);//16bit
          if(split_data[band][i]<NS_CLAMP_MIN || split_data[band][i]>NS_CLAMP_MAX)
          {
              noise_suppressor->clamp_hits++;
          }
          split_data[band][i]=MIN(MAX(split_data[band][i], NS_CLAMP_MIN), NS_CLAMP_MAX);//24bit
//          split_data[band][i]=MIN(MAX(split_data[band][i], -2147483648.0f), 2147483647.0f);//32bit
		}
	}
}

//#pragma pop



