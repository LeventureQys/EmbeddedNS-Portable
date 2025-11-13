#ifndef SIGNAL_MODEL_HEADER
#define SIGNAL_MODEL_HEADER
#include <math.h>
#include "ns_common.h"
//#include "user_task.h"


typedef struct
{
  float lrt;
  float spectral_diff;
  float spectral_flatness;
  // Log LRT factor with time-smoothing.
  float avg_log_lrt[kFftSizeBy2Plus1];
} SignalModel;

void SignalModel_init(SignalModel* var);

#endif