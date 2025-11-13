
#ifndef COMMON_AUDIO_SPARSE_FIR_FILTER_H_
#define COMMON_AUDIO_SPARSE_FIR_FILTER_H_
#include <string.h>
#include <../core/typedefs.h>

//typedef unsigned long size_t;
typedef struct{
	size_t sparsity_;
	size_t offset_;
	float* nonzero_coeffs_;
	float* state_;
	size_t num_nonzero_coeffs;
	size_t sparsity_size;
}SparseFIRFilter;

void InitSFF(SparseFIRFilter* sff,
	const float* nonzero_coeffs,
	size_t num_nonzero_coeffs,
	size_t sparsity,
	size_t offset);

void ReleaseSFF(SparseFIRFilter* sff);

void FilterSFF(SparseFIRFilter* sff, const float* in, size_t length, float* out);
#endif
