/* ============================================================================
 * arm_math_compat.c —— CMSIS-DSP 实数 FFT 的 PC 侧行为等价替代
 *
 * 目的: 让从固件移植出来的 NS 算法(ENC/MainApp/.../dsp/enc/)在 PC(MSVC) 上
 *       复现与固件一致的计算行为。本文件是全部基线对比的可信度支点。
 *
 * 必须严格复刻的行为(计划书 2.4 节):
 *   正向 arm_rfft_fast_f32(S, p, pOut, 0):
 *       无缩放标准 DFT  X[k] = sum_{n=0}^{255} x[n] * e^{-j*2*pi*k*n/256}
 *       压缩输出  pOut[0]    = Re X[0]      (直流)
 *                 pOut[1]    = Re X[128]    (Nyquist)
 *                 pOut[2k]   = Re X[k]      (k = 1..127)
 *                 pOut[2k+1] = Im X[k]      (k = 1..127)
 *   逆向 arm_rfft_fast_f32(S, p, pOut, 1):
 *       输入为与正向输出完全相同的压缩格式;
 *       标准 IDFT  pOut[n] = (1/256) * sum_{k=0}^{255} X[k] * e^{+j*2*pi*k*n/256}
 *
 * 实现路线(与 CMSIS 内部的"原地实数 radix-4/radix-2 + 拆合"结构不同, 但输出
 * 逐点等价): 对实数序列 x[n] 构造复数序列 z[n] = x[n] + j*0, 直接做 256 点
 * 复数 radix-2 Cooley-Tukey FFT(含位反转重排), 取其实部虚部拼装压缩格式。
 * 逆向为对称操作: 由压缩格式重建完整 256 点复数谱, 做共轭方向 FFT 后取实部并
 * 施加 1/256 缩放。
 *
 * 禁止用标量乘积推导缩放: 这里走的是完整数学路径, 唯一保证的性质是
 * riff(fft(x)) == x 与标准 DFT/IDFT 的定义式。
 *
 * 精度: 全程 double 运算, 仅在读写 float 缓冲时做一次转换。相对于 CMSIS 的
 *       float 路径, 本实现误差更低, 不会掩盖被测算法的真实行为。
 * ==========================================================================*/

#include "arm_math_compat.h"

#include <math.h>
#include <string.h>

#define NS_RFFT_LEN 256
#define NS_CFFT_LEN 128

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---------------------------------------------------------------------------
 * 旋转因子表: 按长度缓存, 仅在长度变化时重算。多实例共用, 只读访问。
 * -------------------------------------------------------------------------*/
static unsigned char s_w_ready[NS_RFFT_LEN + 1] = {0}; /* 该长度是否已算 */
static double s_cos_w[NS_RFFT_LEN / 2];
static double s_sin_w[NS_RFFT_LEN / 2];

static void ns_prepare_twiddles(uint16_t len)
{
    uint16_t k;

    if (len > NS_RFFT_LEN || s_w_ready[len]) {
        return;
    }
    for (k = 0; k < (uint16_t)(len / 2); ++k) {
        double ang = -2.0 * M_PI * (double)k / (double)len;
        s_cos_w[k] = cos(ang);
        s_sin_w[k] = sin(ang);
    }
    s_w_ready[len] = 1;
}

/* ---------------------------------------------------------------------------
 * 原地复数 radix-2 Cooley-Tukey FFT。re/im 各 len 个元素, 输入按自然顺序,
 * 输出按自然顺序。forward != 0 时使用 e^{-j...}(正向), 否则使用 e^{+j...}。
 * 本函数不做任何缩放。
 * -------------------------------------------------------------------------*/
static void ns_cfft(double* re, double* im, uint16_t len, int forward)
{
    uint16_t i, j, m, step;
    uint16_t n = len;
    double sgn = forward ? 1.0 : -1.0;

    /* 位反转重排 */
    j = 0;
    for (i = 1; i < n; ++i) {
        uint16_t bit = (uint16_t)(n >> 1);
        while (j & bit) {
            j = (uint16_t)(j ^ bit);
            bit = (uint16_t)(bit >> 1);
        }
        j = (uint16_t)(j | bit);
        if (i < j) {
            double t;
            t = re[i]; re[i] = re[j]; re[j] = t;
            t = im[i]; im[i] = im[j]; im[j] = t;
        }
    }

    /* 蝶形: 长度 2,4,...,n */
    for (m = 2; m <= n; m = (uint16_t)(m << 1)) {
        uint16_t half = (uint16_t)(m >> 1);
        uint16_t stride = (uint16_t)(n / m); /* 旋转因子索引步长 */
        for (i = 0; i < n; i = (uint16_t)(i + m)) {
            uint16_t idx = 0;
            for (step = 0; step < half; ++step) {
                /* idx == 0 时 W = 1 + j0, 直接取表值(sin 表中亦为 0) */
                double wr = s_cos_w[idx];
                double wi = sgn * s_sin_w[idx];
                double tr, ti, ur, ui;
                tr = re[i + step + half] * wr - im[i + step + half] * wi;
                ti = re[i + step + half] * wi + im[i + step + half] * wr;
                ur = re[i + step];
                ui = im[i + step];
                re[i + step] = ur + tr;
                im[i + step] = ui + ti;
                re[i + step + half] = ur - tr;
                im[i + step + half] = ui - ti;
                idx = (uint16_t)(idx + stride);
            }
        }
    }
}

/* ---------------------------------------------------------------------------
 * arm_rfft_fast_init_f32
 * 仅需记载长度。本兼容层只支持 256 点, 其它长度返回非零。
 * -------------------------------------------------------------------------*/
arm_status arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32* S, uint16_t fftLen)
{
    if (S == 0 || fftLen != NS_RFFT_LEN) {
        return -1;
    }
    S->Sint = 0;
    S->fftLenRFFT = fftLen;
    S->pTwiddleRFFT = 0;
    ns_prepare_twiddles(fftLen);
    return ARM_MATH_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * arm_rfft_fast_f32
 * ifftFlag == 0: p(256 实数) -> pOut(压缩频域 256 float), 无缩放
 * ifftFlag != 0: p(压缩频域) -> pOut(256 实数), 标准 IDFT, 含 1/256
 * 输入缓冲 p 不被修改(CMSIS 会原地改 p, 但本仓库调用点不依赖该副作用)。
 * -------------------------------------------------------------------------*/
void arm_rfft_fast_f32(arm_rfft_fast_instance_f32* S, float* p, float* pOut,
                       uint8_t ifftFlag)
{
    double re[NS_RFFT_LEN];
    double im[NS_RFFT_LEN];
    int i;

    if (S == 0 || p == 0 || pOut == 0 || S->fftLenRFFT != NS_RFFT_LEN) {
        if (pOut != 0) {
            memset(pOut, 0, sizeof(float) * NS_RFFT_LEN);
        }
        return;
    }

    ns_prepare_twiddles(NS_RFFT_LEN);

    if (ifftFlag == 0) {
        /* ---- 正向: 实数 -> 压缩频域, 无缩放 ---- */
        for (i = 0; i < NS_RFFT_LEN; ++i) {
            re[i] = (double)p[i];
            im[i] = 0.0;
        }
        ns_cfft(re, im, NS_RFFT_LEN, 1);

        pOut[0] = (float)re[0];                /* 直流 X[0].re */
        pOut[1] = (float)re[NS_RFFT_LEN / 2];  /* Nyquist X[128].re */
        for (i = 1; i < NS_CFFT_LEN; ++i) {
            pOut[2 * i] = (float)re[i];
            pOut[2 * i + 1] = (float)im[i];
        }
    } else {
        /* ---- 逆向: 压缩频域 -> 实数, 标准 IDFT(含 1/256) ---- */
        /* 由共轭对称性重建完整 256 点谱 */
        re[0] = (double)p[0];
        im[0] = 0.0;
        re[NS_RFFT_LEN / 2] = (double)p[1];
        im[NS_RFFT_LEN / 2] = 0.0;
        for (i = 1; i < NS_CFFT_LEN; ++i) {
            double rk = (double)p[2 * i];
            double ik = (double)p[2 * i + 1];
            re[i] = rk;
            im[i] = ik;
            re[NS_RFFT_LEN - i] = rk;
            im[NS_RFFT_LEN - i] = -ik;
        }
        ns_cfft(re, im, NS_RFFT_LEN, 0);

        for (i = 0; i < NS_RFFT_LEN; ++i) {
            pOut[i] = (float)(re[i] / (double)NS_RFFT_LEN);
        }
    }
}
