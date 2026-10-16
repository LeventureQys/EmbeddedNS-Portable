/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "noise_estimator.h"
#include "fast_math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
const float log_table[124]=
{
	1.6094379f, 1.7917595f, 1.9459101f, 2.0794415f, 2.1972246f, 2.3025851f, 2.3978953f, 2.4849067f, 2.5649493f, 2.6390574f,
	2.7080503f, 2.7725887f, 2.8332133f, 2.8903718f, 2.9444389f, 2.9957323f, 3.0445225f, 3.0910425f, 3.1354942f, 3.1780539f,
	3.2188759f, 3.2580965f, 3.2958369f, 3.3322046f, 3.3672957f, 3.4011974f, 3.4339871f, 3.4657359f, 3.4965076f, 3.5263605f,
	3.5553482f, 3.5835190f, 3.6109178f, 3.6375861f, 3.6635616f, 3.6888795f, 3.7135720f, 3.7376697f, 3.7612002f, 3.7841897f,
	3.8066626f, 3.8286414f, 3.8501475f, 3.8712010f, 3.8918202f, 3.9120231f, 3.9318256f, 3.9512436f, 3.9702919f, 3.9889841f,
	4.0073333f, 4.0253515f, 4.0430512f, 4.0604429f, 4.0775375f, 4.0943446f, 4.1108737f, 4.1271343f, 4.1431346f, 4.1588831f,
	4.1743875f, 4.1896548f, 4.2046928f, 4.2195077f, 4.2341065f, 4.2484951f, 4.2626801f, 4.2766662f, 4.2904596f, 4.3040652f,
	4.3174882f, 4.3307333f, 4.3438053f, 4.3567090f, 4.3694477f, 4.3820267f, 4.3944492f, 4.4067192f, 4.4188404f, 4.4308167f,
	4.4426513f, 4.4543471f, 4.4659081f, 4.4773369f, 4.4886365f, 4.4998097f, 4.5108595f, 4.5217886f, 4.5325994f, 4.5432949f,
	4.5538769f, 4.5643482f, 4.5747108f, 4.5849676f, 4.5951200f, 4.6051702f, 4.6151204f, 4.6249728f, 4.6347289f, 4.6443911f,
	4.6539602f, 4.6634393f, 4.6728287f, 4.6821313f, 4.6913481f, 4.7004805f, 4.7095304f, 4.7184987f, 4.7273879f, 4.7361984f,
	4.7449322f, 4.7535901f, 4.7621741f, 4.7706847f, 4.7791233f, 4.7874918f, 4.7957907f, 4.8040209f, 4.8121843f, 4.8202815f,
	4.8283138f, 4.8362818f, 4.8441873f, 4.8520303f
};




/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void NoiseEstimator_Init(NoiseEstimator *noise_estimator)
{
	int32_t i;
	
	QuantileNoiseEstimator_Init(&noise_estimator->quantile_noise_estimator);
	
	noise_estimator->white_noise_level=0.0f;
	noise_estimator->pink_noise_numerator=0.0f;
	noise_estimator->pink_noise_exp=0.0f;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		noise_estimator->prev_noise_spectrum[i]=0.0f;
		noise_estimator->conservative_noise_spectrum[i]=0.0f;
		noise_estimator->parametric_noise_spectrum[i]=0.0f;
		noise_estimator->noise_spectrum[i]=0.0f;
	}
}

void NoiseEstimator_PrepareAnalysis(NoiseEstimator *noise_estimator)
{
	int32_t i;
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		noise_estimator->prev_noise_spectrum[i]=noise_estimator->noise_spectrum[i];
	}
}


//#pragma push
//#pragma O0
void NoiseEstimator_PreUpdate(NoiseEstimator *noise_estimator, SuppressionParams *suppression_params, int32_t num_analyzed_frames, float *signal_spectrum, float signal_spectral_sum)
{
	int32_t kStartBand, i, use_band;
	float sum_log_i, sum_log_i_square, sum_log_magn, sum_log_i_log_magn, log_i, log_signal, denom, num, pink_noise_adjustment, one_by_num_analyzed_frames_plus_1, parametric_exp, parametric_num, parametric_denom, tmp;
	
	QuantileNoiseEstimator_Estimate(&noise_estimator->quantile_noise_estimator, signal_spectrum, noise_estimator->noise_spectrum);
	
	if(num_analyzed_frames<kShortStartupPhaseBlocks)
	{
		kStartBand=5;
		sum_log_i=0.0f;
		sum_log_i_square=0.0f;
		sum_log_magn=0.0f;
		sum_log_i_log_magn=0.0f;
		
		for(i=kStartBand; i<kFftSizeBy2Plus1; i++)
		{
			log_i=log_table[i-5];
			sum_log_i+=log_i;
			sum_log_i_square+=log_i*log_i;
			log_signal=fastlog(signal_spectrum[i]);
			sum_log_magn+=log_signal;
			sum_log_i_log_magn+=log_i*log_signal;
		}
		
		noise_estimator->white_noise_level+=signal_spectral_sum*kOneByFftSizeBy2Plus1*suppression_params->over_subtraction_factor;
		
		denom=sum_log_i_square*(float)(kFftSizeBy2Plus1-kStartBand)-sum_log_i*sum_log_i;
		num=sum_log_i_square*sum_log_magn-sum_log_i*sum_log_i_log_magn;
		pink_noise_adjustment=num/denom;
		
		pink_noise_adjustment=MAX(pink_noise_adjustment, 0.0f);
		noise_estimator->pink_noise_numerator+=pink_noise_adjustment;
		num=sum_log_i*sum_log_magn-(float)(kFftSizeBy2Plus1-kStartBand)*sum_log_i_log_magn;
		pink_noise_adjustment=num/denom;
		
		pink_noise_adjustment=MAX(MIN(pink_noise_adjustment, 1.0f), 0.0f);
		noise_estimator->pink_noise_exp+=pink_noise_adjustment;
		
		one_by_num_analyzed_frames_plus_1=1.0f/(float)(num_analyzed_frames+1);
		parametric_exp=0.0f;
		parametric_num=0.0f;
		if(noise_estimator->pink_noise_exp>0.0f)
		{
			parametric_num=fastexp(noise_estimator->pink_noise_numerator*one_by_num_analyzed_frames_plus_1);
			parametric_num*=(float)(num_analyzed_frames+1);
			parametric_exp=noise_estimator->pink_noise_exp*one_by_num_analyzed_frames_plus_1;
		}
		
		for(i=0; i<kFftSizeBy2Plus1; i++)
		{
			if(noise_estimator->pink_noise_exp==0.0f)
			{
				noise_estimator->parametric_noise_spectrum[i]=noise_estimator->white_noise_level;
			}
			else
			{
				use_band=i<kStartBand ? kStartBand : i;
				parametric_denom=fastpow((float)use_band, parametric_exp);
				noise_estimator->parametric_noise_spectrum[i]=parametric_num/parametric_denom;
			}
		}
		
		for(i=0; i<kFftSizeBy2Plus1; i++)
		{
			noise_estimator->noise_spectrum[i]*=(float)num_analyzed_frames;
			tmp=noise_estimator->parametric_noise_spectrum[i]*(float)(kShortStartupPhaseBlocks-num_analyzed_frames);
			noise_estimator->noise_spectrum[i]+=tmp*one_by_num_analyzed_frames_plus_1;
			noise_estimator->noise_spectrum[i]*=kOneByShortStartupPhaseBlocks;
		}
	}
}
//#pragma pop


//#pragma push
//#pragma O0
void NoiseEstimator_PostUpdate(NoiseEstimator *noise_estimator, float *speech_probability, float *signal_spectrum)
{
	int32_t i;
	float kNoiseUpdate, gamma, prob_speech, prob_non_speech, noise_update_tmp, gamma_old, kProbRange;
	
	kNoiseUpdate=0.9f;
	gamma=kNoiseUpdate;
	
	for(i=0; i<kFftSizeBy2Plus1; i++)
	{
		prob_speech=speech_probability[i];
		prob_non_speech=1.0f-prob_speech;
		
		noise_update_tmp=gamma*noise_estimator->prev_noise_spectrum[i]+(1.0f-gamma)*(prob_non_speech*signal_spectrum[i]+prob_speech*noise_estimator->prev_noise_spectrum[i]);
		
		gamma_old=gamma;
		
		kProbRange=0.2f;
		gamma=prob_speech>kProbRange ? 0.99f : kNoiseUpdate;
		
		if(prob_speech<kProbRange)
		{
			noise_estimator->conservative_noise_spectrum[i]+=0.05f*(signal_spectrum[i]-noise_estimator->conservative_noise_spectrum[i]);
		}
		
		if(gamma==gamma_old)
		{
			noise_estimator->noise_spectrum[i]=noise_update_tmp;
		}
		else
		{
			noise_estimator->noise_spectrum[i]=gamma*noise_estimator->prev_noise_spectrum[i]+(1.0f-gamma)*(prob_non_speech*signal_spectrum[i]+prob_speech*noise_estimator->prev_noise_spectrum[i]);
			noise_estimator->noise_spectrum[i]=MIN(noise_estimator->noise_spectrum[i], noise_update_tmp);
		}	
	}
}
//#pragma pop


