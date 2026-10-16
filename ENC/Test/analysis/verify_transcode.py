#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
verify_transcode.py —— 搬运保真度比对（T2 验收工具）

用途：独立证明「29 个冻结源文件搬运到 ENC/NS_Component/src/ 时，除计划书 5.6 列明的
变更点之外没有任何改动」。这是基线可信度的一部分：若搬运过程改动了算式，基线就不再
代表固件行为。

做法：
  1. 源文件按探测到的编码（UTF-8 → GBK 回退）解码，目标文件按 UTF-8 解码
  2. 统一行尾（CRLF/CR → LF）后逐行比对
  3. 输出每个文件的实际差异（unified diff 摘要）

已知且预期的差异（全部来自计划书 5.6 与 T2 任务书）：
  - 6 个 .h 的厂商 include 块（7 行 → 1 行 vendor_types.h）
  - enc.h 的 6 行固件侧 extern / 函数声明删除
  - noise_suppressor.c：include 替换 + ns_patch.h + D1/D5/D6 插桩 + 诊断插桩
  - noise_suppressor.h：5 个诊断成员新增
  - wiener_filter.c：ns_patch.h + NS_SQRT
  - enc.c → ns_glue.c 属整体改写，不参与逐行比对（单独说明）

用法：
    <python> verify_transcode.py [--show N]     # N = 每个文件最多显示几条差异行（默认 40）
"""

import argparse
import difflib
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
SRC_DIR = os.path.join(REPO_ROOT, "ENC", "MainApp", "source", "driver", "alg", "dsp", "enc")
DST_DIR = os.path.join(REPO_ROOT, "ENC", "NS_Component", "src")

# enc.c 整体改写为 ns_glue.c，不参与逐行比对
FILES = [
    "enc.h",
    "fast_math.c", "fast_math.h",
    "histograms.c", "histograms.h",
    "noise_estimator.c", "noise_estimator.h",
    "noise_suppressor.c", "noise_suppressor.h",
    "ns_common.h",
    "prior_signal_model.c", "prior_signal_model.h",
    "prior_signal_model_estimator.c", "prior_signal_model_estimator.h",
    "quantile_noise_estimator.c", "quantile_noise_estimator.h",
    "signal_model.c", "signal_model.h",
    "signal_model_estimator.c", "signal_model_estimator.h",
    "speech_probability_estimator.c", "speech_probability_estimator.h",
    "suppression_params.c", "suppression_params.h",
    "three_band_filter_bank.c", "three_band_filter_bank.h",
    "wiener_filter.c", "wiener_filter.h",
]

# 预期存在差异的文件（其余应为零差异）
EXPECT_DIFF = {
    "enc.h",
    "noise_suppressor.c", "noise_suppressor.h",
    "wiener_filter.c",
    # fast_math.c: __RAM_CODE 需要 platform.h（计划书 5.6 变更表未列此 include，
    # 属必要编译适配；见执行记录 B-21）
    "fast_math.c",
    "fast_math.h", "three_band_filter_bank.h", "suppression_params.h",
    "prior_signal_model.h", "signal_model.h", "quantile_noise_estimator.h",
}


def decode_source(path):
    """源文件为混合编码（GBK 主体 + 个别 UTF-8 注释行），逐行探测最稳。"""
    raw = open(path, "rb").read()
    lines = []
    for ln in raw.split(b"\n"):
        for enc in ("utf-8", "gbk", "gb18030"):
            try:
                lines.append(ln.decode(enc))
                break
            except UnicodeDecodeError:
                continue
        else:
            lines.append(ln.decode("latin-1"))
    return "\n".join(lines), "mixed/逐行"


def norm_lines(text):
    return text.replace("\r\n", "\n").replace("\r", "\n").split("\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--show", type=int, default=40, help="每个文件最多显示的差异行数")
    args = ap.parse_args()

    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

    print("=" * 76)
    print("搬运保真度比对")
    print("  源: %s" % SRC_DIR)
    print("  目标: %s" % DST_DIR)
    print("=" * 76)

    zero_diff = []
    has_diff = []
    missing = []

    for name in FILES:
        src = os.path.join(SRC_DIR, name)
        dst = os.path.join(DST_DIR, name)
        if not os.path.isfile(src):
            missing.append("源缺失: %s" % name)
            continue
        if not os.path.isfile(dst):
            missing.append("目标缺失: %s" % name)
            continue

        s_text, s_enc = decode_source(src)
        d_text = open(dst, "r", encoding="utf-8").read()
        s_lines = norm_lines(s_text)
        d_lines = norm_lines(d_text)

        if s_lines == d_lines:
            zero_diff.append((name, s_enc, len(s_lines)))
            continue

        sm = difflib.SequenceMatcher(None, s_lines, d_lines, autojunk=False)
        ops = [op for op in sm.get_opcodes() if op[0] != "equal"]
        n_changed = sum(max(o[2] - o[1], o[4] - o[3]) for o in ops)
        has_diff.append((name, s_enc, len(s_lines), len(d_lines), len(ops), n_changed, ops,
                         s_lines, d_lines))

    # ---- 零差异文件
    print("\n[零差异] %d 个文件 —— 逐字节等价（除行尾归一化）" % len(zero_diff))
    for name, enc, n in zero_diff:
        print("  [OK] %-38s 编码=%-8s %d 行" % (name, enc, n))

    # ---- 有差异文件
    print("\n[有差异] %d 个文件" % len(has_diff))
    unexpected = []
    for name, enc, ns, nd, nops, nch, ops, s_lines, d_lines in has_diff:
        flag = "预期" if name in EXPECT_DIFF else "**未预期**"
        print("\n" + "-" * 76)
        print("  %s   [%s]   源 %d 行 → 目标 %d 行, %d 处变更, 共 %d 行受影响"
              % (name, flag, ns, nd, nops, nch))
        if name not in EXPECT_DIFF:
            unexpected.append(name)
        shown = 0
        for tag, i1, i2, j1, j2 in ops:
            if shown >= args.show:
                print("    ... (更多差异已省略)")
                break
            if tag == "replace":
                for k in range(max(i2 - i1, j2 - j1)):
                    a = s_lines[i1 + k] if i1 + k < i2 else ""
                    b = d_lines[j1 + k] if j1 + k < j2 else ""
                    print("    源 L%-5d| %s" % (i1 + k + 1, a[:110]))
                    print("    新 L%-5d| %s" % (j1 + k + 1, b[:110]))
                    shown += 1
                    if shown >= args.show:
                        break
            elif tag == "delete":
                for k in range(i1, i2):
                    print("    删 L%-5d| %s" % (k + 1, s_lines[k][:110]))
                    shown += 1
                    if shown >= args.show:
                        break
            elif tag == "insert":
                for k in range(j1, j2):
                    print("    增 L%-5d| %s" % (k + 1, d_lines[k][:110]))
                    shown += 1
                    if shown >= args.show:
                        break

    # ---- 汇总
    print("\n" + "=" * 76)
    print("汇总：零差异 %d / 有差异 %d / 缺失 %d" % (len(zero_diff), len(has_diff), len(missing)))
    for m in missing:
        print("  [缺失] %s" % m)
    if unexpected:
        print("  [未预期差异] " + ", ".join(unexpected))
    print("说明：enc.c 整体改写为 ns_glue.c，不参与本比对（见 README 摘取对照表）")
    verdict = "PASS" if (not missing and not unexpected) else "REVIEW"
    print("结论：%s" % verdict)
    print("=" * 76)
    return 0 if verdict == "PASS" else 1


if __name__ == "__main__":
    sys.exit(main())
