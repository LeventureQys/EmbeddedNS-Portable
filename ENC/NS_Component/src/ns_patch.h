#ifndef NS_PATCH_H
#define NS_PATCH_H

/* 由 CMake 注入, 默认全关 = 基线。基线构建下行为与固件逐位一致。 */
#ifndef NS_PATCH_D1
#define NS_PATCH_D1 0
#endif
#ifndef NS_PATCH_D5
#define NS_PATCH_D5 0
#endif
#ifndef NS_PATCH_D6
#define NS_PATCH_D6 0
#endif

/* D1: 未初始化 Y[kFftSize+1] 参与 signal_energy 累加
   打开后, 在 FFT 回填 Nyquist 的同时把虚部显式置零。 */
#if NS_PATCH_D1
    #define NS_FIX_D1_AFTER_FFT(Y)   do { (Y)[kFftSize + 1] = 0.0f; } while (0)
#else
    #define NS_FIX_D1_AFTER_FFT(Y)   do { } while (0)
#endif

/* D5: 输出钳位量程写死为 24bit
   打开后, 改用随 ENC_SAMPLE_DEPTH 派生的常量。 */
#ifndef ENC_SAMPLE_DEPTH
#define ENC_SAMPLE_DEPTH 24
#endif

#if NS_PATCH_D5
    #if (ENC_SAMPLE_DEPTH == 16)
        #define NS_CLAMP_MAX   32767.0f
        #define NS_CLAMP_MIN  -32768.0f
    #else
        #define NS_CLAMP_MAX   8388607.0f
        #define NS_CLAMP_MIN  -8388608.0f
    #endif
#else
    #define NS_CLAMP_MAX   8388607.0f
    #define NS_CLAMP_MIN  -8388607.0f
#endif

/* D6: 幅度谱开方使用位近似
   打开后, 改用精确 sqrtf。fastsqrt 本体保留, 供消融对比。 */
#if NS_PATCH_D6
    #define NS_SQRT(x)   sqrtf(x)
#else
    #define NS_SQRT(x)   fastsqrt(x)
#endif

/* D8: fastpow2 位近似的定义域保护 (执行记录 B-24)
   原实现把 8388608.0f*(x+126.9420151f) 截断为 int32_t 后按位重解释为 float:
     x < -126.9420151f      -> 整数为负, 符号位为 1        -> 返回负数
     128.058 <= x < 129.058 -> 位模式落入 NaN 区间         -> 返回 NaN
     x >= 129.058           -> int32 转换溢出(UB), x86 下得 INT32_MIN -> 返回 -0.0
   数学上 2^x 在极小 x 处下溢应为 0, 在极大 x 处上溢应为 float 上限。
   该保护必须写在截断之前 —— 否则截断本身的 UB 已经发生。
   打开后 fastexp(y) 在 y < -88.7 不再产生 NaN/巨大负数。 */
#ifndef NS_PATCH_D8
#define NS_PATCH_D8 0
#endif

#if NS_PATCH_D8
    #define NS_FASTPOW2_GUARD(x)                            \
        do {                                                \
            if ((x) < -126.9420151f) { return 0.0f; }        \
            if ((x) >  128.0f) { return 3.4028235e38f; }     \
        } while (0)
#else
    #define NS_FASTPOW2_GUARD(x)   do { } while (0)
#endif

#endif /* NS_PATCH_H */
