# -*- coding: utf-8 -*-
"""
FigT02 — 组件架构与数据流（静态绘制，无数据依赖）

本图**始终可生成**：不读取任何测量数据，只描述仓库内真实存在的组件与路径。
因此即使 `ENC/Test/data/` 为空，图件目录中也会有这一张。

版位：整页图 1800×845
独立运行：`python fig_architecture.py`
"""

from __future__ import annotations

import os
import sys

import common as K
import fig_style as S
from fig_style import C

#: 图中标注的路径全部为仓库内真实路径（相对仓库根）
ROOT = "D:/workshop/Github/EmbeddedNS-Portable"


def _node(cv, x, y, w, h, tag, title, lines, color, tag_fnt=S.FS_LABEL):
    S.node_box(cv, x, y, w, h, title, lines, tag=tag, fill=color,
               fnt_title=S.FS_AXIS, fnt_body=S.FS_LABEL, tag_fnt=tag_fnt)


def fig_t02() -> str:
    cv = S.Canvas("full", "FigT02", "组件架构与数据流（标注真实文件路径）")
    d = cv.d
    left = cv.pad_l
    right = cv.W - cv.pad_r
    top = cv.top0 + 6
    bottom = cv.H - cv.bottom0 - 30

    # ---------------------------------------------------------------- 三层主体
    lane_gap = 62
    lane_h = (bottom - top - lane_gap * 2) / 3.0
    card_w = 330
    card_gap = 34
    cols_x = [left, left + card_w + card_gap, left + 2 * (card_w + card_gap)]

    # --- 层 1：输入与构建 ---
    y = top
    S.caption(cv, left, y - 6, "① 输入与构建（Phase 0 / P0）", color=C.PRIMARY, fnt=S.FS_AXIS)
    y0 = y + 30
    _node(cv, cols_x[0], y0, card_w, lane_h - 30, "P0-1", "测试音频样本",
          ["Test_Audio/AudioSample-48000hz/",
           "1.wav … 9.wav",
           "2ch / 16bit / 48000Hz",
           "合计 190.42 s"], C.TEXT)
    _node(cv, cols_x[1], y0, card_w, lane_h - 30, "P0-2", "核心库构建产物",
          ["Project/ENC_Core/AudioProcess/",
           "  Api/ENC_V.h  ← 公共 API",
           "  ns_core.* / ns_common.h",
           "输出 ENC_Core.dll（Release）"], C.PRIMARY)
    _node(cv, cols_x[2], y0, card_w, lane_h - 30, "P0-3", "构建指纹",
          ["ENC/Test/data/<variant>/",
           "  build_info.txt",
           "variant / patch_flags",
           "sample_depth / c_flags / git_commit"], C.POSITIVE)

    # --- 层 2：测试驱动 ---
    y = y0 + lane_h - 30 + lane_gap
    S.caption(cv, left, y - 6, "② 测试驱动与测量（T1 / T2 / T3）", color=C.PRIMARY, fnt=S.FS_AXIS)
    y1 = y + 30
    _node(cv, cols_x[0], y1, card_w, lane_h - 30, "T1", "驱动与分帧",
          ["ENC/Test/harness/",
           "固定 480 样本/帧（10 ms）",
           "floatS16 量纲 ±32768",
           "末帧补零后裁剪回原长"], C.PRIMARY)
    _node(cv, cols_x[1], y1, card_w, lane_h - 30, "T2", "逐帧指标记录",
          ["metrics_<name>.csv",
           "12 列，顺序固定（计划书 6.2）",
           "atten_db = 20log10(in/out)",
           "out_rms=0 → 999.0 哨兵"], C.PRIMARY)
    _node(cv, cols_x[2], y1, card_w, lane_h - 30, "T3", "汇总与矩阵",
          ["summary_<variant>_<level>.csv",
           "9 文件 × 3 等级 × 4 变体",
           "变体: baseline/d1/d1d5/optimized",
           "等级: L0 / L1 / L3"], C.PRIMARY)

    # --- 层 3：被测组件 ---
    y = y1 + lane_h - 30 + lane_gap
    S.caption(cv, left, y - 6, "③ 被测组件（WebRTC NS 移植）", color=C.PRIMARY, fnt=S.FS_AXIS)
    y2 = y + 30
    _node(cv, cols_x[0], y2, card_w, lane_h - 30, "C1", "NS 主处理",
          ["NS_Process_48kAudio(handler,",
           "                    in, out)",
           "InitNSHandler(level 0–3)",
           "SuppressionParams 按等级装载"], C.OPTIMIZED)
    _node(cv, cols_x[1], y2, card_w, lane_h - 30, "C2", "诊断量采集",
          ["NsDiagnostics",
           "signal_energy / signal_spectral_sum",
           "diff_normalization / speech_prob_mean",
           "prior_speech_prob / gain_adjustment"], C.WARN)
    _node(cv, cols_x[2], y2, card_w, lane_h - 30, "C3", "缺陷补丁开关",
          ["patch_flags  bit0=D1  bit1=D5",
           "            bit2=D6",
           "同一源码树，仅开关不同",
           "以旁证消融链定位 D1 污染"], C.WARN)

    # ---------------------------------------------------------------- 数据流箭头
    arrow_x = cols_x[2] + card_w + 44
    for ya, yb, lab in ((y0 + 20, y1 + 20, "输入 WAV"),
                        (y1 + 20, y2 + 20, "帧缓冲"),
                        (y2 + 20, y2 + (lane_h - 30) + 16, "输出 WAV + CSV")):
        S.arrow(d, [(arrow_x, ya), (arrow_x, yb)], color=C.PRIMARY, lw=3)
    # 主链路：组件层 → 产物层（横向）
    S.arrow(d, [(cols_x[0] + card_w / 2, y2 + lane_h - 30), (cols_x[0] + card_w / 2, bottom + 6)],
            color=C.OPTIMIZED, lw=3)
    S.arrow(d, [(cols_x[1] + card_w / 2, y2 + lane_h - 30),
                (cols_x[1] + card_w / 2, bottom + 6)], color=C.PRIMARY, lw=3)
    S.arrow(d, [(cols_x[2] + card_w / 2, y2 + lane_h - 30),
                (cols_x[2] + card_w / 2, bottom + 6)], color=C.WARN, lw=3)

    # 右侧纵向说明
    note_x = cols_x[2] + card_w + 96
    note_w = right - note_x
    if note_w > 240:
        S.caption(cv, note_x, top - 6, "④ 产物与消费方", color=C.PRIMARY, fnt=S.FS_AXIS)
        _node(cv, note_x, y0 + 30, note_w, lane_h - 30, "OUT-1", "输出音频",
              ["ENC/Test/data/<variant>/<level>/",
               "<name>.wav  1ch/16bit/48000Hz",
               "长度与输入等长"], C.POSITIVE)
        _node(cv, note_x, y1 + 30, note_w, lane_h - 30, "OUT-2", "分析管线（本图所在）",
              ["ENC/Test/analysis/",
               "common.py / fig_style.py",
               "→ ENC/Document/figures/*.png",
               "run_all.py 一键出图"], C.PRIMARY)
        _node(cv, note_x, y2 + 30, note_w, lane_h - 30, "OUT-3", "交付物",
              ["ENC/Document/",
               "  figures/FigT01…FigT22",
               "  build_deck.py → 34 页 PPT",
               "  check_deck.py 结构校验"], C.OPTIMIZED)

    cv.note("图中路径均相对于仓库根 %s；箭头表示数据流向，编号 P0/T*/C*/OUT* 与开发计划书的任务编号一致。"
            % ROOT)
    return cv.save("FigT02_组件架构与数据流.png")


def main() -> int:
    S.warn_if_formal_output("fig_architecture.py")
    print("=" * 84)
    print("[fig_architecture] 静态图，无数据依赖")
    print("[fig_architecture] 输出目录 %s" % K.figures_dir())
    print("=" * 84)
    fig_t02()
    return 0


if __name__ == "__main__":
    sys.exit(main())
