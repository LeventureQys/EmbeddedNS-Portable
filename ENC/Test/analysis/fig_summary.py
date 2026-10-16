# -*- coding: utf-8 -*-
"""
FigT01 / FigT08 / FigT13 / FigT15 / FigT17 / FigT18 / FigT19 / FigT21 / FigT22

汇总与统计类图件。数据源见各函数 docstring。

| 图号 | 内容 | 数据源 | 版位 |
|------|------|--------|------|
| FigT01 | 9 文件时长与能量衰减（基线 vs 优化双柱） | summary_*.csv | 半页 |
| FigT08 | `Y[257]` 由 `signal_energy` 反推 | metrics_1.csv | 半页 |
| FigT13 | 分频带衰减五段柱状 | 输入 + 两份输出 WAV | 半页 |
| FigT15 | 钳位命中统计 | `clamp_hits` 列 | 半页 |
| FigT17 | 消融五组对比（2×2） | 五份 summary_*.csv | 整页 |
| FigT18 | 9 文件 `atten_db` 箱线图 | metrics_1..9.csv | 半页 |
| FigT19 | 单帧耗时分布 + RTF | `elapsed_us` 列 | 半页 |
| FigT21 | 无语音段 vs 语音段衰减 | metrics_1.csv | 半页 |
| FigT22 | 失败与边界案例 | 全部 CSV | 半页 |

独立运行：`python fig_summary.py`
"""

from __future__ import annotations

import os
import sys

import numpy as np

import common as K
import fig_style as S
from fig_style import C

LEVEL = K.PRIMARY_LEVEL
FRAME_US = 10000.0  # 10 ms 帧 → μs
BANDS = (0.0, 4000.0, 8000.0, 12000.0, 16000.0, 24000.0)
BAND_SHORT = ("0–4k", "4–8k", "8–12k", "12–16k", "16–24k")
#: 消融链标签（5 组）—— 统一取自 common，避免多处硬编码
VAR_LABEL = dict(K.VARIANT_LABEL)

#: FigT22 异常判据（与数据分布形态无关的物理边界）
ATTEN_MAX_DB = 60.0    # 噪声抑制器的合理衰减上限；超过即为异常记录
ATTEN_MIN_DB = -1.0    # 抑制器不应放大信号（输出大于输入）
CLAMP_LIMIT = 480      # 单帧样本数上限（10 ms × 48 kHz），超出即计数不可能


def _variants_in_dir(level: str = LEVEL) -> list[str]:
    """实际存在 summary 的变体（按计划顺序）。"""
    return [v for v in K.ABLATION_ORDER if K.exists_summary(v, level)]


def _color_of(variant: str) -> str:
    return C.VARIANT.get(variant, C.PRIMARY)


def _label_of(variant: str) -> str:
    return VAR_LABEL.get(variant, variant)


# --------------------------------------------------------------------------
# FigT01 —— 9 文件时长与能量衰减（半页）
# --------------------------------------------------------------------------

def fig_t01() -> str:
    base = K.summary_map(K.PRIMARY_VARIANT, LEVEL)
    opt = K.summary_map(K.COMPARE_VARIANT, LEVEL)
    if not base and not opt:
        print("SKIP FigT01 无 summary CSV（%s / %s）"
              % (K.summary_path(K.PRIMARY_VARIANT, LEVEL), K.summary_path(K.COMPARE_VARIANT, LEVEL)))
        return ""
    names = sorted(set(base) | set(opt), key=K._name_sort_key)
    cv = S.Canvas("half", "FigT01", "九文件时长与能量衰减汇总")
    rects = S.Axes.grid_rects((cv.pad_l, cv.top0 - 4, cv.W - cv.pad_r, cv.H - cv.bottom0 - 6),
                              nrow=2, ncol=1, gap_y=76)
    idx = np.arange(len(names), dtype=np.float64)

    # 上：时长（基线 / 优化 双柱，两者应等长）
    ax = cv.panel(rect=rects[0], ylabel="时长 (s)", xlabel="")
    ax.set_xlim(-0.6, len(names) + 0.1)
    dur_b = [base.get(n, {}).get("duration_s", np.nan) for n in names]
    dur_o = [opt.get(n, {}).get("duration_s", np.nan) for n in names]
    ax.bars(dur_b, centers=idx, width=0.36, offset=-0.19,
            color=C.BASELINE, label="时长 · 基线")
    ax.bars(dur_o, centers=idx, width=0.36, offset=0.19,
            color=C.OPTIMIZED, label="时长 · 优化")
    ax.autoscale()
    ax.grid(xlabels=None)
    ax.axes_line()
    ax.frame()
    ax.legend("tr", max_w=ax.width * 0.62)

    # 下：整段能量衰减（双柱）
    ax2 = cv.panel(rect=rects[1], ylabel="能量衰减 (dB)", xlabel="测试文件编号")
    ax2.set_xlim(-0.6, len(names) - 0.4)
    att_b = [base.get(n, {}).get("atten_db", np.nan) for n in names]
    att_o = [opt.get(n, {}).get("atten_db", np.nan) for n in names]
    ax2.bars(att_b, centers=idx, width=0.36, offset=-0.19,
             color=C.BASELINE, label="衰减 · 基线")
    ax2.bars(att_o, centers=idx, width=0.36, offset=0.19,
             color=C.OPTIMIZED, label="衰减 · 优化")
    ax2.autoscale()
    ax2.grid(xlabels=names)
    ax2.axes_line()
    ax2.frame()
    ax2.legend("tr", max_w=ax2.width * 0.62)

    for n in names:
        print("  %-3s 时长 基线%6.2f/优化%6.2f s  衰减 基线%6.2f/优化%6.2f dB"
              % (n, base.get(n, {}).get("duration_s", float("nan")),
                 opt.get(n, {}).get("duration_s", float("nan")),
                 base.get(n, {}).get("atten_db", float("nan")),
                 opt.get(n, {}).get("atten_db", float("nan"))))
    cv.note("上：时长（应两变体等长，验证输出与输入同长）；下：整段能量衰减 "
            "20·log10(in_rms_total/out_rms_total)。基线 %s / 优化 %s，等级 %s。"
            % (K.PRIMARY_VARIANT, K.COMPARE_VARIANT, LEVEL))
    return cv.save("FigT01_九文件衰减汇总.png")


# --------------------------------------------------------------------------
# FigT08 —— Y[257] 反推（半页）
# --------------------------------------------------------------------------

def fig_t08() -> str:
    """`|Y[257]|` 反推（D1 两套构建做差）。

    物理依据
    --------
    D1 机理：`i=128` 那一项读到了未初始化的 `Y[257]`。
      * 基线（D1 未修）：该项 = `Y[256]² + Y[257]²`
      * `d1`（D1 已修，`Y[257]` 显式置零）：该项 = `Y[256]² + 0²`
    而 `signal_energy = Σ_{i=0..128}(Y[2i]² + Y[2i+1]²) / 129`，故逐帧：

        signal_energy_baseline − signal_energy_d1 = Y[257]² / 129
        ⇒ |Y[257]| = sqrt(129 × Δ),   Δ = baseline − d1

    **注意**：`signal_energy` 与 `signal_spectral_sum` 量纲不同（平方和 vs 幅度和），
    两者相减无物理意义，因此不用差值法；只有同一量在两种构建下的差才有意义。

    局限（写入图注）
    ----------------
      * 量纲为 24bit 原始量纲（非归一化）。
      * **只能得到绝对值**：原始实现读到的是栈残留值，符号无确定语义。
      * `Δ ≤ 0` 的帧如实标注为「未检出 D1 效应」，不伪造为 0、不丢弃。
      * `|Y[257]|` 是 sqrt(129Δ) 形式，对 Δ 敏感（Δ 的第 5–6 位有效数字差异即显著）。
    """
    base_ok = K.exists_metrics(K.PRIMARY_VARIANT, LEVEL, "1")
    d1_ok = K.exists_metrics("d1", LEVEL, "1")
    if not base_ok:
        print("SKIP FigT08 缺少 %s" % K.metrics_path(K.PRIMARY_VARIANT, LEVEL, "1"))
        return ""
    if not d1_ok:
        print("SKIP FigT08 缺少 D1 对照 %s（无法做差，公式要求两套构建）"
              % K.metrics_path("d1", LEVEL, "1"))
        return ""

    mb = K.read_metrics(K.PRIMARY_VARIANT, LEVEL, "1")
    md = K.read_metrics("d1", LEVEL, "1")
    n = min(mb["signal_energy"].size, md["signal_energy"].size)
    eb = np.asarray(mb["signal_energy"][:n], dtype=np.float64)
    ed = np.asarray(md["signal_energy"][:n], dtype=np.float64)
    frames = np.asarray(mb["frame_idx"][:n], dtype=np.float64)
    delta = eb - ed

    n_pos = int(np.sum(delta > 0))
    n_zero = int(np.sum(delta == 0))
    n_neg = int(np.sum(delta < 0))
    # Δ<0 → 无实数平方根；Δ==0 → |Y[257]|=0（有效值，但对数轴不可见）
    y_abs = np.where(delta > 0, np.sqrt(129.0 * np.maximum(delta, 0.0)), np.nan)

    fin = delta[np.isfinite(delta)]
    print("  Δ = signal_energy(baseline) − signal_energy(d1)，共 %d 帧" % n)
    if fin.size:
        print("    Δ 统计: min=%.6g  中位=%.6g  max=%.6g  均值=%.6g" %
              (float(fin.min()), float(np.median(fin)), float(fin.max()), float(fin.mean())))
    print("    Δ>0: %d 帧（可反推）  Δ==0: %d 帧（未检出 D1 效应）  Δ<0: %d 帧（物理上不应出现）"
          % (n_pos, n_zero, n_neg))
    yv = y_abs[np.isfinite(y_abs)]
    if yv.size:
        print("    |Y[257]| = sqrt(129·Δ) 范围: %.6g … %.6g（中位 %.6g）"
              % (float(yv.min()), float(yv.max()), float(np.median(yv))))
    print("    反推式 |Y[257]| = sqrt(129 × Δ)，Δ<0 的帧不参与作图")

    cv = S.Canvas("half", "FigT08", "|Y[257]| 反推（D1 两套构建做差）")
    band = cv.set_legend_band(2)
    ax = cv.panel(rect=(cv.pad_l, max(cv.top0, band), cv.W - cv.pad_r, cv.H - cv.bottom0),
                  ylabel="|Y[257]|（24bit 量纲）", xlabel="帧序号（10 ms/帧）", ylog=True)
    ax.line(frames, y_abs, color=C.PRIMARY, lw=3, label="|Y[257]| = √(129·Δ)")
    if n_zero:
        zero_mask = (delta == 0)
        ax.nan_bands(zero_mask, color=C.WARN, label="Δ=0 帧（未检出 D1 效应）")
    if n_neg:
        neg_mask = (delta < 0)
        ax.nan_bands(neg_mask, color=C.OPTIMIZED, label="Δ<0 帧（异常）")
    ax.autoscale()
    ax.grid()
    ax.axes_line()
    ax.frame()
    ax.legend("tl")
    if yv.size:
        cv.note("Δ = baseline − d1 的 signal_energy；|Y[257]| = √(129·Δ)。量纲为 24bit 原始量纲"
                "（非归一化）；因原始实现读到的是栈残留值，**仅能恢复绝对值、符号不可确定**。"
                "Δ>0 %d 帧 / Δ=0 %d 帧 / Δ<0 %d 帧；该量对 Δ 的第 5–6 位有效数字敏感。"
                % (n_pos, n_zero, n_neg))
    else:
        cv.note("所有帧 Δ≤0，未检出 D1 效应（Δ>0 %d / Δ=0 %d / Δ<0 %d）。"
                % (n_pos, n_zero, n_neg))
    return cv.save("FigT08_Y257反推.png")


# --------------------------------------------------------------------------
# FigT13 —— 分频带衰减（半页）
# --------------------------------------------------------------------------

def fig_t13(seconds: float = 8.0) -> str:
    paths = {"input": None, "baseline": K.wav_path(K.PRIMARY_VARIANT, LEVEL, "1"),
             "optimized": K.wav_path(K.COMPARE_VARIANT, LEVEL, "1")}
    if not (os.path.isfile(paths["baseline"]) and os.path.isfile(paths["optimized"])):
        print("SKIP FigT13 缺少输出 WAV（%s / %s）" % (paths["baseline"], paths["optimized"]))
        return ""
    wavs = {"input": K.read_input_left("1")[0],
            "baseline": K.read_wav_mono(paths["baseline"])[0],
            "optimized": K.read_wav_mono(paths["optimized"])[0]}
    rate = K.SAMPLE_RATE
    n = int(seconds * rate)
    band_db = {}
    for key, x in wavs.items():
        seg = np.asarray(x[:n], dtype=np.float64)
        band_db[key] = K.band_energy_db(seg, edges=BANDS, rate=rate)
        print("  %-10s 前 %.1f s 分带能量 (dB): %s" %
              (key, seconds, " ".join("%.1f" % v for v in band_db[key])))

    cv = S.Canvas("half", "FigT13", "分频带衰减对比（前 %.0f s）" % seconds)
    band = cv.set_legend_band(2)
    ax = cv.panel(rect=(cv.pad_l, max(cv.top0, band), cv.W - cv.pad_r, cv.H - cv.bottom0),
                  ylabel="频带电平 (dB)", xlabel="频带 (Hz)")
    idx = np.arange(len(BAND_SHORT), dtype=np.float64)
    ax.set_xlim(-0.6, len(BAND_SHORT) - 0.4)
    ref = band_db["input"]
    series = (("input", "原始输入", C.TEXT, 0.0),
              ("baseline", "基线 baseline", C.BASELINE, 1.0),
              ("optimized", "优化 optimized", C.OPTIMIZED, 2.0))
    w = 0.26
    for key, lab, color, k in series:
        vals = band_db[key] - ref      # 相对原始输入的带内变化
        ax.bars(vals, centers=idx + (k - 1) * (w + 0.02), width=w, color=color, label=lab,
                show_value=(key != "input"))
    ax.autoscale()
    ax.grid(xlabels=BAND_SHORT)
    ax.axes_line()
    ax.frame()
    ax.legend("tl")
    for i, b in enumerate(BAND_SHORT):
        print("  %-8s 基线变化 %+7.2f dB  优化变化 %+7.2f dB  优化−基线 %+7.2f dB"
              % (b, band_db["baseline"][i] - ref[i], band_db["optimized"][i] - ref[i],
                 band_db["optimized"][i] - band_db["baseline"][i]))
    cv.note("纵轴为各频带相对原始输入的电平变化；STFT 512 点 / 汉宁窗 / 50% 重叠，功率谱求和。")
    return cv.save("FigT13_分频带衰减.png")


# --------------------------------------------------------------------------
# FigT15 —— 钳位命中统计（半页）
# --------------------------------------------------------------------------

def fig_t15() -> str:
    base = K.summary_map(K.PRIMARY_VARIANT, LEVEL)
    opt = K.summary_map(K.COMPARE_VARIANT, LEVEL)
    if not base and not opt:
        print("SKIP FigT15 无 summary CSV")
        return ""
    names = sorted(set(base) | set(opt), key=K._name_sort_key)
    cv = S.Canvas("half", "FigT15", "钳位命中统计（逐文件 + 变体合计）")
    rects = S.Axes.grid_rects((cv.pad_l, cv.top0 - 4, cv.W - cv.pad_r, cv.H - cv.bottom0 - 6),
                              nrow=2, ncol=1, gap_y=68)
    ax = cv.panel(rect=rects[0], ylabel="命中样本数", xlabel="测试文件编号")
    idx = np.arange(len(names), dtype=np.float64)
    ax.set_xlim(-0.6, len(names) + 0.1)
    vb = [base.get(n, {}).get("clamp_hits_total", 0) for n in names]
    vo = [opt.get(n, {}).get("clamp_hits_total", 0) for n in names]
    ax.bars(vb, centers=idx, width=0.36, offset=-0.19, color=C.BASELINE, label="基线 baseline")
    ax.bars(vo, centers=idx, width=0.36, offset=0.19, color=C.OPTIMIZED, label="优化 optimized",
            show_value=True)
    ax.autoscale()
    ax.grid(xlabels=names)
    ax.axes_line()
    ax.frame()
    ax.legend("tr", max_w=ax.width * 0.85)
    total_frames_hits = int(sum(int(v) for v in vb) + sum(int(v) for v in vo))
    if total_frames_hits == 0:
        ax.pixel_note(ax.x0 + 14, ax.y0 + 40,
                      "逐文件命中全部为 0 —— D5 在本批 24bit 量纲数据下未触发（非绘图错误）",
                      color=C.POSITIVE, fnt=S.FS_NOTE, bold=True,
                      max_w=ax.width - 28)

    ax2 = cv.panel(rect=rects[1], ylabel="合计命中数", xlabel="变体")
    variants = _variants_in_dir()
    labels, totals, colors = [], [], []
    for v in variants:
        rows = K.summary_map(v, LEVEL)
        tot = sum(int(r.get("clamp_hits_total", 0)) for r in rows.values())
        labels.append(VAR_LABEL.get(v, v))
        totals.append(tot)
        colors.append(_color_of(v))
        print("  %-10s 合计钳位命中 = %d" % (v, tot))
    ax2.set_xlim(-0.6, max(1, len(labels)) - 0.4)
    for i, (lab, tot, col) in enumerate(zip(labels, totals, colors)):
        ax2.bars([tot], centers=np.array([float(i)]), width=0.5, color=col,
                 show_value=True)
    if all(t == 0 for t in totals):
        ax2.set_ylim(0, 1.0)
        ax2.pixel_note(ax2.x0 + 14, ax2.y0 + 10, "全部变体合计命中为 0（D5 未触发）",
                       color=C.POSITIVE, fnt=S.FS_NOTE, bold=True, max_w=ax2.width - 28)
    ax2.autoscale()
    ax2.grid(xlabels=labels)
    ax2.axes_line()
    ax2.frame()
    zero_all = (total_frames_hits == 0) and all(t == 0 for t in totals)
    for n in names:
        print("  %-3s 基线命中 %d  优化命中 %d" %
              (n, base.get(n, {}).get("clamp_hits_total", 0),
               opt.get(n, {}).get("clamp_hits_total", 0)))
    cv.note("钳位命中 = 输出样本触达 ±32768 边界的计数；非零即存在削波风险。"
            + ("**本批数据全部变体命中为 0**：D5 在 24bit 量纲下未触发，"
               "这是「未触发」而非「统计失败」。" if zero_all else ""))
    return cv.save("FigT15_钳位命中统计.png")


# --------------------------------------------------------------------------
# FigT17 —— 消融五组对比（整页，2×2）
# --------------------------------------------------------------------------

def fig_t17() -> str:
    variants = _variants_in_dir()
    if not variants:
        print("SKIP FigT17 无任何 summary CSV（需 %s）"
              % " / ".join(K.summary_path(v, LEVEL) for v in K.ABLATION_ORDER))
        return ""
    data = {v: K.summary_map(v, LEVEL) for v in variants}
    names = sorted({n for rows in data.values() for n in rows}, key=K._name_sort_key)
    chain = " / ".join(VAR_LABEL.get(v, v) for v in variants)
    cv = S.Canvas("full", "FigT17", "消融 %d 组对比（%s）" % (len(variants), chain))
    rects = S.Axes.grid_rects((cv.pad_l, cv.top0 - 6, cv.W - cv.pad_r, cv.H - cv.bottom0 - 12),
                              nrow=2, ncol=2, gap_x=210, gap_y=112)

    # (1) 整段衰减
    ax = cv.panel(rect=rects[0], ylabel="整段能量衰减 (dB)", xlabel="变体")
    ax.set_xlim(-0.6, len(variants) - 0.4)
    means = []
    for i, v in enumerate(variants):
        vals = [np.nan if abs(r.get("atten_db", np.nan)) >= K.ATTEN_DB_SENTINEL - 1
                else r.get("atten_db", np.nan) for r in data[v].values()]
        vals = [x for x in vals if np.isfinite(x)]
        m = float(np.mean(vals)) if vals else 0.0
        means.append(m)
        ax.bars([m], centers=np.array([float(i)]), width=0.5, color=_color_of(v), show_value=True)
    ax.autoscale()
    ax.grid(xlabels=[VAR_LABEL.get(v, v) for v in variants])
    ax.axes_line()
    ax.frame()

    # (2) 输入/输出 RMS 对比
    ax = cv.panel(rect=rects[1], ylabel="整段 RMS（int16 量纲）", xlabel="测试文件编号")
    ax.set_xlim(-0.6, len(names) + 0.1)
    idx = np.arange(len(names), dtype=np.float64)
    for v in variants:
        vals = [data[v].get(n, {}).get("out_rms", np.nan) for n in names]
        ax.line(idx, vals, color=_color_of(v), lw=3, label="%s · 输出 RMS" % VAR_LABEL.get(v, v))
        print("  %-10s 输出 RMS 均值 %.2f  衰减均值 %.2f dB" %
              (v, float(np.nanmean([data[v].get(n, {}).get("out_rms", np.nan) for n in names])),
               means[variants.index(v)]))
    ax.autoscale()
    ax.grid(xlabels=names)
    ax.axes_line()
    ax.frame()
    ax.legend("tr", max_w=ax.width * 0.72)

    # (3) 钳位合计 + 耗时
    ax = cv.panel(rect=rects[2], ylabel="合计钳位命中 (个)", xlabel="变体")
    ax.set_xlim(-0.6, len(variants) - 0.4)
    clamps = [sum(int(r.get("clamp_hits_total", 0)) for r in data[v].values()) for v in variants]
    for i, (v, c) in enumerate(zip(variants, clamps)):
        ax.bars([c], centers=np.array([float(i)]), width=0.5, color=_color_of(v), show_value=True)
    if all(c == 0 for c in clamps):
        ax.set_ylim(0, 1.0)
        ax.note_px(ax.x0 + 16, ax.y0 + 16, "四组合计命中均为 0", color=C.POSITIVE,
                   fnt=S.FS_NOTE, bold=True)
    ax.autoscale()
    ax.grid(xlabels=[VAR_LABEL.get(v, v) for v in variants])
    ax.axes_line()
    ax.frame()

    # (4) 单帧耗时中位数
    ax = cv.panel(rect=rects[3], ylabel="单帧耗时中位数 (μs)", xlabel="变体")
    ax.set_xlim(-0.6, len(variants) - 0.4)
    costs = []
    for v in variants:
        arrs = []
        for n in names:
            if K.exists_metrics(v, LEVEL, n):
                try:
                    arrs.append(K.read_metrics(v, LEVEL, n)["elapsed_us"])
                except (OSError, ValueError):
                    pass
        costs.append(float(np.median(np.concatenate(arrs))) if arrs else float("nan"))
    for i, (v, c) in enumerate(zip(variants, costs)):
        if np.isfinite(c):
            ax.bars([c], centers=np.array([float(i)]), width=0.5, color=_color_of(v),
                    show_value=True)
    ax.autoscale()
    ax.grid(xlabels=[VAR_LABEL.get(v, v) for v in variants])
    ax.axes_line()
    ax.frame()
    for v, c, cl in zip(variants, costs, clamps):
        print("  %-10s 单帧耗时中位 %.0f μs  RTF=%.4f  合计钳位 %d" %
              (v, c, c / FRAME_US if np.isfinite(c) else float("nan"), cl))

    cv.note("四面板：整段衰减 / 输出 RMS / 钳位命中 / 单帧耗时中位数。"
            "消融链 %s（等级 %s）；指标取自各变体 summary 与 metrics。" % (chain, LEVEL))
    return cv.save("FigT17_消融五组对比.png")


# --------------------------------------------------------------------------
# FigT18 —— 9 文件 atten_db 箱线图（半页）
# --------------------------------------------------------------------------

def _collect_atten(variant: str = K.PRIMARY_VARIANT):
    """逐文件的 atten_db 数组（剔除哨兵）。"""
    out = {}
    for n in K.test_file_names():
        if not K.exists_metrics(variant, LEVEL, n):
            continue
        try:
            m = K.read_metrics(variant, LEVEL, n)
        except (OSError, ValueError) as exc:
            print("  跳过 %s/%s: %s" % (variant, n, exc))
            continue
        v = m["_atten"] = np.where(np.abs(m["atten_db"] - K.ATTEN_DB_SENTINEL) < 1e-6,
                                   np.nan, m["atten_db"])
        out[n] = v[np.isfinite(v)]
    return out


def fig_t18() -> str:
    data = _collect_atten(K.PRIMARY_VARIANT)
    if not data:
        print("SKIP FigT18 无任何 metrics_<name>.csv（%s/%s）" % (K.data_dir(), LEVEL))
        return ""
    cv = S.Canvas("half", "FigT18", "九文件 atten_db 分布（箱线图）")
    band = cv.set_legend_band(2)
    ax = cv.panel(rect=(cv.pad_l, max(cv.top0, band), cv.W - cv.pad_r, cv.H - cv.bottom0),
                  ylabel="衰减量 (dB)", xlabel="测试文件编号")
    names = sorted(data, key=K._name_sort_key)
    ax.set_xlim(0.4, len(names) + 0.6)
    for i, n in enumerate(names, start=1):
        st = K.box_stats(data[n])
        ax.boxplot(st, center=float(i), width=0.52, color=C.BASELINE)
        print("  %-3s 帧=%5d  中位 %.2f dB  IQR %.2f–%.2f  须 %.2f–%.2f  离群 %d"
              % (n, st["n"], st["med"], st["q1"], st["q3"], st["whislo"], st["whishi"],
                 st["fliers"].size))
    ax.autoscale()
    ax.grid(xlabels=names)
    ax.axes_line()
    ax.frame()
    ax.legend("tr", entries=[("patch", C.BASELINE, "四分位箱（中位线）", 0),
                             ("marker", C.WARN, "1.5×IQR 外离群点", 5)], max_w=ax.width * 0.8)
    cv.note("哨兵值 999 dB（out_rms=0）已剔除；须端为 1.5×IQR 内极值。变体 %s，等级 %s。"
            % (K.PRIMARY_VARIANT, LEVEL))
    return cv.save("FigT18_atten_db箱线图.png")


# --------------------------------------------------------------------------
# FigT19 —— 单帧耗时分布 + RTF（半页）
# --------------------------------------------------------------------------

def fig_t19() -> str:
    """单帧耗时分布 + RTF。

    数据源为**全部存在 metrics 的变体**（计划书要求 `elapsed_us` 列，未限定变体），
    以便同时反映优化前后的耗时差异。若某变体缺 metrics 则自动跳过。
    """
    arrs = []
    used = []
    for variant in K.ABLATION_ORDER:
        got = []
        for n in K.test_file_names():
            if K.exists_metrics(variant, LEVEL, n):
                try:
                    got.append(K.read_metrics(variant, LEVEL, n)["elapsed_us"].astype(np.float64))
                except (OSError, ValueError):
                    pass
        if got:
            used.append((variant, len(got)))
            arrs.extend(got)
    if not arrs:
        print("SKIP FigT19 无 metrics 数据（elapsed_us 列缺失）")
        return ""
    allus = np.concatenate(arrs)
    med = float(np.median(allus))
    p95 = float(np.percentile(allus, 95))
    rtf = med / FRAME_US
    print("  变体覆盖: %s" % "、".join("%s×%d 文件" % (v, c) for v, c in used))
    print("  帧总数=%d  中位=%.0f μs  P95=%.0f μs  最大=%.0f μs  RTF(中位)=%.4f"
          % (allus.size, med, p95, float(allus.max()), rtf))

    cv = S.Canvas("half", "FigT19", "单帧耗时分布与实时率 RTF")
    rects = S.Axes.grid_rects((cv.pad_l, cv.top0 - 4, cv.W - cv.pad_r, cv.H - cv.bottom0 - 8),
                              nrow=2, ncol=1, gap_y=72)
    hi = float(np.percentile(allus, 99.5)) * 1.05
    bins = np.linspace(0, max(hi, 1.0), 41)
    counts, edges = np.histogram(allus, bins=bins)
    centers = (edges[:-1] + edges[1:]) / 2.0
    band = cv.set_legend_band(2)
    r0 = (rects[0][0], max(rects[0][1], band), rects[0][2], rects[0][3])
    ax = cv.panel(rect=r0, ylabel="帧数 (个)", xlabel="")
    ax.set_xlim(0, bins[-1])
    ax.bars(counts, centers=centers, width=(bins[1] - bins[0]) * 0.92, color=C.PRIMARY,
            label="单帧耗时分布")
    ax.vline(med, color=C.POSITIVE, lw=3, dash=False, label="中位 %.0f μs" % med)
    ax.vline(FRAME_US, color=C.WARN, lw=3, dash=True, label="实时预算 10000 μs")
    ax.autoscale()
    ax.set_ylim(0, float(counts.max()) * 1.25 if counts.size else 1.0)
    ax.grid(horizontal=True, vertical=False)
    ax.axes_line()
    ax.frame()
    ax.legend("tr", max_w=ax.width * 0.95)

    r1 = (rects[1][0], max(rects[1][1], band), rects[1][2], rects[1][3])
    ax2 = cv.panel(rect=r1, ylabel="累计占比 (%)", xlabel="单帧耗时 (μs)")
    ax2.set_xlim(0, bins[-1])
    cdf = np.cumsum(counts) / max(1.0, float(counts.sum())) * 100.0
    ax2.line(centers, cdf, color=C.PRIMARY, lw=3, label="累计分布")
    ax2.hline(95.0, color=C.WARN, lw=2, annotate="P95 = %.0f μs" % p95)
    ax2.autoscale()
    ax2.set_ylim(0, 105)
    ax2.grid(horizontal=True, vertical=True)
    ax2.axes_line()
    ax2.frame()
    ax2.legend("br", max_w=ax2.width * 0.7)
    cv.note("RTF = 单帧耗时 / 10000 μs（10 ms 帧 = 10000 μs）；RTF<1 表示可实时处理。"
            "本例 RTF(中位)=%.4f，样本覆盖 %s。"
            % (rtf, "、".join(v for v, _ in used)))
    return cv.save("FigT19_单帧耗时分布.png")


# --------------------------------------------------------------------------
# FigT21 —— 无语音段 vs 语音段衰减（半页）
# --------------------------------------------------------------------------

DEF_LOW, DEF_HIGH = 0.2, 0.6


def _speech_bins(name: str) -> tuple[list[tuple], str]:
    """按数据实际分布确定概率分档边界（保证三档非空）。

    计划书规定"按 `speech_prob_mean` 分档"。固定 0.2/0.6 在真实数据上可能使
    "无语音档"为空（弱信号下概率整体偏低）。因此先试固定阈值，若某档为空或过小
    （< 5 帧），则改用**数据分位数**（20% / 60%）切分，并在图上注明实际边界。
    """
    probs = []
    atts = []
    for variant in (K.PRIMARY_VARIANT, K.COMPARE_VARIANT):
        if not K.exists_metrics(variant, LEVEL, name):
            continue
        m = K.read_metrics(variant, LEVEL, name)
        att = np.where(np.abs(m["atten_db"] - K.ATTEN_DB_SENTINEL) < 1e-6,
                       np.nan, m["atten_db"])
        ok = np.isfinite(att)
        probs.append(np.asarray(m["speech_prob_mean"], dtype=np.float64)[ok])
        atts.append(att[ok])
    if not probs:
        return [], "无数据"
    p_all = np.concatenate(probs)

    def count(lo, hi):
        return int(np.sum((p_all >= lo) & (p_all < hi)))

    if count(0.0, DEF_LOW) >= 5 and count(DEF_LOW, DEF_HIGH) >= 5 and count(DEF_HIGH, 1.01) >= 5:
        lo, hi, how = DEF_LOW, DEF_HIGH, "固定阈值 0.20 / 0.60"
    else:
        # 真实概率整体偏移时，改用**相对分位**：下三分位 / 上三分位
        lo = float(np.percentile(p_all, 33.0))
        hi = float(np.percentile(p_all, 67.0))
        if hi - lo < 0.02:      # 分布极度集中时给一个最小可见间隔
            mid = 0.5 * (lo + hi)
            lo, hi = mid - 0.01, mid + 0.01
        how = "数据三档分位 P33/P67（概率整体偏移，固定阈值不适用）"
    bins = ((0.0, lo, "无语音(<%.2f)" % lo),
            (lo, hi, "过渡(%.2f-%.2f)" % (lo, hi)),
            (hi, 1.01, "语音(>=%.2f)" % hi))
    return list(bins), how


def fig_t21(name: str = "1") -> str:
    bins, how = _speech_bins(name)
    if not bins:
        print("SKIP FigT21 缺少 metrics_%s.csv" % name)
        return ""
    groups = {}
    for variant in (K.PRIMARY_VARIANT, K.COMPARE_VARIANT):
        if not K.exists_metrics(variant, LEVEL, name):
            continue
        m = K.read_metrics(variant, LEVEL, name)
        att = np.where(np.abs(m["atten_db"] - K.ATTEN_DB_SENTINEL) < 1e-6,
                       np.nan, m["atten_db"])
        prob = m["speech_prob_mean"]
        per = []
        for lo, hi, lab in bins:
            sel = (prob >= lo) & (prob < hi) & np.isfinite(att)
            per.append((lab, att[sel]))
        groups[variant] = per
        print("  %-10s " % variant + "  ".join("%s: n=%d 中位=%s dB"
                                               % (lab.replace("\n", ""), a.size,
                                                  ("%.2f" % float(np.median(a))) if a.size else "n/a")
                                               for lab, a in per))
    if not groups:
        print("SKIP FigT21 缺少 metrics_%s.csv" % name)
        return ""
    cv = S.Canvas("half", "FigT21", "无语音段 vs 语音段的衰减对比")
    band = cv.set_legend_band(2)
    ax = cv.panel(rect=(cv.pad_l, max(cv.top0, band), cv.W - cv.pad_r, cv.H - cv.bottom0),
                  ylabel="衰减量 (dB)", xlabel="按 speech_prob_mean 分档")
    n_grp = len(bins)
    ax.set_xlim(0.4, n_grp * 2 + 0.6)
    positions = []
    for vi, variant in enumerate((K.PRIMARY_VARIANT, K.COMPARE_VARIANT)):
        if variant not in groups:
            continue
        for gi, (lab, vals) in enumerate(groups[variant]):
            center = float(gi * 2 + 1 + vi * 0.78)
            positions.append(center)
            if vals.size:
                st = K.box_stats(vals)
                ax.boxplot(st, center=center, width=0.6, color=_color_of(variant))
                ax.text(center, st["whishi"], "%.1f" % st["med"], color=C.TEXT,
                        fnt=S.FS_LABEL, anchor="cb")
    if positions:
        ax.set_xlim(min(positions) - 0.8, max(positions) + 0.8)
    ax.autoscale()
    ax.grid(xlabels=None)
    # 手绘分档标签（跨越成对箱线）
    f = S.font(S.FS_TICK)
    for gi, (lo, hi, lab) in enumerate(bins):
        xc = ax.px(float(gi * 2 + 1 + 0.39))
        for j, line in enumerate(lab.split("\n")):
            w, h = S.text_size(ax.d, line, f)
            ax.d.text((xc - w / 2.0, ax.y1 + 10 + j * (S.FS_TICK + 4)), line,
                      font=f, fill=C.TEXT)
    ax.axes_line()
    ax.frame()
    ax.legend("tr", entries=[("patch", C.BASELINE, "基线 baseline", 0),
                             ("patch", C.OPTIMIZED, "优化 optimized", 0)],
              max_w=ax.width * 0.7)
    cv.note("分档依据 speech_prob_mean（%s，实际边界见横轴）；每档内左右两根箱线分别为"
            "基线 / 优化。箱上数字为中位数 (dB)。" % how)
    return cv.save("FigT21_无语音段与语音段衰减.png")


# --------------------------------------------------------------------------
# FigT22 —— 失败与边界案例（半页）
# --------------------------------------------------------------------------

def fig_t22() -> str:
    """异常检测：逐文件统计 NaN/Inf 计数、`atten_db` 越界帧、`clamp_hits` 超限帧。

    **判据与分布形态无关**（不使用 IQR 离群，避免把静音/语音双峰分布整段误判为异常）：
      * NaN/Inf：覆盖 6.2 全部 12 列
      * 衰减越界：有限衰减值 > `ATTEN_MAX_DB`(60 dB) 或 < `ATTEN_MIN_DB`(−1 dB)
      * 负输出：`out_rms < 0`
      * 钳位超限：`clamp_hits > CLAMP_LIMIT`(480，即 10 ms 帧内样本数上限)

    无论是否检出异常都出图（无异常时显示「未检出异常」）。
    """
    files = K.test_file_names()
    rows: list[list[str]] = []
    highlights: dict = {}
    issue_total = 0
    checked = 0
    detail = []
    for variant in (K.PRIMARY_VARIANT, K.COMPARE_VARIANT):
        for n in files:
            if not K.exists_metrics(variant, LEVEL, n):
                continue
            checked += 1
            try:
                m = K.read_metrics(variant, LEVEL, n)
            except (OSError, ValueError) as exc:
                detail.append((variant, n, "读取失败", str(exc)))
                issue_total += 1
                continue
            numeric = np.vstack([np.asarray(m[c], dtype=np.float64) for c in K.METRICS_COLUMNS])
            nan_cnt = int(np.sum(np.isnan(numeric)))
            inf_cnt = int(np.sum(np.isinf(numeric)))
            att = np.asarray(m["atten_db"], dtype=np.float64)
            sentinel = int(np.sum(np.abs(att - K.ATTEN_DB_SENTINEL) < 1e-6))
            finite = att[np.isfinite(att) & (np.abs(att - K.ATTEN_DB_SENTINEL) >= 1e-6)]
            out_of_range = int(np.sum((finite > ATTEN_MAX_DB) | (finite < ATTEN_MIN_DB)))
            over_clamp = int(np.sum(m["clamp_hits"] > CLAMP_LIMIT))
            neg_out = int(np.sum(np.asarray(m["out_rms"], dtype=np.float64) < 0.0))
            n_nan = nan_cnt + inf_cnt
            issues = []
            if n_nan:
                issues.append("NaN/Inf×%d" % n_nan)
            if out_of_range:
                issues.append("衰减越界×%d" % out_of_range)
            if over_clamp:
                issues.append("钳位超限×%d" % over_clamp)
            if neg_out:
                issues.append("负输出×%d" % neg_out)
            issue_total += len(issues)
            rows.append([variant, n,
                         str(n_nan), str(sentinel), str(out_of_range),
                         str(int(np.sum(m["clamp_hits"] > 0))),
                         "是" if issues else "否"])
            if issues:
                highlights[(len(rows) - 1, 6)] = C.OPTIMIZED
                detail.append((variant, n, "、".join(issues),
                               "帧数 %d" % int(m["frame_idx"].size)))
            else:
                highlights[(len(rows) - 1, 6)] = C.POSITIVE
            # NaN/Inf 帧位置：标到第 2 列（NaN 列）以强调这不是 0 值
            if n_nan:
                highlights[(len(rows) - 1, 2)] = C.NAN
            print("  %-10s %-3s NaN/Inf=%d(=%d+%d) 999哨兵=%d 衰减越界=%d 钳位帧=%d 超限=%d 负输出=%d"
                  % (variant, n, n_nan, nan_cnt, inf_cnt, sentinel, out_of_range,
                     int(np.sum(m["clamp_hits"] > 0)), over_clamp, neg_out))

    if not rows:
        print("SKIP FigT22 无任何 metrics CSV 可检查（%s）" % K.data_dir())
        return ""

    cv = S.Canvas("half", "FigT22", "失败与边界案例检查")
    ax = cv.panel(rect=(cv.pad_l, cv.top0 - 6, cv.W - cv.pad_r, cv.top0 + 24))
    shown = rows[:13]
    ax.table(["变体", "文件", "NaN/Inf", "999哨兵", "衰减越界", "钳位帧", "异常"],
             shown, rect=(cv.pad_l, cv.top0 - 6, cv.W - cv.pad_r,
                          cv.top0 - 6 + 42 * (len(shown) + 1)),
             col_widths=[1.5, 0.9, 1.1, 1.1, 1.2, 1.0, 0.9],
             highlight=highlights, row_h=30)
    y_after = cv.top0 - 6 + 30 * (len(shown) + 1) + 12

    f = S.font(S.FS_NOTE, True)
    total_nan = sum(int(r[2]) for r in rows)
    if issue_total == 0:
        msg = ("未检出异常：%d 份 CSV 的 NaN/Inf=0、衰减越界=0、钳位超限=0、负输出=0"
               % checked)
        col = C.POSITIVE
    else:
        msg = "检出 %d 处需关注（NaN/Inf 共 %d 帧）：%s" % (
            issue_total, total_nan,
            "；".join("%s/%s %s" % (v, n, d) for v, n, d, _ in detail[:3]))
        col = C.OPTIMIZED
    ax.pixel_note(cv.pad_l, y_after, msg, color=col, fnt=S.FS_NOTE, bold=True,
                  max_w=cv.W - cv.pad_l - cv.pad_r)
    cv.note("判据：NaN/Inf 覆盖 6.2 全部 12 列（原样保留，不做零值替换）；"
            "衰减越界 = 有限衰减值 >%.0f dB 或 <%.0f dB；钳位超限 = clamp_hits >%d（超过单帧 %d 个样本）。"
            "哨兵 999 dB 为契约允许值，不计异常。"
            % (ATTEN_MAX_DB, ATTEN_MIN_DB, CLAMP_LIMIT, K.FRAME_SAMPLES))
    print("  " + msg)
    return cv.save("FigT22_失败与边界案例.png")


def main() -> int:
    S.warn_if_formal_output("fig_summary.py")
    print("=" * 84)
    print("[fig_summary] 数据目录 %s" % K.data_dir())
    print("[fig_summary] 输出目录 %s" % K.figures_dir())
    print("=" * 84)
    fig_t01()
    fig_t08()
    fig_t13()
    fig_t15()
    fig_t17()
    fig_t18()
    fig_t19()
    fig_t21()
    fig_t22()
    return 0


if __name__ == "__main__":
    sys.exit(main())
