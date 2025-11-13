#ifndef PRIOR_SIGNAL_MODEL_HEADER
#define PRIOR_SIGNAL_MODEL_HEADER
#include "ns_common.h"
//#include "user_task.h"
typedef struct
{
  float lrt;
  float flatness_threshold;
  float template_diff_threshold;
  float lrt_weighting;
  float flatness_weighting;
  float difference_weighting;
} PriorSignalModel;

void PriorSignalModel_init(PriorSignalModel* var, float lrt_initial_value);
#endif