# -*- coding: utf-8 -*-
"""长语音（10.69 min）五变体差异定位探针。

回答三个问题：
  Q1 五个变体在同一输入上的段级指标差多少？
  Q2 差异集中在哪一类帧（非语音段 / 语音段）？
  Q3 speech_prob_mean 与 gain_adjustment 的分布是否被 D8 改变？

输出同时写 stdout 与 long_run_probe.txt（避免 Windows 控制台代码页问题）。
"""
from __future__ import annotations

import csv
import os
import sys
import wave

import numpy as np

import common as K

NAME = "merged_speech_10min"
LONG = os.path.join(K.data_dir(), "long_run")
VARIANTS = ("baseline", "d1", "d1d5", "d1d5d6", "optimized")
SR, FRAME, NFFT, HOP = 48000, 480, 2048, 480

_lines: list[str] = []


def out(s: str = "") -> None:
    print(s)
    _lines.append(s)


def read_metrics(variant: str) -> dict:
    p = os.path.join(LONG, variant, "L3", "metrics_%s.csv" % NAME)
    cols: dict[str, list[float]] = {}
    with open(p, newline="", encoding="utf-8") as f:
        rd = csv.DictReader(f)
        for k in rd.fieldnames or []:
            cols.setdefault(k, [])
        for row in rd:
            for k, v in row.items():
                t = v.strip().lower()
                try:
                    cols[k].append(np.nan if t in ("nan", "inf", "-inf", "") else float(t))
                except ValueError:
                    cols[k].append(np.nan)
    return {k: np.array(v) for k, v in cols.items()}


def read_left(path: str) -> np.ndarray:
    with wave.open(path, "rb") as w:
        n, ch = w.getnframes(), w.getnchannels()
        raw = w.readframes(n)
    a = np.frombuffer(raw, dtype="<i2").astype(np.float64)
    return a[0::ch] if ch > 1 else a


def frame_rms(x: np.ndarray) -> np.ndarray:
    n = len(x) // FRAME
    return np.sqrt(np.mean(x[:n * FRAME].reshape(n, FRAME) ** 2, axis=1))


def main() -> int:
    m = {v: read_metrics(v) for v in VARIANTS}
    ref = m["optimized"]
    spm = ref["speech_prob_mean"]
    finite = np.isfinite(spm)
    noise = finite & (spm < 0.20)
    spch = finite & (spm >= 0.60)

    out("=" * 78)
    out("长语音五变体探针  (%s, L3, %d 帧 = %.1f s)" % (NAME, len(spm), len(spm) * 0.01))
    out("=" * 78)

    out("")
    out("Q1 段级指标")
    out("%-10s %8s %10s %10s %10s %10s" %
        ("变体", "非有限帧", "in_rms", "out_rms", "atten_db", "总耗时ms"))
    for v in VARIANTS:
        c = m[v]
        inr = float(np.sqrt(np.nanmean(c["in_rms"] ** 2)))
        our = float(np.sqrt(np.nanmean(c["out_rms"] ** 2)))
        nbad = int(np.sum(~np.isfinite(c["speech_prob_mean"])))
        out("%-10s %8d %10.1f %10.1f %10.3f %10.1f" %
            (v, nbad, inr, our, 20 * np.log10(inr / our), float(np.nansum(c["elapsed_us"])) / 1000.0))

    out("")
    out("Q2 差异定位（帧平均 abs(out_rms) 分三类帧）")
    out("%-10s %12s %12s %12s %12s" %
        ("变体", "非语音RMS", "语音RMS", "prob均值", "非语音prob"))
    for v in VARIANTS:
        c = m[v]
        out("%-10s %12.1f %12.1f %12.4f %12.4f" %
            (v, float(np.nanmean(c["out_rms"][noise])), float(np.nanmean(c["out_rms"][spch])),
             float(np.nanmean(c["speech_prob_mean"])), float(np.nanmean(c["speech_prob_mean"][noise]))))

    out("")
    out("Q3 speech_prob_mean / gain_adjustment / diff_normalization 分布")
    for v in VARIANTS:
        c = m[v]
        s = c["speech_prob_mean"][finite]
        g = c["gain_adjustment"][finite]
        d = c["diff_normalization"][finite]
        out("  %-10s prob 中位=%.4f 均值=%.4f | gain_adj 中位=%.4f 均值=%.4f | "
            "diff_norm 中位=%.4e 均值=%.4e" %
            (v, np.nanmedian(s), np.nanmean(s), np.nanmedian(g), np.nanmean(g),
             np.nanmedian(d), np.nanmean(d)))

    out("")
    out("Q4 非语音段输出谱电平（480 点帧平均幅度谱，0–24 kHz 分段）")
    edges = [(0, 100), (100, 300), (300, 800), (800, 2000), (2000, 4000),
             (4000, 8000), (8000, 12000), (12000, 16000), (16000, 24000)]
    labels = ["0-0.1k", "0.1-0.3k", "0.3-0.8k", "0.8-2k", "2-4k",
              "4-8k", "8-12k", "12-16k", "16-24k"]
    freqs = np.fft.rfftfreq(480, 1.0 / SR)
    x_in = read_left(os.path.join(K.data_dir(), "input_long", NAME + ".wav"))
    spects = {}
    for v in VARIANTS:
        x = read_left(os.path.join(LONG, v, "L3", NAME + ".wav"))
        n = min(len(x), len(x_in)) // FRAME
        xi = x_in[:n * FRAME].reshape(n, FRAME)
        xo = x[:n * FRAME].reshape(n, FRAME)
        sel = noise[:n]
        pi = np.mean(np.abs(np.fft.rfft(xi[sel] * np.hanning(FRAME), axis=1)) ** 2, axis=0)
        po = np.mean(np.abs(np.fft.rfft(xo[sel] * np.hanning(FRAME), axis=1)) ** 2, axis=0)
        spects[v] = (10 * np.log10(np.maximum(pi, 1e-20)), 10 * np.log10(np.maximum(po, 1e-20)))
    out("%-10s %s" % ("变体", " ".join("%8s" % s for s in labels)))
    for v in VARIANTS:
        pi, po = spects[v]
        vals = [10 * np.log10(np.mean(10 ** (po[(freqs >= lo) & (freqs < hi)] / 10))) for lo, hi in edges]
        out("%-10s %s" % (v, " ".join("%+8.2f" % v2 for v2 in vals)))
    out("")
    out("相对处理前的逐带衰减 (dB)：")
    out("%-10s %s" % ("变体", " ".join("%8s" % s for s in labels)))
    for v in VARIANTS:
        pi, po = spects[v]
        vals = [(10 * np.log10(np.mean(10 ** (po[(freqs >= lo) & (freqs < hi)] / 10))) -
                 10 * np.log10(np.mean(10 ** (pi[(freqs >= lo) & (freqs < hi)] / 10)))) for lo, hi in edges]
        out("%-10s %s" % (v, " ".join("%+8.2f" % v2 for v2 in vals)))

    out("")
    out("Q5 语音段输出相对处理前的偏差（保真度）")
    for v in VARIANTS:
        x = read_left(os.path.join(LONG, v, "L3", NAME + ".wav"))
        n = min(len(x), len(x_in)) // FRAME
        xi = x_in[:n * FRAME].reshape(n, FRAME)
        xo = x[:n * FRAME].reshape(n, FRAME)
        sel = spch[:n]
        pi = np.mean(np.abs(np.fft.rfft(xi[sel] * np.hanning(FRAME), axis=1)) ** 2, axis=0)
        po = np.mean(np.abs(np.fft.rfft(xo[sel] * np.hanning(FRAME), axis=1)) ** 2, axis=0)
        vals = [(10 * np.log10(np.mean(10 ** (10 * np.log10(np.maximum(po, 1e-20))[(freqs >= lo) & (freqs < hi)] / 10))) -
                 10 * np.log10(np.mean(10 ** (10 * np.log10(np.maximum(pi, 1e-20))[(freqs >= lo) & (freqs < hi)] / 10)))) for lo, hi in edges]
        out("%-10s %s" % (v, " ".join("%+8.2f" % v2 for v2 in vals)))

    dst = os.path.join(os.path.dirname(os.path.abspath(__file__)), "long_run_probe.txt")
    with open(dst, "w", encoding="utf-8") as f:
        f.write("\n".join(_lines) + "\n")
    print("\n[probe] 写出 %s" % dst)
    return 0


if __name__ == "__main__":
    sys.exit(main())
