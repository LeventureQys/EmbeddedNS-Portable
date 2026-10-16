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

#: 类目轴**短标签**：半页/整页的列宽放不下「基线 baseline」这类中英混排
#: （168–201 px，而单列常只有 119–129 px），会与相邻类目逐字叠印成一团。
#: 英文名由图例行承载，语义不缺失。
SHORT_VAR_LABEL = {"baseline": "基线", "d1": "+D1", "d1d5": "+D1D5",
                   "d1d5d6": "+D1D5D6", "optimized": "优化"}

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
    # 两行子图；每行轴区**上方**留一条图例行（`legend_strip`）。
    # 图例放进轴内必然压柱体（实测：下图图例框把第 4 组柱整体盖住、上图紧贴柱顶 2 px），
    # 放进「图例带」又要吃掉半页画布 1/5 的高度，因此排在两行之间的空档里。
    cv.set_bottom_reserve(80)
    rows = cv.stack_rows(2, top=cv.top0 - 4, bottom=cv.H - cv.bottom0 - 14,
                         gap=30, caption_h=40)
    idx = np.arange(len(names), dtype=np.float64)

    # 上：时长（基线 / 优化 双柱，两者应等长）
    rect0, cap0 = rows[0]
    S.legend_strip(cv, rect0[0], cap0,
                   [("patch", C.BASELINE, "时长 · 基线"), ("patch", C.OPTIMIZED, "时长 · 优化")])
    ax = cv.panel(rect=rect0, ylabel="时长 (s)", xlabel="")
    ax.set_xlim(-0.6, len(names) + 0.1)
    dur_b = [base.get(n, {}).get("duration_s", np.nan) for n in names]
    dur_o = [opt.get(n, {}).get("duration_s", np.nan) for n in names]
    ax.bars(dur_b, centers=idx, width=0.36, offset=-0.19,
            color=C.BASELINE, label="时长 · 基线")
    ax.bars(dur_o, centers=idx, width=0.36, offset=0.19,
            color=C.OPTIMIZED, label="时长 · 优化")
    ax.autoscale()
    # 上下两行共用同一条 x 轴（都是测试文件编号），上行不重复画刻度标签，
    # 省下的刻度带高度全部还给柱体。
    ax.grid(xlabels=None, xtick_labels=False)
    ax.axes_line()
    ax.frame()

    # 下：整段能量衰减（双柱）
    rect1, cap1 = rows[1]
    S.legend_strip(cv, rect1[0], cap1,
                   [("patch", C.BASELINE, "衰减 · 基线"), ("patch", C.OPTIMIZED, "衰减 · 优化")])
    ax2 = cv.panel(rect=rect1, ylabel="衰减 dB", xlabel="测试文件编号")
    ax2.set_xlim(-0.6, len(names) - 0.4)
    att_b = [base.get(n, {}).get("atten_db", np.nan) for n in names]
    att_o = [opt.get(n, {}).get("atten_db", np.nan) for n in names]
    ax2.bars(att_b, centers=idx, width=0.36, offset=-0.19,
             color=C.BASELINE, label="衰减 · 基线")
    ax2.bars(att_o, centers=idx, width=0.36, offset=0.19,
             color=C.OPTIMIZED, label="衰减 · 优化")
    ax2.autoscale()
    # 显式保证 0 基线在范围内并给顶部留 8% 余量：否则 autoscale 的 6% pad 在
    # 半数文件缺 summary（NaN 被过滤）时会把上限压到数据附近、刻度只剩 0/1 两级。
    _lo2, _hi2 = ax2.ylim
    ax2.set_ylim(min(0.0, _lo2), _hi2 + max(_hi2 - _lo2, 1e-6) * 0.08)
    ax2.grid(xlabels=names)
    ax2.axes_line()
    ax2.frame()

    for n in names:
        print("  %-3s 时长 基线%6.2f/优化%6.2f s  衰减 基线%6.2f/优化%6.2f dB"
              % (n, base.get(n, {}).get("duration_s", float("nan")),
                 opt.get(n, {}).get("duration_s", float("nan")),
                 base.get(n, {}).get("atten_db", float("nan")),
                 opt.get(n, {}).get("atten_db", float("nan"))))
    cv.note("等级 %s；时长应两变体等长，衰减=20log10(in/out) rms。" % LEVEL)
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
    # 图例改排到轴区上方：`set_legend_band` 会吃掉 162 px（半页的 1/4），
    # 把轴区与底部注释一起压掉。
    S.legend_strip(cv, cv.pad_l, cv.top0 - 20,
                   [("line", C.PRIMARY, "|Y[257]|=√(129·Δ)"),
                    ("patch", C.WARN, "Δ=0 帧"),
                    ("patch", C.OPTIMIZED, "Δ<0 帧")], fnt=S.FS_MIN)
    cv.set_bottom_reserve(130)
    ax = cv.panel(rect=(cv.pad_l, cv.top0 + 26, cv.W - cv.pad_r, cv.H - cv.bottom0 - 34),
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
    if yv.size:
        cv.note("|Y[257]|=√(129·Δ)，Δ=signal_energy(baseline−d1)。24bit 原始量纲；"
                "因读到的是栈残留值，仅能恢复绝对值、符号不可确定。"
                "Δ 帧数 %d/%d/%d（>/=/<）。" % (n_pos, n_zero, n_neg))
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
    # 图例三分项并排需约 690 px，超出半页可用的 646 px 而被折行压到轴区上，
    # 故改为轴区上方的两行横排图例（不占轴内空间，文案完整不截断）。
    S.legend_strip(cv, cv.pad_l, cv.top0 - 24,
                   [("patch", C.TEXT, "原始输入"), ("patch", C.BASELINE, "基线 baseline")])
    S.legend_strip(cv, cv.pad_l, cv.top0 + 14,
                   [("patch", C.OPTIMIZED, "优化 optimized")])
    cv.set_bottom_reserve(96)
    ax = cv.panel(rect=(cv.pad_l, cv.top0 + 60, cv.W - cv.pad_r, cv.H - cv.bottom0 - 14),
                  ylabel="频带电平 (dB)", xlabel="频带 (Hz)")
    idx = np.arange(len(BAND_SHORT), dtype=np.float64)
    ax.set_xlim(-0.6, len(BAND_SHORT) - 0.4)
    ref = band_db["input"]
    # 绘制顺序 = z 序：柱体并排不重叠，但**标签宽度 84 px 大于柱间距 33 px**，
    # 谁后画谁在上。基线标签必须最后落笔，否则会被同组更高的优化红柱压掉右端
    # （FigT13 的 8–12k / 12–16k / 16–24k 三组曾如此）。位置由 k 决定，与顺序无关。
    series = (("input", "原始输入", C.TEXT, 0.0),
              ("optimized", "优化 optimized", C.OPTIMIZED, 2.0),
              ("baseline", "基线 baseline", C.BASELINE, 1.0))
    w = 0.22
    for key, lab, color, k in series:
        vals = band_db[key] - ref      # 相对原始输入的带内变化
        # 同组两柱中心距仅 33 px，而数值标签宽约 84 px，同排必然末位互相压盖。
        # 基线标签整体上移 34 px 与优化标签**垂直错排**，数字保持原样不做缩短。
        ax.bars(vals, centers=idx + (k - 1) * (w + 0.05), width=w, color=color, label=lab,
                show_value=(key == "optimized"), value_font=S.FS_MIN, value_stroke=3,
                value_dy=(-34.0 if key == "optimized" else 0.0))
    ax.autoscale()
    _lo, _hi = ax.ylim
    ax.set_ylim(_lo, _hi + (_hi - _lo) * 0.35)   # 顶部留出错排标签的空间
    ax.grid(xlabels=BAND_SHORT)
    ax.axes_line()
    ax.frame()
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
    # 半页 625 px 里放"两行子图 + 图例 + 两行注释"，必须显式重排：
    #   ① 图例排在首行轴区上方的空档（不占轴内空间，也不压柱体）；
    #   ② 次行子图的柱体按变体着色、x 轴已标变体名，**不需要图例**，省下一行高度；
    #   ③ 底部注释保留两行（缩字号不可行，故缩短文案而非截断）。
    cv.set_bottom_reserve(80)
    rows = cv.stack_rows(2, top=cv.top0 - 4, bottom=cv.H - cv.bottom0 - 14,
                         gap=52, caption_h=40)

    rect0, cap0 = rows[0]
    S.legend_strip(cv, rect0[0], cap0,
                   [("patch", C.BASELINE, "基线 baseline"),
                    ("patch", C.OPTIMIZED, "优化 optimized")])
    # 上图不画 x 轴标题：刻度 1–9 本身就是文件编号，说明并入底部注释。
    # `stack_rows` 的 gap 必须同时容纳上行的「x 刻度带 + x 轴标题」与下行的
    # 注释放置行，半页 625 px 装不下三者，只能精简掉重复的轴标题。
    ax = cv.panel(rect=rect0, ylabel="命中样本数", xlabel="")
    idx = np.arange(len(names), dtype=np.float64)
    ax.set_xlim(-0.6, len(names) + 0.1)
    vb = [base.get(n, {}).get("clamp_hits_total", 0) for n in names]
    vo = [opt.get(n, {}).get("clamp_hits_total", 0) for n in names]
    total_frames_hits = int(sum(int(v) for v in vb) + sum(int(v) for v in vo))
    if total_frames_hits == 0:
        # ylim 必须在 `bars()` **之前**定下来：`bars()` 会用当时的 `ylim[0]` 作为
        # 柱底像素基准并当场绘制。旧版先画柱、后 `set_ylim(0,1)`，于是柱体是按
        # (-0.06, 0.06) 画的（0 值柱被画成半轴高），而坐标轴显示 (0,1)——柱高与
        # 刻度、与柱顶「0」标注三者互相矛盾。
        ax.set_ylim(0, 1.0)
    ax.bars(vb, centers=idx, width=0.36, offset=-0.19, color=C.BASELINE, label="基线 baseline")
    ax.bars(vo, centers=idx, width=0.36, offset=0.19, color=C.OPTIMIZED, label="优化 optimized",
            show_value=True, value_dy=-12.0)
    ax.autoscale()
    ax.grid(xlabels=names)
    ax.axes_line()
    ax.frame()

    rect1, cap1 = rows[1]
    ax2 = cv.panel(rect=rect1, ylabel="命中样本数", xlabel="变体")
    variants = _variants_in_dir()
    labels, totals, colors = [], [], []
    for v in variants:
        rows_v = K.summary_map(v, LEVEL)
        tot = sum(int(r.get("clamp_hits_total", 0)) for r in rows_v.values())
        # 短标签：半页宽 646 px / 5 个类目 = 129 px，放不下「基线 baseline」这类
        # 中英混排（168–201 px），会与相邻类目逐字叠印。英文名由图例行承载，
        # 语义不缺失。
        labels.append(SHORT_VAR_LABEL.get(v, VAR_LABEL.get(v, v)))
        totals.append(tot)
        colors.append(_color_of(v))
        print("  %-10s 合计钳位命中 = %d" % (v, tot))
    ax2.set_xlim(-0.6, max(1, len(labels)) - 0.4)
    zero_all = (total_frames_hits == 0) and all(t == 0 for t in totals)
    if all(t == 0 for t in totals):
        ax2.set_ylim(0, 1.0)          # 同前：必须在画柱之前
    for i, (lab, tot, col) in enumerate(zip(labels, totals, colors)):
        ax2.bars([tot], centers=np.array([float(i)]), width=0.5, color=col,
                 show_value=True)
    ax2.autoscale()
    ax2.grid(xlabels=labels)
    ax2.axes_line()
    ax2.frame()
    if zero_all:
        # 提示写在**轴区之外**的注释行（cap1）里：写在轴区内会横压 5 个柱顶「0」标签
        S.caption(cv, rect1[0], cap1, "全部变体命中为 0（D5 未触发）",
                  color=C.POSITIVE, fnt=S.FS_LABEL)
    for n in names:
        print("  %-3s 基线命中 %d  优化命中 %d" %
              (n, base.get(n, {}).get("clamp_hits_total", 0),
               opt.get(n, {}).get("clamp_hits_total", 0)))
    cv.note("钳位命中=±32768 边界计数；本批全为 0。")
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
    # 2×2 版式重排：每行轴区上方留一条 40 px 的"图例/说明行"（图例不再进轴区），
    # 两行之间留 40 px 行距，底部注释保留两行。
    cv.set_bottom_reserve(96)
    rows = cv.stack_rows(2, top=cv.top0 - 6, bottom=cv.H - cv.bottom0 - 14,
                         gap=96, caption_h=50)
    gap_x = 236
    cells = []
    for rect, cap_y in rows:
        cw = (rect[2] - rect[0] - gap_x) / 2.0
        cells.append(((rect[0], rect[1], rect[0] + cw, rect[3]), cap_y))
        cells.append(((rect[2] - cw, rect[1], rect[2], rect[3]), cap_y))
    #: 类目轴短标签（定义见模块顶部 `SHORT_VAR_LABEL`）
    SHORT = SHORT_VAR_LABEL
    short_labels = [SHORT.get(v, VAR_LABEL.get(v, v)) for v in variants]

    # (1) 整段衰减
    (rect0, cap0) = cells[0]
    ax = cv.panel(rect=rect0, ylabel="衰减 (dB)", xlabel="变体")
    ax.set_xlim(-0.6, len(variants) - 0.4)
    means = []
    for i, v in enumerate(variants):
        vals = [np.nan if abs(r.get("atten_db", np.nan)) >= K.ATTEN_DB_SENTINEL - 1
                else r.get("atten_db", np.nan) for r in data[v].values()]
        vals = [x for x in vals if np.isfinite(x)]
        m = float(np.mean(vals)) if vals else 0.0
        means.append(m)
    # ylim 必须在 `bars()` **之前**定下来：`bars()` 用**当时的** `ylim` 换算柱底/柱顶
    # 并当场绘制，之后 `set_ylim` 只改坐标轴刻度、柱体像素不会跟着变——这正是
    # 「余量加了却没用」的原因（终检看到的标签贴柱/被裁都源于此）。
    # 顶部留 28% 余量给柱顶数值标签。
    ax.set_ylim(0, max(max([m for m in means if np.isfinite(m)] or [1.0]) * 1.28, 1e-6))
    for i, (v, m) in enumerate(zip(variants, means)):
        ax.bars([m], centers=np.array([float(i)]), width=0.5, color=_color_of(v),
                show_value=True, value_dy=-6.0)
    ax.autoscale()
    ax.grid(xlabels=short_labels)
    ax.axes_line()
    ax.frame()

    # (2) 输入/输出 RMS 对比
    (rect1, _cap1) = cells[1]
    ax = cv.panel(rect=rect1, ylabel="输出 RMS", xlabel="测试文件编号")
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
    # 五个变体的图例排在整行轴区**上方**的空档：放进轴内会盖住第 4–8 点的全部曲线
    # （RMS 五条线几乎覆盖整幅图，四个角落都没有足够空白）。
    S.legend_strip(cv, cells[0][0][0], cap0,
                   [("line", _color_of(v), SHORT.get(v, v)) for v in variants])

    # (3) 钳位合计 + 耗时
    (rect2, _cap2) = cells[2]
    ax = cv.panel(rect=rect2, ylabel="钳位命中", xlabel="变体")
    ax.set_xlim(-0.6, len(variants) - 0.4)
    clamps = [sum(int(r.get("clamp_hits_total", 0)) for r in data[v].values()) for v in variants]
    if all(c == 0 for c in clamps):
        # 与 FigT15 的修法一致：ylim 必须在 `bars()` **之前**定下来。旧版先画柱、
        # 后 `set_ylim(0,1)`，柱体是按 autoscale 的 (-0.06, 0.06) 画的 → 五根零值柱
        # 被画成轴高一半（顶沿落在 0.5 网格线上），与柱顶标注「0」直接矛盾。
        ax.set_ylim(0, 1.0)
    for i, (v, c) in enumerate(zip(variants, clamps)):
        ax.bars([c], centers=np.array([float(i)]), width=0.5, color=_color_of(v),
                show_value=True, value_dy=-6.0)
    ax.autoscale()
    ax.grid(xlabels=short_labels)
    ax.axes_line()
    ax.frame()
    if all(c == 0 for c in clamps):
        S.caption(cv, rect2[0] + 14, rect2[1] + 8, "合计命中均为 0（D5 未触发）",
                  color=C.POSITIVE, fnt=S.FS_LABEL, bg=True)

    # (4) 单帧耗时中位数
    (rect3, _cap3) = cells[3]
    # ylabel 取「单帧耗时」4 字（112 px）：轴区仅 ~140 px 高，带「(μs)」的
    # 126 px 文本会超出，退化成"横排贴轴区左上角"而压住第一根柱的标签。
    ax = cv.panel(rect=rect3, ylabel="单帧耗时", xlabel="变体")
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
    # 同面板 1：ylim 必须**先于** `bars()` 定下来，柱体像素才会按带余量的刻度画。
    # 顶部留 30% 余量（一格字高 + 间隙），柱顶标签 21.0/22.0 才不会被夹到轴区顶边之外。
    _fin_cost = [c for c in costs if np.isfinite(c)] or [1.0]
    ax.set_ylim(0, max(max(_fin_cost) * 1.30, 1e-6))
    for i, (v, c) in enumerate(zip(variants, costs)):
        if np.isfinite(c):
            ax.bars([c], centers=np.array([float(i)]), width=0.5, color=_color_of(v),
                    show_value=True, value_dy=-4.0)
    ax.autoscale()
    ax.grid(xlabels=short_labels)
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
    band = cv.set_legend_band(3)
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
                             ("marker", C.WARN, "1.5×IQR 外离群点", 5)])
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
    # 不再使用"图例带"：半页 625 px 里带高 162 px 会把两行子图各压到不足 100 px，
    # y 轴刻度（40/30/20/10 与 100/80/60/40/20）因此纵向堆叠成黑块。
    # 图例改排在各行轴区上方的空档，子图高度翻倍。
    cv.set_bottom_reserve(96)
    stack = cv.stack_rows(2, top=cv.top0 - 4, bottom=cv.H - cv.bottom0 - 14,
                          gap=36, caption_h=40)
    (r0, cap0), (r1, cap1) = stack
    hi = float(np.percentile(allus, 99.5)) * 1.05
    bins = np.linspace(0, max(hi, 1.0), 41)
    counts, edges = np.histogram(allus, bins=bins)
    centers = (edges[:-1] + edges[1:]) / 2.0

    S.legend_strip(cv, r0[0], cap0,
                   [("patch", C.PRIMARY, "单帧耗时分布"),
                    ("dash", C.WARN, "实时预算 10000 μs")])
    ax = cv.panel(rect=r0, ylabel="帧数", xlabel="")
    ax.set_xlim(0, bins[-1])
    ax.bars(counts, centers=centers, width=(bins[1] - bins[0]) * 0.92, color=C.PRIMARY,
            label="单帧耗时分布")
    ax.vline(med, color=C.POSITIVE, lw=3, dash=False, label="中位")
    ax.vline(FRAME_US, color=C.WARN, lw=3, dash=True, label="实时预算")
    ax.autoscale()
    ax.set_ylim(0, float(counts.max()) * 1.25 if counts.size else 1.0)
    ax.grid(horizontal=True, vertical=False, xtick_labels=False)
    ax.axes_line()
    ax.frame()
    # 中位线标注画在轴区内右上方（白底），避免与柱体/刻度相压
    S.caption(cv, ax.x1 - 14, ax.y0 + 8, "中位 %.0f μs" % med, color=C.POSITIVE,
              fnt=S.FS_LABEL, anchor="rt", bg=True)

    cdf = np.cumsum(counts) / max(1.0, float(counts.sum())) * 100.0
    ax2 = cv.panel(rect=r1, ylabel="累计占比", xlabel="单帧耗时 (μs)")
    ax2.set_xlim(0, bins[-1])
    ax2.line(centers, cdf, color=C.PRIMARY, lw=3, label="累计分布")
    ax2.hline(95.0, color=C.WARN, lw=2, annotate="P95 = %.0f μs" % p95)
    ax2.autoscale()
    ax2.set_ylim(0, 105)
    ax2.grid(horizontal=True, vertical=True, xlabels=None)
    ax2.axes_line()
    ax2.frame()
    cv.note("RTF=单帧耗时/10000 μs，<1 即可实时；本例 RTF(中位)=%.4f，样本覆盖 %d 个变体。"
            % (rtf, len(used)))
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
    bins = ((0.0, lo, "无语音"),
            (lo, hi, "过渡"),
            (hi, 1.01, "语音"))
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
    cv.set_bottom_reserve(96)
    band = cv.set_legend_band(2)
    ax = cv.panel(rect=(cv.pad_l, max(cv.top0, band), cv.W - cv.pad_r, cv.H - cv.bottom0),
                  ylabel="衰减量 (dB)", xlabel="按 speech_prob_mean 分档")
    n_grp = len(bins)
    ax.set_xlim(0.4, n_grp * 2 + 0.6)
    positions = []
    med_labels = []          # (center, whishi, 文本)：箱体全部画完后再统一落笔
    cache = []               # (variant, center, stats)
    for vi, variant in enumerate((K.PRIMARY_VARIANT, K.COMPARE_VARIANT)):
        if variant not in groups:
            continue
        for gi, (lab, vals) in enumerate(groups[variant]):
            center = float(gi * 2 + 1 + vi * 0.78)
            positions.append(center)
            if vals.size:
                st = K.box_stats(vals)
                cache.append((variant, center, st))
                med_labels.append((center, st["whishi"], "%.1f" % st["med"]))
    # 在画箱体**之前**把 y 轴上限抬高 30%，给中位数标签留一条**专用空带**。
    # 理由（两条都踩过坑）：① `boxplot()` 与 `bars()` 一样用调用当时的 `ylim`
    # 换算像素并当场绘制，事后再 `set_ylim` 不会改变已画好的箱体；
    # ② 标签原先落在 `whishi` 正上方 24 px，而该处正好是箱体顶边框与离群点的位置，
    # 无论用白底还是描边去"覆盖"都会切断框线（视觉终检实测顶边框中断 32 px、
    # 中位线中断 28 px）。改"避开"——把标签整体移到箱体上方的空白带里。
    if cache:
        _lo_v = min(st["whislo"] for _, _, st in cache)
        _hi_v = max(st["whishi"] for _, _, st in cache)
        _span = max(_hi_v - _lo_v, 1e-6)
        ax.set_ylim(_lo_v - _span * 0.10, _hi_v + _span * 0.34)
    for variant, center, st in cache:
        ax.boxplot(st, center=center, width=0.6, color=_color_of(variant),
                   flier_hollow=True)
    if positions:
        ax.set_xlim(min(positions) - 0.8, max(positions) + 0.8)
    ax.autoscale()
    # 分档标签由下方**手绘**（跨越成对箱线并居中于档位），故刻度数字必须关掉——
    # 否则数字 1–6 与「无语音/过渡/语音」挤在同一行互相压盖（FigT21 曾如此）。
    ax.grid(xlabels=None, xtick_labels=False)
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
                             ("patch", C.OPTIMIZED, "优化 optimized", 0)])
    # 中位数标签**最后统一落笔**：两组变体是分两批画的，第二批的离群点串会盖住
    # 第一批已画好的标签（终检实测散点与「1.2」「0.6」的小数点有 7–8 个接触像素）。
    # 先画完全部箱体与散点、再画全部标签，标签的实心白底才真正生效。
    # 用**描边**而不是实心白底：白底矩形（高 = 字形高 + 4 px）会向下延伸约 18 px、
    # 把同档箱体的顶边框与中位线刷白（视觉终检实测）。描边只作用于字形边缘 3 px，
    # 既挡得住贴上来的散点，也不切断箱体框线。dy=-12 保证与须线帽（±1 px）留 11 px。
    for _cx, _whishi, _txt in med_labels:
        ax.text(_cx, _whishi, _txt, color=C.TEXT, fnt=S.FS_LABEL,
                anchor="cb", dy=-24.0, stroke=2)
    how_short = "固定阈值" if "固定阈值" in how else "数据分位 P33/P67"
    cv.note("分档 speech_prob_mean：<%.2f / %.2f–%.2f / ≥%.2f（%s）；箱线左右=基线/优化。"
            % (bins[0][1], bins[0][1], bins[1][1], bins[1][1], how_short))
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
    variants = [v for v in (K.PRIMARY_VARIANT, K.COMPARE_VARIANT)]
    per: dict[str, list[list[str]]] = {}
    highlights: dict[str, dict] = {}
    issue_total = 0
    checked = 0
    total_nan = 0
    detail = []
    for variant in variants:
        trows: list[list[str]] = []
        hl: dict = {}
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
            total_nan += n_nan
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
            trows.append([str(n), str(n_nan), str(out_of_range),
                          str(int(np.sum(m["clamp_hits"] > 0))),
                          "是" if issues else "否"])
            hl[(len(trows) - 1, 4)] = C.OPTIMIZED if issues else C.POSITIVE
            # NaN/Inf 帧位置：标到第 2 列以强调这不是 0 值
            if n_nan:
                hl[(len(trows) - 1, 1)] = C.NAN
            if issues:
                detail.append((variant, n, "、".join(issues),
                               "帧数 %d" % int(m["frame_idx"].size)))
            print("  %-10s %-3s NaN/Inf=%d(=%d+%d) 999哨兵=%d 衰减越界=%d 钳位帧=%d 超限=%d 负输出=%d"
                  % (variant, n, n_nan, nan_cnt, inf_cnt, sentinel, out_of_range,
                     int(np.sum(m["clamp_hits"] > 0)), over_clamp, neg_out))
        per[variant] = trows
        highlights[variant] = hl

    if checked == 0:
        print("SKIP FigT22 无任何 metrics CSV 可检查（%s）" % K.data_dir())
        return ""

    cv = S.Canvas("half", "FigT22", "失败与边界案例检查")
    # 表格重排（根因 A/B）：半页 625 px 放不下"7 列 × 19 行"的大表——旧版把表格
    # 铺到画布之外、列宽又不看内容，于是表头被截断、单元格跨列溢出、结论文字压表头。
    # 现在拆成**两个变体各一张 5 列小表并排**（行数 10，正好落在可用高度内），
    # 列宽由 `table()` 按内容自然宽度分配，结果文字移到表格**下方**。
    row_h = 34
    n_row = max(len(v) for v in per.values()) + 1
    tbl_top = cv.top0 - 6
    tbl_h = row_h * n_row
    gap_t = 26
    col_w = (cv.W - cv.pad_l - cv.pad_r - gap_t) / 2.0
    headers = ["文件", "NaN", "越界", "钳位", "异常"]
    for i, variant in enumerate(variants):
        x0 = cv.pad_l + i * (col_w + gap_t)
        S.caption(cv, x0, tbl_top - 30, "变体 %s（等级 %s）" % (variant, LEVEL),
                  color=_color_of(variant), fnt=S.FS_LABEL)
        ax_tmp = cv.panel(rect=(x0, tbl_top, x0 + col_w, tbl_top + tbl_h))
        ax_tmp.table(headers, per[variant],
                     rect=(x0, tbl_top, x0 + col_w, tbl_top + tbl_h),
                     fnt=S.FS_MIN, row_h=row_h, highlight=highlights[variant], pad=12)

    y_after = tbl_top + tbl_h + 16
    if issue_total == 0:
        msg = ("未检出异常：%d 份 CSV 的 NaN/Inf=0、衰减越界=0、钳位超限=0、负输出=0"
               % checked)
        col = C.POSITIVE
    else:
        msg = "检出 %d 处需关注（NaN/Inf 共 %d 帧）：%s" % (
            issue_total, total_nan,
            "；".join("%s/%s %s" % (v, n, d) for v, n, d, _ in detail[:2]))
        col = C.OPTIMIZED
    # 结论文字用 `fit_text` 落笔：**自动折行 + 夹紧在画布内**。
    # `caption` 不折行，长句会直接冲出右边界（曾使 FigT22 右边距变成 0 px）。
    ax_txt = cv.panel(rect=(cv.pad_l, y_after, cv.W - cv.pad_r, y_after + 44))
    ax_txt.fit_text(cv.pad_l, y_after + 16, msg, S.font(S.FS_MIN, True),
                    cv.W - cv.pad_l - cv.pad_r, col, center=False, bold=True, max_lines=2)
    cv.note("判据：NaN/Inf 覆盖 6.2 全 12 列；衰减越界 >%.0f 或 <%.0f dB；钳位超限 >%d。"
            "哨兵 999 dB 属契约允许值，不计入异常。" % (ATTEN_MAX_DB, ATTEN_MIN_DB, CLAMP_LIMIT))
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
