# -*- coding: utf-8 -*-
"""
ENC 降噪算法移植审查 - 图表生成脚本 (Pillow 实现)

环境: Python 3 + Pillow。当前解释器不具备 matplotlib, 因此本脚本全部使用
      Pillow 绘制, 不依赖额外绘图库。

运行:
    python gen_figures.py

输出 (全部写入 ../figures/):
    Fig001_ENC处理链路与缺陷定位.png
    Fig002_D1污染传导链.png
    Fig003_ENC双缓冲时序与丢帧.png
    Fig004_模块级比对结论矩阵.png
    Fig005_数值误差量化.png
"""

import math
import os
import struct

import numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
FIGDIR = os.path.normpath(os.path.join(HERE, "..", "figures"))
os.makedirs(FIGDIR, exist_ok=True)

FONT_R = "C:/Windows/Fonts/msyh.ttc"
FONT_B = "C:/Windows/Fonts/msyhbd.ttc"
FONT_M = "C:/Windows/Fonts/consola.ttf"

_fc = {}


def F(size, bold=False, mono=False):
    key = (size, bold, mono)
    if key not in _fc:
        path = FONT_M if mono else (FONT_B if bold else FONT_R)
        _fc[key] = ImageFont.truetype(path, size)
    return _fc[key]


# ---------------------------------------------------------------- 绘图原语

def new_canvas(w, h, bg="#ffffff"):
    img = Image.new("RGB", (w, h), bg)
    return img, ImageDraw.Draw(img)


def tw(d, text, font):
    b = d.textbbox((0, 0), text, font=font)
    return b[2] - b[0], b[3] - b[1]


def box(d, x, y, w, h, text, fill="#eaf2fb", edge="#2c6fa8", font=None,
        tc="#12314d", radius=8, lw=2):
    font = font or F(17)
    d.rounded_rectangle([x, y, x + w, y + h], radius=radius, fill=fill,
                        outline=edge, width=lw)
    lines = text.split("\n")
    lh = font.size + 5
    ty = y + (h - lh * len(lines)) / 2
    for ln in lines:
        w_, _ = tw(d, ln, font)
        d.text((x + (w - w_) / 2, ty), ln, font=font, fill=tc)
        ty += lh


def arrow(d, x1, y1, x2, y2, color="#2c6fa8", lw=3, head=11):
    d.line([x1, y1, x2, y2], fill=color, width=lw)
    a = math.atan2(y2 - y1, x2 - x1)
    d.polygon([(x2, y2),
               (x2 - head * math.cos(a - 0.4), y2 - head * math.sin(a - 0.4)),
               (x2 - head * math.cos(a + 0.4), y2 - head * math.sin(a + 0.4))],
              fill=color)


def title(d, text, sub=None, w=0):
    f = F(26, bold=True)
    wt, _ = tw(d, text, f)
    d.text((((w - wt) / 2) if w else 30, 20), text, font=f, fill="#10243a")
    if sub:
        fs = F(15)
        ws, _ = tw(d, sub, fs)
        d.text((((w - ws) / 2) if w else 30, 56), sub, font=fs, fill="#5a6b7c")


def legend_box(d, x, y, items, font=None, sw=16):
    font = font or F(14)
    for i, (c, t) in enumerate(items):
        yy = y + i * 24
        d.rectangle([x, yy, x + sw, yy + sw], fill=c, outline="#5a6b7c")
        d.text((x + sw + 8, yy - 1), t, font=font, fill="#33414f")


def wrap(d, text, font, maxw, maxlines=2):
    """按像素宽度折行, 超出 maxlines 时末行加省略号"""
    lines, cur = [], ""
    for ch in text:
        if tw(d, cur + ch, font)[0] > maxw:
            lines.append(cur)
            cur = ch
        else:
            cur += ch
    lines.append(cur)
    if len(lines) > maxlines:
        lines = lines[:maxlines]
        while tw(d, lines[-1] + "...", font)[0] > maxw and lines[-1]:
            lines[-1] = lines[-1][:-1]
        lines[-1] += "..."
    return lines


# ================================================================ Fig001

def fig001():
    W, H = 1680, 1140
    img, d = new_canvas(W, H)
    title(d, "Fig001  ENC 降噪算法处理链路与缺陷定位",
          "被审查代码: ENC/MainApp/source/driver/alg/dsp/enc/   |   "
          "调用方: app_i2s.c / drv_alg.c", W)

    LX, LW = 60, 620
    RX, RW = 790, 830
    y, BH, GAP = 112, 82, 14

    steps = [
        ("I2S / USB-ADC 原始数据 (int32 容器, 24bit 有效值)\n"
         "drv_alg.c:646   *fInAdcL = (float)(*i32InAdcL >> 8)",
         "#eef5ec", "#4a8b3a"),
        ("enc_in[960] 双缓冲采集\n"
         "drv_alg.c:670-677   48 样本/帧 × 10 帧 = 480",
         "#eef5ec", "#4a8b3a"),
        ("Enc_Process()                              enc.c:65\n"
         "ThreeBandFilterBank_Analysis(480 → 3×160)   three_band_filter_bank.c:125",
         "#eaf2fb", "#2c6fa8"),
        ("NoiseSuppressor_Analyze()          noise_suppressor.c:291\n"
         "FormExtendedFrame → ApplyFilterBankWindow → arm_rfft_fast_f32",
         "#eaf2fb", "#2c6fa8"),
        ("NoiseSuppressor_Process()          noise_suppressor.c:366\n"
         "WienerFilter_Update → IFFT → 加窗 → OverlapAndAdd → 上频带增益",
         "#eaf2fb", "#2c6fa8"),
        ("ThreeBandFilterBank_Synthesis(3×160 → 480)   :183\n"
         "结果经 gain_adjustment 加权后写入 enc_out[960]",
         "#eaf2fb", "#2c6fa8"),
        ("回写 pMem[2]   drv_alg.c:675   *fInAdcL = enc_out[...]\n"
         "→ 后续算法链输入 + USB-Mic 上传",
         "#eef5ec", "#4a8b3a"),
    ]

    ann = [
        ("✓ 帧长与块边界正确\n"
         "kFullBandSize=480 / kSplitBandSize=160 / kNsFrameSize=160\n"
         "kNumBands=3, 与原版 48kHz 配置一致",
         "#eef9ee", "#4a8b3a"),
        ("⚠ D2  中断触发可丢失 (严重)\n"
         "app_i2s.c:458-469 无条件翻转 enc_write,\n"
         "但 flagUser2IrqHandler==1 时丢弃本次触发;\n"
         "USER0_IRQn=1 优先级高于 USER2_IRQn=2, 可抢占",
         "#fdecea", "#c0392b"),
        ("✓ 三分频系数与 FilterCore 逐行等价\n"
         "kFilterCoeffs / kDctModulation 逐值相同;\n"
         "kStride=4, kMemorySize=15, 零滤波器 index 3/9 处理相同",
         "#eef9ee", "#4a8b3a"),
        ("✖ D1  Y[257] 未初始化 → 污染 signal_energy (严重)\n"
         "noise_suppressor.c:297   float Y[2*129] 为局部数组;\n"
         "arm_rfft_fast_f32 只写 Y[0..255], :331 仅回填 Y[256];\n"
         ":342 累加 signal_energy 时读入 Y[257] 栈垃圾",
         "#fdecea", "#c0392b"),
        ("✖ D1  同源缺陷 (Process 内 Y 亦未初始化)\n"
         "⚠ D5  输出钳位由 ±32767 放宽至 ±8388607 (:439);\n"
         "与 24bit 量纲匹配, 但原版 16bit 保护失效",
         "#fdecea", "#c0392b"),
        ("✓ 逆变换缩放已核实等价 (非缺陷)\n"
         "原版 rdft(-1) 输出 (N/2)·x[n], 乘 kScaling=2/N 得标准 IDFT;\n"
         "CMSIS RIFFT = merge_rfft + 128 点复逆变换(含 1/128 缩放),\n"
         "同样输出标准 IDFT。两条路径输出同一时域序列",
         "#eef9ee", "#4a8b3a"),
        ("⚠ D3  EncOnOff==0 时采集指针不递增\n"
         "drv_alg.c:672-676   48 次读取同一地址,\n"
         "→ enc_in 被填充为同一采样点的重复值",
         "#fff6e5", "#c98a12"),
    ]

    for i, (t, fill, edge) in enumerate(steps):
        yy = y + i * (BH + GAP)
        box(d, LX, yy, LW, BH, t, fill=fill, edge=edge, font=F(15))
        if i < len(steps) - 1:
            arrow(d, LX + LW / 2, yy + BH, LX + LW / 2, yy + BH + GAP,
                  color=edge)

    for i, (t, fill, edge) in enumerate(ann):
        yy = y + i * (BH + GAP)
        box(d, RX, yy, RW, BH, t, fill=fill, edge=edge, font=F(14))
        mid = yy + BH / 2
        d.line([LX + LW + 4, mid, RX - 6, mid], fill=edge, width=2)
        d.ellipse([RX - 9, mid - 4, RX - 1, mid + 4], fill=edge)

    d.text((LX + 8, y - 28), "主处理链路", font=F(17, bold=True), fill="#10243a")
    d.text((RX + 8, y - 28), "比对结论 / 缺陷定位", font=F(17, bold=True),
           fill="#10243a")

    legend_box(d, 60, H - 96,
               [("#4a8b3a", "核对一致"), ("#c98a12", "偏差 / 风险"),
                ("#c0392b", "确认缺陷")], font=F(15))
    d.text((420, H - 92),
           "注: 行号对应当前工作区文件内容; 基准为 "
           "Webrtc_Source/webrtc-audio-processing-1.0/webrtc/modules/audio_processing/ns/",
           font=F(13), fill="#5a6b7c")

    img.save(os.path.join(FIGDIR, "Fig001_ENC处理链路与缺陷定位.png"))
    print("Fig001 ok")


# ================================================================ Fig002

def fig002():
    W, H = 1680, 920
    img, d = new_canvas(W, H)
    title(d, "Fig002  D1 缺陷传导链: 未初始化栈变量如何改变降噪增益",
          "根因: noise_suppressor.c:340-344 累加 signal_energy 时读入未初始化的 Y[257]", W)

    chain = [
        ("NoiseSuppressor_Analyze\nnoise_suppressor.c:297",
         "float Y[2*kFftSizeBy2Plus1];\n局部数组, 无初值", "#fdecea", "#c0392b"),
        ("arm_rfft_fast_f32(...,Y,0)\nnoise_suppressor.c:330",
         "CMSIS 只写入 Y[0..255]\n:331 Y[256]=Y[1];  Y[1]=0", "#fdecea", "#c0392b"),
        ("signal_energy 累加\nnoise_suppressor.c:340-343",
         "Y[2i]²+Y[2i+1]², i=0..128\ni=128 时读 Y[256], Y[257]",
         "#f9d8d4", "#a5342a"),
        ("AdjustNormalization\nsignal_model_estimator.c:127",
         "diff_norm = (m·n + signal_energy)\n/ (n+1), n<200 每帧执行",
         "#fff6e5", "#c98a12"),
        ("ComputeSpectralDiff\nsignal_model_estimator.c:55",
         "spectral_diff = signal_var − cov²/noise_var\n再 / (diff_normalization + 1e-4)",
         "#fff6e5", "#c98a12"),
        ("features_.spectral_diff\nsignal_model_estimator.c:142",
         "0.3 一阶平滑输出\n作为 indicator2 的输入特征", "#fff6e5", "#c98a12"),
        ("speech_probability\nspeech_probability_estimator.c:76",
         "1/(1+gain_prior·exp(−avg_log_lrt))\n随特征整体偏移", "#fdecea", "#c0392b"),
        ("WienerFilter_Update\nwiener_filter.c:34",
         "filter = snr_prior/(osf + snr_prior)\n再被 minimum_attenuating_gain 截断",
         "#fdecea", "#c0392b"),
        ("降噪增益失真\n听感: 噪声残留 / 语音被啃 / 时变不稳",
         "结果写入 enc_out\n→ 回写 pMem[2] 送往 USB-Mic", "#f9d8d4", "#a5342a"),
    ]

    n = len(chain)
    cols, rows = 5, 2
    bw, bh = 292, 186
    gx, gy = 30, 78
    x0, y0 = 46, 118
    ft = F(15, bold=True)
    fs = F(13)

    for i, (t, sub, fill, edge) in enumerate(chain):
        r, c = divmod(i, cols)
        x = x0 + c * (bw + gx)
        y = y0 + r * (bh + gy)
        d.rounded_rectangle([x, y, x + bw, y + bh], radius=8, fill=fill,
                            outline=edge, width=2)
        ty = y + 12
        for ln in t.split("\n"):
            wl, _ = tw(d, ln, ft)
            d.text((x + (bw - wl) / 2, ty), ln, font=ft, fill="#10243a")
            ty += 21
        d.line([x + 22, ty + 5, x + bw - 22, ty + 5], fill=edge, width=1)
        ty += 15
        for ln in sub.split("\n"):
            wl, _ = tw(d, ln, fs)
            d.text((x + (bw - wl) / 2, ty), ln, font=fs, fill="#3d4b59")
            ty += 20

        if c < cols - 1 and i < n - 1:
            arrow(d, x + bw + 3, y + bh / 2, x + bw + gx - 3, y + bh / 2,
                  color=edge, lw=3)
        elif c == cols - 1 and i < n - 1:
            ax = x + bw / 2
            my = y + bh + gy / 2
            d.line([ax, y + bh, ax, my], fill=edge, width=3)
            d.line([ax, my, x0 + bw / 2, my], fill=edge, width=3)
            arrow(d, x0 + bw / 2, my, x0 + bw / 2, y + bh + gy - 5,
                  color=edge, lw=3)

    yb = y0 + rows * bh + (rows - 1) * gy + 34
    d.rounded_rectangle([46, yb, W - 46, yb + 108], radius=10,
                        fill="#f2f6fa", outline="#8fa6bb", width=2)
    d.text((66, yb + 12), "原版对照 (为什么原版没有这个问题)", font=F(16, bold=True),
           fill="#10243a")
    d.text((66, yb + 44),
           "原版 ns_fft.cc:36   imag[kFftSizeBy2Plus1-1] = 0;   "
           "→ i=128 项为 real[128]² + imag[128]² = Nyquist² + 0, 累加结果确定且可复现。",
           font=F(14), fill="#33414f")
    d.text((66, yb + 72),
           "原版 noise_suppressor.cc:344-348   先累加 signal_energy 再除以 129; "
           "移植版等价改写为乘以 kOneByFftSizeBy2Plus1, 该项本身无差异。",
           font=F(14), fill="#33414f")

    img.save(os.path.join(FIGDIR, "Fig002_D1污染传导链.png"))
    print("Fig002 ok")


# ================================================================ Fig003

def fig003():
    W, H = 1680, 790
    img, d = new_canvas(W, H)
    title(d, "Fig003  D2 缺陷: ENC 双缓冲时序与中断触发丢失",
          "调用方 app_i2s.c:319-469   |   中断优先级 USER0_IRQn=1 高于 "
          "USER1/USER2_IRQn=2", W)

    x0, x1 = 218, 1624
    TOTAL = 1440
    TOP, LH, LBOX = 214, 78, 66

    def X(s):
        return x0 + (x1 - x0) * s / TOTAL

    # ---- 顶部: 抢占说明 ----
    px = X(960)
    d.rounded_rectangle([px, 84, px + 452, 150], radius=8, fill="#fdecea",
                        outline="#c0392b", width=2)
    d.text((px + 12, 90), "USER0 (I2S DMA) 抢占 USER2:", font=F(13, bold=True),
           fill="#8f2c22")
    d.text((px + 12, 110), "enc_write 1→0 已完成翻转, 但此刻", font=F(13),
           fill="#8f2c22")
    d.text((px + 12, 128), "flagUser2IrqHandler==1  →  本次触发被丢弃",
           font=F(13, bold=True), fill="#c0392b")

    # ---- 切换点标注 ----
    for s, txt1, txt2, col in (
            (480, "切换 enc_write 0→1", "触发 USER2 处理采集段 A", "#4a8b3a"),
            (960, "切换 enc_write 1→0", "触发 USER2 处理采集段 B", "#c0392b")):
        bx = X(s) + 8
        d.rounded_rectangle([bx, 158, bx + 250, 200], radius=6,
                            fill="#ffffff", outline=col, width=2)
        d.text((bx + 10, 163), txt1, font=F(13, bold=True), fill=col)
        d.text((bx + 10, 181), txt2, font=F(12), fill="#33414f")
        # 竖线
        for yy in range(206, 442, 10):
            d.line([X(s), yy, X(s), yy + 6], fill="#c98a12", width=2)

    # ---- 泳道 ----
    lanes = [
        (0, "enc_write = 0", "写 enc_in[0..479]", "读 enc_out[0..479]"),
        (1, "enc_write = 1", "写 enc_in[480..959]", "读 enc_out[480..959]"),
        (2, "enc_write = 0", "写 enc_in[960..1439]", "读 enc_out[0..479]"),
    ]
    for i, l0, l1, l2 in lanes:
        ly = TOP + i * LH
        d.text((20, ly + 2), l0, font=F(14, bold=True), fill="#10243a")
        d.text((20, ly + 22), l1, font=F(13), fill="#33414f")
        d.text((20, ly + 40), l2, font=F(13), fill="#33414f")

    def seg(a, b, lane, text, fill, edge, tc="#12314d"):
        y = TOP + lane * LH
        d.rectangle([X(a), y, X(b), y + LBOX], fill=fill, outline=edge,
                    width=2)
        lines = text.split("\n")
        ty = y + (LBOX - len(lines) * 21) / 2
        for ln in lines:
            wl, _ = tw(d, ln, F(15))
            d.text((X(a) + (X(b) - X(a) - wl) / 2, ty), ln, font=F(15), fill=tc)
            ty += 21

    seg(0, 480, 0, "采集段 A (480 样本)", "#dbe9f7", "#2c6fa8")
    seg(480, 960, 1, "采集段 B (480 样本)\n✖ USER2 触发被丢弃, 本段永不被处理",
        "#fbe3e0", "#c0392b", "#8f2c22")
    seg(960, 1440, 2, "采集段 A' (480 样本)\n输出 = 陈旧数据 (再早 960 样本)",
        "#fff4e0", "#c98a12", "#8a5f00")

    # 处理结果箭头
    arrow(d, X(240), TOP + LBOX + 4, X(240), TOP + LH - 4, color="#4a8b3a", lw=3)
    d.text((X(240) + 12, TOP + LBOX + 6),
           "USER2: Enc_Process(A) → enc_out[0..479] 已更新", font=F(12),
           fill="#1e5c22")

    # ---- 时间轴 ----
    ty = TOP + 3 * LH + 14
    d.line([x0, ty, x1, ty], fill="#33414f", width=2)
    for s in range(0, TOTAL + 1, 480):
        d.line([X(s), ty - 6, X(s), ty + 6], fill="#33414f", width=2)
        lbl = "t = %d 样本" % s
        wl, _ = tw(d, lbl, F(13))
        d.text((X(s) - wl / 2, ty + 12), lbl, font=F(13), fill="#33414f")
    d.text((x0, ty + 44),
           "时间轴以 48 kHz 采样计数: 480 样本 = 10 ms; I2S 每 48 样本 (1 ms) "
           "触发一次 USER0_IRQHandler。",
           font=F(14), fill="#33414f")

    # ---- 触发条件说明 ----
    yb = ty + 78
    d.rounded_rectangle([46, yb, W - 46, yb + 128], radius=10, fill="#f2f6fa",
                        outline="#8fa6bb", width=2)
    d.text((66, yb + 10), "为什么检查 flagUser2IrqHandler 无法保护", font=F(16, bold=True),
           fill="#10243a")
    d.text((66, yb + 40),
           "① Cortex-M NVIC 中优先级数值越小越高: USER0=1 可以抢占 USER2=2; "
           "而两个查询/置位都在 USER2 自身的 ISR 内完成, 同优先级下该检查形同虚设。",
           font=F(14), fill="#33414f")
    d.text((66, yb + 66),
           "② 只要 Enc_Process(480 样本) 单次耗时超过 1 ms, USER0 必然在 USER2 "
           "执行期间进入, 检查条件成立, 本次处理即被丢弃。",
           font=F(14), fill="#33414f")
    d.text((66, yb + 92),
           "③ 即使未发生抢占, NVIC 的 pending 位不会累积: 若两次触发之间处理尚未启动, "
           "第二次触发与第一次合并, 同样造成丢帧。",
           font=F(14), fill="#33414f")

    img.save(os.path.join(FIGDIR, "Fig003_ENC双缓冲时序与丢帧.png"))
    print("Fig003 ok")


# ================================================================ Fig004

def fig004():
    rows = [
        ("ns_common.h 常量", "一致",
         "kFftSize/kNsFrameSize/kOverlapSize/kNumBands/启动阈值 全部相同"),
        ("noise_suppressor.c 窗口与帧操作", "一致",
         "kBlocks160w256FirstHalf[96] 逐值相同; FormExtendedFrame / OverlapAndAdd / DelaySignal / ApplyFilterBankWindow 逐行等价"),
        ("noise_suppressor.c ComputeSnr", "一致",
         "参数 (post_snr, prior_snr) 顺序在声明/定义/调用三处自洽, 数据流正确"),
        ("noise_suppressor.c ComputeUpperBandsGain", "一致",
         "kNumAvgBins=32; 以 tanhf 替代双精度 tanh, 精度略高"),
        ("noise_suppressor.c FFT 缩放", "一致",
         "两实现均输出标准 IDFT: 原版 rdft(-1) 输出 (N/2)·x[n] 后乘 2/N; CMSIS RIFFT 内部含 1/fftLen (128) 缩放"),
        ("noise_suppressor.c signal_energy", "缺陷 D1",
         "Y[257] 未初始化仍被累加, 污染 signal_energy (noise_suppressor.c:297 / 342)"),
        ("noise_suppressor.c 输出钳位", "偏差 D5",
         "由 ±32767 放宽为 ±8388607 (:439), 与 24bit 量纲匹配但失去原版保护"),
        ("wiener_filter.c", "一致",
         "Update / ComputeOverallScalingFactor 逐行等价; kBLim=0.5, 系数 1.3 / 0.3 相同"),
        ("noise_estimator.c", "一致",
         "log_table[124] 等价于原版 log_table[129] 的 i>=5 段; PreUpdate / PostUpdate 逐行等价"),
        ("quantile_noise_estimator.c", "一致",
         "kSimult=3, num_updates 初值=1 与原版相同; counter 首值 66/133/199 vs 66/133/200"),
        ("histograms.c / .h", "一致",
         "kHistogramSize=1000, bin 尺寸 0.1 / 0.05 / 0.1 相同"),
        ("signal_model.c / prior_signal_model.c", "一致",
         "lrt / spectral_flatness / spectral_diff / avg_log_lrt 初值全部相同"),
        ("prior_signal_model_estimator.c", "一致",
         "UpdateLrt / FindFirstOfTwoLargestPeaks 逐行等价; 阈值 0.3×500, 0.6, 0.16, 0.95, 0.1 相同"),
        ("signal_model_estimator.c", "一致",
         "AdjustNormalization / Update 等价; kFeatureUpdateWindowSize=500 相同"),
        ("speech_probability_estimator.c", "一致",
         "kWidthPrior0=4; 以 tanhf 替代 tanh; 概率下限 0.01 相同"),
        ("suppression_params.c", "一致",
         "level 0-3 的 over_subtraction_factor / minimum_attenuating_gain / use_attenuation_adjustment 完全相同"),
        ("three_band_filter_bank.c", "一致",
         "kFilterCoeffs / kDctModulation 逐值相同; FilterCore 三分支循环逐行等价; 零滤波器 index 3/9 跳过逻辑相同"),
        ("fast_math.c 开方", "偏差 D6",
         "原版直接调用精确 sqrtf; ENC 用位近似加一次牛顿迭代, 最大相对误差 0.096%"),
        ("fast_math.c 指数", "偏差 D6",
         "原版 Pow2Approximation 为精确 powf; ENC fastpow2 为位近似, 误差 1.4%~3%, 与原版 ExpApproximation 的 0.4%~3.4% 同量级"),
        ("fast_math.c log2 偏置", "偏差 D6",
         "126.9420151 (ENC) vs 126.942695 (原版), 引入 +6.8e-4 的系统性 log2 偏移"),
        ("enc.c / three_band_filter_bank.h", "偏差 D7",
         "g_split_data 清零由 Analysis 内部移至 Enc_Process; 附带 __ALIGNED(4) 与 - 副本.c 冗余文件"),
        ("app_i2s.c 中断调度", "缺陷 D2",
         "enc_write 无条件翻转但触发可被丢弃, 造成双缓冲错位"),
        ("drv_alg.c ENC 采集循环", "缺陷 D3",
         "EncOnOff==0 时 fInAdcL 不递增, enc_in 被填充为重复样本"),
        ("多通道支持", "设计简化",
         "原版支持 num_channels>1 与跨通道滤波器聚合; 移植版为单通道, 48kHz 下 kNumBands 固定为 3"),
    ]

    W = 1900
    rh = 54
    H = 214 + rh * len(rows)
    img, d = new_canvas(W, H)
    title(d, "Fig004  模块级比对结论矩阵",
          "基准: webrtc-audio-processing-1.0/webrtc/modules/audio_processing/ns/"
          "  +  audio_processing/three_band_filter_bank.cc  +  ns_fft.cc", W)

    cx = [46, 610, 800, W - 46]
    hy = 106
    color = {"一致": ("#eaf7ec", "#3d7f33"), "缺陷 D1": ("#fdecea", "#c0392b"),
             "缺陷 D2": ("#fdecea", "#c0392b"), "缺陷 D3": ("#fff6e5", "#c98a12"),
             "偏差 D5": ("#fff6e5", "#c98a12"), "偏差 D6": ("#fff6e5", "#c98a12"),
             "偏差 D7": ("#f7f7f7", "#7a7a7a"), "设计简化": ("#eef1f6", "#5a6b7c")}

    d.rectangle([cx[0], hy, cx[3], hy + 42], fill="#dbe4ee", outline="#8fa6bb",
                width=2)
    for i, t in enumerate(["模块 / 比对对象", "结论", "依据与差异明细"]):
        d.text((cx[i] + 14, hy + 10), t, font=F(16, bold=True), fill="#10243a")
    d.line([cx[1], hy, cx[1], hy + 42 + rh * len(rows)], fill="#8fa6bb", width=2)
    d.line([cx[2], hy, cx[2], hy + 42 + rh * len(rows)], fill="#8fa6bb", width=2)

    for i, (name, verdict, detail) in enumerate(rows):
        y = hy + 42 + i * rh
        fill, edge = color.get(verdict, ("#ffffff", "#8fa6bb"))
        if i % 2 == 0:
            d.rectangle([cx[0], y, cx[3], y + rh], fill="#fbfcfd")
        d.rectangle([cx[1], y, cx[2], y + rh], fill=fill)
        d.text((cx[0] + 14, y + 16), name, font=F(14), fill="#12314d")
        vw, _ = tw(d, verdict, F(14, bold=True))
        d.text((cx[1] + (cx[2] - cx[1] - vw) / 2, y + 16), verdict,
               font=F(14, bold=True), fill=edge)
        lines = wrap(d, detail, F(13), cx[3] - cx[2] - 32, maxlines=2)
        for j, ln in enumerate(lines):
            d.text((cx[2] + 16, y + (11 if len(lines) == 2 else 18) + j * 19),
                   ln, font=F(13), fill="#3d4b59")
        d.line([cx[0], y + rh, cx[3], y + rh], fill="#e2e8ee", width=1)

    d.text((46, H - 46),
           "统计: 逐行等价模块 16 项   |   确认缺陷 3 项 (D1/D2/D3)   |   "
           "数值与实现偏差 4 项 (D5/D6/D7)   |   设计简化 1 项",
           font=F(15, bold=True), fill="#10243a")

    img.save(os.path.join(FIGDIR, "Fig004_模块级比对结论矩阵.png"))
    print("Fig004 ok")


# ================================================================ Fig005

def fig005():
    def f2i(f):
        return struct.unpack("<i", struct.pack("<f", f))[0]

    def i2f(i):
        return struct.unpack("<f", struct.pack("<i", i & 0xFFFFFFFF))[0]

    def fastsqrt(x):
        v = (f2i(x) >> 1) + 532487669
        vf = i2f(v)
        return (vf + x / vf) * 0.5 if vf != 0.0 else 0.0

    def fastlog2_nxp(x):
        return float(f2i(x)) * 0.00000011920929 - 126.9420151

    def fastlog2_rtc(x):
        return float(f2i(x)) * 1.1920929e-7 - 126.942695

    def fastpow2(x):
        return i2f(int(8388608.0 * (x + 126.9420151)))

    def fastexp(x):
        return fastpow2(fastlog2_nxp(10.0) * (x * 0.4342945))

    def exp_rtc(x):
        return math.pow(2.0, x * 0.4342944819 * fastlog2_rtc(10.0))

    W, H = 1700, 790
    img, d = new_canvas(W, H)
    title(d, "Fig005  fast_math.c 近似函数误差量化",
          "原版 fast_math.cc 的 SqrtFastApproximation / Pow2Approximation "
          "直接调用 sqrtf / powf; ENC 版改为位近似", W)

    # ---------- 左: sqrt 误差曲线 ----------
    px0, py0, pw, ph = 110, 156, 660, 430
    d.rectangle([px0, py0, px0 + pw, py0 + ph], fill="#fcfdfe",
                outline="#8fa6bb", width=2)
    for i in range(5):
        yy = py0 + ph - i * ph / 4
        d.line([px0, yy, px0 + pw, yy], fill="#e6ebf0", width=1)
        d.text((px0 - 64, yy - 9), "%.3f%%" % (i * 0.03), font=F(12),
               fill="#5a6b7c")
    for i in range(6):
        xx = px0 + i * pw / 5
        d.line([xx, py0, xx, py0 + ph], fill="#e6ebf0", width=1)

    a = (math.log10(1e4) + 2) / 20.0 * pw + px0
    b = (math.log10(7e13) + 2) / 20.0 * pw + px0
    ov = Image.new("RGB", (int(b - a), ph), "#3498db")
    img.paste(Image.blend(img.crop((int(a), py0, int(b), py0 + ph)), ov, 0.13),
              (int(a), py0))

    xs = np.logspace(-2, 18, 1600)
    pts = []
    for x in xs:
        x = float(x)
        r = abs(fastsqrt(x) - math.sqrt(x)) / math.sqrt(x) * 100
        lx = (math.log10(x) + 2) / 20.0
        pts.append((px0 + lx * pw, py0 + ph - min(r / 0.12, 1.0) * ph))
    d.line(pts, fill="#c0392b", width=3, joint="curve")

    d.text(((a + b) / 2 - 72, py0 + 16), "24bit 信号 FFT\n平方和量级",
           font=F(13), fill="#2471a3")
    d.text((px0 + 6, py0 - 32), "fastsqrt 相对误差 (对比精确 sqrtf)",
           font=F(16, bold=True), fill="#10243a")
    d.text((px0 - 34, py0 + ph + 14), "1e-2", font=F(12), fill="#5a6b7c")
    d.text((px0 + pw - 42, py0 + ph + 14), "1e18", font=F(12), fill="#5a6b7c")
    d.text((px0 + pw / 2 - 130, py0 + ph + 36),
           "输入 x = y[2i]² + y[2i+1]² (对数轴)", font=F(13), fill="#33414f")
    d.text((px0 + 22, py0 + 40), "峰值 0.096%", font=F(15, bold=True),
           fill="#c0392b")

    # ---------- 右: exp 误差柱状 ----------
    qx0, qy0, qw, qh = 900, 156, 700, 430
    d.rectangle([qx0, qy0, qx0 + qw, qy0 + qh], fill="#fcfdfe",
                outline="#8fa6bb", width=2)
    for i in range(5):
        yy = qy0 + qh - i * qh / 4
        d.line([qx0, yy, qx0 + qw, yy], fill="#e6ebf0", width=1)
        d.text((qx0 - 56, yy - 9), "%.1f%%" % (i * 0.9), font=F(12),
               fill="#5a6b7c")

    xs2 = [1.0, 2.0, 5.0, 8.0, 11.5]
    enc = [abs(fastexp(x) - math.exp(x)) / math.exp(x) * 100 for x in xs2]
    rtc = [abs(exp_rtc(x) - math.exp(x)) / math.exp(x) * 100 for x in xs2]
    slot = qw / len(xs2)
    bw = slot * 0.28
    for i in range(len(xs2)):
        cx = qx0 + slot * (i + 0.5)
        for j, (val, col) in enumerate(((enc[i], "#c0392b"), (rtc[i], "#7f8c8d"))):
            hgt = min(val / 3.6, 1.0) * qh
            bx = cx - bw + j * bw
            d.rectangle([bx, qy0 + qh - hgt, bx + bw - 6, qy0 + qh],
                        fill=col, outline="#5a6b7c", width=1)
            d.text((bx + 2, qy0 + qh - hgt - 20), "%.2f" % val, font=F(11),
                   fill=col)
        d.text((cx - 18, qy0 + qh + 14), "x=%g" % xs2[i], font=F(13),
               fill="#33414f")

    d.text((qx0 + 6, qy0 - 32), "fastexp 相对误差 (对比精确 expf)",
           font=F(16, bold=True), fill="#10243a")
    legend_box(d, qx0 + 6, qy0 + qh + 44,
               [("#c0392b", "ENC fastpow2 位近似链路"),
                ("#7f8c8d", "原版 powf + FastLog2f 位近似链路")], font=F(13))
    d.text((qx0 + 360, qy0 + qh + 48), "两者误差同量级, 非本次移植引入",
           font=F(13, bold=True), fill="#3d7f33")

    d.text((110, H - 42),
           "结论: 幅度谱开方引入不超过 0.096% 的固定偏置, 指数链路误差与原版相当; "
           "两项均不足以解释降噪效果异常, 属次要偏差。",
           font=F(14), fill="#33414f")

    img.save(os.path.join(FIGDIR, "Fig005_数值误差量化.png"))
    print("Fig005 ok")


if __name__ == "__main__":
    fig001()
    fig002()
    fig003()
    fig004()
    fig005()
    print("all figures ->", FIGDIR)
