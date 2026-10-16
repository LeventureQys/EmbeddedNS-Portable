#ifndef __ALG_COMMON_H__
#define __ALG_COMMON_H__

#include "jat_alg.h"

#define IDX_CUR                             (0)
#define IDX_NEW                             (1)
#define IDX_SET                             (2)

#define ALG_SAMPLE_NORM_ONE                 ((float)(1U << (ALG_SAMPLE_BIT - 1)))

/* byte address transfer to 32-bit address, range is 128kbyte in mcu */
#define DSP_VIEW_SRAM_ADDR32(mcuaddr)	    (((mcuaddr) & 0x1FFFF) >> 2)
/* byte address transfer to 16-bit address, range is 128kbyte in mcu */
#define DSP_VIEW_SRAM_ADDR16(mcuaddr)	    (((mcuaddr) & 0x1FFFF) >> 1)

#define GET_MIN(a, b)                       ((a) < (b) ? (a) : (b))
#define GET_MAX(a, b)                       ((a) < (b) ? (b) : (a))

// A0实测，math库函数各函数效率，按照MCU运行96MHz的PCLK，使用DualTimer计数（PCLK周期）
// sinf (83),               arm_sin_f32 (124)
// cosf (85),               arm_cos_f32 (121)
// tanf (107),              arm_sin_f32/arm_cos_f32 (240)
// sqrtf(38),               AlgSqrtf (30), 精度比原math稍微差一点点，周期数有一点点优势
// logf (147),              AlgMyLog (237~305), 如果是浮点数，需要调用2次函数计算，优势是能实现定点数运算
// expf (203~浮动),           AlgMyExp (125~浮动), 查表精度差一点，范围小一点，运算速度快一倍
// powf 2底(430~浮动),         AlgMyPow2 (230~浮动), 针对2为底的幂运算，AlgMyPow2精度稍微差一点，但是速度快
// powf(x,y),               AlgMyExp((int32_t)(y * 1024)) * (AlgMyLog(x * 8192) - AlgMyLog(8192)) / 1024) / 1024.0f
// (1300~浮动)               (355)
// 在powf底数是2的N次方, N = -n, -n+1, ..., -1, 0, 1, ..., n-1, n, 运算时，powf和定点的exp+log差不多
// 在powf底数是其他数据的时候，powf时间差不多是定点的exp+log的4倍不到一点
// 总结：
// sinf/cosf/tanf, 直接用math库
// 浮点数logf，直接用math库
// expf，不追求速度时，推荐直接math库
// powf，2的N次方底数，不追求速度，可直接用powf，要快速可用AlgMyPow2
// powf，其他底数，使用AlgMyExp+AlgMyLog组合，速度大大减少
// sqrtf和AlgSqrtf周期差别不大，随意

__attribute__((always_inline)) static __inline float ClipToQ15F(float in)
{
    if (in > 32767.0f)
    {
        return 32767.0f;
    }
    if (in < -32767.0f)
    {
        return -32767.0f;
    }
    return in;
}

__attribute__((always_inline)) static __inline int32_t ClipToQ15(int32_t in)
{
    if (in > 32767)
    {
        return 32767;
    }
    if (in < -32767)
    {
        return -32767;
    }
    return in;
}

__attribute__((always_inline)) static __inline float DistrictF(float in, float min, float max)
{
    if (in > max)
    {
        return max;
    }
    if (in < min)
    {
        return min;
    }
    return in;
}

__attribute__((always_inline)) static __inline int32_t District(int32_t in, int32_t min, int32_t max)
{
    if (in > max)
    {
        return max;
    }
    if (in < min)
    {
        return min;
    }
    return in;
}

void AlgDryWetCalc(int32_t *dry, int32_t *wet, int32_t *dry_wet);

int32_t AlgMyExp(int32_t x);        /* powf(e, x), x range is [-6.93*1024 , +14.0*1024], more smaller return 0, more bigger return 0x7FFFFFFF */
int32_t AlgMyLog(int32_t x);        /* log(x), x range is int format, x >= 1, eg: log(f32X) = log(round(f32X * 4096)) - log(4096), fResult = (float)(AlgMyLog((int32_t)(f32X * 4096.0f)) - AlgMyLog(4096)) / 1024.0f */
int32_t AlgMyPow2(int32_t x);       /* powf(2, x), x range is [-10.0*1024 , +14.0*1024], more smaller return 0, more bigger return 0x7FFFFFFF */
float AlgQSqrtf(float number);      /* return 1/sqrtf(number) */
float AlgSqrtf(float number);       /* return sqrtf(number) */

void *AlgMyMemcpy(void *pDst, const void *pSrc, uint32_t byteSize);
void *AlgMyMemset(void *pDst, uint32_t c, uint32_t byteSize);

uint32_t AlgSinTableSet(int16_t *pTableMem);
uint32_t AlgSinTableGet(void);

extern const int16_t algTbSin[514];

#endif


