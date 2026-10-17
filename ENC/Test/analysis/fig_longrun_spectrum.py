# -*- coding: utf-8 -*-
"""十分钟语音的幅频谱对比（处理前 / 处理后 / 基线）。

幅频谱的定义
------------
    幅度谱 = |X(f)|，逐频点的幅值；本脚本按满幅正弦校准到 dBFS：
        mag = 2 · |DFT(x·w)| / (N · 相干增益) / 32768
    满幅 32767 的单频正弦 → mag = 1.0 → 0 dBFS（已用合成正弦自检，见 selftest()）。
    纵轴取 20·log10(mag)，负值即低于满幅多少 dB。

为什么不做"整段一次 FFT"
------------------------
    10.69 min = 30,789,216 点，一次 FFT 出 15,394,609 根谱线。画在 1432 px 的
    轴宽上等于 10750:1 压缩，任何绘制方式都只能得到包络；若再按像素列取平均，
    得到的是"分段平均"而不是那次 FFT 的谱。故改用 Welch 口径：
        NFFT = 8192（分辨率 5.86 Hz），Hann 窗，帧移 4096（50% 重叠），
        共 7512 个窗，**覆盖全部 30,789,216 个样本**。
    4097 根谱线落在 1432 px 上约 2.9 根/像素，谱线的锯齿得以保留，不做任何平滑。

两栏口径
--------
    (a) 全段：7512 个窗全部参与
    (b) 噪声段：只取 speech_prob_mean < 0.20 的窗（降噪效果所在）

数据
----
    输入   : ENC/Test/data/input_long/merged_speech_10min.wav
    处理后 : ENC/Test/data/long_run/optimized/L3/merged_speech_10min.wav
    基线   : ENC/Test/data/long_run/baseline/L3/merged_speech_10min.wav
"""

from __future__ import annotations

import csv
import os
import sys
import wave

import numpy as np

import common as K
import fig_style as S
from fig_style import C

SR = 48000
FRAME = 480
NFFT = 8192
HOP = 4096
COHERENT_GAIN = 0.5          # Hann 窗相干增益
FULL_SCALE = 32768.0         # int16 满量程
NAME = "merged_speech_10min"

NOISE_THR = 0.20
LOCK_FRAME = 177

LONG_ROOT = os.path.join(K.data_dir(), "long_run")
IN_WAV = os.path.join(K.data_dir(), "input_long", NAME + ".wav")

#: 逐频带衰减的分带（单独切出 0–100 Hz 以核对原版 100 Hz 高通）
BANDS = [(0, 100), (100, 800), (800, 2000), (2000, 4000), (4000, 8000), (8000, 24000)]
BAND_LABELS = ["0-0.1k", "0.1-0.8k", "0.8-2k", "2-4k", "4-8k", "8-24k"]


# --------------------------------------------------------------------------
# 数据读取
# --------------------------------------------------------------------------

def read_left(path: str) -> np.ndarray:
    """按 int16 原值读入，多声道取第 0 声道。"""
    with wave.open(path, "rb") as w:
        n, ch = w.getnframes(), w.getnchannels()
        raw = w.readframes(n)
    a = np.frombuffer(raw, dtype="<i2").astype(np.float64)
    return a[0::ch] if ch > 1 else a


def read_speech_prob(variant: str) -> np.ndarray:
    p = os.path.join(LONG_ROOT, variant, "L3", "metrics_%s.csv" % NAME)
    out: list[float] = []
    with open(p, newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            t = row["speech_prob_mean"].strip().lower()
            out.append(float("nan") if t in ("nan", "inf", "-inf", "") else float(t))
    return np.array(out)


def mag_spectra(x: np.ndarray, block: int = 1024) -> np.ndarray:
    """逐帧幅度谱 |X|，已按满幅正弦校准（0 dBFS = 满量程单频）。

    返回 (n_win, NFFT//2+1) float32。
    """
    win = np.hanning(NFFT)
    n_win = 1 + (len(x) - NFFT) // HOP
    n_bin = NFFT // 2 + 1
    out = np.empty((n_win, n_bin), dtype=np.float32)
    # 单边谱取 ×2；Hann 相干增益 0.5 补偿；再归一到满量程
    scale = 2.0 / (NFFT * COHERENT_GAIN) / FULL_SCALE
    for s in range(0, n_win, block):
        e = min(s + block, n_win)
        idx = (np.arange(s, e) * HOP)[:, None] + np.arange(NFFT)[None, :]
        seg = x[idx] * win[None, :]
        out[s:e] = np.abs(np.fft.rfft(seg, axis=1)) * scale
    return out


def avg_dbfs(mag: np.ndarray, mask: np.ndarray | None = None) -> np.ndarray:
    """帧平均**幅度**后转 dBFS：20·log10(mean|X|)。幅频谱的标准口径。"""
    m = mag if mask is None else mag[mask]
    a = np.mean(m.astype(np.float64), axis=0)
    return 20.0 * np.log10(np.maximum(a, 1e-12))


def band_db(mag_db: np.ndarray, freqs: np.ndarray) -> np.ndarray:
    """频带内幅度平均后转 dB（幅度域平均，不是 dB 域平均）。"""
    vals = []
    for lo, hi in BANDS:
        sel = (freqs >= lo) & (freqs < hi)
        a = np.mean(10.0 ** (mag_db[sel] / 20.0))
        vals.append(20.0 * np.log10(max(a, 1e-12)))
    return np.array(vals)


def selftest() -> None:
    """用满幅单频正弦校准自检：取整数 bin 频率（避免扇贝损失），应给 0 dBFS。"""
    k = 170
    f0 = k * SR / NFFT
    n = np.arange(NFFT)
    x = 32767.0 * np.sin(2.0 * np.pi * f0 * n / SR)
    sp = np.abs(np.fft.rfft(x * np.hanning(NFFT))) * (2.0 / (NFFT * COHERENT_GAIN) / FULL_SCALE)
    peak = 20.0 * np.log10(sp[k])
    print("[自检] 满幅正弦 @ bin %d (%.1f Hz) 的谱峰 = %+.3f dBFS（应为 0.000）" % (k, f0, peak))


# --------------------------------------------------------------------------
# 绘图辅助
# --------------------------------------------------------------------------

def put_caption(cv: S.Canvas, x: float, y: float, text: str, max_w: float, tag: str) -> None:
    f = S.font(S.FS_LABEL, True)
    w, h = S.text_size(cv.d, text, f)
    if w > max_w:
        print("  [警告] 子图标题超宽 %.0f > %.0f px: %s" % (w, max_w, text))
    S.caption(cv, x, y, text, color=C.TEXT, fnt=S.FS_LABEL)
    cv.box(S.EL_CAPTION, x, y, x + w, y + h, tag)


def strip_width(cv: S.Canvas, entries, fnt: int = S.FS_LABEL, gap: int = 26,
                sw: int = 40) -> float:
    w = 0.0
    for k, (_kind, _color, txt) in enumerate(entries):
        w += sw + 12 + S.text_size(cv.d, str(txt), S.font(fnt))[0]
        if k < len(entries) - 1:
            w += gap
    return w


def put_strip(cv: S.Canvas, x: float, y: float, entries, tag: str) -> None:
    right = S.legend_strip(cv, x, y, entries)
    _, h = S.text_size(cv.d, "汉", S.font(S.FS_LABEL))
    cv.box(S.EL_LEGEND, x, y, right, y + h, tag)


def fill_between(ax: S.Axes, x, y_hi, y_lo, color: str, t: float = 0.80) -> None:
    xs = np.asarray(x, dtype=float)
    a = np.asarray(y_hi, dtype=float)
    b = np.asarray(y_lo, dtype=float)
    ok = np.isfinite(xs) & np.isfinite(a) & np.isfinite(b)
    if not np.any(ok):
        return
    xs, a, b = xs[ok], a[ok], b[ok]
    top = [(ax.px(float(u)), ax.py(float(v))) for u, v in zip(xs, a)]
    bot = [(ax.px(float(u)), ax.py(float(v))) for u, v in zip(xs, b)]
    ax.d.polygon(top + bot[::-1], fill=S.mix_white(color, t))


# --------------------------------------------------------------------------
# FigL01 —— 上下两栏幅频谱
# --------------------------------------------------------------------------

def fig_l01(freqs, panels, n_win_total) -> str:
    cv = S.Canvas("full", "FigL01",
                  "十分钟语音的幅频谱（|X(f)|，dBFS）：处理前 vs 处理后（优化版 L3）")
    cv.set_bottom_reserve(30)

    ent = [("line", C.TEXT, "处理前（原始输入）"),
           ("line", C.OPTIMIZED, "处理后（优化版 L3）")]
    put_strip(cv, (cv.pad_l + (cv.W - cv.pad_r) - strip_width(cv, ent)) / 2.0, 60.0,
              ent, "图例：处理前 / 处理后")

    rows = ((150.0, 368.0), (474.0, 692.0))
    panel_w = cv.W - cv.pad_l - cv.pad_r

    allv = [float(np.min(p["before"])) for p in panels] + [float(np.min(p["after"])) for p in panels]
    allh = [float(np.max(p["before"])) for p in panels] + [float(np.max(p["after"])) for p in panels]
    y_lo, y_hi = min(allv) - 3.0, max(allh) + 4.0

    for i, p in enumerate(panels):
        ra, rb = rows[i]
        rect = (cv.pad_l, ra, cv.W - cv.pad_r, rb)
        put_caption(cv, rect[0], ra - 40.0, p["title"], panel_w, "子图标题 %d" % i)
        ax = cv.panel(rect=rect, ylabel="幅频谱 (dBFS)", xlabel="频率 (Hz)")
        before = np.asarray(p["before"], dtype=float)
        after = np.asarray(p["after"], dtype=float)
        ax.set_xlim(0.0, SR / 2.0)
        ax.set_ylim(y_lo, y_hi)
        ax.spine()
        ax.grid(horizontal=True, vertical=False)
        fill_between(ax, freqs, before, after, C.OPTIMIZED)
        # 直接画 4097 个点：不做任何平滑，谱线的锯齿保留
        ax.line(freqs, before, color=C.TEXT, lw=S.LW_CURVE)
        ax.line(freqs, after, color=C.OPTIMIZED, lw=S.LW_CURVE)
        ax.frame()

        sel = (freqs >= 100.0) & (freqs <= 23000.0)
        gap = float(np.mean(before[sel] - after[sel]))
        ax.text(8600.0, y_lo + 0.10 * (y_hi - y_lo),
                "填色 = 被压掉的量，100 Hz–23 kHz 平均 %.1f dB" % gap,
                color=C.OPTIMIZED, fnt=S.FS_MIN)

    cv.note("%d 点 Hann 窗 × %d 窗（帧移 %d），覆盖全部 30789216 样本；分辨率 %.1f Hz，逐 bin 无平滑。"
            "0 dBFS = int16 满量程单频。" % (NFFT, n_win_total, HOP, SR / NFFT))
    for o in cv.overlaps(min_area=150.0):
        print("  [自检] 重叠 %s/%s 面积=%.0f px2" % (o["a"], o["b"], o["area"]))
    return cv.save("FigL01_十分钟语音幅频谱对比.png")


# --------------------------------------------------------------------------
# FigL02 —— 失效根因与消融
# --------------------------------------------------------------------------

TRACK_S = 20.0


def fig_l02(freqs, tracks, psds, lock_frame) -> str:
    cv = S.Canvas("full", "FigL02",
                  "降噪失效的根因链：语音概率在 %.2f s 后锁死，及四项补丁的消融证据" %
                  (lock_frame * 0.01))
    cv.set_bottom_reserve(30)

    y0, y1 = 140.0, 600.0
    rects = S.Axes.grid_rects((cv.pad_l, y0, cv.W - cv.pad_r, y1), nrow=1, ncol=2, gap_x=200)
    panel_w = rects[0][2] - rects[0][0]

    ent_a = [("line", C.BASELINE, "基线 baseline（固件现状）"),
             ("line", C.OPTIMIZED, "优化 optimized（补 D8）")]
    put_strip(cv, (cv.pad_l + (cv.W - cv.pad_r) - strip_width(cv, ent_a)) / 2.0, 62.0,
              ent_a, "图例：(a)")

    put_caption(cv, rects[0][0], 104.0, "(a) 概率轨迹（%d 帧后锁死）" % lock_frame,
                panel_w, "左标题")
    ax = cv.panel(rect=rects[0], ylabel="语音存在概率", xlabel="时间 (s)")
    n = int(TRACK_S * 100)
    t = np.arange(n) * 0.01
    ax.set_xlim(0.0, TRACK_S)
    ax.set_ylim(-0.03, 1.08)
    ax.spine()
    ax.grid(horizontal=True, vertical=False)
    ax.vline(lock_frame * 0.01, color=C.NAN, lw=S.LW_AUX)
    ax.line(t, np.nan_to_num(tracks["baseline"][:n], nan=0.0),
            color=C.BASELINE, lw=S.LW_AUX, dash=True)
    ax.line(t, np.clip(tracks["optimized"][:n], 0.0, 1.0), color=C.OPTIMIZED, lw=S.LW_AUX)
    ax.frame()

    put_caption(cv, rects[1][0], 104.0, "(b) 非语音段逐带衰减（四项补丁消融）", panel_w, "右标题")
    ax2 = cv.panel(rect=rects[1], ylabel="衰减量 (dB)", xlabel="频带 (Hz)")
    x = np.arange(len(BANDS))
    ax2.set_xlim(-0.3, len(BANDS) - 0.7)
    allv = [v for k in psds for v in psds[k]]
    ax2.set_ylim(min(allv) - 1.5, max(allv) + 1.8)
    ax2.spine()
    ax2.grid(horizontal=True, vertical=False, xlabels=BAND_LABELS)
    for lab, col in (("baseline", C.BASELINE), ("d1d5d6", C.VARIANT["d1d5d6"]),
                     ("optimized", C.OPTIMIZED)):
        ax2.line(x, psds[lab], color=col, lw=S.LW_CURVE,
                 label={"baseline": "基线 baseline", "d1d5d6": "+D1D5D6",
                        "optimized": "优化 optimized"}[lab])
        for i, v in enumerate(psds[lab]):
            ax2.scatter([x[i]], [v], color=col, r=4)
    ax2.frame()
    ax2.legend(loc="auto")

    cv.note("(a) 基线自第 %d 帧起 speech_prob_mean 恒为 1.0：NaN 经 MAX(MIN(x,1),0.01) 被钳成 1.0，"
            "prior 永久锁死；(b) 仅补 D8 的 optimized 打开降噪，D1/D5/D6 三项与基线逐带重合。"
            % lock_frame)
    for o in cv.overlaps(min_area=150.0):
        print("  [自检] 重叠 %s/%s 面积=%.0f px2" % (o["a"], o["b"], o["area"]))
    return cv.save("FigL02_失效根因与消融证据.png")


# --------------------------------------------------------------------------

def main() -> int:
    S.warn_if_formal_output("fig_longrun_spectrum.py")
    selftest()

    x_in = read_left(IN_WAV)
    x_out = read_left(os.path.join(LONG_ROOT, "optimized", "L3", NAME + ".wav"))
    x_base = read_left(os.path.join(LONG_ROOT, "baseline", "L3", NAME + ".wav"))
    spm = read_speech_prob("optimized")
    print("[info] 样本 %d / %d / %d, speech_prob 帧 %d" % (len(x_in), len(x_out), len(x_base), len(spm)))

    freqs = np.fft.rfftfreq(NFFT, 1.0 / SR)
    m_in = mag_spectra(x_in)
    m_out = mag_spectra(x_out)
    m_base = mag_spectra(x_base)
    n_win = m_in.shape[0]
    print("[info] 窗数 %d, 分辨率 %.2f Hz, bin 数 %d" % (n_win, SR / NFFT, len(freqs)))

    # 窗 j 的中心落在样本 j*HOP + NFFT/2，对应算法帧号（10 ms/帧）
    idx = ((np.arange(n_win) * HOP + NFFT // 2) // FRAME).astype(int)
    cls = np.full(n_win, np.nan)
    ok = idx < len(spm)
    cls[ok] = spm[idx[ok]]

    mask_noise = np.isfinite(cls) & (cls < NOISE_THR)
    mask_speech = np.isfinite(cls) & (cls >= 0.60)
    print("[info] 非语音窗 %d, 语音窗 %d" % (mask_noise.sum(), mask_speech.sum()))

    psd_in_all = avg_dbfs(m_in)
    psd_out_all = avg_dbfs(m_out)
    psd_noise_in = avg_dbfs(m_in, mask_noise)
    psd_noise_out = avg_dbfs(m_out, mask_noise)
    psd_noise_base = avg_dbfs(m_base, mask_noise)
    psd_spch_in = avg_dbfs(m_in, mask_speech)
    psd_spch_out = avg_dbfs(m_out, mask_speech)

    panels = [
        {"title": "(a) 全段（%d 窗，覆盖全部十分钟）" % n_win,
         "before": psd_in_all, "after": psd_out_all},
        {"title": "(b) 噪声段（非语音窗 %d 个）" % int(mask_noise.sum()),
         "before": psd_noise_in, "after": psd_noise_out},
    ]

    x_d156 = read_left(os.path.join(LONG_ROOT, "d1d5d6", "L3", NAME + ".wav"))
    psd_noise_d156 = avg_dbfs(mag_spectra(x_d156), mask_noise)

    p1 = fig_l01(freqs, panels, n_win)

    psds = {
        "baseline": band_db(psd_noise_base, freqs) - band_db(psd_noise_in, freqs),
        "d1d5d6": band_db(psd_noise_d156, freqs) - band_db(psd_noise_in, freqs),
        "optimized": band_db(psd_noise_out, freqs) - band_db(psd_noise_in, freqs),
    }
    tracks = {"baseline": read_speech_prob("baseline"), "optimized": read_speech_prob("optimized")}
    p2 = fig_l02(freqs, tracks, psds, LOCK_FRAME)

    for lab, a, b in (("全段", psd_in_all, psd_out_all),
                      ("非语音段", psd_noise_in, psd_noise_out),
                      ("语音段", psd_spch_in, psd_spch_out)):
        d = band_db(b, freqs) - band_db(a, freqs)
        print("  %s 逐带衰减(dB): %s" % (lab, " ".join("%+.2f" % v for v in d)))
    print("  非语音段 六带衰减: " + " | ".join(
        "%s %s" % (k, " ".join("%+.2f" % v for v in psds[k])) for k in psds))
    print("  全段谱电平范围 %.1f ~ %.1f dBFS" % (float(psd_in_all.min()), float(psd_in_all.max())))

    print("[fig_longrun_spectrum] 写出 %s" % p1)
    print("[fig_longrun_spectrum] 写出 %s" % p2)
    return 0


if __name__ == "__main__":
    sys.exit(main())
