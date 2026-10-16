# -*- coding: utf-8 -*-
"""
一键出图 + 生成 PPT + 校验

执行顺序
--------
1. 本目录下全部 `fig_*.py`（按编号顺序）
2. `build_deck.py`   —— 生成 34 页测试报告 PPT（属并行任务 T4-II）
3. `check_deck.py`   —— 校验 PPT（属并行任务 T4-II）

任一子步骤失败 → 打印脚本名与退出码，整体以非零码结束。
每个 `fig_*.py` 运行后额外校验其应产出的 PNG 是否真的落盘。

用法
----
    python run_all.py                 # 完整流程
    python run_all.py --skip-deck     # 只出图（deck 脚本尚未就绪时用）
    python run_all.py --only 05,07    # 只跑 FigT05 / FigT07
    python run_all.py --list          # 列出将执行的脚本与预期产物

环境变量
--------
    NS_FIG_OUT=<dir>   重定向图件输出目录（自测用；默认 `ENC/Document/figures`）
    NS_SELFTEST=1      重定向到 `analysis/_selftest/figures`
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time

import common as K

HERE = os.path.dirname(os.path.abspath(__file__))
DECK_SCRIPTS = ("build_deck.py", "check_deck.py")

#: 每个绘图脚本应产出的图件（文件名与计划书/PPT 脚本逐字一致）
EXPECTED = {
    "fig_waveform.py": ("FigT03_波形对比全段.png", "FigT04_波形对比放大.png"),
    "fig_spectrogram.py": ("FigT05_声谱三联.png", "FigT06_声谱差异.png", "FigT14_LTAS对比.png"),
    "fig_trace.py": ("FigT07_signal_energy轨迹.png", "FigT09_diff_normalization轨迹.png",
                     "FigT10_speech_prob_mean轨迹.png", "FigT11_gain_adjustment轨迹.png",
                     "FigT12_逐帧atten_db轨迹.png", "FigT16_启动期放大.png"),
    "fig_summary.py": ("FigT01_九文件衰减汇总.png", "FigT08_Y257反推.png",
                       "FigT13_分频带衰减.png", "FigT15_钳位命中统计.png",
                       "FigT17_消融五组对比.png", "FigT18_atten_db箱线图.png",
                       "FigT19_单帧耗时分布.png", "FigT21_无语音段与语音段衰减.png",
                       "FigT22_失败与边界案例.png"),
    "fig_reference.py": ("FigT20_参照形态对比.png",),
    "fig_architecture.py": ("FigT02_组件架构与数据流.png",),
}

#: 允许"数据缺失时跳过"的脚本 → 缺失不算失败
OPTIONAL_IF_MISSING = {"fig_reference.py", "fig_waveform.py", "fig_spectrogram.py"}

#: 脚本 → 其产出的图件编号（用于 `--only` 过滤；按图号而非文件名匹配）
SCRIPT_FIGS = {
    "fig_waveform.py": ("03", "04"),
    "fig_spectrogram.py": ("05", "06", "14"),
    "fig_trace.py": ("07", "09", "10", "11", "12", "16"),
    "fig_summary.py": ("01", "08", "13", "15", "17", "18", "19", "21", "22"),
    "fig_reference.py": ("20",),
    "fig_architecture.py": ("02",),
}


def discover_fig_scripts(only: set[str] | None = None) -> list[str]:
    """按编号顺序列出本目录的 `fig_*.py`；`only` 为一组图号（如 {'22'}）。"""
    files = [f for f in os.listdir(HERE)
             if f.startswith("fig_") and f.endswith(".py") and f != "fig_style.py"]
    if only:
        files = [f for f in files if only & set(figs_of(f))]
    return sorted(files, key=lambda f: (sorted(figs_of(f))[0] if figs_of(f) else "99", f))


def figs_of(script: str) -> tuple[str, ...]:
    """脚本对应的图件编号（两位字符串）；未登记时按文件名中的数字兜底。"""
    hit = SCRIPT_FIGS.get(script)
    if hit:
        return hit
    base = os.path.splitext(script)[0]
    for part in base.split("_"):
        if part.isdigit():
            return (part.zfill(2),)
    return ()


def run_step(label: str, argv: list[str], env: dict, cwd: str) -> dict:
    """执行一个子步骤，返回结果字典（不抛异常）。"""
    print("\n" + "-" * 96)
    print(">>> %s" % label)
    print("-" * 96)
    t0 = time.perf_counter()
    try:
        proc = subprocess.run(argv, cwd=cwd, env=env, capture_output=True, text=True,
                              encoding="utf-8", errors="replace")
        code = proc.returncode
        out = (proc.stdout or "") + (proc.stderr or "")
    except OSError as exc:
        code = 127
        out = "启动失败: %s" % exc
    dt = time.perf_counter() - t0
    if out.strip():
        print(out.rstrip())
    print("[%s] 退出码=%d  耗时=%.2f s" % (label, code, dt))
    return {"label": label, "code": code, "seconds": dt, "output": out}


def check_artifacts(script: str, fdir: str, only: set[str] | None = None) -> tuple[list[str], list[str]]:
    """返回 (存在的产物, 缺失的产物)；`only` 给定时只校验被选中的图件。"""
    want = EXPECTED.get(script)
    if not want:
        return [], []
    if only:
        want = tuple(f for f in want
                     if any(f.split("_")[0].endswith(n) for n in only))
    present = [f for f in want if os.path.isfile(os.path.join(fdir, f))]
    missing = [f for f in want if f not in present]
    return present, missing


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description="EmbeddedNS 测试报告出图与 PPT 生成流水线")
    ap.add_argument("--skip-deck", action="store_true", help="跳过 build_deck.py / check_deck.py")
    ap.add_argument("--skip-check", action="store_true", help="只跳过 check_deck.py")
    ap.add_argument("--only", default="", help="只运行指定图号，逗号分隔，例如 05,07,22")
    ap.add_argument("--list", action="store_true", help="列出将执行的脚本与预期产物后退出")
    args = ap.parse_args(argv)

    only = {s.strip().zfill(2) for s in args.only.split(",") if s.strip()} or None
    figs = discover_fig_scripts(only)
    if args.only and not figs:
        print("[run_all] 警告: --only %s 未匹配任何脚本。可用图号: %s"
              % (args.only, ", ".join(sorted({n for f in SCRIPT_FIGS.values() for n in f}))))
        return 2

    if args.list:
        print("图件输出目录: %s" % K.figures_dir())
        for f in figs:
            print("  %-22s → %s" % (f, ", ".join(EXPECTED.get(f, ("(未登记)",)))))
        if not args.skip_deck:
            print("  %-22s → 测试报告 PPT" % DECK_SCRIPTS[0])
            print("  %-22s → PPT 结构校验" % DECK_SCRIPTS[1])
        return 0

    env = dict(os.environ)
    env["PYTHONIOENCODING"] = "utf-8"
    env["PYTHONUTF8"] = "1"
    py = sys.executable or "python"

    print("=" * 96)
    print("EmbeddedNS 测试报告流水线")
    print("  仓库根      : %s" % K.repo_root())
    print("  数据目录    : %s" % K.data_dir())
    print("  图件输出    : %s" % K.figures_dir())
    print("  Python      : %s" % py)
    print("  绘图脚本    : %d 个" % len(figs))
    print("=" * 96)

    fdir = K.ensure_figures_dir()
    results: list[dict] = []
    failures: list[str] = []
    skipped: list[str] = []

    for script in figs:
        res = run_step(script, [py, script], env, HERE)
        results.append(res)
        present, missing = check_artifacts(script, fdir, only)
        if res["code"] != 0:
            failures.append("%s (退出码 %d)" % (script, res["code"]))
            continue
        if missing:
            if script in OPTIONAL_IF_MISSING:
                skipped.append("%s → 缺 %s（数据缺失，已按 SKIP 处理）" % (script, ", ".join(missing)))
                print("[artifacts] %s 产出 %d/%d；缺失（允许）: %s"
                      % (script, len(present), len(present) + len(missing), ", ".join(missing)))
            else:
                failures.append("%s (退出码 0 但缺少产物: %s)" % (script, ", ".join(missing)))
                print("[artifacts] ✗ %s 缺少产物: %s" % (script, ", ".join(missing)))
        else:
            print("[artifacts] ✓ %s 产物齐全（%d 个）" % (script, len(present)))

    if not args.skip_deck:
        # deck 脚本（T4-II）通过 NSDECK_FIGURES_DIR 定位图件；把它对齐到本次输出目录，
        # 否则重定向出图时 PPT 会读到另一处的图。
        deck_env = dict(env)
        deck_env.setdefault("NSDECK_FIGURES_DIR", K.figures_dir())
        for i, deck in enumerate(DECK_SCRIPTS):
            if i == 1 and args.skip_check:
                continue
            path = os.path.join(HERE, deck)
            if not os.path.isfile(path):
                failures.append("%s (文件不存在)" % deck)
                print("\n[!] %s 不存在于 %s（属于并行任务 T4-II 的产出）" % (deck, HERE))
                continue
            res = run_step(deck, [py, deck], deck_env, HERE)
            results.append(res)
            if res["code"] != 0:
                failures.append("%s (退出码 %d)" % (deck, res["code"]))

    print("\n" + "=" * 96)
    print("执行摘要")
    print("=" * 96)
    for r in results:
        print("  %-24s 退出码=%-3d  %6.2f s" % (r["label"], r["code"], r["seconds"]))
    for s in skipped:
        print("  [SKIP] %s" % s)

    pngs = sorted(f for f in os.listdir(fdir) if f.lower().endswith(".png"))
    print("\n图件目录共 %d 个 PNG: %s" % (len(pngs), fdir))

    if failures:
        print("\n失败项 %d 个：" % len(failures))
        for f in failures:
            print("  ✗ %s" % f)
        print("\n流水线未通过。")
        return 1
    print("\n流水线通过：全部子步骤退出码 0，产物齐全。")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
