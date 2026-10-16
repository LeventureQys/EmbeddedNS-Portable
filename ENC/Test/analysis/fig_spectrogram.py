# -*- coding: utf-8 -*-
"""
FigT05 / FigT06 / FigT14 — 声谱与频谱

数据源
------
原始：`Test_Audio/AudioSample-48000hz/1.wav`（取左声道）
基线：`<DATA_DIR>/baseline/L3/1.wav`
优化：`<DATA_DIR>/optimized/L3/1.wav`

方法（计划书 9.4）
------------------
* STFT：512 点 FFT、汉宁窗、50% 重叠（hop=256）、单边谱、`20*log10` 转 dB
* 频率轴按对数刻度展示 30 Hz … 24 kHz
* FigT06：基线谱 − 优化谱（dB 差值，发散色带，0 为白）
* FigT14：LTAS 三条曲线（对数频率轴）

版位：FigT05 整页 / FigT06 整页 / FigT14 半页
独立运行：`python fig_spectrogram.py`
"""

from __future__ import annotations

import sys

import numpy as np

import common as K
import fig_style as S
from fig_style import C

NAME = "1"
LEVEL = K.PRIMARY_LEVEL
N_FFT = 512
HOP = 256
F_LO, F_HI = 30.0, 24000.0

SERIES = (
    ("原始（左声道）", "input", C.TEXT, "原始"),
    ("基线 baseline", "baseline", C.BASELINE, "基线"),
    ("优化 optimized", "optimized", C.OPTIMIZED, "优化"),
)


def load_audio() -> dict:
    data: dict[str, np.ndarray] = {}
    try:
        x, rate = K.read_input_left(NAME)
        data["input"] = x
    except (OSError, ValueError) as exc:
        print("[fig_spectrogram] 原始音频不可用: %s" % exc)
    for variant in (K.PRIMARY_VARIANT, K.COMPARE_VARIANT):
        try:
            data[variant], _ = K.read_wav_mono(K.wav_path(variant, LEVEL, NAME))
        except (OSError, ValueError) as exc:
            print("[fig_spectrogram] %s 不可用: %s" % (variant, exc))
    return data


def spectrogram_db(x: np.ndarray, floor_db: float = -110.0) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """512 点 STFT → (freq, time, dB)。dB 以全局峰值为 0 dB 参考。"""
    t, f, mag = K.stft_mag(x, n_fft=N_FFT, hop=HOP)
    if mag.size == 0:
        return f, t, np.zeros((0, f.size), dtype=np.float64)
    peak = float(mag.max())
    db = 20.0 * np.log10(np.maximum(mag, 1e-12) / max(peak, 1e-12))
    return f, t, np.maximum(db, floor_db)


def freq_band_rows(f: np.ndarray, lo: float, hi: float) -> np.ndarray:
    return np.where((f >= lo) & (f <= hi))[0]


# --------------------------------------------------------------------------
# FigT05 —— 声谱三联（整页）
# --------------------------------------------------------------------------

def fig_t05(data: dict) -> str:
    missing = [s[3] for s in SERIES if s[1] not in data]
    if missing:
        print("SKIP FigT05 缺少声谱数据: %s" % ", ".join(missing))
        return ""
    cv = S.Canvas("full", "FigT05", "声谱图三联：原始 / 基线 / 优化（512 点 STFT · 汉宁窗 · 50% 重叠）")
    specs = {}
    for label, key, color, short in SERIES:
        f, t, db = spectrogram_db(data[key])
        specs[key] = (f, t, db)
        print("  %-16s 帧数=%d  频点=%d  dB 范围 %.1f … %.1f" %
              (label, db.shape[0], db.shape[1],
               float(db.min()) if db.size else 0.0, float(db.max()) if db.size else 0.0))
    vmin, vmax = -100.0, 0.0

    t_end = float(max(specs[s[1]][1][-1] if specs[s[1]][1].size else 0.0 for s in SERIES))
    cbw = 84
    plot_x1 = cv.W - cv.pad_r - cbw - 46
    top = cv.top0 + 24
    row_gap = 26
    bottom = cv.H - cv.bottom0
    row_h = (bottom - top - row_gap * 2) / 3.0
    label_h = 30

    for i, (label, key, color, short) in enumerate(SERIES):
        f, t, db = specs[key]
        y0 = top + i * (row_h + row_gap)
        ax_h = row_h - label_h
        S.caption(cv, cv.pad_l, y0 - 2, label, color=color, fnt=S.FS_LABEL)
        ax = cv.panel(rect=(cv.pad_l, y0 + label_h, plot_x1, y0 + label_h + ax_h),
                      ylabel="频率 (Hz)", xlabel="时间 (s)" if i == 2 else "")
        rows = freq_band_rows(f, F_LO, F_HI)
        ax.heatmap(db[:, rows] if db.size else np.zeros((1, 1)), vmin, vmax,
                   cmap=S.SEQ, ext=(cv.pad_l, y0 + label_h, plot_x1, y0 + label_h + ax_h),
                   x_range=(0.0, t_end), y_range=(F_LO, F_HI))
        ax.grid(vertical=(i == 2), horizontal=True)
        ax.frame()
        if i == 0:
            ax.colorbar(S.SEQ, vmin, vmax,
                        (plot_x1 + 22, y0 + label_h + 6, plot_x1 + 58, y0 + label_h + ax_h - 6),
                        label="相对幅度 (dB)")

    cv.note("纵轴对数刻度；横轴为帧中心时间。dB 以各自全局峰值为 0 dB 参考。")
    return cv.save("FigT05_声谱三联.png")


# --------------------------------------------------------------------------
# FigT06 —— 声谱差异（整页）
# --------------------------------------------------------------------------

def fig_t06(data: dict, vlim: float = 12.0) -> str:
    need = ("baseline", "optimized")
    if any(k not in data for k in need):
        print("SKIP FigT06 缺少 baseline/optimized 输出音频")
        return ""
    f1, t1, db_base = spectrogram_db(data["baseline"])
    f2, t2, db_opt = spectrogram_db(data["optimized"])
    n_t = min(db_base.shape[0], db_opt.shape[0])
    if n_t == 0:
        print("SKIP FigT06 STFT 帧数为 0（音频过短）")
        return ""
    diff = db_base[:n_t] - db_opt[:n_t]
    rows = freq_band_rows(f1, F_LO, F_HI)
    t_end = float(t2[min(n_t, t2.size) - 1]) if t2.size else 0.0

    print("  差异 dB 范围 %.2f … %.2f  均值 %.3f  帧数 %d" %
          (float(diff.min()), float(diff.max()), float(diff.mean()), n_t))

    cv = S.Canvas("full", "FigT06", "声谱差异：基线谱 − 优化谱（dB，正=基线更强）")
    cbw = 84
    plot_x1 = cv.W - cv.pad_r - cbw - 46
    top = cv.top0 + 24 + 24
    bottom = cv.H - cv.bottom0
    row_gap = 30
    row_h = (bottom - top - row_gap) / 2.0
    label_h = 30

    # 上：统一尺度的基线谱
    f_b, t_b, db_b = spectrogram_db(data["baseline"])
    ax = cv.panel(rect=(cv.pad_l, top + label_h, plot_x1, top + label_h + row_h - label_h),
                  ylabel="频率 (Hz)", xlabel="")
    S.caption(cv, cv.pad_l, top - 2, "基线谱（参考尺度 −100…0 dB）", color=C.BASELINE, fnt=S.FS_LABEL)
    ax.heatmap(db_b[:, rows], -100.0, 0.0, cmap=S.SEQ,
               ext=(cv.pad_l, top + label_h, plot_x1, top + label_h + row_h - label_h),
               x_range=(0.0, t_end), y_range=(F_LO, F_HI))
    ax.grid(vertical=False, horizontal=True)
    ax.frame()
    ax.colorbar(S.SEQ, -100.0, 0.0,
                (plot_x1 + 22, top + label_h + 6, plot_x1 + 58, top + label_h + row_h - label_h - 6),
                label="幅度 (dB)")

    # 下：差值图（发散色带）
    y0 = top + row_h + row_gap
    ax = cv.panel(rect=(cv.pad_l, y0 + label_h, plot_x1, y0 + label_h + row_h - label_h),
                  ylabel="频率 (Hz)", xlabel="时间 (s)")
    S.caption(cv, cv.pad_l, y0 - 2, "差值：基线 − 优化（−12…+12 dB，白=无差异）",
              color=C.OPTIMIZED, fnt=S.FS_LABEL)
    ax.heatmap(diff[:, rows], -vlim, vlim, cmap=S.DIV,
               ext=(cv.pad_l, y0 + label_h, plot_x1, y0 + label_h + row_h - label_h),
               x_range=(0.0, t_end), y_range=(F_LO, F_HI))
    ax.grid(vertical=True, horizontal=True)
    ax.frame()
    ax.colorbar(S.DIV, -vlim, vlim,
                (plot_x1 + 22, y0 + label_h + 6, plot_x1 + 58, y0 + label_h + row_h - label_h - 6),
                label="差值 (dB)")

    cv.note("正差值（红）表示基线在该时频点能量更强，负差值（蓝）表示优化后更强。")
    return cv.save("FigT06_声谱差异.png")


# --------------------------------------------------------------------------
# FigT14 —— LTAS 对比（半页）
# --------------------------------------------------------------------------

def fig_t14(data: dict, n_fft: int = 1024) -> str:
    missing = [s[3] for s in SERIES if s[1] not in data]
    if missing:
        print("SKIP FigT14 缺少数据: %s" % ", ".join(missing))
        return ""
    cv = S.Canvas("half", "FigT14", "长时平均谱 LTAS 对比")
    band = cv.set_legend_band(2)
    ax = cv.panel(rect=(cv.pad_l, max(cv.top0, band), cv.W - cv.pad_r, cv.H - cv.bottom0),
                  ylabel="平均电平 (dB)", xlabel="频率 (Hz)", xlog=True)

    curves = []
    for label, key, color, short in SERIES:
        freq, level = K.ltas_db(data[key], n_fft=n_fft, n_points=241)
        curves.append((label, color, freq, level))
        band = (freq >= 100) & (freq <= 8000)
        print("  %-16s 100–8k 平均 %.2f dB  峰值 %.2f dB @ %.0f Hz" %
              (label, float(level[band].mean()), float(level.max()),
               float(freq[int(np.argmax(level))])))

    allv = np.concatenate([c[3] for c in curves])
    lo, hi = float(np.nanmin(allv)), float(np.nanmax(allv))
    pad = (hi - lo) * 0.08 or 1.0
    for label, color, freq, level in curves:
        ax.line(freq, level, color=color, lw=3, label=label)
    ax.set_ylim(lo - pad, hi + pad)
    ax.set_xlim(20.0, 20000.0)
    ax.autoscale()
    ax.grid(horizontal=True, vertical=True, xlabels=None)
    ax.axes_line()
    ax.frame()
    ax.legend("tl")
    cv.note("横轴对数刻度，范围 20 Hz–20 kHz；曲线为 1/3 倍频程网格上的平滑平均谱。")
    return cv.save("FigT14_LTAS对比.png")


def main() -> int:
    S.warn_if_formal_output("fig_spectrogram.py")
    print("=" * 84)
    print("[fig_spectrogram] 输入 %s" % K.data_dir())
    print("[fig_spectrogram] 输出 %s" % K.figures_dir())
    print("=" * 84)
    data = load_audio()
    if not data:
        print("SKIP FigT05/FigT06/FigT14 无可用音频数据")
        return 0
    print("已载入: %s" % ", ".join(sorted(data)))
    fig_t05(data)
    fig_t06(data)
    fig_t14(data)
    return 0


if __name__ == "__main__":
    sys.exit(main())
