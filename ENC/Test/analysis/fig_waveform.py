# -*- coding: utf-8 -*-
"""
FigT03 / FigT04 — 波形对比

数据源（计划书 6.1）
-------------------
原始：`Test_Audio/AudioSample-48000hz/1.wav`（2ch/16bit/48k，取**左声道**）
基线：`<DATA_DIR>/baseline/L3/1.wav`（1ch/16bit/48k）
优化：`<DATA_DIR>/optimized/L3/1.wav`

版位
----
FigT03 全段三行对比 —— 条带图 1800×445
FigT04 5.0–5.5 s 放大  —— 半页图 900×625

独立运行：`python fig_waveform.py`
缺失数据时打印 `SKIP <原因>` 并以退出码 0 结束。
"""

from __future__ import annotations

import sys

import numpy as np

import common as K
import fig_style as S
from fig_style import C

NAME = "1"
LEVEL = K.PRIMARY_LEVEL
TRACKS = (
    ("原始（左声道）", "input", C.TEXT),
    ("基线 baseline", "baseline", C.BASELINE),
    ("优化 optimized", "optimized", C.OPTIMIZED),
)


def load_tracks() -> tuple[dict, int]:
    """读取三条波形；任一缺失返回空 dict。"""
    data: dict[str, np.ndarray] = {}
    rate = K.SAMPLE_RATE
    try:
        x, rate = K.read_input_left(NAME)
        data["input"] = x
    except (OSError, ValueError) as exc:
        print("[fig_waveform] 无法读取原始音频: %s" % exc)
    for variant in (K.PRIMARY_VARIANT, K.COMPARE_VARIANT):
        path = K.wav_path(variant, LEVEL, NAME)
        try:
            x, rate = K.read_wav_mono(path)
            data[variant] = x
        except (OSError, ValueError) as exc:
            print("[fig_waveform] 无法读取 %s: %s" % (path, exc))
    return data, rate


# --------------------------------------------------------------------------
# FigT03 —— 全段三行（条带图）
# --------------------------------------------------------------------------

def fig_t03(data: dict, rate: int) -> str:
    missing = [t[1] for t in TRACKS if t[1] not in data]
    if missing:
        print("SKIP FigT03 缺少波形数据: %s" % ", ".join(missing))
        return ""

    n = max(int(data[t[1]].size) for t in TRACKS)
    cv = S.Canvas("strip", "FigT03",
                  "波形对比（1.wav 全段）：原始 / 基线 / 优化（三行共用 ±32768 满量程）")

    # 垂直布局（条带画布 1800×445，高度紧张，故不设底部注释行——说明并入标题）：
    #   标题带 0–60；三条轨道自 y=72 起，每条 = 24 px 名称 + 64 px 轴，行距 96；
    #   第三条轴底 = 352，x 轴标题居中于 y≈398，画布底部保留 ≥8 px 白边。
    top = 68.0
    label_h = 24
    ax_h = 62
    row_pitch = label_h + ax_h + 12

    for i, (label, key, color) in enumerate(TRACKS):
        y0 = top + i * row_pitch
        S.caption(cv, cv.pad_l, y0, label, color=color, fnt=S.FS_LABEL)
        ax_top = y0 + label_h
        ax = cv.panel(rect=(cv.pad_l, ax_top, cv.W - cv.pad_r, ax_top + ax_h),
                      ylabel="幅度" if i == 1 else "",
                      xlabel="时间 (s)" if i == 2 else "")
        samples = np.asarray(data[key], dtype=np.float64)
        n_cols = max(2, int(ax.width))
        lo, hi = K.downmix_envelope(samples, n_cols)
        xsec = np.arange(n_cols, dtype=np.float64) * (samples.size / float(n_cols)) / rate
        ax.envelope(xsec, lo, hi, color=color)
        ax.set_ylim(-32768, 32767)
        ax.set_xlim(0, samples.size / float(rate))
        ax.autoscale()
        ax.grid(horizontal=True, vertical=True)
        ax.axes_line()
        ax.frame()
        print("  %-16s 样本=%d  峰值=%d  RMS=%.1f" %
              (label, samples.size, int(np.max(np.abs(samples))), K.rms(samples)))

    return cv.save("FigT03_波形对比全段.png")


# --------------------------------------------------------------------------
# FigT04 —— 5.0–5.5 s 放大（半页图）
# --------------------------------------------------------------------------

def fig_t04(data: dict, rate: int, t_start: float = 5.0, span: float = 0.5) -> str:
    """放大窗口默认 5.0–5.5 s；若音频短于该窗口则自动回退到有数据的区段并标注实际范围。"""
    missing = [t[1] for t in TRACKS if t[1] not in data]
    if missing:
        print("SKIP FigT04 缺少波形数据: %s" % ", ".join(missing))
        return ""

    n_samp = min(int(data[t[1]].size) for t in TRACKS)
    dur = n_samp / float(rate)
    adjusted = False
    if dur <= t_start + span:
        # 回退：居中取一段（仍保持 span 宽度，不足则缩短）
        span = min(span, max(dur * 0.5, 0.05))
        t_start = max(0.0, (dur - span) / 2.0)
        adjusted = True
        print("  [FigT04] 音频长度 %.3f s < 请求窗口，回退为 %.3f–%.3f s"
              % (dur, t_start, t_start + span))
    t_end = min(t_start + span, dur)
    i0, i1 = int(t_start * rate), int(t_end * rate)

    cv = S.Canvas("half", "FigT04", "波形对比（放大 %.2f–%.2f s）" % (t_start, t_end))
    band = cv.set_legend_band(2)
    ax = cv.panel(rect=(cv.pad_l, max(cv.top0, band), cv.W - cv.pad_r, cv.H - cv.bottom0),
                  ylabel="幅度", xlabel="时间 (s)")
    ax.set_ylim(-32768, 32767)
    ax.set_xlim(t_start, t_end)

    drawn = 0
    for i, (label, key, color) in enumerate(TRACKS):
        seg = np.asarray(data[key][i0:i1], dtype=np.float64)
        if seg.size == 0:
            print("  [FigT04] %s 在 %.2f–%.2f s 无样本，跳过" % (label, t_start, t_end))
            continue
        off = (i - 1) * 24000  # 三行错开，避免完全重叠
        t = np.linspace(t_start, t_start + seg.size / float(rate), seg.size, endpoint=False)
        ax.line(t, seg + off, color=color, lw=2, label=label)
        drawn += 1
        print("  %-16s 片段样本=%d  峰值=%.0f  RMS=%.1f" %
              (label, seg.size, float(np.max(np.abs(seg))), K.rms(seg)))
    if drawn == 0:
        print("SKIP FigT04 回退窗口内仍无样本")
        return ""

    ax.autoscale()
    ax.grid()
    ax.axes_line()
    ax.frame()
    ax.legend("tr")
    cv.note("三条曲线纵向各偏移 ±24000 以分离显示；纵轴刻度为名义值 + 偏移量。%s"
            % ("（窗口已按音频长度自动回退）" if adjusted else ""))
    return cv.save("FigT04_波形对比放大.png")


def main() -> int:
    S.warn_if_formal_output("fig_waveform.py")
    print("=" * 84)
    print("[fig_waveform] 原始目录 %s" % K.input_audio_dir())
    print("[fig_waveform] 输出目录 %s" % K.figures_dir())
    print("=" * 84)
    data, rate = load_tracks()
    if not data:
        print("SKIP FigT03/FigT04 无任何可用波形（原始 %s/1.wav 或两份输出 WAV 均缺失）"
              % K.input_audio_dir())
        return 0
    print("已载入通道: %s，采样率 %d Hz" % (", ".join(sorted(data)), rate))
    fig_t03(data, rate)
    fig_t04(data, rate)
    return 0


if __name__ == "__main__":
    sys.exit(main())
