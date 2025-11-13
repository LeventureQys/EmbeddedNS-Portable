#ifndef HISTOGRAMS_HEADER
#define HISTOGRAMS_HEADER

#include "ns_common.h"
#include "signal_model.h"
#include <string.h>

#define kHistogramSize 1000

typedef struct
{
    int lrt_[kHistogramSize];
    int spectral_flatness_[kHistogramSize];
    int spectral_diff_[kHistogramSize];

} Histograms;

  void Histograms_Clear(Histograms* var);
  void Histograms_Update(SignalModel* features_, Histograms* var);

#endif