# -*- coding: utf-8 -*-
"""
自测合成数据生成器（T4-I 自测专用，**不属于**正式产出图件管线）

用途
----
真实测量数据要等后续阶段（P2/P4）才产生。为了让绘图管线在无数据时也能被完整验证，
本脚本按计划书 6.1/6.2/6.3 的路径与列定义生成一套合成数据：

    <out>/<variant>/L3/<name>.wav                    16bit / 1ch / 48000Hz
    <out>/<variant>/L3/metrics_<name>.csv            列顺序 = 计划书 6.2
    <out>/<variant>/L3/summary_<variant>_L3.csv      列 = 计划书 6.3
    <out>/<variant>/build_info.txt                   键值格式 = 计划书 6.4

关键性质（保证图件自洽）
-----------------------
* WAV 与 metrics 由**同一份合成信号**导出：`atten_db` 用真实 `in_rms/out_rms` 反算，
  `signal_energy` / `signal_spectral_sum` 用真实 STFT 幅度和导出。
* 覆盖任务书要求的边界：静音段（`out_rms==0` → `atten_db==999.0`）、高能量段、
  `clamp_hits>0` 的帧、`atten_db==999` 的帧。
* `--inject` 模式额外制造异常（NaN/Inf 帧、超限钳位），用于验证 FigT22 的**检出**分支；
  默认模式不含异常，用于验证 FigT22 的**未检出异常**分支。

输出位置
--------
`--out <dir>` 指定，默认 `_selftest/data`；`--inject` 时默认 `_selftest/data_inject`。

用法
----
    python make_selftest_data.py
    python make_selftest_data.py --inject
"""

from __future__ import annotations

import argparse
import os
import wave

import numpy as np

import common as K

RATE = K.SAMPLE_RATE
FRAME = K.FRAME_SAMPLES
LEVEL = "L3"
DEFAULT_SECONDS = 2.5

#: 合成场景分段（段起点秒, 段类型）—— 取值必须覆盖边界用例。
#: 末段起点 5.70 s，保证 5.0–5.5 s（FigT04 默认放大窗口）落在"语音段二"内。
SCENES = (
    (0.00, "silence"),    # 静音：out_rms==0 → atten_db=999 哨兵
    (0.40, "speech"),     # 语音：正常衰减
    (1.60, "hot"),        # 高能量：接近满量程 + 钳位命中
    (2.10, "transient"),  # 过渡：概率中值
    (2.60, "speech2"),    # 语音段二（覆盖 5.0–5.5 s 放大窗口）
    (5.70, "silence2"),   # 尾部静音：再制造哨兵与边界
)


def scene_of(t: float) -> str:
    kind = SCENES[0][1]
    for start, k in SCENES:
        if t >= start:
            kind = k
    return kind


# --------------------------------------------------------------------------
# 帧级参数
# --------------------------------------------------------------------------

def frame_params(n_frames: int, variant: str, seed: int) -> dict:
    """按段类型生成每帧的输入幅度 / 输出衰减 / 概率 / 钳位等。"""
    rng = np.random.default_rng(seed)
    t = (np.arange(n_frames, dtype=np.float64) * FRAME) / RATE
    kinds = [scene_of(float(x)) for x in t]

    speech_like = {"speech", "speech2", "transient"}
    base_att = np.where([k in speech_like for k in kinds], 12.0, 0.0)
    base_att = base_att + np.where([k == "hot" for k in kinds], 4.0, 0.0)

    # 语音概率
    prob = np.array([{"silence": 0.06, "silence2": 0.04, "speech": 0.88,
                      "speech2": 0.82, "hot": 0.70, "transient": 0.42}[k] for k in kinds])
    prob = prob + rng.normal(0.0, 0.02, n_frames)
    prob = np.clip(prob, 0.0, 1.0)

    # 衰减抖动：平滑随机游走（避免逐帧白噪声，形态更接近真实）。
    # **d1 与 baseline 共享同一输入**（抖动置零、衰减同 baseline），使两者的
    # in_rms 逐帧一致——真实 D1 机制只会改变 `signal_energy` 的累加项，
    # 因此 `Δ = energy_baseline − energy_d1` 必然 ≥ 0。若给 d1 独立随机抖动，
    # Δ 会出现负值，与物理机理矛盾，会误导 FigT08 的读者。
    if variant == "d1":
        jitter = np.zeros(n_frames, dtype=np.float64)
    else:
        jitter = np.convolve(rng.normal(0.0, 2.2, n_frames + 24),
                             np.ones(25) / 25.0, mode="same")[:n_frames]
    att = base_att + jitter
    if variant != "baseline":
        # 消融链：修复项越多衰减越强（5 组：baseline/d1/d1d5/d1d5d6/optimized）
        boost = {"d1": 0.0, "d1d5": 3.0, "d1d5d6": 5.0, "optimized": 6.0}.get(variant, 0.0)
        att = att + boost * np.clip(prob, 0.0, 1.0)

    # 输入幅度（int16 量纲）
    in_amp = np.array([{"silence": 40.0, "silence2": 25.0, "speech": 5200.0,
                        "speech2": 6100.0, "hot": 30100.0, "transient": 2600.0}[k]
                       for k in kinds])
    in_amp = in_amp * (1.0 + 0.10 * np.sin(np.arange(n_frames) / 7.0))
    in_amp = np.clip(in_amp + rng.normal(0.0, 12.0, n_frames), 1.0, 32767.0)

    out_amp = in_amp * np.power(10.0, -att / 20.0)

    # 钳位：高能量段尾部出现触边样本；指定文件额外注入超限
    hits = np.zeros(n_frames, dtype=np.int64)
    hot_idx = np.array([i for i, k in enumerate(kinds) if k == "hot"])
    if hot_idx.size:
        n_hit = max(1, hot_idx.size // 6)
        sel = rng.choice(hot_idx, size=n_hit, replace=False)
        hits[sel] = rng.integers(1, 5, n_hit)

    # 静音段：输出精确为 0 → out_rms==0 → atten_db 记 999.0
    quiet = np.array([k in ("silence", "silence2") for k in kinds])
    out_amp = np.where(quiet, 0.0, out_amp)

    # 逐帧耗时（μs）：修复项越多越快
    mu = {"baseline": 9800.0, "d1": 9700.0, "d1d5": 9400.0,
          "d1d5d6": 8400.0, "optimized": 7200.0}.get(variant, 9500.0)
    elapsed = rng.normal(mu, 900.0, n_frames)
    elapsed = np.clip(elapsed, 1800.0, 26000.0).astype(np.int64)

    return {"kinds": kinds, "in_amp": in_amp, "out_amp": out_amp, "prob": prob,
            "att": att, "hits": hits, "elapsed": elapsed, "quiet": quiet}


def synth_audio(params: dict) -> np.ndarray:
    """把幅度序列展开成 int16 采样（每帧内用正弦 / 噪声填充）。"""
    n_frames = len(params["kinds"])
    rng = np.random.default_rng(7)
    out = np.zeros(n_frames * FRAME, dtype=np.float64)
    phase = 0.0
    for i, kind in enumerate(params["kinds"]):
        a = params["in_amp"][i]
        if kind in ("silence", "silence2"):
            seg = rng.normal(0.0, 1.5, FRAME)
        elif kind == "hot":
            seg = 0.92 * np.sin(2.0 * np.pi * (1000.0 / RATE) * np.arange(FRAME) + phase)
            seg = seg * a + rng.normal(0.0, 120.0, FRAME)
            phase += 0.31
        else:
            seg = 0.55 * np.sin(2.0 * np.pi * (440.0 / RATE) * np.arange(FRAME) + phase) \
                + 0.45 * np.sin(2.0 * np.pi * (1350.0 / RATE) * np.arange(FRAME) + phase * 1.7)
            seg = seg * a * 0.85 + rng.normal(0.0, a * 0.06, FRAME)
            phase += 0.17
        out[i * FRAME:(i + 1) * FRAME] = seg
    return np.clip(np.rint(out), -32768.0, 32767.0).astype(np.int16)


# --------------------------------------------------------------------------
# 由合成音频反算指标
# --------------------------------------------------------------------------

def metrics_from_audio(in_sig: np.ndarray, out_sig: np.ndarray,
                       params: dict, variant: str, inject: bool, seed: int) -> dict:
    """真实 STFT 反算 `signal_energy` / `signal_spectral_sum` / `diff_normalization`。"""
    n_frames = len(params["kinds"])
    m = {c: np.zeros(n_frames) for c in K.METRICS_COLUMNS}
    m["frame_idx"] = np.arange(n_frames, dtype=np.int64)

    # 中频带（4–8 kHz）功率和 → 与 signal_* 同量纲的线性诊断量
    _, f, mag_in = K.stft_mag(in_sig, n_fft=512, hop=256)
    _, _, mag_out = K.stft_mag(out_sig, n_fft=512, hop=256)
    band = (f >= 4000.0) & (f <= 8000.0)
    psd_in = (mag_in[:, band] ** 2).sum(axis=1)
    psd_out = (mag_out[:, band] ** 2).sum(axis=1)

    # D1 缺陷项 Y[257]²：真实实现读到的是栈残留值，量级随帧起伏且**无确定符号**；
    # 这里用平滑随机游走的平方模拟其强度（保证非负），仅叠加到 baseline 的 signal_energy。
    rng_d1 = np.random.default_rng(seed + 4242)
    walk = np.convolve(rng_d1.normal(0.0, 1.0, n_frames + 40), np.ones(41) / 41.0, mode="same")[:n_frames]
    d1_term = (np.abs(walk) * 3.0e7) ** 2
    # 覆盖"零帧"分工：约 8% 的帧 Δ==0（真实数据里是 `zero_frame` 提前返回、
    # 两变体 signal_energy 相同），用于验证 FigT08 对「未检出 D1 效应」的显式标注。
    if n_frames > 20:
        zero_idx = rng_d1.choice(n_frames, size=max(1, int(n_frames * 0.08)), replace=False)
        d1_term[zero_idx] = 0.0

    for i in range(n_frames):
        sl = slice(i * FRAME, (i + 1) * FRAME)
        fi = min(i * 2, psd_in.size - 1) if psd_in.size else 0
        in_rms = float(np.sqrt(np.mean(np.square(in_sig[sl].astype(np.float64)))))
        out_rms = float(np.sqrt(np.mean(np.square(out_sig[sl].astype(np.float64)))))
        m["in_rms"][i] = in_rms
        m["out_rms"][i] = out_rms
        m["atten_db"][i] = K.atten_db(in_rms, out_rms)
        energy = float(psd_in[fi]) if psd_in.size else 0.0
        # 模拟 D1 缺陷：baseline（D1 未修）在 i=128 项多算一次 Y[257]²，
        # 故 `signal_energy` 比 d1 高 `Y[257]²/129`。以此保证
        # `Δ = energy_baseline − energy_d1 ≥ 0`，与真实机理同号。
        if variant == "baseline":
            energy += d1_term[i] / 129.0
        spec_sum = float(psd_out[fi]) if psd_out.size else 0.0
        m["signal_energy"][i] = energy
        m["signal_spectral_sum"][i] = spec_sum
        m["diff_normalization"][i] = abs(energy - spec_sum) / (abs(energy) + abs(spec_sum) + 1e-9) \
            if (abs(energy) + abs(spec_sum)) > 0 else 0.0
    m["speech_prob_mean"] = params["prob"]
    m["prior_speech_prob"] = np.clip(params["prob"] * 0.92 + 0.05, 0.0, 1.0)
    m["gain_adjustment"] = 0.5 - params["att"] / 40.0
    m["clamp_hits"] = params["hits"].astype(np.int64)
    m["elapsed_us"] = params["elapsed"].astype(np.int64)

    # ---- 注入异常（仅 --inject 模式；用于 FigT22 检出分支）----
    if inject:
        rng = np.random.default_rng(seed + 900)
        if n_frames > 12:
            m["speech_prob_mean"][5] = np.nan
            m["diff_normalization"][9] = np.inf
            m["gain_adjustment"][12] = -np.inf
        # 钳位超限帧：制造绝对离群
        hot = [i for i, k in enumerate(params["kinds"]) if k == "hot"]
        if hot:
            m["clamp_hits"][hot[-1]] = 480
        # 异常衰减：正常段里插入一个 3×IQR 之外的离群帧
        normal = np.where((m["atten_db"] > 0) & (m["atten_db"] < 100))[0]
        if normal.size > 4:
            m["atten_db"][normal[-2]] = float(np.max(m["atten_db"][normal]) + 18.0)
    return m


# --------------------------------------------------------------------------
# 写盘
# --------------------------------------------------------------------------

def write_wav(path: str, sig: np.ndarray, rate: int = RATE) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(rate)
        wf.writeframes(sig.astype("<i2").tobytes())


def write_metrics(path: str, m: dict) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    cols = list(K.METRICS_COLUMNS)
    n = int(m["frame_idx"].size)
    lines = [",".join(cols)]
    for i in range(n):
        cells = []
        for c in cols:
            v = m[c][i]
            if c in ("frame_idx", "clamp_hits", "elapsed_us"):
                cells.append("%d" % int(v))
            else:
                cells.append("%.9g" % float(v))
        lines.append(",".join(cells))
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")


def duration_of(name: str) -> float:
    """每个文件的合成时长。

    刻意 ≥ 6 s：使 FigT04 的默认放大窗口 5.0–5.5 s 在自测中**走真实分支**
    （验证生产口径），而不是回退分支。同时保持各文件时长互不相同。
    """
    base = 6.0 + (int(name) % 4) * 0.25
    return base + (0.75 if name == "6" else 0.0)


#: 全部消融变体都生成完整 metrics + WAV：FigT17/FigT18/FigT19 都会跨变体读取
FULL_VARIANTS = K.ABLATION_ORDER


def generate(out_root: str, inject: bool = False) -> tuple[dict, dict]:
    names = K.test_file_names()
    summary: dict[str, list[dict]] = {}
    stats = {"files": 0, "sentinel_frames": 0, "clamp_frames": 0, "wav_bytes": 0}

    for variant in K.ABLATION_ORDER:
        rows: list[dict] = []
        full = variant in FULL_VARIANTS
        for k, name in enumerate(names):
            dur = duration_of(name)
            n_frames = int(dur * RATE / FRAME)
            params = frame_params(n_frames, variant, seed=100 + k * 7 + (abs(hash(variant)) % 1000))
            in_sig = synth_audio(params)
            out_sig = np.clip(np.rint(
                in_sig.astype(np.float64) * np.power(10.0, -params["att"].repeat(FRAME) / 20.0)),
                -32768.0, 32767.0).astype(np.int16)
            # 静音段的输出精确为 0（保证 out_rms==0 → atten_db 哨兵）
            out_sig[np.repeat(params["quiet"], FRAME)] = 0

            m = metrics_from_audio(in_sig, out_sig, params, variant, inject, seed=300 + k)
            if full:
                d = os.path.join(out_root, variant, LEVEL)
                wav_p = os.path.join(d, "%s.wav" % name)
                write_wav(wav_p, out_sig)
                write_metrics(os.path.join(d, "metrics_%s.csv" % name), m)
                stats["files"] += 1
                stats["wav_bytes"] += os.path.getsize(wav_p)
                stats["sentinel_frames"] += int(
                    np.sum(np.abs(m["atten_db"] - K.ATTEN_DB_SENTINEL) < 1e-6))
                stats["clamp_frames"] += int(np.sum(m["clamp_hits"] > 0))

            in_rms_t = K.rms(in_sig)
            out_rms_t = K.rms(out_sig)
            rows.append({
                "file": name, "duration_s": dur, "frames": n_frames,
                "total_elapsed_ms": float(m["elapsed_us"].sum() / 1000.0),
                "in_rms": in_rms_t, "out_rms": out_rms_t,
                "atten_db": K.atten_db(in_rms_t, out_rms_t),
                "in_peak": float(np.max(np.abs(in_sig))),
                "out_peak": float(np.max(np.abs(out_sig))),
                "clamp_hits_total": int(m["clamp_hits"].sum()),
            })
        summary[variant] = rows
        path = os.path.join(out_root, variant, LEVEL, "summary_%s_%s.csv" % (variant, LEVEL))
        write_summary(path, variant, rows)
        write_build_info(os.path.join(out_root, variant, "build_info.txt"), variant, inject)

    # reference：与主流程同结构（供 FigT20 走完整分支）
    ref_rows = []
    for k, name in enumerate(names[:2]):
        dur = duration_of(name)
        n_frames = int(dur * RATE / FRAME)
        params = frame_params(n_frames, "baseline", seed=770 + k)
        # 参照路径衰减略强于主路径基线（形态对比用）
        params["att"] = params["att"] + 4.5 * np.clip(params["prob"], 0.0, 1.0)
        in_sig = synth_audio(params)
        out_sig = np.clip(np.rint(
            in_sig.astype(np.float64) * np.power(10.0, -params["att"].repeat(FRAME) / 20.0)),
            -32768.0, 32767.0).astype(np.int16)
        out_sig[np.repeat(params["quiet"], FRAME)] = 0
        m = metrics_from_audio(in_sig, out_sig, params, "baseline", False, seed=880 + k)
        d = os.path.join(out_root, "reference", LEVEL)
        write_wav(os.path.join(d, "%s.wav" % name), out_sig)
        write_metrics(os.path.join(d, "metrics_%s.csv" % name), m)
        in_rms_t, out_rms_t = K.rms(in_sig), K.rms(out_sig)
        ref_rows.append({"file": name, "duration_s": dur, "frames": n_frames,
                         "total_elapsed_ms": float(m["elapsed_us"].sum() / 1000.0),
                         "in_rms": in_rms_t, "out_rms": out_rms_t,
                         "atten_db": K.atten_db(in_rms_t, out_rms_t),
                         "in_peak": float(np.max(np.abs(in_sig))),
                         "out_peak": float(np.max(np.abs(out_sig))),
                         "clamp_hits_total": int(m["clamp_hits"].sum())})
    if ref_rows:
        write_summary(os.path.join(out_root, "reference", LEVEL, "summary_reference_%s.csv" % LEVEL),
                      "reference", ref_rows)
        write_build_info(os.path.join(out_root, "reference", "build_info.txt"), "reference", inject)
    return stats, summary


def write_summary(path: str, variant: str, rows: list[dict]) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    cols = list(K.SUMMARY_COLUMNS)
    lines = [",".join(cols)]
    for r in rows:
        cells = []
        for c in cols:
            v = r[c]
            if c == "file":
                cells.append(str(v))
            elif c == "frames" or c == "clamp_hits_total":
                cells.append("%d" % int(v))
            else:
                cells.append("%.9g" % float(v))
        lines.append(",".join(cells))
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")


def write_build_info(path: str, variant: str, inject: bool) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    flags = K.PATCH_FLAGS.get(variant, 0)
    text = (
        "variant         = %s\n"
        "patch_flags     = %d\n"
        "sample_depth    = 24\n"
        "cmake_generator = Visual Studio 17 2022\n"
        "build_type      = Release\n"
        "compiler        = MSVC 19.44.35211\n"
        "c_flags         = /O2 /W3\n"
        "git_commit      = N/A\n"
        "build_time      = SELFTEST-SYNTHETIC%s\n"
        % (variant.upper(), flags, " (INJECTED)" if inject else "")
    )
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description="生成 T4-I 自测合成数据")
    ap.add_argument("--out", default="", help="输出根目录（默认 _selftest/data 或 _selftest/data_inject）")
    ap.add_argument("--inject", action="store_true", help="注入 NaN/Inf 与超限钳位（验证 FigT22 检出分支）")
    args = ap.parse_args(argv)

    here = os.path.dirname(os.path.abspath(__file__))
    default_root = "_selftest/data_inject" if args.inject else "_selftest/data"
    out_root = os.path.abspath(args.out) if args.out else os.path.join(here, *default_root.split("/"))

    print("=" * 84)
    print("[make_selftest_data] 输出根目录 %s" % out_root)
    print("[make_selftest_data] 模式 %s" % ("INJECT（含异常）" if args.inject else "CLEAN（无异常）"))
    print("=" * 84)
    stats, summary = generate(out_root, inject=args.inject)
    print("  生成文件数        : %d" % stats["files"])
    print("  WAV 总字节        : %d" % stats["wav_bytes"])
    print("  哨兵帧 (atten=999): %d" % stats["sentinel_frames"])
    print("  含钳位命中的帧    : %d" % stats["clamp_frames"])
    for variant, rows in summary.items():
        tot = sum(r["clamp_hits_total"] for r in rows)
        print("  %-10s 文件 %d  合计钳位 %d" % (variant, len(rows), tot))
    print("[make_selftest_data] 完成")
    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main(sys.argv[1:]))
