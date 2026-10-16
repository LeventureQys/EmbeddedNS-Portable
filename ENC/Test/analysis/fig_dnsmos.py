# -*- coding: utf-8 -*-
"""
FigT23 — DNS-MOS 语音质量对比（报告重点页）

数据源（**不硬编码任何数值**）
----------------------------
`<DATA_DIR>/dnsmos/dnsmos_summary.csv`  逐变体汇总：SIG / BAK / OVRL / P808_MOS
`<DATA_DIR>/dnsmos/dnsmos_detail.csv`   逐文件明细：用于在柱上叠加 9 文件的分布散点

变体含义
--------
`input`     **完全不处理**的原始含噪输入 —— 报告的"不处理"基准线
`baseline`  本项目基线（无修复）
`d1`/`d1d5`/`d1d5d6`  消融中间态
`optimized` 本项目最终优化版
`reference` 另一条移植路径（`Project/ENC_Core`），作对照

本图要突出的三处结论（自动计算，不写死数值）
--------------------------------------------
1. `baseline` 的 OVRL 是否**低于** `input` → 负收益
2. `optimized` 的 OVRL 是否**高于** `input` → 正收益
3. `optimized` 与 `reference` 的 OVRL 差距是否在 0.02 以内 → 与另一条路径持平

版位：整页图 1800×845
独立运行：`python fig_dnsmos.py`
"""

from __future__ import annotations

import csv
import os
import sys

import numpy as np

import common as K
import fig_style as S
from fig_style import C

LEVEL = K.PRIMARY_LEVEL

#: 展示顺序与短标签（完整含义写在图注，避免 x 轴标签过长互相顶撞）
ORDER = ("input", "baseline", "d1", "d1d5", "d1d5d6", "optimized", "reference")
LABEL = {
    "input": "原始输入",
    "baseline": "基线",
    "d1": "+D1",
    "d1d5": "+D1D5",
    "d1d5d6": "+D1D5D6",
    "optimized": "优化版",
    "reference": "参照",
}
#: 配色：input 中性、baseline 基线灰、optimized 优化红、reference 正向绿，其余浅色
COLOR = {
    "input": "#5b6b7a",
    "baseline": C.BASELINE,
    "d1": "#a9b6c0",
    "d1d5": "#bfc9d1",
    "d1d5d6": "#d3dae0",
    "optimized": C.OPTIMIZED,
    "reference": C.POSITIVE,
}

METRICS = (
    ("OVRL", "整体质量 OVRL", (0.42, 0.60)),
    ("SIG", "语音失真 SIG", (0.40, 0.56)),
    ("BAK", "背景噪声 BAK", (0.40, 0.56)),
    # y 轴标题取 `nice.split()[0]`：竖排可用高度受子图高度限制，
    # 「P808-MOS」（8 字符 ≈ 150 px）在 175 px 高的子图里放不下，会退化成
    # 横排贴轴内左上角、把柱顶数值标签盖住（FigT23 右下子图）。故取短名。
    ("P808_MOS", "P808 MOS", (0.40, 0.56)),
)


def data_dir() -> str:
    return os.path.join(K.data_dir(), "dnsmos")


def read_summary() -> dict[str, dict[str, float]]:
    path = os.path.join(data_dir(), "dnsmos_summary.csv")
    if not os.path.isfile(path):
        return {}
    out: dict[str, dict[str, float]] = {}
    with open(path, "r", newline="", encoding="utf-8-sig") as fh:
        for row in csv.DictReader(fh):
            v = (row.get("variant") or "").strip()
            if not v:
                continue
            out[v] = {}
            for k, val in row.items():
                if k in ("variant", ""):
                    continue
                try:
                    out[v][k] = float(val)
                except (TypeError, ValueError):
                    pass
    return out


def read_detail() -> dict[str, dict[str, list[float]]]:
    """`{变体: {指标: [逐文件值]}}`。"""
    path = os.path.join(data_dir(), "dnsmos_detail.csv")
    if not os.path.isfile(path):
        return {}
    out: dict[str, dict[str, list[float]]] = {}
    cols = ("SIG", "BAK", "OVRL", "P808_MOS")
    with open(path, "r", newline="", encoding="utf-8-sig") as fh:
        for row in csv.DictReader(fh):
            v = (row.get("variant") or "").strip()
            if not v:
                continue
            d = out.setdefault(v, {c: [] for c in cols})
            for c in cols:
                try:
                    d[c].append(float(row[c]))
                except (TypeError, ValueError):
                    pass
    return out


def fig_t23() -> str:
    summ = read_summary()
    if not summ:
        print("SKIP FigT23 缺少 %s" % os.path.join(data_dir(), "dnsmos_summary.csv"))
        return ""
    detail = read_detail()
    variants = [v for v in ORDER if v in summ]
    if not variants:
        print("SKIP FigT23 summary 中无可用变体（含 %s）" % ", ".join(ORDER))
        return ""

    cv = S.Canvas("full", "FigT23", "DNS-MOS 语音质量对比（9 文件 · 7 变体）")
    # 版式重排（根因 A）：2×2 子图原先只用 `grid_rects` 均分高度，于是
    #   ① 每行的 x 类目标签（7 个变体名）在 628 px 宽的单元格里必然粘连；
    #   ② 结论注释只能塞进轴区，压住柱顶数据标签。
    # 现在显式分配垂直预算：每行 = 40 px 注释行（轴区**之外**）+ 轴区；
    # 行距放到 100 px 以容纳两行错排的 x 刻度带；x 类目轴允许两行错排。
    cv.set_bottom_reserve(130)
    stack = cv.stack_rows(2, top=cv.top0 + 6, bottom=cv.H - cv.bottom0 - 26,
                          gap=100, caption_h=40)
    gap_x = 176
    cells = []
    for rect, cap_y in stack:
        cw = (rect[2] - rect[0] - gap_x) / 2.0
        cells.append(((rect[0], rect[1], rect[0] + cw, rect[3]), cap_y))
        cells.append(((rect[2] - cw, rect[1], rect[2], rect[3]), cap_y))
    row_cap = [stack[0][1], stack[1][1]]

    idx = np.arange(len(variants), dtype=np.float64)
    stats: dict[str, dict[str, float]] = {}
    for k, (col, nice, _pad) in enumerate(METRICS):
        vals = [summ.get(v, {}).get(col, np.nan) for v in variants]
        finite = [x for x in vals if np.isfinite(x)]
        if not finite:
            continue
        lo_base, hi_base = min(finite), max(finite)
        # 值域必须覆盖**逐文件散点**：只按均值定标会让散点大面积越出轴区
        # （FigT23 的散点曾上溢压住结论注释行、下溢落进 x 轴类别标签带）。
        all_pts: list[float] = []
        for v in variants:
            all_pts.extend((detail.get(v) or {}).get(col) or [])
        all_pts = [x for x in all_pts if np.isfinite(x)]
        if all_pts:
            lo_base = min(lo_base, min(all_pts))
            hi_base = max(hi_base, max(all_pts))
        span = max(hi_base - lo_base, 1e-3)
        ylo, yhi = lo_base - span * 0.9, hi_base + span * 0.9
        # 保留一位小数刻度更好读
        rect, _cap = cells[k]
        ax = cv.panel(rect=rect, ylabel="%s" % nice.split()[0],
                      xlabel="变体" if k >= 2 else "", ylim=(ylo, yhi),
                      xtick_rows=2)
        ax.set_xlim(-0.7, len(variants) - 0.3)
        for i, v in enumerate(variants):
            val = summ.get(v, {}).get(col, np.nan)
            # 绘制顺序：**散点先、柱体与柱顶数值标签后**，标签才在最上层，
            # 其白底才能真正挡住穿过数字的散点（反之散点会盖住标签）。
            pts = (detail.get(v) or {}).get(col) or []
            if len(pts) >= 2:
                xs = np.full(len(pts), float(i), dtype=np.float64) + np.linspace(-0.14, 0.14, len(pts))
                ax.scatter(xs, pts, color=C.TEXT, r=2, shape="o")
            if not np.isfinite(val):
                continue
            ax.bars([val], centers=np.array([float(i)]), width=0.56,
                    color=COLOR.get(v, C.PRIMARY), show_value=True,
                    value_dy=-6.0, value_stroke=2, value_bg=True,
                    edge=(C.TEXT if v == "reference" else None))
        ax.autoscale()
        # 七个变体名在单个子图宽度内单行必然粘连（曾糊成「+D1D1D5D1D5D优化版」）：
        # 由 `panel(xtick_rows=2)` + `grid(xlabel_rows=2)` 两行错排，同排间距翻倍。
        ax.grid(xlabels=[LABEL.get(v, v) for v in variants], xlabel_rows=2)
        ax.axes_line()
        ax.frame()
        stats[col] = {v: summ.get(v, {}).get(col, float("nan")) for v in variants}
        print("  %-9s " % col + "  ".join("%s=%.4f" % (LABEL.get(v, v), summ.get(v, {}).get(col, float("nan")))
                                          for v in variants))

    # 结论注释画在**每行轴区之上**的专用注释行里（轴区之外），绝不压柱顶数据标签。
    # 只保留结论性数字，语义完整；数值自动计算，不写死。
    def _an(v, c="OVRL"):
        return summ.get(v, {}).get(c, np.nan)

    base_, inp_, opt_, ref_ = _an("baseline"), _an("input"), _an("optimized"), _an("reference")
    if all(np.isfinite(x) for x in (base_, inp_, opt_, ref_)):
        S.caption(cv, cv.pad_l, row_cap[0],
                  "OVRL：基线 %.4f %s不处理 %.4f（%s）｜优化版 %.4f %s不处理（%s）｜与参照 %.4f 差 %.4f（%s）"
                  % (base_, "低于" if base_ < inp_ else "高于", inp_,
                     "负收益" if base_ < inp_ else "正收益",
                     opt_, "高于" if opt_ > inp_ else "低于",
                     "正收益（反超）" if opt_ > inp_ else "负收益",
                     ref_, abs(opt_ - ref_),
                     "基本持平" if abs(opt_ - ref_) <= 0.02 else "有差距"),
                  color=C.OPTIMIZED if base_ < inp_ else C.POSITIVE,
                  fnt=S.FS_MIN, bg=False)
    p_o, p_b = _an("optimized", "P808_MOS"), _an("baseline", "P808_MOS")
    if np.isfinite(p_o) and np.isfinite(p_b):
        S.caption(cv, cv.pad_l, row_cap[1],
                  "P808-MOS 与 OVRL 同向：优化版 %.4f %s 基线 %.4f（%s）"
                  % (p_o, ">" if p_o > p_b else "<", p_b,
                     "结论一致" if p_o > p_b else "方向相反"),
                  color=C.PRIMARY, fnt=S.FS_MIN)

    cv.note("SIG=语音失真 / BAK=背景噪声 / OVRL=整体质量 / P808-MOS=主观质量预测"
            "（No-Reference DNS-MOS，越高越好）。x 轴：原始输入=不处理、基线=无修复、"
            "+D1/+D1D5/+D1D5D6=消融中间态、优化版=最终修复、参照=另一条移植路径。"
            "柱上数字为 9 文件均值，散点为逐文件分布。")
    return cv.save("FigT23_DNSMOS语音质量对比.png")


def main() -> int:
    S.warn_if_formal_output("fig_dnsmos.py")
    print("=" * 84)
    print("[fig_dnsmos] 数据目录 %s" % data_dir())
    print("[fig_dnsmos] 输出目录 %s" % K.figures_dir())
    print("=" * 84)
    fig_t23()
    return 0


if __name__ == "__main__":
    sys.exit(main())
