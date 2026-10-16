#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
dns_mos_eval.py —— 用 DNS-MOS 对修复前后的输出做语音整体质量对比（无参考感知 MOS）

## 为什么用 DNS-MOS 而不是 PESQ

PESQ / STOI 这类**有参考**指标要求「干净参考语音 vs 退化信号」。本项目不满足该前提：
- 被测输入 `Test_Audio/AudioSample-48000hz/*.wav` **本身就是含噪语音**（降噪算法的输入）；
- 仓库内不存在对应的干净语音，无法构造合法参考；
- 若硬把"含噪输入"当参考，测出的只是「降噪输出偏离输入多少」——
  而降噪的目标恰恰就是偏离输入，该分数没有解释力。

DNS-MOS 是**无参考**（no-reference）的感知质量预估模型（Microsoft DNS-Challenge 官方发布），
直接对音频给出 SIG / BAK / OVRL 三项 MOS 与 P808_MOS，正好回答
「修复前后，语音整体质量是否变好」。它是当前降噪领域事实上的标准评价工具。

## 实现说明

严格复现官方参考实现（`_dnsmos/dnsmos_local_ref.py`）的流程，包括：
- 重采样到 16 kHz（`librosa.resample`）；
- 以 **9.01 s** 为窗、**1 s** 为跳步切段，逐段推理后取**算术平均**；
- `sig_bak_ovr.onnx`（主模型）输入为**原始波形** `[1, 144160]`；
- `model_v8.onnx`（P808）输入为 mel 频谱 `[1, T, 120]`（`n_mels=120, n_fft=321, hop=160`，
  `power_to_db(ref=max)` 后 `(dB+40)/40`）；
- 输出经官方固定的多项式系数映射（非 personalized 版本）。

**未自行重写任何预处理**——mel 与重采样直接调用 `librosa`，以保证与官方分数可比。
模型与参考实现来源：https://github.com/microsoft/DNS-Challenge （DNSMOS/ 目录）

## 用法

    <python> dns_mos_eval.py                    # 评估全部（输入 + 各变体 + 参照）
    <python> dns_mos_eval.py --variants baseline optimized
    <python> dns_mos_eval.py --limit 2          # 只跑前 2 个文件（快速验证）
"""

import argparse
import csv
import os
import sys
import time

import librosa
import numpy as np
import onnxruntime as ort
import soundfile as sf

HERE = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
MODEL_DIR = os.path.join(HERE, "_dnsmos")
PRIMARY_MODEL = os.path.join(MODEL_DIR, "sig_bak_ovr.onnx")
P808_MODEL = os.path.join(MODEL_DIR, "model_v8.onnx")
INPUT_DIR = os.path.join(REPO_ROOT, "Test_Audio", "AudioSample-48000hz")
DATA_DIR = os.path.join(REPO_ROOT, "ENC", "Test", "data")
OUT_DIR = os.path.join(DATA_DIR, "dnsmos")

SAMPLING_RATE = 16000
INPUT_LENGTH = 9.01
NAMES = [str(i) for i in range(1, 10)]

#: 评估目标：标签 -> (目录模板, 说明)
#: 「输入」为未经处理的含噪原始音频（单声道取左），作为"不处理"的基线参照
TARGETS = {
    "input":     ("@INPUT",   "原始含噪输入（未处理，取左声道）"),
    "baseline":  ("{v}/L3",   "基线：四项缺陷全保留 (patch_flags=0)"),
    "d1":        ("{v}/L3",   "+D1 (flags=1)"),
    "d1d5":      ("{v}/L3",   "+D1+D5 (flags=3)"),
    "d1d5d6":    ("{v}/L3",   "+D1+D5+D6 (flags=7)"),
    "optimized": ("{v}/L3",   "优化版：+D1+D5+D6+D8 (flags=15)"),
    "reference": ("{v}/L3",   "另一条移植路径 Project/ENC_Core（Ooura）"),
}


def build_session(path):
    if not os.path.isfile(path):
        raise SystemExit("[FAIL] 缺少模型文件: %s\n       请从 DNS-Challenge 的 DNSMOS/ 目录下载" % path)
    so = ort.SessionOptions()
    so.intra_op_num_threads = 1
    so.inter_op_num_threads = 1
    return ort.InferenceSession(path, sess_options=so, providers=["CPUExecutionProvider"])


def audio_melspec(audio, n_mels=120, frame_size=320, hop_length=160, sr=16000, to_db=True):
    """与官方参考实现逐字一致（librosa 默认 center=True / hann / power=2）。"""
    mel_spec = librosa.feature.melspectrogram(
        y=audio, sr=sr, n_fft=frame_size + 1, hop_length=hop_length, n_mels=n_mels)
    if to_db:
        mel_spec = (librosa.power_to_db(mel_spec, ref=np.max) + 40) / 40
    return mel_spec.T


def polyfit_val(sig, bak, ovr, personalized=False):
    if personalized:
        p_ovr = np.poly1d([-0.00533021, 0.005101, 1.18058466, -0.11236046])
        p_sig = np.poly1d([-0.01019296, 0.02751166, 1.19576786, -0.24348726])
        p_bak = np.poly1d([-0.04976499, 0.44276479, -0.1644611, 0.96883132])
    else:
        p_ovr = np.poly1d([-0.06766283, 1.11546468, 0.04602535])
        p_sig = np.poly1d([-0.08397278, 1.22083953, 0.0052439])
        p_bak = np.poly1d([-0.13166888, 1.60915514, -0.39604546])
    return float(p_sig(sig)), float(p_bak(bak)), float(p_ovr(ovr))


def score_file(sess_primary, sess_p808, path):
    """返回该文件的 MOS 汇总（多段平均）。"""
    aud, fs = sf.read(path, always_2d=True)   # always_2d 保证形状恒为 (n_samples, n_ch)
    aud = aud[:, 0].astype(np.float64)        # 无条件取第 0 声道（单声道输出亦被子化）
    audio = librosa.resample(aud, orig_sr=fs, target_sr=SAMPLING_RATE) if fs != SAMPLING_RATE else aud
    n = len(audio)
    len_samples = int(INPUT_LENGTH * SAMPLING_RATE)
    while len(audio) < len_samples:
        audio = np.append(audio, audio)
    num_hops = int(np.floor(len(audio) / SAMPLING_RATE) - INPUT_LENGTH) + 1
    if num_hops < 1:
        num_hops = 1

    sig_l, bak_l, ovr_l, p808_l, sigr_l, bakr_l, ovrr_l = ([] for _ in range(7))
    for idx in range(num_hops):
        seg = audio[int(idx * SAMPLING_RATE): int((idx + INPUT_LENGTH) * SAMPLING_RATE)]
        if len(seg) < len_samples:
            continue
        wave = np.array(seg).astype("float32")[np.newaxis, :]
        p808_feat = np.array(audio_melspec(audio=seg[:-160])).astype("float32")[np.newaxis, :, :]
        p808_mos = float(sess_p808.run(None, {"input_1": p808_feat})[0][0][0])
        sig_r, bak_r, ovr_r = [float(v) for v in sess_primary.run(None, {"input_1": wave})[0][0]]
        sig, bak, ovr = polyfit_val(sig_r, bak_r, ovr_r)
        sig_l.append(sig); bak_l.append(bak); ovr_l.append(ovr); p808_l.append(p808_mos)
        sigr_l.append(sig_r); bakr_l.append(bak_r); ovrr_l.append(ovr_r)

    if not sig_l:
        return None
    return {
        "dur_s": round(n / SAMPLING_RATE, 3),
        "segments": len(sig_l),
        "SIG": float(np.mean(sig_l)),
        "BAK": float(np.mean(bak_l)),
        "OVRL": float(np.mean(ovr_l)),
        "P808_MOS": float(np.mean(p808_l)),
        "SIG_raw": float(np.mean(sigr_l)),
        "BAK_raw": float(np.mean(bakr_l)),
        "OVRL_raw": float(np.mean(ovrr_l)),
    }


def resolve(target_key, name):
    tmpl = TARGETS[target_key][0]
    if tmpl == "@INPUT":
        return os.path.join(INPUT_DIR, "%s.wav" % name)
    return os.path.join(DATA_DIR, tmpl.format(v=target_key), "%s.wav" % name)


def main():
    ap = argparse.ArgumentParser(description="DNS-MOS 语音整体质量对比（无参考）")
    ap.add_argument("--variants", nargs="*", default=list(TARGETS.keys()))
    ap.add_argument("--limit", type=int, default=0, help="只跑前 N 个文件（0=全部）")
    args = ap.parse_args()

    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

    names = NAMES if args.limit <= 0 else NAMES[:args.limit]
    os.makedirs(OUT_DIR, exist_ok=True)

    print("=" * 96)
    print("DNS-MOS 语音整体质量对比（无参考感知 MOS）")
    print("  主模型 : %s" % PRIMARY_MODEL)
    print("  P808   : %s" % P808_MODEL)
    print("  采样率 : %d Hz（内部统一重采样）" % SAMPLING_RATE)
    print("  窗口   : %.2f s / 跳步 1 s（官方流程）" % INPUT_LENGTH)
    print("=" * 96)

    sess_primary = build_session(PRIMARY_MODEL)
    sess_p808 = build_session(P808_MODEL)
    print("  主模型输入: %s" % [(i.name, i.shape) for i in sess_primary.get_inputs()])
    print("  P808 输入 : %s" % [(i.name, i.shape) for i in sess_p808.get_inputs()])

    rows = []
    for key in args.variants:
        d = TARGETS[key][1]
        print("\n" + "-" * 96)
        print("[%s] %s" % (key, d))
        for nm in names:
            p = resolve(key, nm)
            if not os.path.isfile(p):
                print("  %-3s SKIP 文件不存在: %s" % (nm, p))
                continue
            t0 = time.time()
            r = score_file(sess_primary, sess_p808, p)
            dt = time.time() - t0
            if r is None:
                print("  %-3s SKIP 无有效分段" % nm)
                continue
            r.update({"variant": key, "file": nm})
            rows.append(r)
            print("  %-3s SIG=%.4f BAK=%.4f OVRL=%.4f P808=%.4f  seg=%2d  %.2fs"
                  % (nm, r["SIG"], r["BAK"], r["OVRL"], r["P808_MOS"], r["segments"], dt))

    if not rows:
        print("\n[FAIL] 没有产出任何结果")
        return 1

    fields = ["variant", "file", "dur_s", "segments", "SIG", "BAK", "OVRL", "P808_MOS",
              "SIG_raw", "BAK_raw", "OVRL_raw"]
    detail = os.path.join(OUT_DIR, "dnsmos_detail.csv")
    with open(detail, "w", encoding="utf-8", newline="\n") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        for r in rows:
            w.writerow({k: r[k] for k in fields})

    # ---- 汇总
    print("\n" + "=" * 96)
    print("汇总（各变体在 %d 个文件上的均值）" % len(names))
    print("=" * 96)
    print("%-11s %8s %8s %8s %9s %10s" % ("variant", "SIG", "BAK", "OVRL", "P808_MOS", "OVRL_std"))
    print("-" * 66)
    summary = []
    for key in args.variants:
        sub = [r for r in rows if r["variant"] == key]
        if not sub:
            continue
        sig = float(np.mean([r["SIG"] for r in sub]))
        bak = float(np.mean([r["BAK"] for r in sub]))
        ovr = float(np.mean([r["OVRL"] for r in sub]))
        p808 = float(np.mean([r["P808_MOS"] for r in sub]))
        std = float(np.std([r["OVRL"] for r in sub]))
        summary.append({"variant": key, "files": len(sub), "SIG": sig, "BAK": bak,
                        "OVRL": ovr, "P808_MOS": p808, "OVRL_std": std})
        print("%-11s %8.4f %8.4f %8.4f %9.4f %10.4f" % (key, sig, bak, ovr, p808, std))

    sm = os.path.join(OUT_DIR, "dnsmos_summary.csv")
    with open(sm, "w", encoding="utf-8", newline="\n") as f:
        w = csv.DictWriter(f, fieldnames=["variant", "files", "SIG", "BAK", "OVRL", "P808_MOS", "OVRL_std"])
        w.writeheader()
        for r in summary:
            w.writerow(r)

    if "input" in args.variants and "optimized" in args.variants:
        gi = next((r for r in summary if r["variant"] == "input"), None)
        go = next((r for r in summary if r["variant"] == "optimized"), None)
        gb = next((r for r in summary if r["variant"] == "baseline"), None)
        if gi and go and gb:
            print("\n" + "=" * 96)
            print("关键对比（OVRL 越高越好；BAK 越高表示背景噪声越小）")
            print("=" * 96)
            print("  原始含噪输入   OVRL=%.4f  BAK=%.4f" % (gi["OVRL"], gi["BAK"]))
            print("  基线 baseline  OVRL=%.4f  BAK=%.4f   (相对输入 ΔOVRL=%+.4f)"
                  % (gb["OVRL"], gb["BAK"], gb["OVRL"] - gi["OVRL"]))
            print("  优化 optimized OVRL=%.4f  BAK=%.4f   (相对输入 ΔOVRL=%+.4f, 相对基线 ΔOVRL=%+.4f)"
                  % (go["OVRL"], go["BAK"], go["OVRL"] - gi["OVRL"], go["OVRL"] - gb["OVRL"]))

    print("\n明细 -> %s" % detail)
    print("汇总 -> %s" % sm)
    return 0


if __name__ == "__main__":
    sys.exit(main())
