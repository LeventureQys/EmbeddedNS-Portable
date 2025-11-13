#include <stdint.h>
#include <math.h>

#include "splitting_filter.h"
#include "common.h"

void FloatS16ToS16(const float* src, size_t size, int16_t* dest);
void S16ToFloatS16(const int16_t* src, size_t size, float* dest);


void SplittingFilter_init(const SplittingFilter* SPTF, size_t num_bands) {
    SPTF->num_bands_ = num_bands;
    if (SPTF->num_bands_ == 2) {
        memset(SPTF->two_bands_states_.analysis_state1, 0, kStateSize * sizeof(int));
        memset(SPTF->two_bands_states_.analysis_state2, 0, kStateSize * sizeof(int));
        memset(SPTF->two_bands_states_.synthesis_state1, 0, kStateSize * sizeof(int));
        memset(SPTF->two_bands_states_.synthesis_state2, 0, kStateSize * sizeof(int));
    } else if (SPTF->num_bands_ == 3) {
        InitTBFB(SPTF->three_band_filter_banks_, kSamplesPerBand);
    }
}

void SplittingFilter_Analysis_short(const SplittingFilter* SPTF, const short* in, short* const* out) {
    if (SPTF->num_bands_ == 2) {
        Spl_AnalysisQMF(in, out[0], out[1], SPTF->two_bands_states_.analysis_state1, SPTF->two_bands_states_.analysis_state2);
    } else if (SPTF->num_bands_ == 3) {
        float fullband_floatS16[SPTF->num_bands_ * kSamplesPerBand];
        float band_FloatS16[SPTF->num_bands_][kSamplesPerBand];

        S16ToFloatS16(in, SPTF->num_bands_ * kSamplesPerBand, fullband_floatS16);
        AnalysisTBFB(SPTF->three_band_filter_banks_, fullband_floatS16, kSamplesPerBand, band_FloatS16);
        FloatS16ToS16(band_FloatS16[0], kSamplesPerBand, out[0]);
        FloatS16ToS16(band_FloatS16[1], kSamplesPerBand, out[1]);
        FloatS16ToS16(band_FloatS16[2], kSamplesPerBand, out[2]);
    }
}

void SplittingFilter_Synthesis_short(const SplittingFilter* SPTF, const short* const* in, short* out) {
    if (SPTF->num_bands_ == 2) {
        Spl_AnalysisQMF(full_band16, in[0], in[1], SPTF->two_bands_states_.analysis_state1, SPTF->two_bands_states_.analysis_state2);
    } else if (SPTF->num_bands_ == 3) {
        float fullband_floatS16[SPTF->num_bands_ * kSamplesPerBand];
        float band_FloatS16[SPTF->num_bands_][kSamplesPerBand];

        S16ToFloatS16(in[0], kSamplesPerBand, band_FloatS16[0]);
        S16ToFloatS16(in[1], kSamplesPerBand, band_FloatS16[1]);
        S16ToFloatS16(in[2], kSamplesPerBand, band_FloatS16[2]);
        SynthesisTBFB(SPTF->three_band_filter_banks_, band_FloatS16, kSamplesPerBand, fullband_floatS16);
        FloatS16ToS16(fullband_floatS16, SPTF->num_bands_ * kSamplesPerBand, out);
    }
}


void SplittingFilter_Analysis_float(const SplittingFilter* SPTF, const float* in, float* const* out) {
    if (SPTF->num_bands_ == 2) {
        int16_t full_band16[kSamplesPerBand * SPTF->num_bands_];
        int16_t bands16[SPTF->num_bands_][kSamplesPerBand];
        FloatS16ToS16(in, kSamplesPerBand * SPTF->num_bands_, full_band16);
        Spl_AnalysisQMF(full_band16, bands16[0], bands16[1], SPTF->two_bands_states_.analysis_state1, SPTF->two_bands_states_.analysis_state2);
        S16ToFloatS16(bands16[0], kSamplesPerBand, out[0]);
        S16ToFloatS16(bands16[1], kSamplesPerBand, out[1]);

    } else if (SPTF->num_bands_ == 3) {
        AnalysisTBFB(SPTF->three_band_filter_banks_, in, kSamplesPerBand, out);
    }
}
 
void SplittingFilter_Synthesis_float(const SplittingFilter* SPTF, const float* const* in, float* out) {
    if (SPTF->num_bands_ == 2) {
        int16_t full_band16[kSamplesPerBand * SPTF->num_bands_];
        int16_t bands16[SPTF->num_bands_][kSamplesPerBand];
        FloatS16ToS16(in[0], kSamplesPerBand, bands16[0]);
        FloatS16ToS16(in[1], kSamplesPerBand, bands16[1]);
        Spl_SynthesisQMF(bands16[0], bands16[1], full_band16, SPTF->two_bands_states_.synthesis_state1, SPTF->two_bands_states_.synthesis_state2);
        S16ToFloatS16(full_band16, kSamplesPerBand * SPTF->num_bands_, out);
    } else if (SPTF->num_bands_ == 3) {
        SynthesisTBFB(SPTF->three_band_filter_banks_, in, kSamplesPerBand, out);
    }  
}

void FloatS16ToS16(const float* src, int size, int16_t* dest) {
  for (int i = 0; i < size; ++i)
    dest[i] = FloatS16ToS16(src[i]);
}

void S16ToFloatS16(const int16_t* src, size_t size, float* dest) {
  for (size_t i = 0; i < size; ++i)
    dest[i] = src[i];
}

