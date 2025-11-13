#ifndef MODULES_AUDIO_PROCESSING_NS_NOISE_SUPPRESSOR_H_
#define MODULES_AUDIO_PROCESSING_NS_NOISE_SUPPRESSOR_H_

#include "AudioBuffer.h"
#include "noise_estimator.h"
#include "ns_common.h"
#include "speech_probability_estimator.h"
#include "suppression_params.h"
#include "wiener_filter.h"
#include <stdbool.h>
#include "fft_cal_transient.h"
#include "SplitFilter/SplitFilter.h"
#define sample_rate_hz 48000
#define num_bands_ (sample_rate_hz / 16000)
#define num_channels_ 1

typedef struct
{
    SpeechProbabilityEstimator speech_probability_estimator;
    WienerFilter wiener_filter;
    NoiseEstimator noise_estimator;

    float prev_analysis_signal_spectrum[kFftSizeBy2Plus1];
    float analyze_analysis_memory[kFftSize - kNsFrameSize];
    float process_analysis_memory[kOverlapSize];
    float process_synthesis_memory[kOverlapSize];
    float process_delay_memory[num_bands_ - 1][kOverlapSize];
} ChannelState;

typedef struct
{
    float real[kFftSize];
    float imag[kFftSize];
    float extended_frame[kFftSize];
} FilterBankState;

typedef struct
{
    SuppressionParams suppression_params_;
    int num_analyzed_frames_;
    bool capture_output_used_;
    float upper_band_gains_heap_[num_channels_];
    float energies_before_filtering_heap_[num_channels_];
    float gain_adjustments_heap_[num_channels_];
    FilterBankState filter_bank_states_heap_;
    ThreeBandFilterBank splitting_filter;
    ChannelState channels_0;
} NoiseSuppressor;
int NumChannelsOnHeap(int num_channels);
void ApplyFilterBankWindow(float* x);
void FormExtendedFrame(float* frame, float* old_data, float* extended_frame);
void OverlapAndAdd(float* extended_frame, float* overlap_memory, float* output_frame);
void DelaySignal(float* frame, float* delay_buffer, float* delayed_frame);
float ComputeEnergyOfExtendedFrame_long(float* in, float len);
float ComputeEnergyOfExtendedFrame_split(float* in1, float* in2, float len1, float len2);
void ComputeMagnitudeSpectrum(float* in, float* signal_spectrum);
void ComputeSnr(float* filter, float* prev_signal_spectrum, float* signal_spectrum,
    float* prev_noise_spectrum, float* noise_spectrum, float* prior_snr, float* post_snr);
float ComputeUpperBandsGain(float minimum_attenuating_gain, float* filter, float* speech_probability,
    float* prev_analysis_signal_spectrum, float* signal_spectrum);
void NoiseSuppressor_init(NoiseSuppressor* var, int suppression_level);
void NoiseSuppressor_AggregateWienerFilters(NoiseSuppressor* var, float* filter);
void NoiseSuppressor_Analyze(NoiseSuppressor* var, FrameBuffer* audio);
void NoiseSuppressor_Process(NoiseSuppressor* var, FrameBuffer* audio);
#endif  // MODULES_AUDIO_PROCESSING_NS_NOISE_SUPPRESSOR_H_











    

