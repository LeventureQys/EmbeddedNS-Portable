# -*- coding: utf-8 -*-
"""
FigT20 — 与 `Project/ENC_Core` 参照路径的形态对比

数据源（T6 交叉参照，命名与主流程完全一致，仅把 `<variant>` 换成字面量 `reference`）
-----------------------------------------------------------------------------
`<DATA_DIR>/reference/L3/1.wav`                    16bit/1ch/48000Hz
`<DATA_DIR>/reference/L3/metrics_1.csv`            列定义同计划书 6.2
`<DATA_DIR>/reference/L3/summary_reference_L3.csv` 列定义同计划书 6.3
`<DATA_DIR>/reference/build_info.txt`

`common.py` 把 `variant='reference'` 当普通变体处理，路径拼接无需特殊分支。

内容（计划书 T6：**仅做形态对比，不做逐点相减**）
------------------------------------------------
* 主图：逐帧 `atten_db` 轨迹——主路径 `baseline`(L3) 与参照 `reference`(L3)，横轴帧序号
* 副图：两条曲线的整段 `atten_db` 分布（同分箱直方图），比较衰减量分布形态
* 显式标注量纲差异：主路径 24bit 量纲（±8388608）／参照 floatS16 量纲（±32768）；
  `atten_db` 为无量纲比值，故可同图对比
* **禁止**逐点相减、逐点误差统计、逐点相关系数

数据缺失（`data/reference/` 不存在或为空）→ 打印 `SKIP <原因>` 并以退出码 0 结束。

版位：整页图 1800×845
独立运行：`python fig_reference.py`
"""

from __future__ import annotations

import os
import sys

import numpy as np

import common as K
import fig_style as S
from fig_style import C

LEVEL = K.PRIMARY_LEVEL
NAME = "1"
REF = "reference"


def _load(variant: str, name: str = NAME):
    """返回 (frame_idx, atten_db 有效值)；不可用返回 None。"""
    if not K.exists_metrics(variant, LEVEL, name):
        return None
    try:
        m = K.read_metrics(variant, LEVEL, name)
    except (OSError, ValueError) as exc:
        print("  读取 %s 失败: %s" % (K.metrics_path(variant, LEVEL, name), exc))
        return None
    att = np.asarray(m["atten_db"], dtype=np.float64)
    valid = np.where(np.abs(att - K.ATTEN_DB_SENTINEL) < 1e-6, np.nan, att)
    return m["frame_idx"], valid


def _hist_overlay(ax, series, bins, ylabel, xlabel):
    """同分箱叠加直方图：第一个用填充块，其余用粗轮廓。"""
    counts = []
    for label, color, vals, filled in series:
        v = np.asarray(vals, dtype=np.float64)
        v = v[np.isfinite(v)]
        c, _ = np.histogram(v, bins=bins) if v.size else (np.zeros(len(bins) - 1), bins)
        counts.append(c)
        ax.bars(c, centers=(bins[:-1] + bins[1:]) / 2.0, width=(bins[1] - bins[0]) * 0.94,
                color=color, label=label, outline_only=not filled, show_value=False)
    return counts


def fig_t20() -> str:
    ref_dir = os.path.join(K.data_dir(), REF)
    if not os.path.isdir(ref_dir):
        print("SKIP FigT20 参照目录不存在: %s（T6 未执行或参照产物未生成）" % ref_dir)
        return ""
    ref = _load(REF)
    base = _load(K.PRIMARY_VARIANT)
    if ref is None and base is None:
        print("SKIP FigT20 参照与主路径的 metrics_%s.csv 均不存在（目录 %s）" % (NAME, ref_dir))
        return ""
    if ref is None:
        print("SKIP FigT20 参照 metrics 不存在: %s" % K.metrics_path(REF, LEVEL, NAME))
        return ""

    cv = S.Canvas("full", "FigT20", "与 Project/ENC_Core 参照路径的形态对比（形态级，非逐点）")
    rects = S.Axes.grid_rects((cv.pad_l, cv.top0 - 4, cv.W - cv.pad_r, cv.H - cv.bottom0 - 10),
                              nrow=2, ncol=2, gap_x=140, gap_y=86)

    # ---- 主图：逐帧衰减轨迹（占上半两格 + 下半左格 => 用独立大面板重排）----
    main_rect = (cv.pad_l, cv.top0 - 4, cv.W - cv.pad_r, cv.top0 + 300)
    band = cv.set_legend_band(2)
    main_rect = (main_rect[0], max(main_rect[1], band), main_rect[2], main_rect[3])
    ax = cv.panel(rect=main_rect, ylabel="衰减量 (dB)", xlabel="帧序号（10 ms/帧）")

    series = []
    if base is not None:
        series.append(("主路径 baseline（24bit 量纲）", C.BASELINE, base[0], base[1]))
    series.append(("参照 reference（floatS16 量纲）", C.POSITIVE, ref[0], ref[1]))
    for label, color, xs, ys in series:
        ax.line(xs, ys, color=color, lw=3, label=label)
        fin = ys[np.isfinite(ys)]
        if fin.size:
            print("  %-34s 帧=%d  中位 %.2f dB  均值 %.2f dB  P5 %.2f  P95 %.2f"
                  % (label, fin.size, float(np.median(fin)), float(fin.mean()),
                     float(np.percentile(fin, 5)), float(np.percentile(fin, 95))))
    ax.autoscale()
    ax.grid(horizontal=True, vertical=True)
    ax.axes_line()
    ax.frame()
    ax.legend("tl")

    # ---- 副图：两条曲线的 atten_db 分布（同分箱直方图）----
    allv = np.concatenate([ys[np.isfinite(ys)] for _, _, _, ys in series])
    lo, hi = float(np.percentile(allv, 1)), float(np.percentile(allv, 99))
    if hi - lo < 1.0:
        hi = lo + 1.0
    pad = (hi - lo) * 0.08
    bins = np.linspace(lo - pad, hi + pad, 25)

    dist_rects = S.Axes.grid_rects((cv.pad_l, main_rect[3] + 96, cv.W - cv.pad_r,
                                    cv.H - cv.bottom0),
                                   nrow=1, ncol=2, gap_x=150)
    panels = []
    if base is not None:
        panels.append(("主路径 baseline 分布", C.BASELINE, base[1]))
    panels.append(("参照 reference 分布", C.POSITIVE, ref[1]))
    for i, (title, color, vals) in enumerate(panels):
        axs = cv.panel(rect=dist_rects[i], ylabel="帧数 (个)",
                       xlabel="逐帧衰减量 (dB)" if i == len(panels) - 1 else "")
        axs.set_xlim(bins[0], bins[-1])
        _hist_overlay(axs, [(title, color, vals, True)], bins, "帧数 (个)", "")
        axs.autoscale()
        axs.set_xlim(bins[0], bins[-1])
        axs.grid(horizontal=True, vertical=False)
        axs.axes_line()
        axs.frame()
        axs.legend("tr", max_w=axs.width * 0.8)
        fin = vals[np.isfinite(vals)]
        if fin.size:
            axs.vline(float(np.median(fin)), color=C.WARN, lw=2, dash=False,
                      label="中位 %.2f dB" % float(np.median(fin)))

    info = K.read_build_info(REF)
    if info:
        print("  参照 build_info: %s" % ", ".join("%s=%s" % (k, v) for k, v in list(info.items())[:4]))
    cv.note("量纲：主路径 24bit（±8388608）、参照 floatS16（±32768）；atten_db 无量纲故可同图对比；仅形态对比。")
    return cv.save("FigT20_参照形态对比.png")


def main() -> int:
    S.warn_if_formal_output("fig_reference.py")
    print("=" * 84)
    print("[fig_reference] 参照目录 %s" % os.path.join(K.data_dir(), REF))
    print("[fig_reference] 输出目录 %s" % K.figures_dir())
    print("=" * 84)
    fig_t20()
    return 0


if __name__ == "__main__":
    sys.exit(main())
