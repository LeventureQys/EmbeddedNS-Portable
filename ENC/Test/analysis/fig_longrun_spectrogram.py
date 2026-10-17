# -*- coding: utf-8 -*-
"""十分钟语音的时频图（spectrogram）：处理前 vs 处理后。

横轴时间、纵轴频率、颜色表示该时频点的幅度强度——即"随时间变化的幅频谱"。

口径
----
    STFT：512 点 Hann 窗，帧移 480 样本（10 ms，与算法帧 1:1 对齐），
          64144 帧覆盖全部 30,789,216 个样本（641.44 s）。
    幅度：|X| 按满幅正弦校准后取 20·log10，单位 dBFS（0 dBFS = int16 满量程单频）。
    色带：深蓝=弱、白=强（fig_style.SEQ）。
    两栏共用同一色标与同一时间/频率轴，故颜色可跨栏直接比较。

产出
----
    FigL03_十分钟时频图.png     全程两栏（全段 0–8 kHz）
    FigL04_时频图局部放大.png   同一段 6 s 的放大，看清语音谐波与噪声底
"""

from __future__ import annotations

import os
import sys
import wave

import numpy as np

import common as K
import fig_style as S
from fig_style import C

SR = 48000
N_FFT = 512
HOP = 480
COHERENT_GAIN = 0.5
FULL_SCALE = 32768.0
NAME = "merged_speech_10min"
LONG_ROOT = os.path.join(K.data_dir(), "long_run")

F_LO, F_HI = 20.0, 8000.0          # NS 的维纳滤波作用带
VMIN, VMAX = -115.0, -25.0         # 色标范围 dBFS

ZOOM_T0 = 300.0                    # 局部放大起点（秒）
ZOOM_LEN = 6.0


def read_left(path: str) -> np.ndarray:
    with wave.open(path, "rb") as w:
        n, ch = w.getnframes(), w.getnchannels()
        raw = w.readframes(n)
    a = np.frombuffer(raw, dtype="<i2").astype(np.float64)
    return a[0::ch] if ch > 1 else a


def spectrogram_dbfs(x: np.ndarray, block: int = 2048) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """返回 (times, freqs, db)，db 形状 (n_win, n_bin)。"""
    win = np.hanning(N_FFT)
    n_win = 1 + (len(x) - N_FFT) // HOP
    n_bin = N_FFT // 2 + 1
    db = np.empty((n_win, n_bin), dtype=np.float32)
    scale = 2.0 / (N_FFT * COHERENT_GAIN) / FULL_SCALE
    for s in range(0, n_win, block):
        e = min(s + block, n_win)
        idx = (np.arange(s, e) * HOP)[:, None] + np.arange(N_FFT)[None, :]
        seg = x[idx] * win[None, :]
        mag = np.abs(np.fft.rfft(seg, axis=1)) * scale
        db[s:e] = 20.0 * np.log10(np.maximum(mag, 1e-12))
    freqs = np.fft.rfftfreq(N_FFT, 1.0 / SR)
    times = (np.arange(n_win) * HOP + N_FFT // 2) / float(SR)
    return times, freqs, db


def band_rows(freqs: np.ndarray) -> np.ndarray:
    return np.where((freqs >= F_LO) & (freqs <= F_HI))[0]


# --------------------------------------------------------------------------
# FigL03 —— 全程时频图（两栏）
# --------------------------------------------------------------------------

def fig_l03(times, rows, db_in, db_out) -> str:
    cv = S.Canvas("full", "FigL03",
                  "十分钟语音的时频图：处理前 vs 处理后（优化版 L3，颜色 = 该时频点的幅度）")
    cv.set_bottom_reserve(100)

    cbw = 84
    plot_x1 = cv.W - cv.pad_r - cbw - 46
    label_h = 34
    stack = cv.stack_rows(2, top=cv.top0 + 10, bottom=cv.H - cv.bottom0 - 34,
                          gap=50, caption_h=label_h, x_left=cv.pad_l, x_right=plot_x1)

    t_end = float(times[-1])
    panels = (("处理前（原始输入）", db_in, C.TEXT),
              ("处理后（优化版 L3）", db_out, C.OPTIMIZED))

    for i, (label, db, color) in enumerate(panels):
        rect, cap_y = stack[i]
        S.caption(cv, rect[0], cap_y, label, color=color, fnt=S.FS_LABEL)
        ax = cv.panel(rect=rect, ylabel="频率 (Hz)" if i == 0 else "",
                      xlabel="时间 (s)" if i == 1 else "")
        x0, y0, x1, y1 = ax.x0, ax.y0, ax.x1, ax.y1
        ax.heatmap(db[:, rows], VMIN, VMAX, cmap=S.SEQ, ext=(x0, y0, x1, y1),
                   x_range=(0.0, t_end), y_range=(F_LO, F_HI))
        ax.grid(vertical=(i == 1), horizontal=True, xtick_labels=(i == 1))
        ax.frame()
        ax.colorbar(S.SEQ, VMIN, VMAX, (x1 + 22, y0 + 6, x1 + 58, y1 - 6),
                    label="幅度 (dBFS)")

    cv.note("512 点 Hann 窗、帧移 10 ms，%d 帧覆盖全部 641.44 s；深蓝=弱、白=强，两栏共用色标。"
            "对比看：处理后语音谐波条纹保留，条纹之间的底色整体变暗。" % len(times))
    for o in cv.overlaps(min_area=150.0):
        print("  [自检] 重叠 %s/%s 面积=%.0f px2" % (o["a"], o["b"], o["area"]))
    return cv.save("FigL03_十分钟时频图.png")


# --------------------------------------------------------------------------
# FigL04 —— 局部放大（三栏：前 / 后 / 差值）
# --------------------------------------------------------------------------

def fig_l04(times, rows, db_in, db_out) -> str:
    i0 = int(np.searchsorted(times, ZOOM_T0))
    i1 = int(np.searchsorted(times, ZOOM_T0 + ZOOM_LEN))
    if i1 - i0 < 10:
        print("SKIP FigL04 放大区间帧数不足")
        return ""
    seg_in = db_in[i0:i1]
    seg_out = db_out[i0:i1]
    seg_t = times[i0:i1]
    diff = seg_in - seg_out

    cv = S.Canvas("full", "FigL04",
                  "时频图局部放大：%.1f-%.1f s（同一段音频，改前 / 改后 / 差值）"
                  % (ZOOM_T0, ZOOM_T0 + ZOOM_LEN))
    cv.set_bottom_reserve(100)

    cbw = 84
    plot_x1 = cv.W - cv.pad_r - cbw - 46
    label_h = 30
    stack = cv.stack_rows(3, top=cv.top0 + 6, bottom=cv.H - cv.bottom0 - 30,
                          gap=34, caption_h=label_h, x_left=cv.pad_l, x_right=plot_x1)
    t_end = float(seg_t[-1])

    entries = (("处理前", seg_in, S.SEQ, VMIN, VMAX, "幅度 (dBFS)", C.TEXT),
               ("处理后", seg_out, S.SEQ, VMIN, VMAX, "幅度 (dBFS)", C.OPTIMIZED),
               ("差值 前-后", diff, S.DIV, -18.0, 18.0, "差值 (dB)", C.WARN))

    for i, (label, arr, cmap, vmin, vmax, cblab, col) in enumerate(entries):
        rect, cap_y = stack[i]
        S.caption(cv, rect[0], cap_y, label, color=col, fnt=S.FS_LABEL)
        ax = cv.panel(rect=rect, ylabel="频率 (Hz)" if i == 0 else "",
                      xlabel="时间 (s)" if i == 2 else "")
        x0, y0, x1, y1 = ax.x0, ax.y0, ax.x1, ax.y1
        ax.heatmap(arr[:, rows], vmin, vmax, cmap=cmap, ext=(x0, y0, x1, y1),
                   x_range=(float(seg_t[0]), t_end), y_range=(F_LO, F_HI))
        ax.grid(vertical=(i == 2), horizontal=True, xtick_labels=(i == 2))
        ax.frame()
        ax.colorbar(cmap, vmin, vmax, (x1 + 22, y0 + 6, x1 + 58, y1 - 6), label=cblab)

    cv.note("差值栏红色=处理后在此处能量更低（被压掉），白/浅色=基本不变。"
            "深层条纹（语音谐波）在差值栏应为浅色，而条纹间的底色应为红。")
    for o in cv.overlaps(min_area=150.0):
        print("  [自检] 重叠 %s/%s 面积=%.0f px2" % (o["a"], o["b"], o["area"]))
    return cv.save("FigL04_时频图局部放大.png")


# --------------------------------------------------------------------------

def main() -> int:
    S.warn_if_formal_output("fig_longrun_spectrogram.py")

    x_in = read_left(os.path.join(K.data_dir(), "input_long", NAME + ".wav"))
    x_out = read_left(os.path.join(LONG_ROOT, "optimized", "L3", NAME + ".wav"))
    print("[info] 样本 %d / %d, 时长 %.2f s" % (len(x_in), len(x_out), len(x_in) / SR))

    print("[info] STFT 处理前 ...")
    t_in, freqs, db_in = spectrogram_dbfs(x_in)
    print("[info] STFT 处理后 ...")
    t_out, _, db_out = spectrogram_dbfs(x_out)
    print("[info] 帧数 %d, bin 数 %d, 频率 %.1f–%.1f Hz 内 %d bin"
          % (db_in.shape[0], db_in.shape[1], F_LO, F_HI, len(band_rows(freqs))))

    rows = band_rows(freqs)
    sub_in = db_in[:, rows]
    sub_out = db_out[:, rows]
    print("  处理前 幅度范围 %.1f … %.1f dBFS" % (float(sub_in.min()), float(sub_in.max())))
    print("  处理后 幅度范围 %.1f … %.1f dBFS" % (float(sub_out.min()), float(sub_out.max())))
    d = sub_in - sub_out
    print("  前-后 差值范围 %.1f ~ %.1f dB, 均值 %.1f dB"
          % (float(d.min()), float(d.max()), float(d.mean())))

    p1 = fig_l03(t_in, rows, db_in, db_out)
    p2 = fig_l04(t_in, rows, db_in, db_out)
    print("[fig_longrun_spectrogram] 写出 %s" % p1)
    if p2:
        print("[fig_longrun_spectrogram] 写出 %s" % p2)
    return 0


if __name__ == "__main__":
    sys.exit(main())
