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


def _node(cv, x, y, w, h, tag, title, lines, color, tag_fnt=S.FS_MIN):
    S.node_box(cv, x, y, w, h, title, lines, tag=tag, fill=color,
               fnt_title=S.FS_MIN, fnt_body=S.FS_MIN, tag_fnt=tag_fnt)


def fig_t02() -> str:
    cv = S.Canvas("full", "FigT02", "组件架构与数据流（标注真实文件路径）")
    d = cv.d
    left = cv.pad_l
    right = cv.W - cv.pad_r
    cv.set_bottom_reserve(56)

    # 垂直预算（根因 A）：旧版按 `lane_h = (bottom-top-2*gap)/3` 手算行高，没把
    # 「层标题 + 卡片内 tag/标题行」算进去，卡片正文只剩 1 行可用，于是路径被
    # `_clip_lines` 砍成 `Test_Audio/AudioSa…`。现在用 `stack_rows` 把层标题
    # 计入预算，卡片高度按「tag 行 36 + 标题行 30 + 3 行正文 ×30」= 156 px 反推，
    # 并用 24 px 字号保证每条路径都能整行落地、不折行、不截断。
    card_w = 330
    card_gap = 30
    cols_x = [left, left + card_w + card_gap, left + 2 * (card_w + card_gap)]
    # 实测（full 版位 top0=100）：`stack_rows(3, top=106, bottom=749, gap=8, caption_h=30)`
    # 每层 179 px，扣掉「tag 行 36 + 标题行 30 + 上下留白 12」后正文可用 95 px → 3 行。
    lanes = cv.stack_rows(3, top=cv.top0 + 6, bottom=cv.H - cv.bottom0 - 40,
                          gap=8, caption_h=30)
    lane_caps = ("① 输入与构建（Phase 0 / P0）",
                 "② 测试驱动与测量（T1 / T2 / T3）",
                 "③ 被测组件（WebRTC NS 移植）")

    # --- 层 1：输入与构建 ---
    rect, cap_y = lanes[0]
    y0, lane_h = rect[1], rect[3] - rect[1]
    S.caption(cv, left, cap_y, lane_caps[0], color=C.PRIMARY, fnt=S.FS_MIN)
    _node(cv, cols_x[0], y0, card_w, lane_h, "P0-1", "测试音频样本",
          ["Test_Audio/1.wav–9.wav",
           "2ch / 16bit / 48000Hz",
           "合计 190.42 s"], C.TEXT)
    _node(cv, cols_x[1], y0, card_w, lane_h, "P0-2", "核心库构建产物",
          ["Project/ENC_Core/",
           "Api/ENC_V.h ← 公共 API",
           "→ 输出 ENC_Core.dll"], C.PRIMARY)
    _node(cv, cols_x[2], y0, card_w, lane_h, "P0-3", "构建指纹",
          ["ENC/Test/data/",
           "build_info.txt（构建指纹）",
           "variant / patch_flags"], C.POSITIVE)

    # --- 层 2：测试驱动 ---
    rect, cap_y = lanes[1]
    y1, lane_h = rect[1], rect[3] - rect[1]
    S.caption(cv, left, cap_y, lane_caps[1], color=C.PRIMARY, fnt=S.FS_MIN)
    _node(cv, cols_x[0], y1, card_w, lane_h, "T1", "驱动与分帧",
          ["ENC/Test/harness/",
           "480 样本/帧（10 ms）",
           "floatS16 量纲 ±32768"], C.PRIMARY)
    _node(cv, cols_x[1], y1, card_w, lane_h, "T2", "逐帧指标记录",
          ["metrics_<name>.csv",
           "12 列顺序固定",
           "atten_db=20log10(in/out)"], C.PRIMARY)
    _node(cv, cols_x[2], y1, card_w, lane_h, "T3", "汇总与矩阵",
          ["summary_<var>_<lvl>.csv",
           "9 文件 × 3 等级 × 4 变体",
           "变体 baseline/d1/d1d5"], C.PRIMARY)

    # --- 层 3：被测组件 ---
    rect, cap_y = lanes[2]
    y2, lane_h = rect[1], rect[3] - rect[1]
    S.caption(cv, left, cap_y, lane_caps[2], color=C.PRIMARY, fnt=S.FS_MIN)
    _node(cv, cols_x[0], y2, card_w, lane_h, "C1", "NS 主处理",
          ["NS_Process_48kAudio()",
           "InitNSHandler(0–3)",
           "SuppressionParams 装载"], C.OPTIMIZED)
    _node(cv, cols_x[1], y2, card_w, lane_h, "C2", "诊断量采集",
          ["NsDiagnostics 诊断量",
           "signal_energy / diff_norm",
           "speech_prob / gain_adjust"], C.WARN)
    _node(cv, cols_x[2], y2, card_w, lane_h, "C3", "缺陷补丁开关",
          ["patch_flags：bit0=D1",
           "bit1=D5  bit2=D6",
           "同一源码树，仅开关不同"], C.WARN)

    # ---------------------------------------------------------------- 数据流箭头
    arrow_x = cols_x[2] + card_w + 44
    for ya, yb in ((y0 + 24, y1 - 20), (y1 + 24, y2 - 20)):
        S.arrow(d, [(arrow_x, ya), (arrow_x, yb)], color=C.PRIMARY, lw=3)
    # 组件层 → 产物层（三条竖箭头，起点在卡片下沿之内、终点止于层标题之上）
    arrow_end = lanes[2][0][3] + 26
    for i, colr in enumerate((C.OPTIMIZED, C.PRIMARY, C.WARN)):
        S.arrow(d, [(cols_x[i] + card_w / 2, y2 + lane_h), (cols_x[i] + card_w / 2, arrow_end)],
                color=colr, lw=3)

    # 右侧纵向说明
    note_x = cols_x[2] + card_w + 40
    note_w = right - note_x
    if note_w > 240:
        S.caption(cv, note_x, cap_y, "④ 产物与消费方", color=C.PRIMARY, fnt=S.FS_MIN)
        _node(cv, note_x, y0, note_w, lane_h, "OUT-1", "输出音频",
              ["ENC/Test/data/",
               "<variant>/<level>/",
               "<name>.wav 1ch/48000Hz"], C.POSITIVE)
        _node(cv, note_x, y1, note_w, lane_h, "OUT-2", "分析管线（本图所在）",
              ["ENC/Test/analysis/",
               "common / fig_style",
               "→ Document/figures/"], C.PRIMARY)
        _node(cv, note_x, y2, note_w, lane_h, "OUT-3", "交付物",
              ["ENC/Document/figures/",
               "FigT01–FigT23 全部图件",
               "build_deck → 34 页 PPT"], C.OPTIMIZED)
    else:
        print("[fig_architecture] 右侧说明列宽仅 %.0f px，已略去 OUT-1..3（见底部注释）" % note_w)

    cv.note("图中路径相对于仓库根；箭头=数据流向，编号同开发计划书任务编号。")
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
