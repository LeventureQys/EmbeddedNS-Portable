# -*- coding: utf-8 -*-
"""
ENC 降噪算法移植审查 - 数值验证脚本 (无第三方绘图依赖)

用途:
  1. 量化 ENC 版 fast_math.c 近似函数相对原版精确函数的误差
     (原版 SqrtFastApproximation / Pow2Approximation 分别调用 sqrtf / powf)
  2. 核对 WebRTC Ooura rdft 与 CMSIS arm_rfft_fast_f32 逆变换缩放的等价性
  3. 量化 ENC 与 WebRTC 常数表差异 (fastlog2 偏置 / kOneByFftSizeBy2Plus1 /
     QuantileNoiseEstimator counter 初值)

对应图件由 gen_figures.py 的 fig005() 生成 (figures/Fig005_数值误差量化.png)。

运行:
    python ns_review_sim.py
输出:
    控制台文本 + 同目录 ns_review_sim_output.txt
"""

import math
import os
import random
import struct

HERE = os.path.dirname(os.path.abspath(__file__))


# ---------------------------------------------------------------- 位级工具

def f2i(f):
    return struct.unpack("<i", struct.pack("<f", f))[0]


def i2f(i):
    return struct.unpack("<f", struct.pack("<i", i & 0xFFFFFFFF))[0]


# ---------------------------------------------------------------- ENC 版近似

def fastsqrt(x):
    """ENC/MainApp/source/driver/alg/dsp/enc/fast_math.c: fastsqrt"""
    v = f2i(x)
    v = (v >> 1) + 532487669
    vf = i2f(v)
    if vf == 0.0:
        return 0.0
    return (vf + x / vf) * 0.5


def fastlog2_nxp(x):
    """fast_math.c: fastlog2, 偏置常数 126.9420151"""
    return float(f2i(x)) * 0.00000011920929 - 126.9420151


def fastlog2_rtc(x):
    """原版 fast_math.cc: FastLog2f, 偏置常数 126.942695"""
    return float(f2i(x)) * 1.1920929e-7 - 126.942695


def fastpow2(x):
    """fast_math.c: fastpow2 (位近似, 截断取整)"""
    return i2f(int(8388608.0 * (x + 126.9420151)))


def fastexp_nxp(x):
    """fast_math.c: fastexp = fastpow(10, x*0.4342945)"""
    return fastpow2(fastlog2_nxp(10.0) * (x * 0.4342945))


def exp_rtc(x):
    """原版 fast_math.cc: ExpApproximation = powf(2, x*0.4342944819*FastLog2f(10))"""
    return math.pow(2.0, x * 0.4342944819 * fastlog2_rtc(10.0))


# ---------------------------------------------------------------- 主流程

def main():
    lines = []

    def out(s=""):
        print(s)
        lines.append(s)

    out("=" * 78)
    out("ENC 降噪移植审查 - 数值误差量化")
    out("=" * 78)

    # --- 1 ---
    out()
    out("[1] signal_spectrum 开方: ENC fastsqrt  vs  原版 sqrtf")
    out("    ENC 调用点: noise_suppressor.c ComputeMagnitudeSpectrum()")
    random.seed(20260728)
    worst, worst_x = 0.0, 0.0
    for _ in range(300000):
        x = 10 ** random.uniform(-2, 18.0)
        ref = math.sqrt(x)
        got = fastsqrt(x)
        rel = abs(got - ref) / ref
        if rel > worst:
            worst, worst_x = rel, x
    out("    随机 3e5 样本 (量程 1e-2 .. 1e18, 覆盖 24bit 信号 FFT 平方和)")
    out("    最大相对误差 = %.4f%%  (出现在 x=%.3e)" % (worst * 100, worst_x))
    out("    典型量级:")
    for x in (1.0, 1e4, 1e7, 1e10, 1e14, 4.6e18):
        ref, got = math.sqrt(x), fastsqrt(x)
        out("      x=%9.3e   sqrtf=%13.6e   fastsqrt=%13.6e   rel=%7.4f%%"
            % (x, ref, got, abs(got - ref) / ref * 100))

    # --- 2 ---
    out()
    out("[2] fastexp 链路: ENC 位近似  vs  原版 (powf + FastLog2f 位近似)")
    out("    ENC 调用点: quantile_noise_estimator.c / noise_estimator.c /")
    out("                signal_model_estimator.c / speech_probability_estimator.c")
    xs = [1.0, 2.0, 5.0, 8.0, 11.5]
    out("    %-8s %-16s %-16s %-10s %-16s %-10s"
        % ("x", "expf(基准)", "ENC fastexp", "ENC误差",
           "原版ExpApproximation", "原版误差"))
    for x in xs:
        ref = math.exp(x)
        a, b = fastexp_nxp(x), exp_rtc(x)
        out("    %-8.1f %-16.6e %-16.6e %-9.4f%% %-16.6e %-9.4f%%"
            % (x, ref, a, abs(a - ref) / ref * 100,
               b, abs(b - ref) / ref * 100))
    out("    结论: 两者误差同量级 (1%~3%), 均源自位近似 log2 的偏置,")
    out("          不属于本次移植引入的新缺陷。")

    # --- 3 ---
    out()
    out("[3] fastlog2 偏置常数: ENC 126.9420151  vs  原版 126.942695")
    for x in (1.0, 10.0, 1e3, 1e7):
        n, r, t = fastlog2_nxp(x), fastlog2_rtc(x), math.log2(x)
        out("    x=%-9.1e  true=%9.6f   ENC偏差=%+.6f   原版偏差=%+.6f"
            % (x, t, n - t, r - t))
    out("    两常数之差 6.8e-4 表现为 log2 输出的系统性平移,")
    out("    经 exp 还原后相对影响约 5e-4, 可忽略。")

    # --- 4 ---
    out()
    out("[4] 标量常数与查表初值对比")
    out("    kOneByFftSizeBy2Plus1 : ENC 0.0077519f  原版 1.f/129.f = %.10f"
        % (1.0 / 129.0))
    out("                            相对偏差 = %.2e"
        % (abs(0.0077519 - 1.0 / 129.0) / (1.0 / 129.0)))
    out("    kOneBySimult          : ENC 0.3333333f  原版 1.f/3.f = %.10f"
        % (1.0 / 3.0))
    out("    QuantileNoiseEstimator counter[i]")
    out("      原版: floor(200*(i+1)*(1/3))   ENC: (int)(200*(i+1)*0.3333333f)")
    for i in range(3):
        rtc = math.floor(200.0 * (i + 1) * (1.0 / 3.0))
        nxp = int(200.0 * (i + 1) * 0.3333333)
        flag = "   <-- 相差 %d" % (rtc - nxp) if rtc != nxp else ""
        out("      i=%d : 原版=%3d   ENC=%3d%s" % (i, rtc, nxp, flag))

    # --- 5 ---
    out()
    out("[5] 逆变换缩放约定核对 (用于排除 IFFT 缺缩放假设)")
    out("    原版 NsFft::Ifft (ns_fft.cc:54-61):")
    out("        WebRtc_rdft(kFftSize, -1, ...) 之后整体乘")
    out("        kScaling = 2.f / kFftSize = %.10f" % (2.0 / 256.0))
    out("        Ooura rdft 对 half-complex 输入的逆变换输出 (N/2)*x[n],")
    out("        乘 2/N 后即标准 IDFT 时域序列 x[n]。")
    out("    ENC noise_suppressor.c:408 调用")
    out("        arm_rfft_fast_f32(&g_InstRfftF32, X, extended_frame, 1)")
    out("        CMSIS 路径 = merge_rfft_f32(压缩格式 -> 128 点复数)")
    out("                     + arm_cfft_f32(..., ifftFlag=1, ...)")
    out("        arm_cfft_f32 源码末尾执行  invL = 1.0f / L   (L = 128),")
    out("        官方文档: (The inverse transform includes a scale of 1/fftLen,")
    out("                   matches the textbook definition of the inverse FFT).")
    out("    手工推演 (两组边界输入):")
    for name, desc in (("x[n] = 1", "p = [256,0,0,...] -> pOut = [128+128j,0,...] "
                                    "-> 128 点逆变换含 1/128 -> 输出全 1"),
                       ("x[n] = delta[n]", "p 全 1 实部 -> pOut[k] = 1+0j 全通 "
                                           "-> 含 1/128 逆变换 -> 输出 delta")):
        out("      %-16s %s" % (name, desc))
    out("    两条路径最终都等于标准 IDFT, 输出同一时域序列")
    out("    => ENC 不存在 IFFT 缩放缺失问题。")
    out("    注意: merge_rfft 内部的 0.5 因子属于 (压缩格式 -> 复数格式) 的")
    out("          结构转换, 不是整体增益, 不能与 1/128 相乘后与原版 2/N")
    out("          直接比较 (0.5*1/128 = %.8f 与 2/256 = %.8f 不是同一层面的量)。"
        % (0.5 / 128.0, 2.0 / 256.0))

    # --- 6 ---
    out()
    out("[6] 24bit 量纲下 Y[257] 污染的数值敏感性说明")
    out("    ENC 输入量纲: I2S_INOUT_RESOLUTION_LEVEL=2 -> (float)(i32 >> 8),")
    out("    即 ±8388608 (24bit)。256 点 FFT 的 DC 分量可达 256*8.4e6 ≈ 2.1e9,")
    out("    平方和可达 4.6e18, 仍在 float 范围内(上限 3.4e38)。")
    out("    因此 Y[257] 一旦取到栈上残留的大值, signal_energy 会被同量级放大,")
    out("    而不是溢出为 inf —— 表现为持续的、非随机的特征偏移。")

    out()
    out("=" * 78)

    with open(os.path.join(HERE, "ns_review_sim_output.txt"), "w",
              encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print("文本输出 -> ns_review_sim_output.txt")


if __name__ == "__main__":
    main()
