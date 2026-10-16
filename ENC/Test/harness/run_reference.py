#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
run_reference.py —— T6 交叉参照运行器

用途：用**另一条独立移植路径** `Project/ENC_Core`（Ooura fft4g + 独立 NS 实现，
经 `ENC_Core.dll` 导出）处理与本项目完全相同的一批输入音频，产出可做形态对比的数据：

    <data>/reference/L3/<name>.wav
    <data>/reference/L3/metrics_<name>.csv
    <data>/reference/L3/summary_reference_L3.csv
    <data>/reference/build_info.txt

设计说明（执行记录 B-19 / B-20）：
  * `Project/ENC_Core` 以 SHARED library 构建，导出 C 链接符号（x64 无名字修饰），
    故用 ctypes 直接加载即可，无需在 C 侧写 LoadLibrary 桩。
    接入逻辑与 `ns_runner.c` 保持同构：同样的输入读取、同样的取左声道、同样的 480 分帧、
    同样的量纲约定与同样的指标口径 —— 满足计划书 T6「接入同一套 harness」的实质要求。
  * **量纲**：`Project/ENC_Core` 工作在 floatS16 量纲（±32768），
    因此**不做 ×256 换算**（计划书 T6 明确规定），输入直接送 int16 值，输出直接钳位回 int16。
  * **诊断量**：该路径不暴露 `NsDiagnostics`，故 metrics 的 6 个诊断列写 `nan`
    （列定义与计划书 6.2 完全一致，不做删列）；可由输出计算的
    in_rms / out_rms / atten_db / clamp_hits / elapsed_us 均正常产出。
  * **禁止**：不修改 `Project/` 下任何源码（它是参照物）；不对两条路径做逐点相减
    （计划书 T6：仅做形态对比）。

用法：
    <python> run_reference.py [--level 3] [--data <dir>] [--limit N]
"""

import argparse
import ctypes
import math
import os
import struct
import sys
import time
import wave

HERE = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
DLL_PATH = os.path.join(REPO_ROOT, "ENC", "Test", "build", "enc_core_probe",
                        "Release", "ENC_Core.dll")
INPUT_DIR = os.path.join(REPO_ROOT, "Test_Audio", "AudioSample-48000hz")
DEFAULT_DATA = os.path.join(REPO_ROOT, "ENC", "Test", "data")

FRAME = 480
RATE = 48000
INPUT_NAMES = [str(i) for i in range(1, 10)]

METRICS_HEADER = ("frame_idx,in_rms,out_rms,atten_db,signal_energy,signal_spectral_sum,"
                  "diff_normalization,speech_prob_mean,prior_speech_prob,gain_adjustment,"
                  "clamp_hits,elapsed_us")
SUMMARY_HEADER = ("file,duration_s,frames,total_elapsed_ms,in_rms,out_rms,atten_db,"
                  "in_peak,out_peak,clamp_hits_total")

CLAMP_LO, CLAMP_HI = -32768, 32767


def log(msg):
    print(msg, flush=True)


# ----------------------------------------------------------------- 输入读取

def read_wav_left(path):
    """读 16bit PCM WAV 的左声道，返回 (samples:int16 list, n_frames, n_ch, rate)。"""
    with wave.open(path, "rb") as w:
        n_ch = w.getnchannels()
        sw = w.getsampwidth()
        rate = w.getframerate()
        n_frames = w.getnframes()
        raw = w.readframes(n_frames)
    if sw != 2:
        raise ValueError("%s: 仅支持 16bit PCM（实际 %d 字节）" % (path, sw))
    total = n_frames * n_ch
    vals = struct.unpack("<%dh" % total, raw)
    left = list(vals[0::n_ch])
    return left, n_frames, n_ch, rate


def write_wav_mono(path, samples):
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(struct.pack("<%dh" % len(samples), *samples))


# ----------------------------------------------------------------- DLL 绑定

def load_engine():
    if not os.path.isfile(DLL_PATH):
        raise SystemExit("[FAIL] 找不到 ENC_Core.dll: %s\n"
                         "       请先构建：cmake --build ENC/Test/build/enc_core_probe --config Release"
                         % DLL_PATH)
    dll = ctypes.CDLL(DLL_PATH)
    dll.InitNSHandler.argtypes = [ctypes.c_int]
    dll.InitNSHandler.restype = ctypes.c_void_p
    dll.FreeNSHandler.argtypes = [ctypes.c_void_p]
    dll.FreeNSHandler.restype = None
    # 注意：该函数实现缺 return 语句（编译期 C4715），返回值未定义，因此不读取其返回值
    dll.NS_Process_48kAudio.argtypes = [ctypes.c_void_p,
                                        ctypes.POINTER(ctypes.c_float),
                                        ctypes.POINTER(ctypes.c_float)]
    dll.NS_Process_48kAudio.restype = None
    return dll


# ----------------------------------------------------------------- 单文件处理

def process_file(dll, name, level, out_dir):
    src = os.path.join(INPUT_DIR, "%s.wav" % name)
    left, n_frames, n_ch, rate = read_wav_left(src)
    if rate != RATE:
        raise ValueError("%s: 采样率 %d != %d" % (src, rate, RATE))

    handler = dll.InitNSHandler(int(level))
    if not handler:
        raise RuntimeError("InitNSHandler(%d) 返回 NULL" % level)

    in_buf = (ctypes.c_float * FRAME)()
    out_buf = (ctypes.c_float * FRAME)()

    out16 = [0] * n_frames
    rows = []
    sum_in_sq = 0.0
    sum_out_sq = 0.0
    total_samples = 0
    total_us = 0
    clamp_total = 0
    in_peak = 0
    out_peak = 0

    total_frames = (n_frames + FRAME - 1) // FRAME
    for fi in range(total_frames):
        base = fi * FRAME
        valid = min(FRAME, n_frames - base)

        for k in range(FRAME):
            idx = base + k
            in_buf[k] = float(left[idx]) if idx < n_frames else 0.0

        t0 = time.perf_counter()
        dll.NS_Process_48kAudio(handler, in_buf, out_buf)
        t1 = time.perf_counter()
        elapsed_us = int((t1 - t0) * 1e6)
        total_us += elapsed_us

        in_sq = 0.0
        out_sq = 0.0
        for k in range(FRAME):
            iv = in_buf[k]
            ov = out_buf[k]
            in_sq += iv * iv
            out_sq += ov * ov
            if abs(iv) > in_peak:
                in_peak = abs(iv)
            if abs(ov) > out_peak:
                out_peak = abs(ov)

        in_rms = math.sqrt(in_sq / FRAME)
        out_rms = math.sqrt(out_sq / FRAME)
        atten_db = 999.0 if out_rms == 0.0 else 20.0 * math.log10(in_rms / out_rms)

        # 还原为 int16（floatS16 量纲，不除以 256）并统计钳位
        frame_clamp = 0
        for k in range(FRAME):
            idx = base + k
            if idx >= n_frames:
                break
            v = int(round(out_buf[k]))
            if v < CLAMP_LO:
                v = CLAMP_LO
                frame_clamp += 1
            elif v > CLAMP_HI:
                v = CLAMP_HI
                frame_clamp += 1
            out16[idx] = v
        clamp_total += frame_clamp

        # 诊断量在该路径不可得 -> 写 nan（列定义不变）
        rows.append("%d,%.9g,%.9g,%.9g,nan,nan,nan,nan,nan,nan,%d,%d"
                    % (fi, in_rms, out_rms, atten_db, frame_clamp, elapsed_us))

        sum_in_sq += in_sq
        sum_out_sq += out_sq
        total_samples += FRAME

    dll.FreeNSHandler(handler)

    os.makedirs(out_dir, exist_ok=True)
    write_wav_mono(os.path.join(out_dir, "%s.wav" % name), out16)
    with open(os.path.join(out_dir, "metrics_%s.csv" % name), "w",
              encoding="utf-8", newline="\n") as f:
        f.write(METRICS_HEADER + "\n")
        f.write("\n".join(rows) + "\n")

    total_in_rms = math.sqrt(sum_in_sq / total_samples)
    total_out_rms = math.sqrt(sum_out_sq / total_samples)
    total_atten = 999.0 if total_out_rms == 0.0 else 20.0 * math.log10(total_in_rms / total_out_rms)

    log("  [%s] frames=%d atten_db=%.3f clamp=%d total_ms=%.3f"
        % (name, total_frames, total_atten, clamp_total, total_us / 1000.0))

    return {
        "file": name,
        "duration_s": n_frames / float(RATE),
        "frames": total_frames,
        "total_elapsed_ms": total_us / 1000.0,
        "in_rms": total_in_rms,
        "out_rms": total_out_rms,
        "atten_db": total_atten,
        "in_peak": in_peak,
        "out_peak": out_peak,
        "clamp_hits_total": clamp_total,
    }


# ----------------------------------------------------------------- main

def main():
    ap = argparse.ArgumentParser(description="T6 交叉参照运行器（Project/ENC_Core 路径）")
    ap.add_argument("--level", type=int, default=3, choices=[0, 1, 3])
    ap.add_argument("--data", default=DEFAULT_DATA)
    ap.add_argument("--limit", type=int, default=0, help="仅处理前 N 个文件（0=全部）")
    args = ap.parse_args()

    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

    level_tag = "L%d" % args.level
    out_dir = os.path.join(args.data, "reference", level_tag)

    log("=" * 72)
    log("T6 交叉参照运行   引擎 = %s" % DLL_PATH)
    log("  量纲      = floatS16 (±32768)，不做 ×256 换算（计划书 T6）")
    log("  输入      = %s" % INPUT_DIR)
    log("  输出      = %s" % out_dir)
    log("  level     = %d" % args.level)
    log("=" * 72)

    dll = load_engine()
    if not os.path.isfile(DLL_PATH):
        return 1

    names = INPUT_NAMES if args.limit <= 0 else INPUT_NAMES[:args.limit]
    os.makedirs(out_dir, exist_ok=True)

    rows = []
    for name in names:
        src = os.path.join(INPUT_DIR, "%s.wav" % name)
        if not os.path.isfile(src):
            log("  [SKIP] 缺输入 %s" % src)
            continue
        rows.append(process_file(dll, name, args.level, out_dir))

    summ_path = os.path.join(out_dir, "summary_reference_%s.csv" % level_tag)
    with open(summ_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(SUMMARY_HEADER + "\n")
        for r in rows:
            f.write("%s,%.9g,%d,%.6g,%.9g,%.9g,%.9g,%.9g,%.9g,%d\n"
                    % (r["file"], r["duration_s"], r["frames"], r["total_elapsed_ms"],
                       r["in_rms"], r["out_rms"], r["atten_db"], r["in_peak"],
                       r["out_peak"], r["clamp_hits_total"]))

    info_path = os.path.join(args.data, "reference", "build_info.txt")
    with open(info_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("variant         = REFERENCE\n")
        f.write("engine          = Project/ENC_Core (Ooura fft4g, 独立移植路径)\n")
        f.write("engine_binary   = %s\n" % DLL_PATH)
        f.write("patch_flags     = N/A (该路径无本项目开关)\n")
        f.write("sample_depth    = 16 (floatS16 量纲, 未做 ×256 换算)\n")
        f.write("level           = %d\n" % args.level)
        f.write("diagnostics     = 不可得 (该路径不暴露 NsDiagnostics, metrics 诊断列写 nan)\n")
        f.write("runner          = ENC/Test/harness/run_reference.py (ctypes)\n")
        f.write("note            = 与主路径仅做形态对比, 不做逐点相减 (计划书 T6)\n")

    log("-" * 72)
    log("产出 %d 个文件 -> %s" % (len(rows), out_dir))
    log("summary   -> %s" % summ_path)
    log("build_info-> %s" % info_path)
    if rows:
        avg = sum(r["atten_db"] for r in rows) / len(rows)
        log("整段 atten_db 均值 = %.3f dB (level %d)" % (avg, args.level))
    log("=" * 72)
    return 0


if __name__ == "__main__":
    sys.exit(main())
