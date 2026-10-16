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
    label_h = 34

    # 垂直预算（根因 A：三行"行标题 + 刻度带 + 注释"相加超出可用高度就会互相压叠）：
    #   ① 用 `stack_rows` 把行标题计入预算（不再让行标题落到上一行的刻度带里）；
    #   ② x 轴三行共享同一时间列 → 只有最后一行画刻度标签与轴标题，
    #      前两行让出的刻度带高度全部还给声谱本体。
    rows = cv.stack_rows(3, top=cv.top0 + 10, bottom=cv.H - cv.bottom0 - 34,
                         gap=26, caption_h=label_h,
                         x_left=cv.pad_l, x_right=plot_x1)

    for i, (label, key, color, short) in enumerate(SERIES):
        rect, cap_y = rows[i]
        f, t, db = specs[key]
        S.caption(cv, rect[0], cap_y, label, color=color, fnt=S.FS_LABEL)
        ax = cv.panel(rect=rect, ylabel="频率 (Hz)" if i == 1 else "",
                      xlabel="时间 (s)" if i == 2 else "")
        # 热力图必须铺在 **Axes 实际生效的矩形**上：`panel()` 会为 x 轴刻度带与
        # 轴标题向上收紧 y1，若沿用传入的 rect 值，热力图会铺到刻度带里面去
        # （FigT06 下图曾因此把 x 刻度与轴标题画在差谱数据区内部）。
        x0, y0, x1, y1 = ax.x0, ax.y0, ax.x1, ax.y1
        band_rows = freq_band_rows(f, F_LO, F_HI)
        ax.heatmap(db[:, band_rows] if db.size else np.zeros((1, 1)), vmin, vmax,
                   cmap=S.SEQ, ext=(x0, y0, x1, y1),
                   x_range=(0.0, t_end), y_range=(F_LO, F_HI))
        ax.grid(vertical=(i == 2), horizontal=True, xtick_labels=(i == 2))
        ax.frame()
        if i == 0:
            ax.colorbar(S.SEQ, vmin, vmax,
                        (x1 + 22, y0 + 6, x1 + 58, y1 - 6),
                        label="相对幅度 (dB)")

    cv.note("纵轴对数刻度；横轴为帧中心时间；dB 以各自全局峰值为 0 dB 参考。")
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
    label_h = 34

    # 垂直预算（与 FigT05 同源重排）：
    #   ① 两张谱图共用同一条时间轴 → 只有下图画 x 刻度标签与轴标题，
    #      上图让出的刻度带高度还给声谱本体；
    #   ② 子图标题计入 `stack_rows` 预算，不再落进上图的刻度带里；
    #   ③ 热力图的 `ext` 一律取 `Axes` **实际生效**的矩形——旧版用了传入的
    #      未收紧矩形，把下图整排 x 刻度与「时间(s)」画进了差谱数据区内部。
    cv.set_bottom_reserve(100)
    stack = cv.stack_rows(2, top=cv.top0 + 10, bottom=cv.H - cv.bottom0 - 34,
                          gap=50, caption_h=label_h,
                          x_left=cv.pad_l, x_right=plot_x1)

    # 上：统一尺度的基线谱
    f_b, t_b, db_b = spectrogram_db(data["baseline"])
    rect, cap_y = stack[0]
    S.caption(cv, rect[0], cap_y, "基线谱（参考尺度 −100…0 dB）",
              color=C.BASELINE, fnt=S.FS_LABEL)
    ax = cv.panel(rect=rect, ylabel="频率 (Hz)", xlabel="")
    bx0, by0, bx1, by1 = ax.x0, ax.y0, ax.x1, ax.y1
    ax.heatmap(db_b[:, rows], -100.0, 0.0, cmap=S.SEQ,
               ext=(bx0, by0, bx1, by1), x_range=(0.0, t_end), y_range=(F_LO, F_HI))
    ax.grid(vertical=False, horizontal=True, xtick_labels=False)
    ax.frame()
    ax.colorbar(S.SEQ, -100.0, 0.0,
                (bx1 + 22, by0 + 6, bx1 + 58, by1 - 6), label="幅度 (dB)")

    # 下：差值图（发散色带）
    rect, cap_y = stack[1]
    S.caption(cv, rect[0], cap_y, "差值：基线 − 优化（−12…+12 dB，白=无差异）",
              color=C.OPTIMIZED, fnt=S.FS_LABEL)
    ax = cv.panel(rect=rect, ylabel="频率 (Hz)", xlabel="时间 (s)")
    bx0, by0, bx1, by1 = ax.x0, ax.y0, ax.x1, ax.y1
    ax.heatmap(diff[:, rows], -vlim, vlim, cmap=S.DIV,
               ext=(bx0, by0, bx1, by1), x_range=(0.0, t_end), y_range=(F_LO, F_HI))
    ax.grid(vertical=True, horizontal=True)
    ax.frame()
    ax.colorbar(S.DIV, -vlim, vlim,
                (bx1 + 22, by0 + 6, bx1 + 58, by1 - 6), label="差值 (dB)")

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
    band = cv.set_legend_band(3)
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
