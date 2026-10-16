#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
verify_data.py —— 测量数据完整性校验（P6 验收工具）

用途：为计划书第 11.2 节测试验收项 B1–B6 提供可直接引用的证据。
本脚本是**验收工具**，不参与绘图与 PPT 生成；其文件名不匹配 `fig_*.py`，
因此不会被 run_all.py 误调用。

用法：
    <python> verify_data.py                 # 校验已存在的组合，报告缺失项
    <python> verify_data.py --strict        # 按计划书 B5/B6 完整要求校验，缺失即判定失败
    <python> verify_data.py --data <dir>    # 指定数据根目录（默认 ENC/Test/data）

校验内容：
  1. 组合齐备性（variant × level）
  2. 每个组合：9 个输出 WAV + 9 个 metrics CSV + 1 个 summary CSV + build_info.txt
  3. metrics CSV 表头与计划书 6.2 逐字一致
  4. summary CSV 表头与计划书 6.3 逐字一致
  5. 全部数值有限（无 NaN / Inf）
  6. summary 的 frames 与 metrics 行数一致
  7. 输出 WAV 帧数 == 对应输入 WAV 帧数（补零裁剪后应等长）
  8. build_info.txt 的 patch_flags 与变体期望一致（baseline=0 / d1=1 / d1d5=3 / optimized=7）

退出码：0 = 全部通过；1 = 存在失败项
"""

import argparse
import math
import os
import sys
import wave

# ---------------------------------------------------------------- 路径与常量

HERE = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
DEFAULT_DATA = os.path.join(REPO_ROOT, "ENC", "Test", "data")
INPUT_AUDIO = os.path.join(REPO_ROOT, "Test_Audio", "AudioSample-48000hz")

METRICS_HEADER = [
    "frame_idx", "in_rms", "out_rms", "atten_db", "signal_energy",
    "signal_spectral_sum", "diff_normalization", "speech_prob_mean",
    "prior_speech_prob", "gain_adjustment", "clamp_hits", "elapsed_us",
]
SUMMARY_HEADER = [
    "file", "duration_s", "frames", "total_elapsed_ms", "in_rms", "out_rms",
    "atten_db", "in_peak", "out_peak", "clamp_hits_total",
]
INPUT_NAMES = [str(i) for i in range(1, 10)]

# variant -> (期望 patch_flags, 期望齐备的 level 列表)
# bit0=D1 bit1=D5 bit2=D6 bit3=D8（D8 为执行记录 B-24 新发现缺陷，经用户决策纳入修复范围）
EXPECTED = {
    "baseline":  (0,  ["L0", "L1", "L3"]),
    "optimized": (15, ["L0", "L1", "L3"]),
    "d1":        (1,  ["L3"]),
    "d1d5":      (3,  ["L3"]),
    "d1d5d6":    (7,  ["L3"]),
}
# 只有含 D8 的变体才被要求"无 NaN"；其余变体出现 nan 是固件固有行为（D8），作为警告呈现
VARIANTS_REQUIRE_FINITE = {"optimized"}
# reference 由 T6 生成，结构同构；不参与 B5/B6 的强制性判定
OPTIONAL_VARIANTS = {"reference": (0, ["L3"])}


# ---------------------------------------------------------------- 工具函数

class Report(object):
    def __init__(self):
        self.errors = []
        self.warnings = []
        self.notes = []
        self.nan_hits = []

    def err(self, msg):
        self.errors.append(msg)

    def warn(self, msg):
        self.warnings.append(msg)

    def note(self, msg):
        self.notes.append(msg)

    def nan(self, where, text):
        """非有限值：默认作为警告统计（基线含固件固有的 D8 缺陷，会写出 nan）。
        含 D8 的变体若出现则另行判为错误，见 main()。"""
        self.nan_hits.append(where)


def parse_float(text, where, rep):
    try:
        v = float(text)
    except ValueError:
        rep.err("%s: 非法浮点值 %r" % (where, text))
        return None
    if math.isnan(v) or math.isinf(v):
        rep.nan(where, text)
    return v


def read_csv_rows(path):
    with open(path, "r", encoding="utf-8", newline="") as f:
        raw = f.read()
    lines = [ln for ln in raw.replace("\r\n", "\n").split("\n") if ln.strip() != ""]
    if not lines:
        return None, []
    header = [h.strip() for h in lines[0].split(",")]
    rows = [ln.split(",") for ln in lines[1:]]
    return header, rows


def wav_frames(path):
    with wave.open(path, "rb") as w:
        return (w.getnframes(), w.getnchannels(), w.getsampwidth(), w.getframerate())


def check_build_info(path, variant, expected_flags, rep):
    if not os.path.isfile(path):
        rep.err("%s: build_info.txt 缺失" % variant)
        return
    kv = {}
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            if "=" in line:
                k, v = line.split("=", 1)
                kv[k.strip()] = v.strip().split("#")[0].strip()
    if "patch_flags" not in kv:
        rep.err("%s: build_info.txt 缺少 patch_flags" % variant)
        return
    try:
        actual = int(kv["patch_flags"])
    except ValueError:
        rep.err("%s: patch_flags 非整数: %r" % (variant, kv["patch_flags"]))
        return
    if actual != expected_flags:
        rep.err("%s: patch_flags=%d 与期望 %d 不符" % (variant, actual, expected_flags))
    else:
        rep.note("%s: patch_flags=%d ✓  variant=%s" % (variant, actual, kv.get("variant", "?")))
    if kv.get("sample_depth") != "24":
        rep.err("%s: sample_depth=%r 应为 24" % (variant, kv.get("sample_depth")))


def check_combo(data_root, variant, level, rep):
    tag = "%s/%s" % (variant, level)
    d = os.path.join(data_root, variant, level)
    if not os.path.isdir(d):
        rep.err("%s: 目录不存在" % tag)
        return False

    for name in INPUT_NAMES:
        # --- 输出 WAV
        wav_out = os.path.join(d, "%s.wav" % name)
        if not os.path.isfile(wav_out):
            rep.err("%s: 缺少输出 WAV %s.wav" % (tag, name))
        else:
            nf_out, ch, sw, rate = wav_frames(wav_out)
            if ch != 1:
                rep.err("%s: %s.wav 声道数=%d 应为 1" % (tag, name, ch))
            if sw != 2:
                rep.err("%s: %s.wav 位深=%d 字节 应为 2" % (tag, name, sw))
            if rate != 48000:
                rep.err("%s: %s.wav 采样率=%d 应为 48000" % (tag, name, rate))
            wav_in = os.path.join(INPUT_AUDIO, "%s.wav" % name)
            if os.path.isfile(wav_in):
                nf_in = wav_frames(wav_in)[0]
                if nf_out != nf_in:
                    rep.err("%s: %s.wav 帧数=%d 与输入 %d 不一致" % (tag, name, nf_out, nf_in))

        # --- metrics CSV
        met = os.path.join(d, "metrics_%s.csv" % name)
        if not os.path.isfile(met):
            rep.err("%s: 缺少 metrics_%s.csv" % (tag, name))
            continue
        header, rows = read_csv_rows(met)
        if header != METRICS_HEADER:
            rep.err("%s: metrics_%s.csv 表头不符\n      实际: %s\n      期望: %s"
                    % (tag, name, header, METRICS_HEADER))
            continue
        bad = 0
        for r_i, row in enumerate(rows):
            if len(row) != len(METRICS_HEADER):
                rep.err("%s: metrics_%s.csv 第 %d 行列数=%d" % (tag, name, r_i + 1, len(row)))
                bad += 1
                if bad > 3:
                    break
                continue
            for c_i, cell in enumerate(row):
                col = METRICS_HEADER[c_i]
                if col in ("frame_idx", "clamp_hits", "elapsed_us"):
                    try:
                        int(cell)
                    except ValueError:
                        rep.err("%s: metrics_%s.csv 行%d 列%s 非整数: %r"
                                % (tag, name, r_i + 1, col, cell))
                else:
                    # reference 路径（Project/ENC_Core）不暴露诊断量，其 metrics 的 6 个诊断列
                    # 按约定写 nan 作为"不可得"占位 —— 与主路径的 D8 缺陷 NaN 语义不同，此处豁免。
                    if variant == "reference" and col in (
                            "signal_energy", "signal_spectral_sum", "diff_normalization",
                            "speech_prob_mean", "prior_speech_prob", "gain_adjustment"):
                        continue
                    parse_float(cell, "%s metrics_%s.csv 行%d 列%s" % (tag, name, r_i + 1, col), rep)
        rep.note("%s: metrics_%s.csv  %d 帧 ✓" % (tag, name, len(rows)))

    # --- summary
    summ = os.path.join(d, "summary_%s_%s.csv" % (variant, level))
    if not os.path.isfile(summ):
        rep.err("%s: 缺少 summary_%s_%s.csv" % (tag, variant, level))
    else:
        header, rows = read_csv_rows(summ)
        if header != SUMMARY_HEADER:
            rep.err("%s: summary 表头不符\n      实际: %s\n      期望: %s"
                    % (tag, header, SUMMARY_HEADER))
        elif len(rows) != 9:
            rep.err("%s: summary 行数=%d 应为 9" % (tag, len(rows)))
        else:
            for r_i, row in enumerate(rows):
                if len(row) != len(SUMMARY_HEADER):
                    rep.err("%s: summary 第 %d 行列数=%d" % (tag, r_i + 1, len(row)))
                    continue
                for c_i, cell in enumerate(row):
                    col = SUMMARY_HEADER[c_i]
                    if col == "file":
                        continue
                    if col == "frames":
                        try:
                            int(cell)
                        except ValueError:
                            rep.err("%s: summary 行%d frames 非整数" % (tag, r_i + 1))
                    else:
                        parse_float(cell, "%s summary 行%d 列%s" % (tag, r_i + 1, col), rep)
            # frames 与 metrics 行数交叉核对
            for r_i, row in enumerate(rows):
                fname = row[0]
                met = os.path.join(d, "metrics_%s.csv" % fname)
                if os.path.isfile(met) and len(row) > 2:
                    _, mrows = read_csv_rows(met)
                    try:
                        sf = int(row[2])
                    except ValueError:
                        continue
                    if sf != len(mrows):
                        rep.err("%s: summary 中 %s 的 frames=%d 与 metrics 行数 %d 不一致"
                                % (tag, fname, sf, len(mrows)))
            rep.note("%s: summary 9 行 ✓" % tag)

    return True


# ---------------------------------------------------------------- 主流程

def main():
    ap = argparse.ArgumentParser(description="测量数据完整性校验")
    ap.add_argument("--data", default=DEFAULT_DATA, help="数据根目录")
    ap.add_argument("--strict", action="store_true",
                    help="按计划书 B5/B6 完整要求校验，缺失组合即失败")
    args = ap.parse_args()

    rep = Report()
    data_root = os.path.abspath(args.data)
    print("=" * 72)
    print("数据完整性校验    data = %s" % data_root)
    print("=" * 72)

    if not os.path.isdir(data_root):
        print("[FAIL] 数据根目录不存在: %s" % data_root)
        return 1

    # --- build_info 与组合齐备性
    for variant, (flags, levels) in EXPECTED.items():
        check_build_info(os.path.join(data_root, variant, "build_info.txt"), variant, flags, rep)
        have = [lv for lv in levels if os.path.isdir(os.path.join(data_root, variant, lv))]
        miss = [lv for lv in levels if lv not in have]
        if miss:
            msg = "%s: 缺失 level %s" % (variant, ",".join(miss))
            (rep.err if args.strict else rep.warn)(msg)
        for lv in have:
            check_combo(data_root, variant, lv, rep)

    # --- 可选的 reference
    for variant, (flags, levels) in OPTIONAL_VARIANTS.items():
        base = os.path.join(data_root, variant)
        if os.path.isdir(base):
            for lv in levels:
                if os.path.isdir(os.path.join(base, lv)):
                    check_combo(data_root, variant, lv, rep)
        else:
            rep.note("%s: 未生成（T6 可降级项）" % variant)

    # --- 含 D8 的变体必须无 NaN（D8 的修复目标就是消除它；其余变体出现 nan 属固件固有行为）
    for v in sorted(VARIANTS_REQUIRE_FINITE):
        hits = [h for h in rep.nan_hits if h.startswith(v + "/")]
        if hits:
            rep.err("%s: 该变体已启用 D8 修复，不应出现 NaN/Inf，实测 %d 处（首处: %s）"
                    % (v, len(hits), hits[0]))

    # --- 报告
    print("\n" + "-" * 72)
    if rep.nan_hits:
        by_variant = {}
        for h in rep.nan_hits:
            key = h.split("/")[0].split(":")[0]
            by_variant[key] = by_variant.get(key, 0) + 1
        print("[非有限值] 共 %d 处（固件固有 D8 缺陷的表现，非测量故障）" % len(rep.nan_hits))
        for k in sorted(by_variant):
            mark = "（已启用 D8，不应出现）" if k in VARIANTS_REQUIRE_FINITE else ""
            print("  %-12s %6d 处 %s" % (k, by_variant[k], mark))
        print("  首处: %s" % rep.nan_hits[0])
    if rep.notes:
        print("[信息]")
        for m in rep.notes:
            print("  " + m)
    if rep.warnings:
        print("\n[警告]（非 strict 模式下不判失败）")
        for m in rep.warnings:
            print("  " + m)
    if rep.errors:
        print("\n[失败]")
        for m in rep.errors:
            print("  " + m)
    print("\n" + "=" * 72)
    verdict = "PASS" if not rep.errors else "FAIL"
    print("结论：%s   错误 %d 项 / 警告 %d 项" % (verdict, len(rep.errors), len(rep.warnings)))
    print("=" * 72)
    return 0 if not rep.errors else 1


if __name__ == "__main__":
    sys.exit(main())
