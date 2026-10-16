/* ============================================================================
 * _diag_nan.c —— T4 临时诊断程序(非交付物; 只做测量, 不参与任何算法计算)
 *
 * 目标: 定位基线测量中 speech_probability[] 首个 NaN 的精确产生点。
 *
 * 可信度设计:
 *   1. 本程序持有与 ns_glue.c 同构的处理器结构体, 并逐行复刻 ns_glue.c:106-112
 *      的调用序列(含 memset 步骤与顺序), 因此可直接读取 NoiseSuppressor 内部状态。
 *   2. 每帧调用**之前**快照所有"会被本帧读到的输入状态"; 调用**之后**用 src 内
 *      同源函数的克隆式重算本帧的 signal_spectrum / noise_used / post_snr /
 *      prior_snr / bessel_tmp / avg_log_lrt, 并与库的真实状态做逐位(bit)比对。
 *      比对全部通过 => 重算式与库的数值路径完全一致, "哪一帧哪个变量先变成
 *      非有限值"这一结论由库自身数值决定, 不依赖本程序的近似。
 *   3. 所有浮点量都打印位模式(0xXXXXXXXX), 不依赖 printf 的 NaN 显示习惯,
 *      可区分 +NaN / -NaN / ±Inf / ±0。
 *
 * 命令行:
 *   _diag_nan.exe probe
 *   _diag_nan.exe frames <wav> <max_frames> [lo] [hi]
 *   _diag_nan.exe sweep  <input_dir> <max_frames>
 *
 * 编译(不走 CMake; vcvars64 + cl):
 *   cl /nologo /W3 /wd4068 /utf-8 /I api /I src /I port /I ..\..\Test\harness
 *      /Fe:_diag_nan.exe _diag_nan.c ..\..\NS_Component\src\*.c
 *      ..\..\NS_Component\port\arm_math_compat.c ..\..\Test\harness\wav_io.c
 * ==========================================================================*/

#define NOMINMAX
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>

#include "platform.h"
#include "vendor_types.h"
#include "arm_math_compat.h"
#include "ns_patch.h"
#include "fast_math.h"
#include "noise_suppressor.h"
#include "enc.h"
#include "wav_io.h"

extern arm_rfft_fast_instance_f32 g_InstRfftF32;   /* ns_glue.c 定义 */
extern const float log_table[124];                 /* noise_estimator.c 定义 */

/* fast_math.h 未声明 fastlog2/fastpow2(它们在 fast_math.c 内部前向声明过),
   这里补声明以免隐式 int 原型把返回值截断。 */
float fastlog2(float x);
float fastpow2(float x);

static const int kN = kFftSizeBy2Plus1;            /* 129 */

/* ---------------------------------------------------------------------------
 * 小工具
 * -------------------------------------------------------------------------*/
static uint32_t fb(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static int is_nan_f(float v) { return (v != v) ? 1 : 0; }
static int is_inf_f(float v) { return (!is_nan_f(v) && (v * 0.0f != 0.0f)) ? 1 : 0; }
static int is_bad_f(float v) { return (is_nan_f(v) || is_inf_f(v)) ? 1 : 0; }

typedef struct {
    int      nan_cnt, inf_cnt, neg_cnt;
    int      bad_idx, nan_idx, inf_idx;
    uint32_t bad_bits;
    float    mn, mx;
    int      have_fin;
} VecStat;

static void vec_stat(const float* a, int n, VecStat* s)
{
    int i;
    s->nan_cnt = 0; s->inf_cnt = 0; s->neg_cnt = 0;
    s->bad_idx = -1; s->nan_idx = -1; s->inf_idx = -1; s->bad_bits = 0u;
    s->mn = 0.0f; s->mx = 0.0f; s->have_fin = 0;
    for (i = 0; i < n; ++i) {
        float v = a[i];
        if (is_nan_f(v)) {
            s->nan_cnt++;
            if (s->nan_idx < 0) { s->nan_idx = i; }
            if (s->bad_idx < 0) { s->bad_idx = i; s->bad_bits = fb(v); }
        } else if (is_inf_f(v)) {
            s->inf_cnt++;
            if (s->inf_idx < 0) { s->inf_idx = i; }
            if (s->bad_idx < 0) { s->bad_idx = i; s->bad_bits = fb(v); }
        } else {
            if (v < 0.0f) { s->neg_cnt++; }
            if (!s->have_fin || v < s->mn) { s->mn = v; }
            if (!s->have_fin || v > s->mx) { s->mx = v; }
            s->have_fin = 1;
        }
    }
}

static void print_vec(const char* tag, const float* a, int n)
{
    VecStat s;
    vec_stat(a, n, &s);
    printf("  %-24s min=%-15.7g max=%-15.7g nan=%-3d inf=%-3d neg=%-3d first_bad=%d",
           tag, (double)s.mn, (double)s.mx, s.nan_cnt, s.inf_cnt, s.neg_cnt, s.bad_idx);
    if (s.bad_idx >= 0) { printf(" [bits=0x%08X]", (unsigned)s.bad_bits); }
    printf("\n");
}

/* ---------------------------------------------------------------------------
 * fast_math.c 的逐字克隆(只用于打印中间量与自检; 不参与算法)
 * -------------------------------------------------------------------------*/
static float d_fastlog2(float x)
{
    union { int32_t i; float f; } v; v.f = x;
    return (float)(v.i) * 0.00000011920929f - 126.9420151f;
}
static float d_fastpow2(float x)
{
    union { int32_t i; float f; } v; v.i = (int32_t)(8388608.0f * (x + 126.9420151f));
    return v.f;
}
static float d_fastpow(float x, float y) { return d_fastpow2(d_fastlog2(x) * y); }
static float d_fastexp(float x) { return d_fastpow(10.0f, x * 0.4342945f); }

static void chain_report(const char* tag, float x)
{
    float    l2, y, fs;
    int32_t  fi;
    uint32_t bits;
    float    vf;
    l2 = d_fastlog2(10.0f);
    y  = l2 * (x * 0.4342945f);
    fs = 8388608.0f * (y + 126.9420151f);
    fi = (int32_t)fs;
    bits = (uint32_t)fi;
    memcpy(&vf, &bits, 4);
    printf("    %s  x=%.9g [0x%08X]\n", tag, (double)x, (unsigned)fb(x));
    printf("      fastlog2(10.0f)=%.9g   y=fastlog2(10)*x*0.4342945=%.9g\n", (double)l2, (double)y);
    printf("      F=8388608*(y+126.9420151)=%.9g   (int32_t)F=%d   0x%08X\n",
           (double)fs, (int)fi, (unsigned)bits);
    printf("      reinterpret as float = %.9g [0x%08X]\n", (double)vf, (unsigned)fb(vf));
    printf("      fastexp(lib)=%.9g [0x%08X]  clone=%.9g  expf=%.9g\n",
           (double)fastexp(x), (unsigned)fb(fastexp(x)), (double)d_fastexp(x), (double)expf(x));
}

/* ---------------------------------------------------------------------------
 * 处理器(与 ns_glue.c 同构)
 * -------------------------------------------------------------------------*/
#define DIAG_FRAME_LEN 480

typedef struct {
    Enc    enc;
    float  split_data[kNumBands][kSplitBandSize];
    float* row_pointers[kNumBands];
} DiagCtx;

static DiagCtx* diag_init(int level)
{
    DiagCtx* c = (DiagCtx*)calloc(1, sizeof(DiagCtx));
    int32_t  i;
    if (c == NULL) { return NULL; }
    ThreeBandFilterBank_Init(&c->enc.three_band_filter_bank);
    NoiseSuppressor_Init(&c->enc.noise_suppressor, level);
    if (arm_rfft_fast_init_f32(&g_InstRfftF32, 256) != ARM_MATH_SUCCESS) { free(c); return NULL; }
    for (i = 0; i < kNumBands; ++i) { c->row_pointers[i] = c->split_data[i]; }
    return c;
}

typedef struct {
    float filter[kFftSizeBy2Plus1];
    float prev_noise[kFftSizeBy2Plus1];
    float prev_spectrum[kFftSizeBy2Plus1];
    float conservative[kFftSizeBy2Plus1];
    float avg_pre[kFftSizeBy2Plus1];
    float prev_prob[kFftSizeBy2Plus1];
    float diff_norm;
    float white, pink_num, pink_exp;
} Snapshot;

typedef struct {
    float    sig [kFftSizeBy2Plus1];
    float    ql  [kFftSizeBy2Plus1];
    float    noise_used[kFftSizeBy2Plus1];
    float    post[kFftSizeBy2Plus1];
    float    prior[kFftSizeBy2Plus1];
    float    tmp1[kFftSizeBy2Plus1];
    float    tmp2[kFftSizeBy2Plus1];
    float    bessel[kFftSizeBy2Plus1];
    float    avg_rec[kFftSizeBy2Plus1];
    float    inv_lib[kFftSizeBy2Plus1];
    float    prob_rec[kFftSizeBy2Plus1];
    float    gain_prior;
    int      mismatch;
} FrameCalc;

/* 重算本帧中间量并做位级自检 */
static void recompute_frame(const Snapshot* pre, int32_t naf, NoiseSuppressor* ns,
                            float signal_spectral_sum, float over_sub, FrameCalc* fc)
{
    int   i;
    int   mismatch = 0;
    float* avg_actual = ns->speech_probability_estimator
                          .signal_model_estimator.signal_model.avg_log_lrt;
    float* prob_actual = ns->speech_probability_estimator.speech_probability;
    float  prior_speech_prob = ns->speech_probability_estimator.prior_speech_prob;

    for (i = 0; i < kN; ++i) {
        fc->sig[i] = ns->prev_analysis_signal_spectrum[i];                       /* = signal_spectrum(本帧) */
        fc->ql[i]  = ns->noise_estimator.quantile_noise_estimator.quantile[i];   /* = quantile(本帧) */
    }

    /* 复刻 NoiseEstimator_PreUpdate 中 Estimate 之后的 noise_spectrum 写入 */
    if (naf < kShortStartupPhaseBlocks) {
        int32_t kStartBand = 5;
        float   sum_log_i = 0.0f, sum_log_i_square = 0.0f, sum_log_magn = 0.0f;
        float   sum_log_i_log_magn = 0.0f, log_i, log_signal, denom, num;
        float   adj, one_by_n1, parametric_exp, parametric_num, parametric_denom, tmp;
        float   white = pre->white + signal_spectral_sum * kOneByFftSizeBy2Plus1 * over_sub;
        float   pink_num = pre->pink_num, pink_exp = pre->pink_exp;
        float   parametric[kFftSizeBy2Plus1];

        for (i = kStartBand; i < kN; ++i) {
            log_i = log_table[i - 5];
            sum_log_i += log_i;
            sum_log_i_square += log_i * log_i;
            log_signal = fastlog(fc->sig[i]);
            sum_log_magn += log_signal;
            sum_log_i_log_magn += log_i * log_signal;
        }
        denom = sum_log_i_square * (float)(kN - kStartBand) - sum_log_i * sum_log_i;
        num = sum_log_i_square * sum_log_magn - sum_log_i * sum_log_i_log_magn;
        adj = num / denom;
        adj = MAX(adj, 0.0f);
        pink_num += adj;

        num = sum_log_i * sum_log_magn - (float)(kN - kStartBand) * sum_log_i_log_magn;
        adj = num / denom;
        adj = MAX(MIN(adj, 1.0f), 0.0f);
        pink_exp += adj;

        one_by_n1 = 1.0f / (float)(naf + 1);
        parametric_exp = 0.0f;
        parametric_num = 0.0f;
        if (pink_exp > 0.0f) {
            parametric_num = fastexp(pink_num * one_by_n1);
            parametric_num *= (float)(naf + 1);
            parametric_exp = pink_exp * one_by_n1;
        }
        for (i = 0; i < kN; ++i) {
            if (pink_exp == 0.0f) {
                parametric[i] = white;
            } else {
                int32_t use_band = (i < kStartBand) ? kStartBand : i;
                parametric_denom = fastpow((float)use_band, parametric_exp);
                parametric[i] = parametric_num / parametric_denom;
            }
        }
        for (i = 0; i < kN; ++i) {
            float v = fc->ql[i] * (float)naf;
            tmp = parametric[i] * (float)(kShortStartupPhaseBlocks - naf);
            v += tmp * one_by_n1;
            v *= kOneByShortStartupPhaseBlocks;
            fc->noise_used[i] = v;
        }
    } else {
        for (i = 0; i < kN; ++i) { fc->noise_used[i] = fc->ql[i]; }
    }

    /* 复刻 ComputeSnr (noise_suppressor.c:139-159) */
    for (i = 0; i < kN; ++i) {
        float prev_estimate = pre->prev_spectrum[i] / (pre->prev_noise[i] + 0.0001f) * pre->filter[i];
        if (fc->sig[i] > fc->noise_used[i]) {
            fc->post[i] = fc->sig[i] / (fc->noise_used[i] + 0.0001f) - 1.0f;
        } else {
            fc->post[i] = 0.0f;
        }
        fc->prior[i] = 0.98f * prev_estimate + (1.0f - 0.98f) * fc->post[i];
    }

    /* 复刻 UpdateSpectralLrt (signal_model_estimator.c:89-109) */
    for (i = 0; i < kN; ++i) {
        fc->tmp1[i]   = 1.0f + 2.0f * fc->prior[i];
        fc->tmp2[i]   = 2.0f * fc->prior[i] / (fc->tmp1[i] + 0.0001f);
        fc->bessel[i] = (fc->post[i] + 1.0f) * fc->tmp2[i];
        fc->avg_rec[i] = pre->avg_pre[i] +
                         0.5f * (fc->bessel[i] - fastlog(fc->tmp1[i]) - pre->avg_pre[i]);
    }
    for (i = 0; i < kN; ++i) {
        if (fb(fc->avg_rec[i]) != fb(avg_actual[i])) { mismatch++; }
    }

    /* 复刻 SpeechProbabilityEstimator_Update 的 prob 计算(数学意义, 见报告) */
    fc->gain_prior = (1.0f - prior_speech_prob) / (prior_speech_prob + 0.0001f);
    for (i = 0; i < kN; ++i) {
        fc->inv_lib[i]  = fastexp(-avg_actual[i]);                  /* 库函数 */
        fc->prob_rec[i] = 1.0f / (1.0f + fc->gain_prior * fc->inv_lib[i]);
    }
    for (i = 0; i < kN; ++i) {
        if (fb(fc->prob_rec[i]) != fb(prob_actual[i])) { mismatch++; }
    }
    fc->mismatch = mismatch;
}

/* ---------------------------------------------------------------------------
 * probe
 * -------------------------------------------------------------------------*/
static void probe_mode(void)
{
    float x;
    long  i;
    int   prevk, in_nan = 0, in_neg = 0;
    float nan_lo = 0.0f, neg_lo = 0.0f;

    printf("================ fastexp / fastpow2 定性探针 ================\n");
    printf("-- 1. 关键点表 --\n");
    {
        const float pts[] = { -400.0f, -300.0f, -270.0f, -266.5f, -200.0f, -130.0f, -127.0f,
                              -126.9f, -100.0f, -89.5f, -89.06f, -89.0564f, -88.5f, -88.3604f,
                              -88.0f, -80.0f, -10.0f, -1.0f, 0.0f, 1.0f, 10.0f, 80.0f,
                              88.5f, 89.0f, 89.1363f, 89.5f, 89.8324f, 90.0f, 130.0f, 300.0f };
        size_t k;
        for (k = 0; k < sizeof(pts) / sizeof(pts[0]); ++k) {
            float v;
            x = pts[k];
            v = fastexp(x);
            printf("  x=%-11.4g lib=%-16.9g [0x%08X] clone=%-16.9g expf=%-12.5g %s\n",
                   (double)x, (double)v, (unsigned)fb(v), (double)d_fastexp(x), (double)expf(x),
                   is_nan_f(v) ? "NaN" : (is_inf_f(v) ? "Inf" : (v < 0.0f ? "NEGATIVE" : "ok")));
        }
    }

    printf("-- 2. 克隆与库函数的位级一致性(网格) --\n");
    {
        long bad = 0, tot = 0, bad2 = 0;
        for (i = -200000; i <= 200000; ++i) {
            x = (float)i * 0.001f;
            tot++;
            if (fb(fastexp(x)) != fb(d_fastexp(x))) { bad++; }
            if (fb(fastpow2(x * 1.4366396f)) != fb(d_fastpow2(x * 1.4366396f))) { bad2++; }
        }
        printf("  fastexp      : x in [-200,200] step 0.001 samples=%ld mismatches=%ld\n", tot, bad);
        printf("  fastpow2     : arg in [-287,287] step 0.0014366 samples=%ld mismatches=%ld\n", tot, bad2);
    }

    printf("-- 3. 库函数特殊输入(位模式) --\n");
    {
        uint32_t nbits = 0x7FC00000u;   /* +NaN */
        uint32_t ibits = 0x7F800000u;   /* +Inf */
        float nanv, infv, v;
        memcpy(&nanv, &nbits, 4);
        memcpy(&infv, &ibits, 4);
        v = fastexp(nanv);   printf("  fastexp(NaN)  = %-12.9g [0x%08X] %s\n", (double)v, (unsigned)fb(v), is_nan_f(v) ? "NaN" : "finite");
        v = fastexp(infv);   printf("  fastexp(+Inf) = %-12.9g [0x%08X]\n", (double)v, (unsigned)fb(v));
        v = fastexp(-infv);  printf("  fastexp(-Inf) = %-12.9g [0x%08X]\n", (double)v, (unsigned)fb(v));
        v = fastpow2(nanv);  printf("  fastpow2(NaN) = %-12.9g [0x%08X] %s\n", (double)v, (unsigned)fb(v), is_nan_f(v) ? "NaN" : "finite");
        v = fastpow2(-400.0f); printf("  fastpow2(-400)= %-12.9g [0x%08X]\n", (double)v, (unsigned)fb(v));
        v = fastpow2(400.0f);  printf("  fastpow2(+400)= %-12.9g [0x%08X]\n", (double)v, (unsigned)fb(v));
        v = fastpow2(-127.0f); printf("  fastpow2(-127)= %-12.9g [0x%08X] %s\n", (double)v, (unsigned)fb(v), is_nan_f(v) ? "NaN" : "finite");
        v = fastpow2(-127.5f); printf("  fastpow2(-127.5)=%-12.9g [0x%08X] %s\n", (double)v, (unsigned)fb(v), is_nan_f(v) ? "NaN" : "finite");
        v = fastpow2(-126.5f); printf("  fastpow2(-126.5)=%-12.9g [0x%08X] %s\n", (double)v, (unsigned)fb(v), is_nan_f(v) ? "NaN" : "finite");
    }

    printf("-- 4. 区间扫描 x in [-140,140] step 0.0005 --\n");
    {
        long  n;
        float step = 0.0005f, xa = -140.0f, prev = fastexp(xa);
        prevk = is_nan_f(prev) ? 2 : (prev < 0.0f ? 1 : 0);
        if (prevk == 2) { in_nan = 1; nan_lo = xa; }
        if (prevk == 1) { in_neg = 1; neg_lo = xa; }
        for (n = 1; n <= (long)(280.0f / step); ++n) {
            float xc = xa + (float)n * step;
            float v = fastexp(xc);
            int   k = is_nan_f(v) ? 2 : (v < 0.0f ? 1 : 0);
            if (k != prevk) {
                if (prevk == 2) { printf("  NaN  区间: [%.6f, %.6f]\n", (double)nan_lo, (double)(xc - step)); in_nan = 0; }
                if (prevk == 1) { printf("  负值 区间: [%.6f, %.6f]\n", (double)neg_lo, (double)(xc - step)); in_neg = 0; }
                if (k == 2) { nan_lo = xc; in_nan = 1; }
                if (k == 1) { neg_lo = xc; in_neg = 1; }
            }
            prevk = k;
        }
        if (in_nan) { printf("  NaN  区间: [%.6f, 140.0] (开区间到扫描边界)\n", (double)nan_lo); }
        if (in_neg) { printf("  负值 区间: [%.6f, 140.0] (开区间到扫描边界)\n", (double)neg_lo); }
    }

    printf("-- 5. 解析判据 --\n");
    printf("  fastexp(x) = fastpow2(1.4366396*x),  fastpow2 把 (int32_t)(8388608*(y+126.9420151)) 按位当 float。\n");
    printf("  令 u = 1.4366396*x + 126.9420151, F = 8388608*u:\n");
    printf("    u < -256 (x < -266.71)      : F 超出 int32 -> x86 cvttss2si 给 0x80000000 -> -0.0f\n");
    printf("    u in [-256,-1] (x in [-266.71,-89.0564)) : 结果为大负数/负Inf(符号翻转, 定性错误)\n");
    printf("    u in (-1,0)    (x in (-89.0564,-88.3604)): 结果 NaN(0xFF800001..0xFFFFFFFF)\n");
    printf("    u in [255,256) (x in [89.1363,89.8324))  : 结果 NaN(0x7F800001..0x7FFFFFFF)\n");
}

/* ---------------------------------------------------------------------------
 * frames: 单文件逐帧诊断
 * -------------------------------------------------------------------------*/
#define NHIST 8
typedef struct {
    int      fi;
    float    avg_pre, avg_post, inv, prob, noise_used, sig, post, prior, bessel, tmp1, tmp2;
    uint32_t inv_bits, prob_bits;
} BinStep;

#define NWATCH 14
static const char* kWName[NWATCH] = {
    "signal_spectrum", "quantile(noise)", "noise_used", "post_snr", "prior_snr",
    "tmp1", "tmp2", "bessel_tmp", "avg_log_lrt", "inv_lrt(fastexp)",
    "speech_probability", "filter(post)", "conservative", "avg_log_lrt_rerun"
};

static void dump_bin(const FrameCalc* fc, const Snapshot* pre, const float* avg_actual,
                     const float* prob_actual, int b, int fi, float gain_prior)
{
    printf("    ---- bin %d @ frame %d ----\n", b, fi);
    printf("      signal_spectrum       = %.9g [0x%08X]\n", (double)fc->sig[b], (unsigned)fb(fc->sig[b]));
    printf("      quantile(noise E.)    = %.9g [0x%08X]\n", (double)fc->ql[b], (unsigned)fb(fc->ql[b]));
    printf("      noise_used(ComputeSnr)= %.9g [0x%08X]\n", (double)fc->noise_used[b], (unsigned)fb(fc->noise_used[b]));
    printf("      prev_signal_spectrum  = %.9g [0x%08X]\n", (double)pre->prev_spectrum[b], (unsigned)fb(pre->prev_spectrum[b]));
    printf("      prev_noise_spectrum   = %.9g [0x%08X]\n", (double)pre->prev_noise[b], (unsigned)fb(pre->prev_noise[b]));
    printf("      filter                = %.9g [0x%08X]\n", (double)pre->filter[b], (unsigned)fb(pre->filter[b]));
    printf("      post_snr              = %.9g [0x%08X]\n", (double)fc->post[b], (unsigned)fb(fc->post[b]));
    printf("      prior_snr             = %.9g [0x%08X]\n", (double)fc->prior[b], (unsigned)fb(fc->prior[b]));
    printf("      tmp1                  = %.9g [0x%08X]\n", (double)fc->tmp1[b], (unsigned)fb(fc->tmp1[b]));
    printf("      tmp2                  = %.9g [0x%08X]\n", (double)fc->tmp2[b], (unsigned)fb(fc->tmp2[b]));
    printf("      fastlog(tmp1)         = %.9g [0x%08X]\n", (double)fastlog(fc->tmp1[b]), (unsigned)fb(fastlog(fc->tmp1[b])));
    printf("      bessel_tmp            = %.9g [0x%08X]\n", (double)fc->bessel[b], (unsigned)fb(fc->bessel[b]));
    printf("      avg_log_lrt  before   = %.9g [0x%08X]\n", (double)pre->avg_pre[b], (unsigned)fb(pre->avg_pre[b]));
    printf("      avg_log_lrt  after    = %.9g [0x%08X]  (rerun %.9g, %s)\n",
           (double)avg_actual[b], (unsigned)fb(avg_actual[b]), (double)fc->avg_rec[b],
           (fb(avg_actual[b]) == fb(fc->avg_rec[b])) ? "bit-match" : "MISMATCH");
    printf("      gain_prior            = %.9g\n", (double)gain_prior);
    printf("      inv_lrt=fastexp(-avg) = %.9g [0x%08X]  expf(-avg) = %.9g\n",
           (double)fc->inv_lib[b], (unsigned)fb(fc->inv_lib[b]), (double)expf(-avg_actual[b]));
    printf("      speech_probability    = %.9g [0x%08X]\n", (double)prob_actual[b], (unsigned)fb(prob_actual[b]));
    printf("      判定: avg_log_lrt %s; fastexp(有限输入) 却给出 %s => %s\n",
           is_bad_f(pre->avg_pre[b]) ? "进入本帧前已非有限" : "进入本帧时有限",
           is_nan_f(fc->inv_lib[b]) ? "NaN" : (is_inf_f(fc->inv_lib[b]) ? "Inf" : "有限值"),
           (!is_bad_f(pre->avg_pre[b]) && is_bad_f(fc->inv_lib[b])) ?
               "NaN 诞生于 fastexp/fastpow2 内部(输入有限)" :
               (is_bad_f(pre->avg_pre[b]) ? "avg_log_lrt 更早已非有限" : "NaN 未在本处产生"));
}

static int diag_run_file(const char* path, int max_frames, int lo, int hi, int quiet)
{
    int16_t*  samples = NULL;
    int32_t   n_frames = 0, n_ch = 0, rate = 0;
    float*    in24 = NULL;
    float*    in_frame = (float*)malloc(sizeof(float) * DIAG_FRAME_LEN);
    float*    out_frame = (float*)malloc(sizeof(float) * DIAG_FRAME_LEN);
    DiagCtx*  ctx;
    Snapshot* pre = (Snapshot*)malloc(sizeof(Snapshot));
    FrameCalc* fc = (FrameCalc*)malloc(sizeof(FrameCalc));
    BinStep (*hist)[NHIST] = (BinStep(*)[NHIST])malloc(sizeof(BinStep) * kN * NHIST);
    int       hist_pos[129];
    int       rc, fi, i, k;
    long long total_mismatch = 0;
    long long first_bad_frame[NWATCH];
    int       first_bad_bin[NWATCH];
    uint32_t  first_bad_bits[NWATCH];
    int       first_bad_isnan[NWATCH];
    long long first_runner_bad = -1;
    int       runner_bad_isnan = -1;
    long long prob_first_bad = -1, inv_first_bad = -1, avg_first_bad = -1, noise_neg_first = -1;
    long long exp_regime_first = -1;    /* 首个 avg_log_lrt > 88.3604 的帧(fastexp 负值区) */
    int       exp_regime_bins = 0;      /* 该帧受影响的 bin 数 */
    long long inv_neg_first = -1;       /* 首个 inv_lrt < 0 的帧 */
    long long prob_oou_first = -1;      /* 首个 speech_probability 越界(<0 或 >1)的帧 */
    long long prob_all_one_first = -1;  /* 首个全部 129 个 prob 恒为 1.0 的帧 */
    long long frames_with_regime = 0;   /* 含 avg_log_lrt>88.3604 的帧数 */
    long long frames_with_inv_nan = 0;  /* inv_lrt 出现 NaN 的帧数 */
    long long nan_frame_n = 0;          /* speech_probability 出现 NaN 的帧数 */
    long long nan_frame_idx[4096];
    int       nan_bin_hist[129];
    long long global_max_avg_frame = -1;
    int       global_max_avg_bin = -1;
    float     global_max_avg = -3.0e38f;
    float     global_max_avg_inv = 0.0f;
    uint32_t  global_max_avg_inv_bits = 0u;
    long long nframes_run = 0;
    int       prob_persist = 1, nan_seen_frames = 0;
    int32_t   prev_naf = -2;

    memset(hist, 0, sizeof(BinStep) * kN * NHIST);
    memset(nan_bin_hist, 0, sizeof(nan_bin_hist));
    for (i = 0; i < kN; ++i) { hist_pos[i] = -1; }
    for (i = 0; i < NWATCH; ++i) {
        first_bad_frame[i] = -1; first_bad_bin[i] = -1; first_bad_bits[i] = 0u; first_bad_isnan[i] = 0;
    }

    rc = wav_read_pcm16(path, &samples, &n_frames, &n_ch, &rate);
    if (rc != 0) { printf("  [ERR] wav_read rc=%d for %s\n", rc, path); return 1; }
    if (rate != 48000 || n_ch < 1) { printf("  [ERR] rate=%d ch=%d\n", (int)rate, (int)n_ch); wav_free(samples); return 1; }
    in24 = (float*)malloc((size_t)n_frames * sizeof(float));
    for (i = 0; i < n_frames; ++i) { in24[i] = (float)samples[(size_t)i * (size_t)n_ch] * 256.0f; }
    wav_free(samples); samples = NULL;

    ctx = diag_init(3);
    if (ctx == NULL) { printf("  [ERR] diag_init NULL\n"); return 1; }

    if (!quiet) {
        printf("\n============ frames: %s (max_frames=%d) ============\n", path, max_frames);
        printf("  wav frames=%d ch=%d rate=%d\n", (int)n_frames, (int)n_ch, (int)rate);
    }

    for (fi = 0; fi < max_frames; ++fi) {
        NoiseSuppressor* ns = &ctx->enc.noise_suppressor;
        int32_t base = fi * DIAG_FRAME_LEN, valid, naf;
        int     verbose;
        VecStat s;
        char    hx[16];
        float*  avg_actual;
        float*  prob_actual;

        if (base >= n_frames) { break; }
        valid = n_frames - base;
        if (valid > DIAG_FRAME_LEN) { valid = DIAG_FRAME_LEN; }
        for (k = 0; k < DIAG_FRAME_LEN; ++k) { in_frame[k] = (k < valid) ? in24[base + k] : 0.0f; }
        memset(out_frame, 0, sizeof(float) * DIAG_FRAME_LEN);

        memcpy(pre->filter,     ns->wiener_filter.filter, sizeof(pre->filter));
        memcpy(pre->prev_noise, ns->noise_estimator.noise_spectrum, sizeof(pre->prev_noise));
        memcpy(pre->prev_spectrum, ns->prev_analysis_signal_spectrum, sizeof(pre->prev_spectrum));
        memcpy(pre->conservative, ns->noise_estimator.conservative_noise_spectrum, sizeof(pre->conservative));
        memcpy(pre->avg_pre,    ns->speech_probability_estimator.signal_model_estimator.signal_model.avg_log_lrt,
               sizeof(pre->avg_pre));
        memcpy(pre->prev_prob,  ns->speech_probability_estimator.speech_probability, sizeof(pre->prev_prob));
        pre->diff_norm = ns->speech_probability_estimator.signal_model_estimator.diff_normalization;
        pre->white     = ns->noise_estimator.white_noise_level;
        pre->pink_num  = ns->noise_estimator.pink_noise_numerator;
        pre->pink_exp  = ns->noise_estimator.pink_noise_exp;

        /* ns_glue.c:106-112 等价调用序列 */
        memset(ctx->split_data, 0, kFullBandSize * sizeof(float));
        ThreeBandFilterBank_Analysis(&ctx->enc.three_band_filter_bank, in_frame, ctx->row_pointers);
        NoiseSuppressor_Analyze(ns, ctx->row_pointers);
        NoiseSuppressor_Process(ns, ctx->row_pointers);
        ThreeBandFilterBank_Synthesis(&ctx->enc.three_band_filter_bank, ctx->row_pointers, out_frame);

        naf = ns->num_analyzed_frames;
        if (naf == prev_naf) {
            if (!quiet) { printf("[frame %d] Analyze 被跳过(零帧), 不做重算\n", fi); }
            prev_naf = naf;
            continue;
        }
        prev_naf = naf;

        recompute_frame(pre, naf, ns, ns->last_signal_spectral_sum,
                        ns->suppression_params.over_subtraction_factor, fc);
        total_mismatch += fc->mismatch;

        avg_actual  = ns->speech_probability_estimator.signal_model_estimator.signal_model.avg_log_lrt;
        prob_actual = ns->speech_probability_estimator.speech_probability;

        /* 逐量检查(按因果顺序) */
        {
            const float* arrs[NWATCH];
            arrs[0]  = fc->sig;
            arrs[1]  = fc->ql;
            arrs[2]  = fc->noise_used;
            arrs[3]  = fc->post;
            arrs[4]  = fc->prior;
            arrs[5]  = fc->tmp1;
            arrs[6]  = fc->tmp2;
            arrs[7]  = fc->bessel;
            arrs[8]  = avg_actual;
            arrs[9]  = fc->inv_lib;
            arrs[10] = prob_actual;
            arrs[11] = ns->wiener_filter.filter;
            arrs[12] = ns->noise_estimator.conservative_noise_spectrum;
            arrs[13] = fc->avg_rec;
            for (k = 0; k < NWATCH; ++k) {
                vec_stat(arrs[k], kN, &s);
                if (s.bad_idx >= 0 && first_bad_frame[k] < 0) {
                    first_bad_frame[k] = fi;
                    first_bad_bin[k] = s.bad_idx;
                    first_bad_bits[k] = s.bad_bits;
                    first_bad_isnan[k] = is_nan_f(arrs[k][s.bad_idx]);
                    if (k == 8)  { avg_first_bad = fi; }
                    if (k == 9)  { inv_first_bad = fi; }
                    if (k == 10) { prob_first_bad = fi; }
                }
                if (k == 1 && s.neg_cnt > 0 && noise_neg_first < 0) { noise_neg_first = fi; }
            }
        }

        /* 缺陷"激活区间"的量化: fastexp 在 x < -88.3604(即 avg_log_lrt > 88.3604) 起返回负数 */
        {
            int n_reg = 0, n_invneg = 0, n_oou = 0, n_one = 0;
            for (i = 0; i < kN; ++i) {
                if (avg_actual[i] > 88.3604f)      { n_reg++; }
                if (fc->inv_lib[i] < 0.0f)         { n_invneg++; }
                if (prob_actual[i] < 0.0f ||
                    prob_actual[i] > 1.0f)         { n_oou++; }
                if (fb(prob_actual[i]) == 0x3F800000u) { n_one++; }
            }
            if (n_reg > 0 && exp_regime_first < 0) { exp_regime_first = fi; exp_regime_bins = n_reg; }
            if (n_reg > 0) { frames_with_regime++; }
            if (n_invneg > 0 && inv_neg_first < 0) { inv_neg_first = fi; }
            if (n_oou > 0 && prob_oou_first < 0)   { prob_oou_first = fi; }
            if (n_one == kN && prob_all_one_first < 0) { prob_all_one_first = fi; }
        }

        /* inv_lrt 的 NaN、avg_log_lrt 的帧级最大值(用于 D8 前后对比) */
        {
            int   bi = -1, n_inv_nan = 0, n_prob_nan = 0;
            float mv = -3.0e38f;
            for (i = 0; i < kN; ++i) {
                if (is_nan_f(avg_actual[i])) { continue; }
                if (avg_actual[i] > mv) { mv = avg_actual[i]; bi = i; }
                if (is_nan_f(fc->inv_lib[i])) { n_inv_nan++; }
                if (is_nan_f(prob_actual[i])) { n_prob_nan++; }
            }
            if (n_inv_nan > 0) { frames_with_inv_nan++; }
            if (bi >= 0 && mv > global_max_avg) {
                global_max_avg = mv;
                global_max_avg_frame = fi;
                global_max_avg_bin = bi;
                global_max_avg_inv = fc->inv_lib[bi];
                global_max_avg_inv_bits = fb(fc->inv_lib[bi]);
            }
            /* NaN 事件: 逐帧列出 bin 下标 */
            if (n_prob_nan > 0) {
                int prev_bin, range_start;
                if (nan_frame_n < (long long)(sizeof(nan_frame_idx) / sizeof(nan_frame_idx[0]))) {
                    nan_frame_idx[nan_frame_n] = fi;
                }
                nan_frame_n++;
                for (i = 0; i < kN; ++i) { if (is_nan_f(prob_actual[i])) { nan_bin_hist[i]++; } }
                if (!quiet) {
                    printf("  NANEV frame=%d bins=%d list=[", fi, n_prob_nan);
                    prev_bin = -2; range_start = -1;
                    for (i = 0; i < kN; ++i) {
                        if (!is_nan_f(prob_actual[i])) { continue; }
                        if (range_start < 0) { range_start = i; prev_bin = i; continue; }
                        if (i == prev_bin + 1) { prev_bin = i; continue; }
                        if (prev_bin > range_start) { printf("%d-%d,", range_start, prev_bin); }
                        else { printf("%d,", range_start); }
                        range_start = i; prev_bin = i;
                    }
                    if (range_start >= 0) {
                        if (prev_bin > range_start) { printf("%d-%d", range_start, prev_bin); }
                        else { printf("%d", range_start); }
                    }
                    printf("]\n");
                }
            }
        }
        nframes_run++;

        /* runner 可见量(与 ns_runner.c:465-483 同一集合) */        {
            float sum = 0.0f;
            double in_sq = 0.0, out_sq = 0.0;
            float in_rms, out_rms, atten_db;
            for (i = 0; i < kN; ++i) { sum += prob_actual[i]; }
            for (i = 0; i < DIAG_FRAME_LEN; ++i) {
                in_sq  += (double)in_frame[i] * (double)in_frame[i];
                out_sq += (double)out_frame[i] * (double)out_frame[i];
            }
            in_rms  = (float)sqrt(in_sq / (double)DIAG_FRAME_LEN);
            out_rms = (float)sqrt(out_sq / (double)DIAG_FRAME_LEN);
            atten_db = (out_rms == 0.0f) ? 999.0f : (float)(20.0 * log10((double)in_rms / (double)out_rms));
            if (first_runner_bad < 0) {
                int bad = is_bad_f(in_rms) || is_bad_f(out_rms) || is_bad_f(atten_db) ||
                          is_bad_f(ns->last_signal_energy) || is_bad_f(ns->last_signal_spectral_sum) ||
                          is_bad_f(pre->diff_norm) || is_bad_f(sum / (float)kN) ||
                          is_bad_f(ns->speech_probability_estimator.prior_speech_prob) ||
                          is_bad_f(ns->last_gain_adjustment);
                if (bad) {
                    first_runner_bad = fi;
                    runner_bad_isnan = is_nan_f(sum / (float)kN) ? 1 : 0;
                }
            }
        }

        /* 历史记录 */
        for (i = 0; i < kN; ++i) {
            BinStep* e;
            hist_pos[i] = (hist_pos[i] + 1) % NHIST;
            e = &hist[i][hist_pos[i]];
            e->fi = fi;
            e->avg_pre = pre->avg_pre[i];
            e->avg_post = avg_actual[i];
            e->inv = fc->inv_lib[i];
            e->prob = prob_actual[i];
            e->noise_used = fc->noise_used[i];
            e->sig = fc->sig[i];
            e->post = fc->post[i];
            e->prior = fc->prior[i];
            e->bessel = fc->bessel[i];
            e->tmp1 = fc->tmp1[i];
            e->tmp2 = fc->tmp2[i];
            e->inv_bits = fb(e->inv);
            e->prob_bits = fb(e->prob);
        }

        {
            int has_nan = 0;
            vec_stat(prob_actual, kN, &s);
            if (s.nan_cnt > 0) { has_nan = 1; nan_seen_frames++; }
            else if (nan_seen_frames > 0 && !quiet) { /* 不再持久 */ }
        }

        verbose = (!quiet && ((fi >= lo && fi <= hi)));
        /* 首次出现非有限值的那一帧强制详报 */
        {
            int forced = 0;
            for (k = 0; k < NWATCH; ++k) { if (first_bad_frame[k] == fi) { forced = 1; } }
            if (fi == 0) { forced = 1; }
            if (forced) { verbose = 1; }
        }

        if (verbose) {
            float sum = 0.0f;
            SignalModel*     sm = &ns->speech_probability_estimator.signal_model_estimator.signal_model;
            PriorSignalModel* pm = &ns->speech_probability_estimator.signal_model_estimator.prior_signal_model;
            float kWidthPrior0 = 4.0f, kWidthPrior1 = 2.0f * kWidthPrior0, width_prior, ind0, ind1, ind2, ind_prior;
            width_prior = (sm->lrt < pm->lrt) ? kWidthPrior1 : kWidthPrior0;
            ind0 = 0.5f * (tanhf(width_prior * (sm->lrt - pm->lrt)) + 1.0f);
            width_prior = (sm->spectral_flatness > pm->flatness_threshold) ? kWidthPrior1 : kWidthPrior0;
            ind1 = 0.5f * (tanhf(width_prior * (pm->flatness_threshold - sm->spectral_flatness)) + 1.0f);
            width_prior = (sm->spectral_diff < pm->template_diff_threshold) ? kWidthPrior1 : kWidthPrior0;
            ind2 = 0.5f * (tanhf(width_prior * (sm->spectral_diff - pm->template_diff_threshold)) + 1.0f);
            ind_prior = pm->lrt_weighting * ind0 + pm->flatness_weighting * ind1 + pm->difference_weighting * ind2;

            for (i = 0; i < kN; ++i) { sum += prob_actual[i]; }
            printf("[frame %4d] naf=%d mismatch=%d  speech_prob_mean=%.9g [0x%08X]\n",
                   fi, (int)naf, fc->mismatch, (double)(sum / (float)kN), (unsigned)fb(sum / (float)kN));
            printf("  scalars: prior_speech_prob=%.9g [0x%08X] gain_prior=%.9g\n",
                   (double)ns->speech_probability_estimator.prior_speech_prob,
                   (unsigned)fb(ns->speech_probability_estimator.prior_speech_prob),
                   (double)fc->gain_prior);
            printf("  model  : lrt=%.9g flatness=%.9g diff=%.9g\n",
                   (double)sm->lrt, (double)sm->spectral_flatness, (double)sm->spectral_diff);
            printf("  priorm : lrt=%.9g flat_thr=%.9g diff_thr=%.9g w=(%.6g,%.6g,%.6g)\n",
                   (double)pm->lrt, (double)pm->flatness_threshold, (double)pm->template_diff_threshold,
                   (double)pm->lrt_weighting, (double)pm->flatness_weighting, (double)pm->difference_weighting);
            printf("  indicators: ind0=%.9g ind1=%.9g ind2=%.9g ind_prior=%.9g\n",
                   (double)ind0, (double)ind1, (double)ind2, (double)ind_prior);
            {
                int   bi = -1, k2, n_inv_nan = 0, n_prob_nan = 0;
                float mv = -3.0e38f;
                for (k2 = 0; k2 < kN; ++k2) {
                    if (!is_nan_f(avg_actual[k2]) && avg_actual[k2] > mv) { mv = avg_actual[k2]; bi = k2; }
                    if (is_nan_f(fc->inv_lib[k2])) { n_inv_nan++; }
                    if (is_nan_f(prob_actual[k2])) { n_prob_nan++; }
                }
                printf("  extremes: max_avg_log_lrt=%.9g @bin%d  inv_lrt=%.9g [0x%08X]  NaN(inv)=%d NaN(prob)=%d\n",
                       (double)mv, bi, (bi >= 0) ? (double)fc->inv_lib[bi] : 0.0,
                       (bi >= 0) ? (unsigned)fb(fc->inv_lib[bi]) : 0u, n_inv_nan, n_prob_nan);
            }
            print_vec("signal_spectrum", fc->sig, kN);
            print_vec("quantile(noise)", fc->ql, kN);
            print_vec("noise_used", fc->noise_used, kN);
            print_vec("noise_spectrum(post)", ns->noise_estimator.noise_spectrum, kN);
            print_vec("prev_noise_spectrum", pre->prev_noise, kN);
            print_vec("post_snr", fc->post, kN);
            print_vec("prior_snr", fc->prior, kN);
            print_vec("tmp1", fc->tmp1, kN);
            print_vec("tmp2", fc->tmp2, kN);
            print_vec("bessel_tmp", fc->bessel, kN);
            print_vec("avg_log_lrt(before)", pre->avg_pre, kN);
            print_vec("avg_log_lrt(after)", avg_actual, kN);
            print_vec("inv_lrt(fastexp)", fc->inv_lib, kN);
            print_vec("speech_probability", prob_actual, kN);
            print_vec("filter(post)", ns->wiener_filter.filter, kN);
            print_vec("conservative", ns->noise_estimator.conservative_noise_spectrum, kN);
        }

        /* 首次非有限值出现时, 打印该 bin 的完整轨迹与链路 */
        for (k = 0; k < NWATCH; ++k) {
            if (first_bad_frame[k] == fi) {
                int b = first_bad_bin[k];
                int h;
                printf("  *** 首次非有限值: frame=%d 量=%s bin=%d bits=0x%08X %s ***\n",
                       fi, kWName[k], b, (unsigned)first_bad_bits[k],
                       first_bad_isnan[k] ? "(NaN)" : "(Inf)");
                printf("    bin %d 最近 %d 帧轨迹(旧->新):\n", b, NHIST);
                for (h = 0; h < NHIST; ++h) {
                    int idx = (hist_pos[b] + 1 + h) % NHIST;
                    BinStep* e = &hist[b][idx];
                    if (e->fi < 0) { continue; }
                    printf("      fi=%-4d avg_pre=%-16.9g post=%-13.7g prior=%-13.7g tmp1=%-12.6g tmp2=%-12.6g bessel=%-14.8g avg_post=%-16.9g inv=[0x%08X] prob=[0x%08X]\n",
                           e->fi, (double)e->avg_pre, (double)e->post, (double)e->prior,
                           (double)e->tmp1, (double)e->tmp2, (double)e->bessel,
                           (double)e->avg_post, (unsigned)e->inv_bits, (unsigned)e->prob_bits);
                }
                dump_bin(fc, pre, avg_actual, prob_actual, b, fi, fc->gain_prior);
                if (is_bad_f(fc->inv_lib[b])) {
                    chain_report("fastexp 链路:", -avg_actual[b]);
                }
                printf("\n");
            }
        }
    }

    printf("  ---- %s 汇总 ----\n", path);
    for (i = 0; i < NWATCH; ++i) {
        const char* nm = (i == 13) ? "avg_log_lrt(rerun)" : kWName[i];
        if (first_bad_frame[i] >= 0) {
            printf("    首次非有限值 %-22s: frame=%-5lld bin=%-4d bits=0x%08X %s\n",
                   nm, first_bad_frame[i], first_bad_bin[i], (unsigned)first_bad_bits[i],
                   first_bad_isnan[i] ? "NaN" : "Inf");
        } else {
            printf("    首次非有限值 %-22s: 无(全程有限)\n", nm);
        }
    }
    printf("    runner 口径首次报错帧 = %lld (%s) ; 重算位级不一致总数 = %lld\n",
           first_runner_bad, (runner_bad_isnan > 0) ? "NaN" : "Inf/其他",
           total_mismatch);
    printf("    avg_log_lrt 首次 NaN 帧 = %lld ; fastexp 首次 NaN 帧 = %lld ; speech_probability 首次 NaN 帧 = %lld\n",
           avg_first_bad, inv_first_bad, prob_first_bad);
    printf("    quantile 出现负值的首个帧 = %lld (无则 -1)\n", noise_neg_first);
    printf("    fastexp 负值区(avg_log_lrt>88.3604)首个帧 = %lld (该帧受影响 bin 数 = %d)\n",
           exp_regime_first, exp_regime_bins);
    printf("    inv_lrt<0 首个帧 = %lld ; speech_probability 越界(<0或>1)首个帧 = %lld\n",
           inv_neg_first, prob_oou_first);
    printf("    全部 129 个 prob 恒为 1.0 的首个帧 = %lld\n", prob_all_one_first);

    /* ---- NaN 分布(point 2) ---- */
    printf("    NaN 帧合计 = %lld / %lld 帧 (%.3f%%) ; inv_lrt 出现 NaN 的帧 = %lld ; avg_log_lrt>88.3604 的帧 = %lld\n",
           nan_frame_n, nframes_run, (nframes_run > 0) ? (100.0 * (double)nan_frame_n / (double)nframes_run) : 0.0,
           frames_with_inv_nan, frames_with_regime);
    {
        long long j;
        long long lim = (nan_frame_n < 120) ? nan_frame_n : 120;
        printf("    NaN 帧号列表(前 %lld 个) = [", lim);
        for (j = 0; j < lim; ++j) { printf("%s%lld", (j == 0) ? "" : ",", nan_frame_idx[j]); }
        printf("%s]\n", (nan_frame_n > lim) ? ", ..." : "");
        printf("    相邻 NaN 帧间隔(最后一个不算) = [");
        for (j = 1; j < lim; ++j) {
            printf("%s%lld", (j == 1) ? "" : ",", nan_frame_idx[j] - nan_frame_idx[j - 1]);
        }
        printf("]\n");
        printf("    NaN bin 直方图(仅列出出现过的 bin): ");
        for (j = 0; j < kN; ++j) {
            if (nan_bin_hist[j] > 0) { printf("bin%d:%d ", (int)j, nan_bin_hist[j]); }
        }
        printf("\n");
    }
    printf("    avg_log_lrt 全文件帧级最大值 = %.9g @frame %lld bin %d ; 该 bin 的 inv_lrt = %.9g [0x%08X]\n",
           (double)global_max_avg, global_max_avg_frame, global_max_avg_bin,
           (double)global_max_avg_inv, (unsigned)global_max_avg_inv_bits);

    free(in24); free(in_frame); free(out_frame); free(pre); free(fc); free(hist);
    free(ctx);
    return 0;
}

/* ---------------------------------------------------------------------------
 * sweep: 目录内所有 wav
 * -------------------------------------------------------------------------*/
static int name_cmp(const void* a, const void* b)
{
    return strcmp(*(const char* const*)a, *(const char* const*)b);
}

static int sweep_mode(const char* dir, int max_frames)
{
    char      pattern[1024];
    WIN32_FIND_DATAA fd;
    HANDLE    h;
    char**    items = NULL;
    int       count = 0, cap = 0;
    int       i;

    snprintf(pattern, sizeof(pattern), "%s\\*.wav", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) { printf("no *.wav in %s\n", dir); return 1; }
    do {
        size_t len;
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0u) { continue; }
        len = strlen(fd.cFileName);
        if (len <= 4u) { continue; }
        if (_stricmp(fd.cFileName + (len - 4u), ".wav") != 0) { continue; }
        if (count == cap) {
            int    nc = (cap == 0) ? 16 : cap * 2;
            char** p = (char**)realloc(items, (size_t)nc * sizeof(char*));
            if (p == NULL) { FindClose(h); return 1; }
            items = p; cap = nc;
        }
        items[count] = (char*)malloc(len + 1u);
        strcpy(items[count], fd.cFileName);
        count++;
    } while (FindNextFileA(h, &fd) != 0);
    FindClose(h);
    qsort(items, (size_t)count, sizeof(char*), name_cmp);

    printf("============ sweep: %s files=%d max_frames=%d ============\n", dir, count, max_frames);
    for (i = 0; i < count; ++i) {
        char path[1200];
        snprintf(path, sizeof(path), "%s\\%s", dir, items[i]);
        printf("\n>>> %s\n", items[i]);
        diag_run_file(path, max_frames, 0, -1, 1);
    }
    for (i = 0; i < count; ++i) { free(items[i]); }
    free(items);
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("usage: _diag_nan.exe probe\n");
        printf("       _diag_nan.exe frames <wav> <max_frames> [lo] [hi]\n");
        printf("       _diag_nan.exe sweep  <input_dir> <max_frames>\n");
        return 2;
    }
    if (strcmp(argv[1], "probe") == 0) {
        probe_mode();
        return 0;
    }
    if (strcmp(argv[1], "frames") == 0 && argc >= 4) {
        int lo = (argc >= 5) ? atoi(argv[4]) : 170;
        int hi = (argc >= 6) ? atoi(argv[5]) : 180;
        return diag_run_file(argv[2], atoi(argv[3]), lo, hi, 0);
    }
    if (strcmp(argv[1], "sweep") == 0 && argc >= 4) {
        return sweep_mode(argv[2], atoi(argv[3]));
    }
    printf("bad args\n");
    return 2;
}
