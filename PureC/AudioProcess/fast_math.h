#ifndef MODULES_AUDIO_PROCESSING_NS_FAST_MATH_H_
#define MODULES_AUDIO_PROCESSING_NS_FAST_MATH_H_
#include <string.h>
// Sqrt approximation.
float SqrtFastApproximation(float f);

// Log base conversion log(x) = log2(x)/log2(e).
float LogApproximation(float x);
void LogApproximation_vect(float* x, float* y, size_t len);

// 2^x approximation.
float Pow2Approximation(float p);

// x^p approximation.
float PowApproximation(float x, float p);

// e^x approximation.
float ExpApproximation(float x);
void ExpApproximation_vect(float* x, float* y, size_t len);
void ExpApproximationSignFlip(float* x, float* y, size_t len);

#endif  // MODULES_AUDIO_PROCESSING_NS_FAST_MATH_H_
