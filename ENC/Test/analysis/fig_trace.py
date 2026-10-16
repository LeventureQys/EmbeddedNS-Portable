# -*- coding: utf-8 -*-
"""
FigT07 / FigT09 / FigT10 / FigT11 / FigT12 / FigT16 — 逐帧指标轨迹

数据源：`<DATA_DIR>/{baseline,optimized}/L3/metrics_1.csv`（计划书 6.2 列定义）

| 图号 | 指标 | 备注 |
|------|------|------|
| FigT07 | `signal_energy` | 对数纵轴 |
| FigT08 | `Y[257]` 反推 见 `fig_summary.py` | 半页 |
| FigT09 | `diff_normalization` | |
| FigT10 | `speech_prob_mean` | |
| FigT11 | `gain_adjustment` | |
| FigT12 | `atten_db` | 哨兵 999.0 视为无效点断线 |
| FigT16 | 启动期前 200 帧放大 | 双面板 |

版位：除 FigT08 外全部为条带图 1800×445（FigT08 在 `fig_summary.py`）。
独立运行：`python fig_trace.py`
"""

from __future__ import annotations

import sys

import numpy as np

import common as K
import fig_style as S
from fig_style import C

NAME = "1"
LEVEL = K.PRIMARY_LEVEL
STARTUP_FRAMES = 200


def load_metrics(variants=None) -> dict:
    """读取指定变体的 metrics；返回 {variant: dict}，缺失的 variant 不出现。

    NaN 处理原则：**不替换、不丢弃**——原样保留，由绘图层显式标记 NaN 帧位置。
    """
    out: dict = {}
    for variant in (variants or (K.PRIMARY_VARIANT, K.COMPARE_VARIANT)):
        if not K.exists_metrics(variant, LEVEL, NAME):
            print("[fig_trace] 缺失 metrics: %s" % K.metrics_path(variant, LEVEL, NAME))
            continue
        try:
            m = K.read_metrics(variant, LEVEL, NAME)
        except (OSError, ValueError) as exc:
            print("[fig_trace] 读取失败 %s: %s" % (variant, exc))
            continue
        # 哨兵 atten_db==999 为契约允许值，与 NaN 缺陷性质不同，分别统计
        sentinel = int(np.sum(np.abs(m["atten_db"] - K.ATTEN_DB_SENTINEL) < 1e-6))
        m["_sentinel_count"] = sentinel
        m["_atten_valid"] = np.where(np.abs(m["atten_db"] - K.ATTEN_DB_SENTINEL) < 1e-6,
                                     np.nan, m["atten_db"])
        rep = K.nan_report(m)
        m["_nan"] = rep
        m["_frames"] = int(rep.get("_frames", 0))
        out[variant] = m
        bad = {c: v["invalid"] for c, v in rep.items()
               if c != "_frames" and isinstance(v, dict) and v["invalid"]}
        print("  %-10s 帧数=%d  atten 有效 %d / 哨兵 %d  elapsed 中位 %.0f μs%s" %
              (variant, m["frame_idx"].size, m["frame_idx"].size - sentinel, sentinel,
               float(np.median(m["elapsed_us"])),
               ("  NaN/Inf: %s" % ", ".join("%s×%d" % (c, n) for c, n in bad.items()))
               if bad else "  NaN/Inf: 无"))
    return out


def _both(metrics: dict):
    return [(v, metrics[v]) for v in (K.PRIMARY_VARIANT, K.COMPARE_VARIANT) if v in metrics]


def _style_of(variant: str) -> str:
    return C.VARIANT.get(variant, C.PRIMARY)


def _label_of(variant: str) -> str:
    return K.VARIANT_LABEL.get(variant, variant)


def _rng(arr: np.ndarray, fn) -> str:
    """对含 NaN 的数组取范围，全 NaN 时返回 'n/a'。"""
    with np.errstate(all="ignore"):
        v = fn(arr)
    return "%.4f" % float(v) if np.isfinite(v) else "n/a"


# --------------------------------------------------------------------------
# 通用：单指标双曲线条带图
# --------------------------------------------------------------------------

def trace_figure(fig_id: str, title: str, column: str, filename: str,
                 ylabel: str, logy: bool = False, xlabel: str = "帧序号（10 ms/帧）",
                 extra_note: str = "", use_valid_atten: bool = False,
                 mark_nan: bool = False) -> str:
    """单指标双曲线条带图。

    `mark_nan=True` 时把该列的 NaN/Inf 帧画成醒目竖带并在图例中注明
    （D8 缺陷下 `speech_prob_mean` 会出现 NaN 字面量；不做静默丢点）。

    空间分配（条带画布只有 445 px 高，必须显式重排而非就地挤）：
    图例横排在标题与轴区之间的空档，**不再使用会吃掉 162 px 的「图例带」**——
    那条带曾把轴区压到 95 px，使 5 个对数刻度值（1e+14…1e+10）互相叠印、
    竖排轴标题也放不下而退化成压数据的横排贴字。
    """
    metrics = load_metrics()
    pairs = _both(metrics)
    if not pairs:
        print("SKIP %s 无可用 metrics 数据（%s）" % (fig_id, K.metrics_path(K.PRIMARY_VARIANT, LEVEL, NAME)))
        return ""
    cv = S.Canvas("strip", fig_id, title)
    # 图例：基线与优化的曲线在本批数据上大量重合，用「实线 / 虚线」区分，
    # 避免其中一条被完全盖住而让图例与所见不符。
    entries = []
    for i, (v, _m) in enumerate(pairs):
        entries.append(("dash" if i == 0 else "line", _style_of(v), _label_of(v)))
    S.legend_strip(cv, cv.pad_l, cv.top0 - 20, entries)
    cv.set_bottom_reserve(64)
    ax = cv.panel(rect=(cv.pad_l, cv.top0 + 26, cv.W - cv.pad_r, cv.H - cv.bottom0 - 34),
                  ylabel=ylabel, xlabel=xlabel, ylog=logy)

    nan_info = []
    for i, (variant, m) in enumerate(pairs):
        key = "_atten_valid" if use_valid_atten else column
        y = np.asarray(m[key], dtype=np.float64)
        x = np.asarray(m["frame_idx"], dtype=np.float64)
        if logy:
            y = np.where(y > 0, y, np.nan)
        ax.line(x, y, color=_style_of(variant), lw=3, label=_label_of(variant),
                dash=(i == 0))
        finite = y[np.isfinite(y)]
        n_nan = int(np.sum(np.isnan(y)))
        if finite.size:
            print("  %-10s %-22s 帧=%d  min=%.5g  max=%.5g  中位=%.5g  末值=%.5g  NaN=%d" %
                  (variant, column, x.size, float(finite.min()), float(finite.max()),
                   float(np.median(finite)), float(finite[-1]), n_nan))
        if n_nan:
            nan_info.append((variant, n_nan, x.size))
    if mark_nan and nan_info:
        for variant, m in pairs:
            if ~np.isnan(np.asarray(m[column], dtype=np.float64)).any():
                continue
            ax.nan_bands(np.isnan(np.asarray(m[column], dtype=np.float64)),
                         label="%s NaN" % _label_of(variant))

    ax.autoscale()
    ax.grid()
    ax.axes_line()
    ax.frame()
    note = extra_note or ("纵轴：%s；横轴：帧序号（10 ms/帧，480 样本）。" % ylabel)
    # 只有当调用方**尚未**说明 NaN 处理时，才追加这段补充；否则同一件事会说两遍，
    # 把注释撑成两行、在 445 px 高的条带图里被整段丢弃（FigT10 曾如此）。
    if nan_info and "NaN" not in note:
        note += "  NaN/Inf 帧已用竖带标出（%s），非缺失数据，不作零值替换。" % \
            "、".join("%s %d/%d" % (K.VARIANT_LABEL.get(v, v), n, t) for v, n, t in nan_info)
    cv.note(note)
    return cv.save(filename)


# --------------------------------------------------------------------------
# FigT16 —— 启动期放大（双面板条带）
# --------------------------------------------------------------------------

def fig_t16() -> str:
    metrics = load_metrics()
    pairs = _both(metrics)
    if not pairs:
        print("SKIP FigT16 无可用 metrics 数据")
        return ""
    cv = S.Canvas("strip", "FigT16", "启动期放大（前 %d 帧 / 2.0 s）" % STARTUP_FRAMES)
    # 三面板各画一个图例时，图例框（约 260×90 px）会遮断 442 px 宽面板里的曲线中段
    # （实测三个面板全部被遮断）。改为在标题与轴区之间的空档放**一条共用图例**，
    # 三个面板的图例项完全同构（基线/优化），共用不丢任何信息。
    S.legend_strip(cv, cv.pad_l, cv.top0 - 20,
                   [("dash", _style_of(K.PRIMARY_VARIANT), _label_of(K.PRIMARY_VARIANT)),
                    ("line", _style_of(K.COMPARE_VARIANT), _label_of(K.COMPARE_VARIANT))])
    cv.set_bottom_reserve(64)
    # 列间距必须容得下第二列**竖排 y 轴标题**的横向宽度（刻度字宽 + 间隙 + 标题宽
    # ≈ 116 px）。原 gap_x=52 时第二列的「语音概率」被画进第一列绘图区，与曲线交叠。
    rects = S.Axes.grid_rects((cv.pad_l, cv.top0 + 26, cv.W - cv.pad_r, cv.H - cv.bottom0 - 34),
                              nrow=1, ncol=3, gap_x=126)

    ax1 = cv.panel(rect=rects[0], ylabel="衰减量", xlabel="帧序号（10 ms/帧）")
    ax1.set_xlim(-2, STARTUP_FRAMES + 2)
    for variant, m in pairs:
        n = min(STARTUP_FRAMES, m["_atten_valid"].size)
        ax1.line(m["frame_idx"][:n], m["_atten_valid"][:n],
                 color=_style_of(variant), lw=3, dash=(variant == K.PRIMARY_VARIANT))
        seg = m["_atten_valid"][:n]
        fin = seg[np.isfinite(seg)]
        print("  %-10s 前 %d 帧 atten: 均值 %.3f dB  最大 %.3f dB" %
              (variant, n, float(fin.mean()) if fin.size else 0.0,
               float(fin.max()) if fin.size else 0.0))
    ax1.autoscale()
    ax1.grid()
    ax1.axes_line()
    ax1.frame()

    ax2 = cv.panel(rect=rects[1], ylabel="语音概率", xlabel="帧序号（10 ms/帧）")
    ax2.set_xlim(-2, STARTUP_FRAMES + 2)
    # 中面板只画 speech_prob_mean（0–1）。
    # 注意：`diff_normalization` 实测量级为 1e10~1e13（非 0–1），与概率量跨 14 个数量级，
    # 同轴线性混画会把概率曲线压成贴地直线。故拆到下方第 3 面板（对数量纲），不混画。
    drawn = False
    for variant, m in pairs:
        n = min(STARTUP_FRAMES, m["speech_prob_mean"].size)
        if n == 0:
            continue
        ax2.line(m["frame_idx"][:n], m["speech_prob_mean"][:n],
                 color=_style_of(variant), lw=3, dash=(variant == K.PRIMARY_VARIANT))
        drawn = True
    if not drawn:
        print("SKIP FigT16 中面板无数据")
    ax2.autoscale()
    ax2.set_ylim(-0.05, 1.05)
    ax2.grid()
    ax2.axes_line()
    ax2.frame()

    # ---- 第三面板：diff_normalization（对数量纲独立成图）----
    ax3 = cv.panel(rect=rects[2], ylabel="log10(D)", xlabel="帧序号（10 ms/帧）",
                   xlog=False)
    ax3.set_xlim(-2, STARTUP_FRAMES + 2)
    for variant, m in pairs:
        n = min(STARTUP_FRAMES, m["diff_normalization"].size)
        if n == 0:
            continue
        dn = np.asarray(m["diff_normalization"][:n], dtype=np.float64)
        dn = np.where(np.isfinite(dn) & (dn > 0), np.log10(np.maximum(dn, 1e-300)), np.nan)
        ax3.line(m["frame_idx"][:n], dn, color=_style_of(variant), lw=3,
                 dash=(variant == K.PRIMARY_VARIANT))
    ax3.autoscale()
    ax3.grid()
    ax3.axes_line()
    ax3.frame()

    for variant, m in pairs:
        n = min(STARTUP_FRAMES, m["speech_prob_mean"].size)
        sp = np.asarray(m["speech_prob_mean"][:n], dtype=np.float64)
        dn = np.asarray(m["diff_normalization"][:n], dtype=np.float64)
        print("  %-10s speech_prob_mean 范围 %s…%s（NaN %d）  diff_norm 范围 %s…%s（NaN %d）"
              "  log10(diff_norm) 范围 %s…%s" %
              (variant, _rng(sp, np.nanmin), _rng(sp, np.nanmax), int(np.isnan(sp).sum()),
               _rng(dn, np.nanmin), _rng(dn, np.nanmax), int(np.isnan(dn).sum()),
               _rng(np.log10(np.where(dn > 0, dn, np.nan)), np.nanmin),
               _rng(np.log10(np.where(dn > 0, dn, np.nan)), np.nanmax)))

    cv.note("三面板同帧轴（前 200 帧）：衰减量 / 语音概率 / log10(diff_normalization)；"
            "后者量级 1e10–1e13，独立成图不与 0–1 量混画。")
    return cv.save("FigT16_启动期放大.png")


def main() -> int:
    S.warn_if_formal_output("fig_trace.py")
    print("=" * 84)
    print("[fig_trace] 指标目录 %s" % K.data_dir())
    print("[fig_trace] 输出目录 %s" % K.figures_dir())
    print("=" * 84)
    trace_figure("FigT07", "signal_energy 轨迹对比（对数纵轴）", "signal_energy",
                 "FigT07_signal_energy轨迹.png", "信号能量", logy=True)
    trace_figure("FigT09", "diff_normalization 轨迹对比（对数纵轴）", "diff_normalization",
                 "FigT09_diff_normalization轨迹.png", "diff_norm", logy=True,
                 extra_note="纵轴 diff_normalization（对数量纲 1e10–1e13，故取对数）；"
                            "非正值/NaN 帧自动断线，不作零值替换。")
    trace_figure("FigT10", "speech_prob_mean 轨迹对比（NaN 帧已标出）", "speech_prob_mean",
                 "FigT10_speech_prob_mean轨迹.png", "语音概率均值",
                 mark_nan=True,
                 extra_note="纵轴 speech_prob_mean（0–1）；NaN/Inf 帧以竖带标出"
                            "（D8 缺陷），不替换零值。")
    trace_figure("FigT11", "gain_adjustment 轨迹对比", "gain_adjustment",
                 "FigT11_gain_adjustment轨迹.png", "增益调整量")
    trace_figure("FigT12", "逐帧 atten_db 轨迹对比", "atten_db",
                 "FigT12_逐帧atten_db轨迹.png", "衰减量 (dB)",
                 extra_note="衰减量 = 20·log10(in_rms/out_rms)；out_rms=0 的帧记为哨兵 999 dB，已置为断点。",
                 use_valid_atten=True)
    fig_t16()
    return 0


if __name__ == "__main__":
    sys.exit(main())
