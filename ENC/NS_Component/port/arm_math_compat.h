#ifndef ARM_MATH_COMPAT_H
#define ARM_MATH_COMPAT_H

#include <stdint.h>

typedef int arm_status;
#define ARM_MATH_SUCCESS 0

/* 与原版 CMSIS 结构体的字段名保持一致的占位定义。
   兼容层内部不使用这些字段, 但源码会声明同类型的局部变量。 */
typedef struct {
    void*   Sint;            /* 占位 */
    uint16_t fftLenRFFT;
    float*  pTwiddleRFFT;
} arm_rfft_fast_instance_f32;

arm_status arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32* S, uint16_t fftLen);

/* ifftFlag: 0 = 正向, 1 = 逆向。
   输入输出格式与缩放约定见计划文档第 2.4 节 —— 必须严格遵守:
   正向无缩放(标准 DFT); 逆向为标准 IDFT(含 1/256)。
   输出压缩格式: pOut[0]=DC, pOut[1]=Nyquist, pOut[2k]=Re, pOut[2k+1]=Im */
void arm_rfft_fast_f32(arm_rfft_fast_instance_f32* S, float* p, float* pOut,
                       uint8_t ifftFlag);

#endif
