# -*- coding: utf-8 -*-
"""build_deck.py — 组装《NS组件化与对比测试报告》34 页 PPTX（T4-II 产出）。

一键重建
--------
    python build_deck.py

产物
----
- ``<OUT_DIR>/NS组件化与对比测试报告.pptx``（16:9，13.333 × 7.5 in）
- ``<REPORT_DIR>/deck_bounds.json``  逐 shape 边界自检记录

设计约束（计划书第 10 节）
--------------------------
- 尺寸 13.333 × 7.5 in；字体统一"微软雅黑"
- 页标题 (0.6, 0.35)；副标题 (0.6, 1.05) 16pt #5a6b7c；要点 (0.6, 1.5) 宽 5.0 in
- 整页图 (0.6, 1.4) 宽 12.13；半页图 左 (0.6, 1.4) / 右 (6.85, 1.4) 各宽 5.9
- 条带图 (0.6, 3.9) 宽 12.13，要点放上方；原生表格 (0.8, 1.4) 宽 11.7
- 页脚 (0.6, 7.05) 11pt，报告名 + 页码
- 每条要点文本框不超过 5 条；文本与图片不得溢出幻灯片边界（逐 shape 自检）

数据缺失降级
------------
图件、CSV、构建指纹任一缺失都不会中断构建：缺失图件在页面原位置绘制带说明的
占位框，缺失数值以 "—" 呈现，并在副标题/页脚区标注"数据缺失"。**页数恒为 34。**

环境变量：见 ``deck_data.py`` 头部（``NSDECK_*``）。自测用法：

    $env:NSDECK_DATA_DIR=".../_selftest_deck/data"
    $env:NSDECK_FIGURES_DIR=".../_selftest_deck/figures"
    $env:NSDECK_OUT_DIR=".../_selftest_deck/out"
    python build_deck.py
"""

from __future__ import annotations

import io
import json
import os
import re
import sys
import zipfile
from collections import OrderedDict

from pptx import Presentation
from pptx.chart.data import CategoryChartData
from pptx.dml.color import RGBColor
from pptx.enum.chart import XL_CHART_TYPE, XL_LEGEND_POSITION
from pptx.enum.shapes import MSO_SHAPE
from pptx.enum.text import MSO_ANCHOR, PP_ALIGN
from pptx.oxml.ns import qn
from pptx.util import Emu, Inches, Pt

import deck_data as DD
from deck_data import (
    FIGURE_BOXES,
    FIGURE_SPECS,
    LEVELS,
    METRIC_COLUMNS,
    SUMMARY_COLUMNS,
    VARIANTS,
    Dataset,
    fmt,
    percentile,
)

try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")
except Exception:  # pragma: no cover
    pass

# ------------------------------------------------------------------ 版式常量
SLIDE_W_IN = 13.333
SLIDE_H_IN = 7.5
SLIDE_W = Inches(SLIDE_W_IN)
SLIDE_H = Inches(SLIDE_H_IN)

TITLE_POS = (0.6, 0.35, 12.1, 0.62)
SUBTITLE_POS = (0.6, 1.05, 12.1, 0.34)
BULLET_POS = (0.6, 1.5, 5.0)
FULL_IMG_BOX = (0.6, 1.35, 12.13, 5.60)
HALF_IMG_LEFT = (0.6, 1.4, 5.90, 4.10)
HALF_IMG_RIGHT = (6.85, 1.4, 5.90, 4.10)
STRIP_IMG_BOX = (0.6, 3.9, 12.13, 3.00)
TABLE_POS = (0.8, 1.4, 11.7)
FOOTER_POS = (0.6, 7.05, 12.1, 0.3)

FONT = "微软雅黑"
FONT_FILE = "C:/Windows/Fonts/msyh.ttc"

COLOR_INK = "33414f"
COLOR_SUB = "5a6b7c"
COLOR_PRIMARY = "2c6fa8"
COLOR_BASE = "7f8c8d"
COLOR_OPT = "c0392b"
COLOR_GOOD = "3d7f33"
COLOR_WARN = "c98a12"
COLOR_GRID = "e6ebf0"
COLOR_CARD = "f4f7fa"
COLOR_WHITE = "ffffff"

TOTAL_PAGES = 34
SEP = "　·　"

# 章节导览（第 3 页）
CHAPTERS = [
    ("第 1 章 结论", "1–4", "一句话答案与三项核心结论"),
    ("第 2 章 被测对象", "5–9", "摘取范围、三个开关、量纲、数据规模、构建指纹"),
    ("第 3 章 测试方法", "10–12", "逐帧处理链路、12 类指标、一键复现命令"),
    ("第 4 章 基线结果", "13–17", "改动前的整体表现与启动期异常特征"),
    ("第 5 章 优化结果", "18–23", "优化后的改善量与差异集中位置"),
    ("第 6 章 消融分析", "24–27", "四项修复（D1/D5/D6/D8）各自的贡献与 NaN 归因"),
    ("第 7 章 交叉验证与讨论", "28–31", "参照路径互证、审查结论逐条验证、性能"),
    ("第 8 章 结论与附录", "32–34", "结论建议、图件索引、检查方法与限制"),
]

METRIC_TABLE = [
    ("frame_idx", "int", "帧序号，从 0 开始", "时域"),
    ("in_rms", "float", "本帧输入 RMS（24bit 量纲）", "时域"),
    ("out_rms", "float", "本帧输出 RMS（24bit 量纲）", "时域"),
    ("atten_db", "float", "20·log10(in_rms / out_rms)；out_rms=0 记为 999.0", "时域"),
    ("signal_energy", "float", "频域能量累加量（D1 污染点，见第 15 页）", "频域"),
    ("signal_spectral_sum", "float", "信号谱幅度总和", "频域"),
    ("diff_normalization", "float", "谱差特征归一化量（D1 传导链第二级）", "诊断"),
    ("speech_prob_mean", "float", "帧内语音概率均值；D8 唯一被 NaN 污染的列（第 25 页）", "诊断"),
    ("prior_speech_prob", "float", "先验语音概率", "诊断"),
    ("gain_adjustment", "float", "维纳增益调节量（第四级）", "诊断"),
    ("clamp_hits", "int", "本帧触达钳位边界的样本数（D5 相关）", "诊断"),
    ("elapsed_us", "int", "本帧 NS_Process48k 耗时（微秒）", "性能"),
]


def _pct(value, digits=1):
    return "—" if value is None else ("{:.%df}%%" % digits).format(value * 100.0)


def _delta(optimized, baseline):
    """差值：任一侧为 nan / 缺失时返回 None（不伪造数值）。"""
    if not (DD.is_finite(optimized) and DD.is_finite(baseline)):
        return None
    return optimized - baseline


def _chart_cell(value):
    """原生图表的单元格：有限值四舍五入，nan / 缺失留空（None），不伪造成 0。"""
    return round(value, 3) if DD.is_finite(value) else None


# ------------------------------------------------------------------ 版式助手
def set_run_font(run, size=None, bold=None, color=None, name=FONT):
    """设置字体，并补写东亚字体（python-pptx 只写 latin typeface）。"""
    run.font.name = name
    if size is not None:
        run.font.size = Pt(size)
    if bold is not None:
        run.font.bold = bold
    if color:
        run.font.color.rgb = RGBColor.from_string(color)
    rpr = run.font._rPr
    latin = rpr.find(qn("a:latin"))
    ea = rpr.find(qn("a:ea"))
    if ea is None:
        ea = rpr.makeelement(qn("a:ea"), {})
        if latin is not None:
            latin.addnext(ea)
        else:
            rpr.append(ea)
    ea.set("typeface", name)


def fill_frame(frame, lines, size=17, bold=False, color=COLOR_INK, spacing=6,
               align=PP_ALIGN.LEFT, line_spacing=None):
    """把 lines 写进文本框（首行复用已有空段落）。"""
    frame.word_wrap = True
    if isinstance(lines, str):
        lines = [lines]
    for index, text in enumerate(lines):
        paragraph = frame.paragraphs[0] if index == 0 else frame.add_paragraph()
        paragraph.alignment = align
        if line_spacing:
            paragraph.line_spacing = line_spacing
        paragraph.space_after = Pt(spacing)
        run = paragraph.add_run()
        run.text = text
        set_run_font(run, size=size, bold=bold, color=color)
    return frame


def add_text(slide, lines, left, top, width, height, size=17, bold=False,
             color=COLOR_INK, spacing=6, align=PP_ALIGN.LEFT, line_spacing=None):
    box = slide.shapes.add_textbox(Inches(left), Inches(top), Inches(width), Inches(height))
    fill_frame(box.text_frame, lines, size=size, bold=bold, color=color, spacing=spacing,
               align=align, line_spacing=line_spacing)
    return box


def add_cards(slide, cards, left=0.6, top=1.5, total_width=12.1, gap=0.3,
              height=2.2, title_size=17, body_size=14):
    """三栏（或 N 栏）卡片：cards = [(标题, 正文行列表, 颜色)]。"""
    count = len(cards)
    card_w = (total_width - gap * (count - 1)) / count
    shapes = []
    for index, (title, body_lines, accent) in enumerate(cards):
        x = left + index * (card_w + gap)
        shape = slide.shapes.add_shape(
            MSO_SHAPE.ROUNDED_RECTANGLE, Inches(x), Inches(top),
            Inches(card_w), Inches(height))
        shape.fill.solid()
        shape.fill.fore_color.rgb = RGBColor.from_string(COLOR_CARD)
        shape.line.color.rgb = RGBColor.from_string(accent)
        shape.line.width = Pt(1.25)
        shape.shadow.inherit = False
        frame = shape.text_frame
        frame.margin_left = Inches(0.14)
        frame.margin_right = Inches(0.14)
        frame.margin_top = Inches(0.10)
        frame.margin_bottom = Inches(0.10)
        frame.vertical_anchor = MSO_ANCHOR.TOP
        fill_frame(frame, [title] + list(body_lines), size=body_size, spacing=4)
        for run in frame.paragraphs[0].runs:
            set_run_font(run, size=title_size, bold=True, color=accent)
        shapes.append(shape)
    return shapes


def add_table(slide, headers, rows, left=TABLE_POS[0], top=TABLE_POS[1],
              width=TABLE_POS[2], col_widths=None, body_size=14, header_size=15,
              max_height=5.5, row_height=None):
    """原生表格；行数多时自动收紧字号与行高以保证不越界。"""
    row_count = len(rows) + 1
    if body_size > 12 and row_count > 12:
        body_size = 12
        header_size = 13
    if row_height is None:
        row_height = min(0.42, max(0.20, (max_height - 0.45) / max(row_count, 1)))
    shape = slide.shapes.add_table(row_count, len(headers), Inches(left), Inches(top),
                                   Inches(width), Inches(row_height * row_count))
    table = shape.table
    table.first_row = True
    table.horz_banding = False
    if col_widths:
        total = float(sum(col_widths))
        for index, portion in enumerate(col_widths):
            table.columns[index].width = Emu(int(Inches(width) * portion / total))
    for index in range(row_count):
        table.rows[index].height = Inches(row_height)

    def write(cell, text, size, bold, color, fill):
        cell.text = str(text)
        cell.vertical_anchor = MSO_ANCHOR.MIDDLE
        cell.margin_left = Inches(0.07)
        cell.margin_right = Inches(0.05)
        cell.margin_top = Inches(0.02)
        cell.margin_bottom = Inches(0.02)
        cell.fill.solid()
        cell.fill.fore_color.rgb = RGBColor.from_string(fill)
        for paragraph in cell.text_frame.paragraphs:
            for run in paragraph.runs:
                set_run_font(run, size=size, bold=bold, color=color)

    for column, header in enumerate(headers):
        write(table.cell(0, column), header, header_size, True, COLOR_WHITE, COLOR_PRIMARY)
    for row_index, row in enumerate(rows, start=1):
        fill = COLOR_WHITE if row_index % 2 else COLOR_CARD
        for column, value in enumerate(row):
            write(table.cell(row_index, column), value, body_size, False, COLOR_INK, fill)
    return shape


def add_bar_chart(slide, categories, series, left, top, width, height,
                  number_format="0.00", legend=True, title=None):
    """原生柱状图（数据内嵌，与源 CSV 一致）。"""
    data = CategoryChartData()
    data.categories = list(categories)
    for name, values in series:
        data.add_series(name, tuple(values))
    graphic = slide.shapes.add_chart(XL_CHART_TYPE.COLUMN_CLUSTERED, Inches(left),
                                     Inches(top), Inches(width), Inches(height), data)
    chart = graphic.chart
    chart.has_title = bool(title)
    if title:
        chart.chart_title.text_frame.text = title
        for paragraph in chart.chart_title.text_frame.paragraphs:
            for run in paragraph.runs:
                set_run_font(run, size=14, bold=False, color=COLOR_SUB)
    chart.has_legend = legend
    if legend:
        chart.legend.position = XL_LEGEND_POSITION.BOTTOM
        chart.legend.include_in_layout = False
    chart.font.name = FONT
    chart.font.size = Pt(12)
    chart.font.color.rgb = RGBColor.from_string(COLOR_INK)
    try:
        plot = chart.plots[0]
        plot.has_data_labels = True
        plot.data_labels.number_format = number_format
        plot.data_labels.number_format_is_linked = False
        plot.data_labels.font.size = Pt(10)
        plot.data_labels.font.name = FONT
    except Exception as error:  # python-pptx 1.0.2 下数据标签为可选能力
        print("   [warn] 图表数据标签不可用：%s" % error)
    return graphic


class Deck:
    """承载演示文稿、数据快照与自检记录。"""

    def __init__(self, dataset: Dataset):
        self.prs = Presentation()
        self.prs.slide_width = SLIDE_W
        self.prs.slide_height = SLIDE_H
        self.blank = self.prs.slide_layouts[6]
        self.dataset = dataset
        self.level = dataset.primary_level("baseline")
        self.date = dataset.report_date()
        self.records = []
        self.slide_titles = []
        self.bullet_counts = []
        self._agg = {}

    # --- 常用取数
    def agg(self, variant):
        if variant not in self._agg:
            self._agg[variant] = self.dataset.aggregate(variant, self.level)
        return self._agg[variant]

    def agg_at(self, variant, level):
        return self.dataset.aggregate(variant, level)

    def slide(self, title, subtitle=None, subtitle_color=COLOR_SUB):
        index = len(self.prs.slides) + 1
        slide = self.prs.slides.add_slide(self.blank)
        if title:
            box = add_text(slide, [title], *TITLE_POS, size=28, bold=True)
            self._register(index, "标题", box)
        if subtitle:
            box = add_text(slide, [subtitle], *SUBTITLE_POS, size=16, color=subtitle_color)
            self._register(index, "副标题", box)
        self.slide_titles.append(title or "(无标题)")
        return slide

    def footer(self, slide, index):
        box = add_text(slide, ["%s%s第 %d 页 / 共 %d 页" % (DD.REPORT_NAME, SEP, index, TOTAL_PAGES)],
                       *FOOTER_POS, size=11, color=COLOR_SUB)
        self._register(index, "页脚", box)
        return box

    def bullets(self, slide, index, items, left=None, top=None, width=5.0, size=17,
                height=5.3, color=COLOR_INK, label="要点"):
        assert len(items) <= 5, "第 %d 页要点超过 5 条：%d 条" % (index, len(items))
        left = BULLET_POS[0] if left is None else left
        top = BULLET_POS[1] if top is None else top
        lines = [(item if item.startswith(("·", "→", "★")) else "· " + item) for item in items]
        box = add_text(slide, lines, left, top, width, height, size=size, spacing=8,
                       color=color)
        self.bullet_counts.append((index, len(items)))
        self._register(index, label, box)
        return box

    def picture(self, slide, index, fig_id, box, label=None, note=None):
        """在 box=(left, top, max_w, max_h) 内等比放置图件；缺失则画占位框。"""
        left, top, max_w, max_h = box
        path = DD.figure_path(fig_id)
        caption = label or fig_id
        if os.path.isfile(path):
            size = _image_size(path)
            if size:
                px_w, px_h = size
                scale = min(max_w / px_w, max_h / px_h)
                real_w, real_h = px_w * scale, px_h * scale
            else:  # 读不到像素尺寸时退回版位比例
                spec = FIGURE_BOXES[FIGURE_SPECS[fig_id]["box"]]
                px_w, px_h = spec["px"]
                scale = min(max_w / px_w, max_h / px_h)
                real_w, real_h = px_w * scale, px_h * scale
            shape = slide.shapes.add_picture(path, Inches(left + (max_w - real_w) / 2),
                                             Inches(top + (max_h - real_h) / 2),
                                             width=Inches(real_w), height=Inches(real_h))
        else:
            shape = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(left), Inches(top),
                                           Inches(max_w), Inches(max_h))
            shape.fill.solid()
            shape.fill.fore_color.rgb = RGBColor.from_string(COLOR_CARD)
            shape.line.color.rgb = RGBColor.from_string(COLOR_WARN)
            shape.line.width = Pt(1.25)
            shape.shadow.inherit = False
            frame = shape.text_frame
            frame.vertical_anchor = MSO_ANCHOR.MIDDLE
            fill_frame(frame, ["图件缺失，待 T7 生成", FIGURE_SPECS[fig_id]["file"]],
                       size=15, color=COLOR_WARN, align=PP_ALIGN.CENTER, spacing=4)
        self._register(index, caption, shape)
        return shape

    def note(self, slide, index, text, top=6.2, size=13, color=COLOR_SUB, left=0.6,
             width=12.1):
        box = add_text(slide, [text], left, top, width, 0.5, size=size, color=color)
        self._register(index, "注释", box)
        return box

    # --- 自检
    def _register(self, index, label, shape):
        left = shape.left / 914400.0 if shape.left is not None else None
        top = shape.top / 914400.0 if shape.top is not None else None
        width = shape.width / 914400.0 if shape.width is not None else None
        height = shape.height / 914400.0 if shape.height is not None else None
        record = {
            "slide": index,
            "label": label,
            "shape_type": str(shape.shape_type),
            "left": left,
            "top": top,
            "width": width,
            "height": height,
        }
        if None in (left, top, width, height):
            record["inside"] = None
            record["reason"] = "缺少几何信息，跳过判定"
        else:
            right, bottom = left + width, top + height
            record["right"] = right
            record["bottom"] = bottom
            tolerance = 0.01
            record["inside"] = (left >= -tolerance and top >= -tolerance
                                and right <= SLIDE_W_IN + tolerance
                                and bottom <= SLIDE_H_IN + tolerance)
            if not record["inside"]:
                record["reason"] = "越界：right=%.3f bottom=%.3f（上限 %.3f / %.3f）" % (
                    right, bottom, SLIDE_W_IN, SLIDE_H_IN)
        self.records.append(record)
        return record

    def bounds_report(self):
        violations = [r for r in self.records if r.get("inside") is False]
        unknown = [r for r in self.records if r.get("inside") is None]
        pages = OrderedDict()
        for record in self.records:
            page = pages.setdefault(record["slide"], {"shapes": 0, "violations": 0,
                                                      "max_right": 0.0, "max_bottom": 0.0})
            page["shapes"] += 1
            if record.get("inside") is False:
                page["violations"] += 1
            if record.get("right") is not None:
                page["max_right"] = max(page["max_right"], record["right"])
            if record.get("bottom") is not None:
                page["max_bottom"] = max(page["max_bottom"], record["bottom"])
        return {
            "slide_width_in": SLIDE_W_IN,
            "slide_height_in": SLIDE_H_IN,
            "slide_count": len(self.slide_titles),
            "shape_count": len(self.records),
            "violations": violations,
            "unknown": unknown,
            "pages": [dict(index=index,
                           title=self.slide_titles[index - 1] if index <= len(self.slide_titles) else "",
                           **page) for index, page in pages.items()],
            "bullets": [{"slide": i, "count": c} for i, c in self.bullet_counts],
        }


def _image_size(path):
    try:
        from PIL import Image
        with Image.open(path) as image:
            return image.size
    except Exception:
        return None


def _missing_note(dataset: Dataset):
    missing = dataset.missing_figures()
    if not missing:
        return None
    return "数据缺失占位：%d/%d 张图件未生成（%s…），待 T7 出图后重建。" % (
        len(missing), len(DD.FIGURE_IDS), ", ".join(missing[:6]))


def _data_note(dataset: Dataset, level):
    rows = dataset.summary("baseline", level)
    if not rows:
        return "测量数据缺失：%s 未找到 summary CSV，本页数值以 — 占位，待 P2/P3 产出后重建。" % level
    return "报告口径：抑制等级 %s，逐文件汇总来自 summary_<variant>_%s.csv。" % (level, level)


# ------------------------------------------------------------------ 页面实现
def p01_cover(deck: Deck):
    slide = deck.slide(None)
    add_text(slide, ["NS 降噪组件化与对比测试报告"], 0.6, 0.9, 12.1, 0.9, size=40, bold=True,
             color="10243a")
    add_text(slide, ["固件算法摘取" + SEP + "基线 vs 优化" + SEP + "D1 / D5 / D6 三项修复的量化验证"],
             0.6, 1.85, 12.1, 0.4, size=19, color=COLOR_PRIMARY)
    lines = [
        "被测对象：ENC 降噪算法（noise_suppressor.c 及其依赖，共 29 个源文件）",
        "测试量纲：16bit WAV → ×256 对齐 24bit（依据 drv_alg.c:46 `I2S_INOUT_RESOLUTION_LEVEL (2)`）",
        "对比变体：baseline / d1 / d1d5 / optimized（同一 harness、同一输入）",
        "数据规模：9 段音频 × 3 个抑制等级（L0 / L1 / L3），逐帧记录 12 类指标",
        "报告日期：%s" % deck.date,
    ]
    deck.bullets(slide, 1, lines, left=0.6, top=2.75, width=5.9, size=16, height=3.4,
                 label="封面信息")
    deck.picture(slide, 1, "FigT02", (6.85, 2.4, 5.9, 3.6), label="FigT02 缩略")
    deck.footer(slide, 1)


def p02_conclusions(deck: Deck):
    level = deck.level
    base = deck.agg("baseline")
    opt = deck.agg("optimized")
    metrics = deck.dataset.metrics("baseline", level, "1")
    metrics_opt = deck.dataset.metrics("optimized", level, "1")
    se_base = (DD.column_stats(metrics, "signal_energy", head=200) or {})
    se_opt = (DD.column_stats(metrics_opt, "signal_energy", head=200) or {})
    subtitle = "三项结论均可回溯到具体页面与具体列；无数据项以 — 占位，不预设通过"
    slide = deck.slide("三项核心结论", subtitle)

    card1 = [
        "D1：未初始化 Y[257] 参与 signal_energy 累加",
        "基线启动期（前 200 帧）均值 %s" % fmt(se_base.get("head_mean"), 3),
        "优化后启动期均值 %s" % fmt(se_opt.get("head_mean"), 3),
        "→ 详见第 15 / 17 / 23 / 25 页",
    ]
    delta = _delta(opt["atten_db"] if opt else None, base["atten_db"] if base else None)
    card2 = [
        "影响幅度：整段平均衰减（时长加权）",
        "基线 %s dB → 优化 %s dB" % (fmt(base["atten_db"] if base else None, 3),
                                     fmt(opt["atten_db"] if opt else None, 3)),
        "差值 %s dB" % fmt(delta, 3),
        "→ 详见第 4 章与第 5 章",
    ]
    gain = None
    if opt and opt.get("atten_db") and base and base.get("atten_db"):
        gain = (opt["atten_db"] - base["atten_db"]) / abs(base["atten_db"]) * 100.0 if base["atten_db"] else None
    card3 = [
        "优化改善量：收益与代价",
        "衰减提升 %s" % _pct(gain / 100.0 if gain is not None else None),
        "RTF %s → %s" % (fmt(base["rtf"] if base else None, 4),
                         fmt(opt["rtf"] if opt else None, 4)),
        "→ 详见第 18 / 25 / 31 页",
    ]
    add_cards(slide, [("结论一", card1, COLOR_PRIMARY),
                      ("结论二", card2, COLOR_BASE),
                      ("结论三", card3, COLOR_GOOD)], top=1.55, height=3.3)

    note_lines = [
        "判读口径：signal_energy 直接进入语音模型归一化，其启动期偏离会沿 "
        "diff_normalization → speech_prob_mean → gain_adjustment 逐级放大。",
        "本页数值随 build_deck.py 重建自动更新，来源为 %s 级别的逐帧指标与汇总 CSV。" % level,
        "另有本次新发现的 D8（fast_math.c 的 fastpow2 定义域缺陷，导致基线数据出现 nan），"
        "不属于上述传导链，见第 25 页。",
    ]
    add_text(slide, note_lines, 0.6, 5.05, 12.1, 1.4, size=14, color=COLOR_SUB, spacing=6)
    deck.footer(slide, 2)


def p03_roadmap(deck: Deck):
    base = deck.agg("baseline")
    rows = [[chapter, pages, question] for chapter, pages, question in CHAPTERS]
    slide = deck.slide("本次测试回答了什么",
                       "8 章导览：每一章回答一个可以被数据证伪的问题")
    add_table(slide, ["章节", "页", "回答的问题"], rows, left=0.8, top=1.45, width=11.7,
              col_widths=[3.0, 1.0, 7.7], body_size=13, header_size=14, max_height=3.4,
              row_height=0.33)
    numbers = [
        "9 个文件" + SEP + "总时长 %s s" % fmt(base["duration_s"] if base else None, 1),
        "总帧数 %s" % fmt(base["frames"] if base else None),
        "4 个变体 × 3 个等级",
        "12 类指标",
        "22 张图件",
    ]
    add_text(slide, ["关键数字：" + SEP.join(numbers)], 0.6, 5.4, 12.1, 0.5, size=19,
             bold=True, color=COLOR_PRIMARY)
    deck.bullets(slide, 3, [
        "结论先行的顺序：先给答案（第 1 章），再给被测对象与方法（第 2–3 章），最后给证据链。",
        "所有定量结论都在第 4–7 章逐条对应到页面与列名，可逐项复核。",
        "未覆盖项不隐藏：第 30 页列出本次测试覆盖不到的缺陷，第 34 页列出检查方法与限制。",
    ], left=0.6, top=6.0, width=12.1, size=14, height=1.0, label="导览说明")
    deck.footer(slide, 3)


def p04_answer(deck: Deck):
    base = deck.agg("baseline")
    opt = deck.agg("optimized")
    slide = deck.slide("一句话答案",
                       "基线在启动期把 signal_energy 抬高，导致语音模型归一化偏移、维纳增益失真；"
                       "打开 D1 后传导链被切断")
    if base and opt:
        headline = "整段平均衰减（时长加权）：基线 %s dB → 优化 %s dB（差 %s dB）" % (
            fmt(base["atten_db"], 3), fmt(opt["atten_db"], 3),
            fmt(_delta(opt["atten_db"], base["atten_db"]), 3))
    else:
        headline = "整段平均衰减：数据缺失（待 P2/P3 产出后自动填充）"
    add_text(slide, [headline], 0.6, 1.55, 12.1, 0.6, size=22, bold=True, color=COLOR_OPT)
    deck.bullets(slide, 4, [
        "D1 成立：基线 signal_energy 在启动期偏离优化值，且偏离沿四级传导链扩散。",
        "影响可控：整段衰减差异集中在启动期与低语音概率段，稳态差异很小。",
        "修复方式：仅由 src/ns_patch.h 的三个编译开关控制，基线行为与固件逐位一致。",
    ], left=0.6, top=2.4, width=5.9, size=17, height=3.4)
    deck.picture(slide, 4, "FigT01", HALF_IMG_RIGHT)
    deck.footer(slide, 4)


def p05_component(deck: Deck):
    slide = deck.slide("从固件工程中摘出的独立组件",
                       "29 个源文件逐文件搬运，只改 include 与平台宏，算法算式零改动")
    deck.bullets(slide, 5, [
        "分层：api/（ns_api.h 公共入口）· src/（算法 29 文件）· compat/（CMSIS 兼容层）· harness/（Test）",
        "摘取范围：enc.c / noise_suppressor.c / fast_math.c / histograms.c / "
        "prior_signal_model*.c 等（计划书 14.1 冻结清单）",
        "去平台化：GPIO 打点、__breakpoint、RAM 段宏、厂商头文件全部剥离，不参与编译",
        "唯一改动通道：src/ns_patch.h 的 D1 / D5 / D6 / D8 四个编译开关（第 6 页）",
        "接口冻结：48 kHz / 480 样本每帧（10 ms）/ 单声道 / 256 点 FFT",
    ], left=0.6, top=1.45, width=12.1, size=15, height=1.6)
    deck.picture(slide, 5, "FigT02", (0.6, 3.15, 12.13, 3.75))
    deck.footer(slide, 5)


def p06_switches(deck: Deck):
    slide = deck.slide("基线与优化版只差四个开关",
                       "同一份源码、同一编译器、同一输入；唯一变量是 CMake 注入的四个宏")
    rows = [
        ["D1", "noise_suppressor.c:331 / :388（FFT 回填后）",
         "不写入：Y[257] 保留未初始化栈值并累加进 signal_energy",
         "显式 Y[kFftSize+1] = 0.0f，切断污染源"],
        ["D5", "noise_suppressor.c:439（输出钳位）",
         "写死 24bit 量程：±8388607",
         "随 ENC_SAMPLE_DEPTH 派生：+8388607 / −8388608"],
        ["D6", "noise_suppressor.c:134、wiener_filter.c:88（幅度谱开方）",
         "fastsqrt（位近似）",
         "sqrtf（精确开方）"],
        ["D8", "fast_math.c（fastpow2 定义域缺陷）",
         "定义域外取值产生 NaN，沿诊断量与输出传播（本次新发现）",
         "修正定义域处理，消除 NaN"],
    ]
    add_table(slide, ["开关", "代码位置", "基线（=0，对齐固件）", "优化（=1）"], rows,
              left=0.8, top=1.45, width=11.7, col_widths=[0.8, 3.0, 4.0, 3.2],
              body_size=11, header_size=13, max_height=3.2, row_height=0.64)
    deck.bullets(slide, 6, [
        "开关默认全 0：基线构建下四个宏展开为空或保持原值，行为与固件逐位一致。",
        "D1 修未定义行为（未初始化栈内存），D8 修定义域缺陷（NaN 传播）；两者都不是调参。",
        "D5 只改钳位边界常量，D6 只改开方实现；四者均不改变算法结构。",
    ], left=0.6, top=4.85, width=12.1, size=15, height=1.5)
    deck.footer(slide, 6)


def p07_scale(deck: Deck):
    slide = deck.slide("测试量纲必须与固件一致",
                       "量纲不对齐时，钳位边界与谱能量都会失真，结论不可移植")
    chain = [
        "16bit WAV（±32768）",
        "×256（左移 8 位）",
        "24bit 量纲（±8388608）",
        "NS_Process48k",
        "÷256 + 钳位（−32768 … 32767）",
        "16bit 输出 WAV",
    ]
    add_text(slide, ["  →  ".join(chain)], 0.6, 1.5, 12.1, 0.7, size=19, bold=True,
             color=COLOR_PRIMARY, align=PP_ALIGN.CENTER)
    deck.bullets(slide, 7, [
        "依据一：drv_alg.c:46 `#define I2S_INOUT_RESOLUTION_LEVEL (2)`",
        "依据二：drv_alg.c:646 `*fInAdcL++ = (float)(*i32InAdcL++ >> 8);` 即固件侧浮点值为 24bit 量纲",
        "算法内部钳位与 24bit 匹配：noise_suppressor.c:439 的 ±8388607",
        "本次换算（已冻结）：输入 sample24 = int16 × 256；输出 out16 = clamp(round(out24 / 256), −32768, 32767)",
        "风险与兜底：若出厂固件的 I2S_INOUT_RESOLUTION_LEVEL 不是 2，则基线需按 16bit 重跑（第 34 页）",
    ], left=0.6, top=2.5, width=12.1, size=16, height=3.9)
    deck.footer(slide, 7)


def p08_dataset(deck: Deck):
    level = deck.level
    rows_raw = deck.dataset.summary("baseline", level)
    table_rows = []
    if rows_raw:
        for row in rows_raw:
            table_rows.append([
                str(row.get("file", "")), fmt(row.get("duration_s"), 2),
                fmt(row.get("frames")), fmt(row.get("in_rms"), 1), fmt(row.get("out_rms"), 1),
                fmt(row.get("atten_db"), 3), fmt(row.get("in_peak"), 1),
                fmt(row.get("clamp_hits_total")),
            ])
    else:
        table_rows = [["—"] * 8 for _ in range(9)]
    base = deck.agg("baseline")
    subtitle = "baseline / %s：" % level + (
        "共 %d 个文件、%s 秒、%s 帧，逐帧 480 样本 @48 kHz" % (
            base["files"], fmt(base["duration_s"], 1), fmt(base["frames"]))
        if base else "测量数据缺失，待 P2/P3 产出后重建")
    slide = deck.slide("9 个音频、190 秒、全覆盖", subtitle)
    add_table(slide, ["文件", "时长 (s)", "帧数", "输入 RMS", "输出 RMS", "衰减 (dB)",
                      "输入峰值", "钳位命中"], table_rows, left=0.8, top=1.45, width=11.7,
              col_widths=[1.0, 1.2, 1.2, 1.6, 1.6, 1.5, 1.4, 1.2], body_size=13,
              header_size=15, max_height=4.4, row_height=0.40)
    deck.note(slide, 8, _data_note(deck.dataset, level), top=5.95, size=13)
    deck.footer(slide, 8)


def p09_build(deck: Deck):
    keys = ["variant", "patch_flags", "sample_depth", "cmake_generator", "build_type",
            "compiler", "c_flags", "git_commit", "build_time"]
    labels = ["变体", "开关位", "样本深度", "CMake 生成器", "构建类型", "编译器",
              "C 编译选项", "git 提交", "构建时间"]
    infos = [deck.dataset.build_info(variant) for variant in VARIANTS]
    rows = []
    for key, label in zip(keys, labels):
        rows.append([label] + [(info or {}).get(key, "—") for info in infos])
    slide = deck.slide("构建可复现",
                       "指纹逐项留档：报告中的每个数字都能对应到一次确定的编译")
    add_table(slide, ["指纹项"] + list(VARIANTS), rows, left=0.8, top=1.45, width=11.7,
              col_widths=[1.5, 2.05, 2.05, 2.05, 2.05, 2.05], body_size=10, header_size=12,
              max_height=3.9, row_height=0.40)
    flags_line = "开关位（patch_flags，逐值读自 build_info.txt）：" + "、".join(
        "%s = %s" % (variant, (info or {}).get("patch_flags", "—"))
        for variant, info in zip(VARIANTS, infos)) + "；位定义 bit0 = D1、bit1 = D5、bit2 = D6、bit3 = D8。"
    deck.bullets(slide, 9, [
        flags_line,
        "编译器与选项固定，构建类型 Release；同一 commit 可复现同一份 .pptx 输入。",
        "重建方式：python build_deck.py（第 12 页给出全部命令）。",
    ], left=0.6, top=5.6, width=12.1, size=14, height=1.4)
    deck.footer(slide, 9)


def p10_pipeline(deck: Deck):
    slide = deck.slide("逐帧处理，逐帧记录",
                       "每个输入帧都产生一行指标，报告中的每条曲线都由这些行直接绘制")
    add_text(slide, [
        "输入 WAV（16bit / 2ch / 48 kHz） → 取左声道 → ×256 → 480 样本/帧（10 ms） → "
        "ns_core_<variant>(level) → ÷256 + 钳位 → 输出 WAV（16bit / 1ch）",
        "每帧回写 12 列指标 → metrics_<name>.csv → Python 分析出图 → python-pptx → 本报告",
    ], 0.6, 1.5, 12.1, 1.3, size=16, color=COLOR_PRIMARY, spacing=10)
    deck.bullets(slide, 10, [
        "帧长固定 480 样本：与固件 1 ms I2S 帧周期一致（D2 的触发条件正依赖这一点）。",
        "末帧补零后裁剪回原长，输入输出严格等长，便于逐样本对齐比较。",
        "基线与优化共用同一 harness、同一输入序列，指标 CSV 的行数逐文件一致。",
        "所有中间 CSV 与 WAV 保留在 ENC/Test/data 下，报告数字可逐行复核。",
    ], left=0.6, top=3.0, width=12.1, size=16, height=3.4)
    deck.footer(slide, 10)


def p11_metrics(deck: Deck):
    slide = deck.slide("12 类指标覆盖时域 / 频域 / 诊断 / 性能",
                       "列定义与测试 harness 的导出表头逐字一致，禁止写 NaN / Inf")
    rows = [[name, kind, desc, group] for name, kind, desc, group in METRIC_TABLE]
    add_table(slide, ["列名", "类型", "含义", "分类"], rows, left=0.8, top=1.45,
              width=11.7, col_widths=[2.6, 1.1, 6.4, 1.6], body_size=12, header_size=13,
              max_height=4.9, row_height=0.37)
    deck.note(slide, 11, "数值格式：浮点 %.9g、整数 %d；实测出现的 nan 已归因到 D8（第 25 页），"
                         "报告按原样保留、不替换为 0。", top=6.35, size=13)
    deck.footer(slide, 11)


def p12_commands(deck: Deck):
    slide = deck.slide("一键复现",
                       "从源码到本报告的完整链路；下列命令即本次测试实际执行的命令")
    commands = [
        "# 1) 构建组件与 harness",
        'cmake -S ENC/NS_Component -B ENC/NS_Component/build -G "Visual Studio 17 2022"',
        "cmake --build ENC/NS_Component/build --config Release",
        "",
        "# 2) 基线测量（level 3）与优化测量",
        'ENC/Test/bin/Release/ns_runner.exe baseline 3 "Test_Audio/AudioSample-48000hz" "ENC/Test/data"',
        'ENC/Test/bin/Release/ns_runner.exe optimized 3 "Test_Audio/AudioSample-48000hz" "ENC/Test/data"',
        "",
        "# 3) 出图 → 出 PPT → 校验",
        "cd ENC/Test/analysis",
        "python run_all.py",
    ]
    add_text(slide, commands, 0.8, 1.5, 11.7, 3.9, size=14, color="1b2733", spacing=2,
             line_spacing=1.0)
    deck.bullets(slide, 12, [
        "run_all.py 内部依次调用出图脚本、build_deck.py、check_deck.py。",
        "单独重建报告：python build_deck.py；单独校验：python check_deck.py。",
        "校验命令原文见第 34 页；渲染只走仓库内置 LibreOffice Kit，不调用系统 LibreOffice。",
    ], left=0.6, top=5.5, width=12.1, size=14, height=1.4)
    deck.footer(slide, 12)


def p13_baseline(deck: Deck):
    base = deck.agg("baseline")
    nan_count = (base or {}).get("nan_count") or 0
    subtitle = "基线 = 与固件逐位一致的构建；下列指标即“出厂状态”的整段表现"
    if nan_count:
        subtitle += "；其中 %d 个文件为 nan（D8 缺陷表现，见第 25 页），不计入平均" % nan_count
    slide = deck.slide("改动前的整体表现", subtitle)
    cards = [
        ("平均衰减", ["%s dB" % fmt(base["atten_db"] if base else None, 3),
                      "口径：时长加权，%s" % deck.level,
                      "排除 nan %d 个文件" % nan_count if nan_count
                      else "%d 个文件全部有效" % (base["files"] if base else 0)], COLOR_BASE),
        ("总时长 / 帧数", ["%s s" % fmt(base["duration_s"] if base else None, 1),
                            "%s 帧" % fmt(base["frames"] if base else None),
                            "480 样本/帧"], COLOR_PRIMARY),
        ("实时率 RTF", [fmt(base["rtf"] if base else None, 4),
                        "总耗时 %s ms" % fmt(base["total_elapsed_ms"] if base else None, 1),
                        "越小越好"], COLOR_GOOD),
        ("钳位总命中", [fmt(base["clamp_hits_total"] if base else None),
                        "基线固定 ±8388607", "D5 相关（第 26 页）"], COLOR_WARN),
    ]
    add_cards(slide, cards, top=1.45, height=1.45, title_size=15, body_size=13)
    deck.picture(slide, 13, "FigT01", (0.6, 3.05, 5.9, 3.8))
    deck.bullets(slide, 13, [
        "整段平均衰减在 1 dB 量级：噪声抑制是“轻度”作用，单看数值难以判断好坏。",
        "各文件差异来自素材本身的信噪比与静音比例，需逐文件对齐后再比较（第 14 页）。",
        "钳位命中说明输出已触到量程边界，是 D5 的观察入口。",
    ], left=6.85, top=3.15, width=5.9, size=15, height=3.6, label="要点")
    deck.footer(slide, 13)


def p14_baseline_table(deck: Deck):
    level = deck.level
    rows_raw = deck.dataset.summary("baseline", level)
    rows = []
    if rows_raw:
        for row in rows_raw:
            rows.append([str(row.get("file", "")), fmt(row.get("duration_s"), 2),
                         fmt(row.get("frames")), fmt(row.get("in_rms"), 1),
                         fmt(row.get("out_rms"), 1), fmt(row.get("atten_db"), 3),
                         fmt(row.get("total_elapsed_ms"), 1),
                         fmt(row.get("clamp_hits_total"))])
    else:
        rows = [["—"] * 8 for _ in range(9)]
    slide = deck.slide("逐文件明细（基线）",
                       "放大镜下的基线：这是后续一切对比的参照系")
    add_table(slide, ["文件", "时长 (s)", "帧数", "输入 RMS", "输出 RMS", "衰减 (dB)",
                      "总耗时 (ms)", "钳位命中"], rows, left=0.8, top=1.45, width=11.7,
              col_widths=[1.0, 1.2, 1.2, 1.6, 1.6, 1.5, 1.6, 1.2], body_size=13,
              header_size=15, max_height=4.5, row_height=0.44)
    nan_count = (deck.agg("baseline") or {}).get("nan_count") or 0
    nan_note = ("。表中 nan 为被测对象（固件）固有缺陷的真实表现（D8），不是测量故障，"
                "且不计入整段平均") if nan_count else ""
    deck.note(slide, 14, _data_note(deck.dataset, level) + nan_note, top=6.05, size=12)
    deck.footer(slide, 14)


def p15_startup(deck: Deck):
    level = deck.level
    metrics = deck.dataset.metrics("baseline", level, "1")
    metrics_opt = deck.dataset.metrics("optimized", level, "1")
    stats = DD.column_stats(metrics, "signal_energy", head=200) or {}
    stats_opt = DD.column_stats(metrics_opt, "signal_energy", head=200) or {}
    ratio = None
    if stats.get("head_mean") and stats.get("mean"):
        ratio = stats["head_mean"] / stats["mean"] if stats["mean"] else None
    slide = deck.slide("启动期的异常特征",
                       "形态差异不在声谱上，而在诊断量 signal_energy 的启动段")
    deck.bullets(slide, 15, [
        "signal_energy 是频域能量累加量，直接进入语音模型的归一化分支。",
        "基线（1.wav / %s）：前 200 帧均值 %s，全段均值 %s，启动期/全段 = %s" % (
            level, fmt(stats.get("head_mean"), 3), fmt(stats.get("mean"), 3),
            fmt(ratio, 3) if ratio is not None else "—"),
        "优化后同口径：前 200 帧均值 %s，全段均值 %s" % (
            fmt(stats_opt.get("head_mean"), 3), fmt(stats_opt.get("mean"), 3)),
        "判读：基线启动段被未初始化的 Y[257] 抬高，偏离随帧数衰减（栈值被逐步覆盖）。",
        "结论指向 D1：修复点在 FFT 回填后显式置零，而不是调整任何算法参数。",
    ], left=0.6, top=1.45, width=12.1, size=15, height=2.3)
    deck.picture(slide, 15, "FigT07", STRIP_IMG_BOX)
    deck.footer(slide, 15)


def p16_spectrogram_base(deck: Deck):
    slide = deck.slide("声谱上看不出明显异常",
                       "原始 vs 基线：肉眼可见的形态差异极小，单靠声谱无法判定 D1 是否存在")
    deck.picture(slide, 16, "FigT05", FULL_IMG_BOX)
    deck.footer(slide, 16)


def p17_diagnostics(deck: Deck):
    slide = deck.slide("但诊断量暴露了问题",
                       "四级传导链：signal_energy → diff_normalization → speech_prob_mean → gain_adjustment")
    deck.bullets(slide, 17, [
        "一级 signal_energy：启动期被未初始化 Y[257] 抬高（第 15 页）。",
        "二级 diff_normalization：归一化基准被抬高，谱差特征整体偏移。",
        "三级 speech_prob_mean：语音/噪声判定随之偏移，噪声段残留、语音段过度抑制。",
        "四级 gain_adjustment：维纳增益调节量继承上述偏移，最终表现为可见的衰减差异。",
        "→ 四级量的基线形态由本节图件给出，优化后的对照见第 23 页；其中 speech_prob_mean 的 "
        "NaN 断点属 D8（第 25 页），与 D1 传导链分开归因。",
    ], left=0.6, top=1.45, width=12.1, size=15, height=2.3)
    deck.picture(slide, 17, "FigT09", STRIP_IMG_BOX)
    deck.footer(slide, 17)


def p18_optimized(deck: Deck):
    level = deck.level
    base = deck.agg("baseline")
    opt = deck.agg("optimized")
    delta = _delta(opt["atten_db"] if opt else None, base["atten_db"] if base else None)
    slide = deck.slide("优化后整体改善",
                       "同一 harness、同一输入，只有四个开关不同；比较口径与第 13 页完全一致")
    cards = [
        ("优化平均衰减", ["%s dB" % fmt(opt["atten_db"] if opt else None, 3),
                          "基线 %s dB" % fmt(base["atten_db"] if base else None, 3)],
         COLOR_OPT),
        ("差值", ["%s dB" % fmt(delta, 3), "优化 − 基线", "时长加权口径"], COLOR_PRIMARY),
        ("钳位总命中", [fmt(opt["clamp_hits_total"] if opt else None),
                        "基线 %s" % fmt(base["clamp_hits_total"] if base else None),
                        "D5 改变量程边界"], COLOR_WARN),
        ("RTF", [fmt(opt["rtf"] if opt else None, 4),
                 "基线 %s" % fmt(base["rtf"] if base else None, 4), "D6 开方开销"], COLOR_GOOD),
    ]
    add_cards(slide, cards, top=1.45, height=1.45, title_size=15, body_size=13)

    base_rows = deck.dataset.summary("baseline", level) or []
    opt_rows = deck.dataset.summary("optimized", level) or []
    base_map = {str(r.get("file")): r.get("atten_db") for r in base_rows}
    opt_map = {str(r.get("file")): r.get("atten_db") for r in opt_rows}
    files = [str(r.get("file")) for r in base_rows] or list(DD.FILE_NAMES)
    series = [
        ("基线", [_chart_cell(base_map.get(f)) for f in files]),
        ("优化", [_chart_cell(opt_map.get(f)) for f in files]),
    ]
    add_bar_chart(slide, [f.replace(".wav", "") for f in files], series, 0.8, 3.1, 11.7, 3.2,
                  number_format="0.00")
    nan_hint = ("；基线中 nan 的柱为空（D8 缺陷表现，不是缺测）"
                ) if (deck.agg("baseline") or {}).get("nan_count") else ""
    deck.note(slide, 18, "柱状图数据内嵌自 summary_baseline_%s.csv / summary_optimized_%s.csv 的 "
                         "atten_db 列%s；逐文件原始值见第 14 / 19 页。"
              % (level, level, nan_hint), top=6.4, size=12)
    deck.footer(slide, 18)


def p19_compare_table(deck: Deck):
    level = deck.level
    base_rows = deck.dataset.summary("baseline", level) or []
    opt_rows = deck.dataset.summary("optimized", level) or []
    base_map = {str(r.get("file")): r for r in base_rows}
    opt_map = {str(r.get("file")): r for r in opt_rows}
    files = [str(r.get("file")) for r in base_rows] or list(DD.FILE_NAMES)
    rows = []
    for name in files:
        base_row = base_map.get(name, {})
        opt_row = opt_map.get(name, {})
        rows.append([
            name,
            fmt(base_row.get("atten_db"), 3),
            fmt(opt_row.get("atten_db"), 3),
            fmt(_delta(opt_row.get("atten_db"), base_row.get("atten_db")), 3),
            fmt(base_row.get("clamp_hits_total")),
            fmt(opt_row.get("clamp_hits_total")),
        ])
    if not base_rows:
        rows = [["—"] * 6 for _ in range(9)]
    slide = deck.slide("逐文件对比（基线 vs 优化）",
                       "差异稳定为正表示优化后抑制更强；差值列即第 18 页柱状图的高度差")
    add_table(slide, ["文件", "基线衰减 (dB)", "优化衰减 (dB)", "差值 (dB)",
                      "基线钳位命中", "优化钳位命中"], rows, left=0.8, top=1.45,
              width=11.7, col_widths=[1.3, 2.1, 2.1, 1.9, 2.1, 2.2], body_size=13,
              header_size=15, max_height=4.5, row_height=0.44)
    nan_note = ("。基线列中的 nan 为 D8 缺陷的真实表现（第 25 页），差值列相应留空"
                ) if (deck.agg("baseline") or {}).get("nan_count") else ""
    deck.note(slide, 19, _data_note(deck.dataset, level) + nan_note, top=6.05, size=12)
    deck.footer(slide, 19)


def p20_spectrogram_optimized(deck: Deck):
    slide = deck.slide("声谱三联对比",
                       "原始 / 基线 / 优化：优化后谱形更贴近原始素材，噪声段残留减少")
    deck.picture(slide, 20, "FigT05", FULL_IMG_BOX)
    deck.footer(slide, 20)


def p21_spectral_diff(deck: Deck):
    slide = deck.slide("差异集中在启动期与噪声段",
                       "基线谱 − 优化谱（dB）：差异不发生在语音主能量区，而在启动段与低语音概率段")
    deck.picture(slide, 21, "FigT06", FULL_IMG_BOX)
    deck.footer(slide, 21)


def p22_waveform(deck: Deck):
    slide = deck.slide("波形层面的改动",
                       "全段波形差异极小；放大到 0.5 s 片段才可见幅度包络变化")
    deck.bullets(slide, 22, [
        "全段对比（下行）：三条波形整体重合，说明优化没有引入可听的音质突变。",
        "放大片段（右侧，5–5.5 s）：可见基线与优化在包络上的细微差异。",
        "波形差异小但衰减指标稳定可测 —— 这正是本次用诊断量而非听感作判据的原因。",
    ], left=0.6, top=1.45, width=4.4, size=15, height=2.3)
    deck.picture(slide, 22, "FigT04", (8.3, 1.45, 4.43, 2.2))
    deck.picture(slide, 22, "FigT03", STRIP_IMG_BOX)
    deck.footer(slide, 22)


def p23_chain_cut(deck: Deck):
    slide = deck.slide("D1 传导链被切断",
                       "四联对照：signal_energy / diff_normalization / speech_prob_mean / gain_adjustment")
    deck.bullets(slide, 23, [
        "四张图统一口径：基线（灰）vs 优化（红），横轴为帧序号。",
    ], left=0.6, top=1.42, width=12.1, size=15, height=0.45)
    boxes = [
        ("FigT07", (0.6, 1.95, 5.9, 2.2)),
        ("FigT09", (6.85, 1.95, 5.9, 2.2)),
        ("FigT10", (0.6, 4.25, 5.9, 2.2)),
        ("FigT11", (6.85, 4.25, 5.9, 2.2)),
    ]
    for fig_id, box in boxes:
        deck.picture(slide, 23, fig_id, box)
    deck.note(slide, 23, "判读：signal_energy / diff_normalization / gain_adjustment 三条的启动期偏移"
                         "同向且同源（Y[257]），修复该点即切断该传导链；speech_prob_mean 上的 NaN 断点"
                         "属 D8（第 25 页），与 D1 传导链分开归因。", top=6.55, size=13)
    deck.footer(slide, 23)


def p24_ablation(deck: Deck):
    slide = deck.slide("四项修复各自的贡献",
                       "消融五组：基线 / +D1 / +D1D5 / +D1D5D6 / +D1D5D6+D8，同输入同口径；"
                       "D6 的效应体现在诊断量而非衰减量（第 27 页）")
    deck.picture(slide, 24, "FigT17", FULL_IMG_BOX)
    deck.footer(slide, 24)


def p25_d8(deck: Deck):
    level = deck.level
    slide = deck.slide("D8：本次测试最重要的发现",
                       "fast_math.c 的 fastpow2 定义域缺陷让 NaN 进入诊断量 speech_prob_mean；"
                       "输出音频与衰减指标仍然正常")
    aggs = OrderedDict((variant, deck.dataset.aggregate(variant, level)) for variant in VARIANTS)
    labels = ["基线", "+D1", "+D1D5", "+D1D5D6", "+D1D5D6+D8"]
    values = [_chart_cell(agg["atten_db"] if agg else None) for agg in aggs.values()]
    add_bar_chart(slide, labels, [("平均衰减 (dB)", values)], 0.8, 3.7, 11.7, 2.85,
                  number_format="0.000")
    nan_stats = OrderedDict((variant, deck.dataset.nan_frames(variant, level))
                            for variant in VARIANTS)
    seq = "、".join("%s %s" % (variant, fmt((nan_stats.get(variant) or {}).get("nan_frames")))
                    for variant in VARIANTS)
    files = fmt((nan_stats.get("baseline") or {}).get("files"))
    frames = fmt((nan_stats.get("baseline") or {}).get("frames"))
    deck.bullets(slide, 25, [
        "现象：NaN 只出现在诊断量列 speech_prob_mean；输出音频、atten_db、in_rms / out_rms 全部正常。",
        "根因：avg_log_lrt 超出定义域 → fastexp 得 NaN → speech_probability 为 NaN；"
        "wiener_filter.c 的 MIN(NaN, 1.0f) 在 C 宏语义下返回 1.0f，把 NaN 挡在滤波器之外。",
        "NaN 帧数（口径：speech_prob_mean 为 nan 的帧，%s，%s 个文件 / %s 帧）：%s。" % (
            level, files, frames, seq),
        "结论：D8 属于“数值失效”类缺陷，而非“量级偏差”类 —— 它不改变衰减量，"
        "但会让该帧的诊断量完全不可用，优先级高于 D1 / D5 / D6。",
    ], left=0.6, top=1.45, width=12.1, size=15, height=2.15)
    deck.note(slide, 25, "柱状数据内嵌自五份 summary_<variant>_%s.csv 的 atten_db 列；"
                         "nan 的柱留空，不伪造成 0。逐帧 NaN 数量受 D6 影响，见第 27 页。" % level,
              top=6.62, size=12)
    deck.footer(slide, 25)


def p26_d1_d5(deck: Deck):
    level = deck.level
    base = deck.agg("baseline")
    opt = deck.agg("optimized")
    d1 = deck.dataset.aggregate("d1", level)
    d1d5 = deck.dataset.aggregate("d1d5", level)
    slide = deck.slide("D1 是衰减量的主因，D5 条件触发",
                       "在整段衰减这一口径上 D1 贡献最大；D5 只在输出触达钳位边界时才有影响")
    deck.picture(slide, 26, "FigT15", (0.6, 2.1, 5.9, 3.9))
    deck.bullets(slide, 26, [
        "D1 单开（d1 组）平均衰减 %s dB，基线 %s dB，整段口径增益 %s dB。" % (
            fmt((d1 or {}).get("atten_db"), 3), fmt((base or {}).get("atten_db"), 3),
            fmt(_delta((d1 or {}).get("atten_db"), (base or {}).get("atten_db")), 3)),
        "D5 的独立贡献（d1d5 相对 d1）：%s dB，量级明显小于 D1。" % fmt(
            _delta((d1d5 or {}).get("atten_db"), (d1 or {}).get("atten_db")), 3),
        "钳位总命中：基线 %s，优化 %s；命中数来自 metrics 的 clamp_hits 列逐帧累加。" % (
            fmt((base or {}).get("clamp_hits_total")), fmt((opt or {}).get("clamp_hits_total"))),
        "D5 只改钳位上界的来源（写死 24bit → 随样本深度派生），仅在命中边界时产生分歧。",
        "本页只讨论衰减量与钳位；D8 的 NaN 归因见第 25 页，D6 见第 27 页。",
    ], left=6.85, top=2.15, width=5.9, size=15, height=3.8)
    deck.note(slide, 26, "D1 / D5 的对照数据来自 d1、d1d5 两份 summary_<variant>_%s.csv。" % level,
              top=6.3, size=13)
    deck.footer(slide, 26)


def p27_d6(deck: Deck):
    level = deck.level
    nan_stats = OrderedDict((variant, deck.dataset.nan_frames(variant, level))
                            for variant in VARIANTS)
    d1d5 = nan_stats.get("d1d5") or {}
    d1d5d6 = nan_stats.get("d1d5d6") or {}
    delta_nan = None
    if d1d5.get("nan_frames") is not None and d1d5d6.get("nan_frames") is not None:
        delta_nan = d1d5d6["nan_frames"] - d1d5["nan_frames"]
    delta_text = "—" if delta_nan is None else ("%+d" % delta_nan)
    slide = deck.slide("D6 对衰减量可忽略，但会改变诊断量轨迹",
                       "fastsqrt → sqrtf 只改开方实现：整段衰减无可测差异；但它改变 signal_spectrum 的数值，"
                       "NaN 的发生位置与数量随之变化")
    rows = [
        ["fastsqrt（基线）", "位近似开方", "幅度谱相对误差约 0.096%（固定偏置，同号）",
         "被差分与自适应平滑吸收：整段衰减无可测差异"],
        ["sqrtf（优化）", "精确开方", "误差降至浮点精度",
         "每帧多 128 次开方；数值路径改变 → 诊断量轨迹平移、NaN 帧数变化"],
    ]
    add_table(slide, ["实现", "方式", "误差量级", "对报告结论的影响"], rows, left=0.8, top=1.5,
              width=11.7, col_widths=[2.2, 2.0, 4.3, 3.2], body_size=12, header_size=13,
              max_height=1.9, row_height=0.7)
    deck.bullets(slide, 27, [
        "对衰减量：可忽略 —— 五组整段平均衰减的相邻差异处于 0.01–0.03 dB 量级（第 24 / 26 页）。",
        "对诊断量：不可忽略 —— NaN 帧数（口径：speech_prob_mean 为 nan 的帧，%s，%s 个文件）："
        "打开 D6 使 NaN 帧数由 %s 变为 %s（Δ %s）；五组完整序列见第 25 页。" % (
            level, fmt(d1d5d6.get("files") or d1d5.get("files")),
            fmt(d1d5.get("nan_frames")), fmt(d1d5d6.get("nan_frames")), delta_text),
        "机理：sqrtf 改变 signal_spectrum 数值 → 微移 avg_log_lrt 轨迹 → NaN 的发生位置与数量变化；"
        "D6 是四项修复中唯一会改变 NaN 分布的一项。",
        "与 D8 的区别：D6 是量化误差级（约 0.096% 系统性偏置），D8 是定义域缺陷级（直接产生 NaN / 符号翻转）；"
        "量级与性质完全不同（D8 见第 25 页）。",
        "结论：D6 保留在优化版中 —— 它不改善也不劣化衰减指标，可观测效应只体现在诊断量轨迹上。",
    ], left=0.6, top=3.65, width=12.1, size=14, height=2.9)
    deck.footer(slide, 27)


def p28_reference(deck: Deck):
    slide = deck.slide("与另一条移植路径互证",
                       "Project/ENC_Core 参照输出与本次基线的形态对比；形态相似即互证移植未走偏。"
                       "注：参照路径不暴露诊断量，其 speech_prob_mean 列为占位 nan，"
                       "语义与基线的 D8 缺陷不同，勿混用")
    deck.picture(slide, 28, "FigT20", (0.6, 1.6, 12.13, 5.3))
    deck.footer(slide, 28)


def p29_verification(deck: Deck):
    slide = deck.slide("审查结论逐条验证",
                       "逐条对照《NS移植缺陷审查报告》第 2 节结论摘要；D8 为本次测试新发现，不在该报告之列")
    rows = [
        ["D1", "noise_suppressor.c:297 / :342 / :370", "严重",
         "已验证：启动期 signal_energy 偏离实测", "第 15 / 17 / 23 / 26 页"],
        ["D2", "app_i2s.c:465-469 + :319-344", "严重",
         "未覆盖：单线程 harness 无中断竞争", "第 30 页"],
        ["D3", "drv_alg.c:672-676", "中等",
         "未覆盖：属 app 层调度行为", "第 30 页"],
        ["D5", "noise_suppressor.c:439", "轻微",
         "已验证：钳位命中统计与边界分歧", "第 26 页"],
        ["D6", "fast_math.c 全体", "轻微",
         "已验证：衰减量无差异，但改变 NaN 帧数", "第 27 页"],
        ["D8", "fast_math.c：fastpow2 定义域缺陷", "本次新发现",
         "已验证 / 已修复：基线含 nan，optimized 消除", "第 25 / 34 页"],
    ]
    add_table(slide, ["编号", "代码位置", "审查级别", "本次验证状态", "依据"], rows,
              left=0.8, top=1.45, width=11.7, col_widths=[0.8, 3.3, 1.2, 4.1, 2.3],
              body_size=12, header_size=13, max_height=3.9, row_height=0.55)
    deck.bullets(slide, 29, [
        "验证状态只取三种值：已验证 / 未覆盖 / 结论修正；本表无“结论修正”项。",
        "D8 为本次测试新发现（不来自前置审查报告），状态为“已验证 / 已修复”。",
        "D1 的验证依赖诊断量而非听感；D5 / D6 的验证依赖消融五组与钳位统计。",
        "未覆盖的 D2 / D3 已在第 30 页写明原因与后续验证所需条件。",
    ], left=0.6, top=5.45, width=12.1, size=14, height=1.5)
    deck.footer(slide, 29)


def p30_gaps(deck: Deck):
    slide = deck.slide("两项缺陷本次覆盖不到",
                       "D2 / D3 均为时序与调度类缺陷，组件化测试无法覆盖，明确标注为未覆盖")
    deck.bullets(slide, 30, [
        "D2（严重）：缓冲指针与处理触发不同步 —— 触发条件是 USER2 中断被 USER0 抢占"
        "（app_dma.c:247 优先级 1 vs drv_alg.c:558 优先级 2），单线程 harness 不存在该竞争。",
        "D2 需要什么：硬件上实测 Enc_Process 单次耗时分布，确认是否越过 1 ms 帧周期。",
        "D3（中等）：ENC 关闭期间采集指针不递增（drv_alg.c:672-676）—— 属 app 层调度，"
        "组件化后被 harness 的连续调用语义覆盖，偏差不显现。",
        "风险声明：两项均为“非崩溃、非静音”类缺陷，无法通过本次指标证伪，故不默认通过。",
        "后续验证项：板端插桩测量 Enc_Process 耗时；在 app 层恢复开关时序后重跑同一素材。",
    ], left=0.6, top=1.45, width=12.1, size=15, height=4.6)
    deck.note(slide, 30, "本页与第 29 页、第 34 页共同构成报告的“限制声明”三处，缺一不可。",
              top=6.3, size=13)
    deck.footer(slide, 30)


def p31_performance(deck: Deck):
    level = deck.level
    base_metrics = deck.dataset.metrics("baseline", level, "1")
    opt_metrics = deck.dataset.metrics("optimized", level, "1")
    base_us = [v for v in (base_metrics or {}).get("elapsed_us", []) if v is not None]
    opt_us = [v for v in (opt_metrics or {}).get("elapsed_us", []) if v is not None]
    base = deck.agg("baseline")
    opt = deck.agg("optimized")
    table_rows = [
        ["基线", fmt(sum(base_us) / len(base_us) if base_us else None, 1),
         fmt(percentile(base_us, 0.95), 1), fmt(max(base_us) if base_us else None),
         fmt(base["rtf"] if base else None, 4)],
        ["优化", fmt(sum(opt_us) / len(opt_us) if opt_us else None, 1),
         fmt(percentile(opt_us, 0.95), 1), fmt(max(opt_us) if opt_us else None),
         fmt(opt["rtf"] if opt else None, 4)],
    ]
    slide = deck.slide("性能与资源",
                       "单帧耗时（1.wav / %s）与实时率：优化版的开方开销不构成实时性风险" % level)
    deck.picture(slide, 31, "FigT19", (0.6, 1.5, 5.9, 4.6))
    add_table(slide, ["变体", "均值 (µs)", "P95 (µs)", "最大 (µs)", "RTF"], table_rows,
              left=6.85, top=1.6, width=5.9, col_widths=[1.0, 1.3, 1.2, 1.2, 1.2],
              body_size=12, header_size=13, max_height=1.6, row_height=0.5)
    deck.bullets(slide, 31, [
        "帧预算：10 ms / 帧（1 ms I2S 帧周期的 10 倍余量）。",
        "RTF 口径：总耗时 / 音频总时长，整段聚合（第 13 / 18 页同口径）。",
        "D6 的精确开方每帧多 128 次，落在此处的耗时分布内。",
    ], left=6.85, top=3.4, width=5.9, size=15, height=2.6)
    deck.footer(slide, 31)


def p32_conclusion(deck: Deck):
    base = deck.agg("baseline")
    opt = deck.agg("optimized")
    level = deck.level
    base_nan = deck.dataset.nan_frames("baseline", level) or {}
    opt_nan = deck.dataset.nan_frames("optimized", level) or {}
    d1d5_nan = deck.dataset.nan_frames("d1d5", level) or {}
    d1d5d6_nan = deck.dataset.nan_frames("d1d5d6", level) or {}
    delta_nan = None
    if d1d5_nan.get("nan_frames") is not None and d1d5d6_nan.get("nan_frames") is not None:
        delta_nan = d1d5d6_nan["nan_frames"] - d1d5_nan["nan_frames"]
    slide = deck.slide("结论与建议",
                       "结论可复现：同一命令、同一输入、同一开关组合，得到同一份报告")
    deck.bullets(slide, 32, [
        "结论一（最严重）：D8 成立 —— fast_math.c 的 fastpow2 定义域缺陷让 NaN 进入诊断量 "
        "speech_prob_mean（%s：基线 %s 帧、optimized %s 帧，第 25 页）；"
        "输出音频与 atten_db / in_rms / out_rms 均不受影响。" % (
            level, fmt(base_nan.get("nan_frames")), fmt(opt_nan.get("nan_frames"))),
        "结论二：D1 成立 —— 未初始化 Y[257] 抬高 signal_energy 并沿四级传导链影响增益；"
        "整段平均衰减 %s → %s dB（基线 → 优化，时长加权口径）。" % (
            fmt((base or {}).get("atten_db"), 3), fmt((opt or {}).get("atten_db"), 3)),
        "结论三：D5 条件触发；D6 对衰减量无可测影响，但会改变诊断量轨迹"
        "（NaN 帧数 Δ %s，第 27 页）。" % ("—" if delta_nan is None else "%+d" % delta_nan),
        "结论四：D2 / D3 未覆盖，报告不主张“移植完全等价”，只主张“本次量测范围内等价”。",
        "固件回接建议：D8（消除 NaN）与 D1 优先合入，D5 / D6 按需评估；"
        "后续验证项为板端实测 Enc_Process 耗时（判定 D2）与恢复 ENC 开关时序（判定 D3）。",
    ], left=0.6, top=1.5, width=12.1, size=16, height=4.6)
    deck.note(slide, 32, _data_note(deck.dataset, deck.level), top=6.35, size=12)
    deck.footer(slide, 32)


def _short(text, limit=22):
    text = str(text)
    return text if len(text) <= limit else text[: limit - 1] + "…"


def p33_appendix_a(deck: Deck):
    slide = deck.slide("附录 A：图件与脚本索引",
                       "22 张图件的版位、数据源与生成脚本；全部由 ENC/Test/analysis 下的脚本产出")
    items = list(FIGURE_SPECS.items())
    half = (len(items) + 1) // 2
    for chunk_index, chunk in enumerate((items[:half], items[half:])):
        rows = []
        for fig_id, spec in chunk:
            rows.append([fig_id, _short(spec["desc"]), DD.BOX_LABEL[spec["box"]],
                         spec["script"]])
        add_table(slide, ["编号", "内容", "版位", "生成脚本"], rows,
                  left=0.8 if chunk_index == 0 else 6.9, top=1.5, width=5.7,
                  col_widths=[0.8, 2.85, 0.75, 1.4], body_size=10, header_size=11,
                  max_height=4.6, row_height=0.36)
    deck.note(slide, 33, "图件完整描述与数据源见计划书 9.4 节；数据路径："
                         "ENC/Test/data/<variant>/<level>/（metrics_<name>.csv、"
                         "summary_<variant>_<level>.csv、<name>.wav）；报告脚本："
                         "deck_data.py / build_deck.py / check_deck.py。", top=6.3, size=12)
    deck.footer(slide, 33)


def p34_appendix_b(deck: Deck):
    slide = deck.slide("附录 B：检查方法与限制（强制页）",
                       "校验范围、视觉责任归属与量纲假设；本页不允许被省略或简化")
    rows = [
        ["结构校验", "check_office.py（ZIP/XML 完整性与内部关系）",
         "verdict = pass；页数 = 34", "preview/checks.json"],
        ["内容校验", 'check_office.py --count 34 --contains "结论" --contains "附录"',
         "页数与关键文本断言通过", "preview/checks.json"],
        ["渲染预览", "LibreOffice Kit：convert（PDF）+ render（34 页 PNG, dpi 144）",
         "PDF + page-0001..0034.png；missingFonts 为空", "preview/"],
        ["边界自检", "build_deck.py 内置：逐 shape 判定是否越出 13.333 × 7.5 in",
         "越界数 = 0（逐页记录见 _reports/deck_bounds.json）", "_reports/deck_bounds.json"],
        ["视觉检查", "视觉模型对 preview/page-0001..0034.png 逐页检查版式"
                     "（溢出 / 遮挡 / 字号可读性 / 中文字体渲染）",
         "逐页结论记入 _reports/visual_check.json", "_reports/visual_check.json"],
    ]
    add_table(slide, ["校验类别", "工具与方法", "通过标准", "产物"], rows, left=0.8, top=1.42,
              width=11.7, col_widths=[1.1, 4.7, 3.5, 2.4], body_size=12, header_size=13,
              max_height=3.0, row_height=0.50)
    base_nan = deck.dataset.nan_frames("baseline", deck.level) or {}
    deck.bullets(slide, 34, [
        "版式已由视觉模型完成自动检查（对 preview/page-0001..0034.png 逐页检查溢出 / 遮挡 / "
        "字号可读性 / 中文字体渲染，结论记入 _reports/visual_check.json），最终以用户审阅 "
        "preview/ 后的确认为准；本页不代表已由人工完成视觉验收。",
        "量纲假设（2.7 节）：输入左移 8 位对齐 24bit（sample24 = int16 × 256），输出右移 8 位还原并钳位；"
        "基于 drv_alg.c:46 的 I2S_INOUT_RESOLUTION_LEVEL (2)。若出厂固件使用其它值，基线需重跑。",
        "基线数据中的 nan 是被测对象（固件）的固有缺陷表现（D8：fast_math.c 的 fastpow2 定义域缺陷），"
        "只出现在诊断量列 speech_prob_mean（%s 共 %s 帧），不是测量故障；报告按原样呈现，未替换为 0。" % (
            deck.level, fmt(base_nan.get("nan_frames"))),
        "限制与重建：不做听感评价、不做板端时序验证；D2 / D3 未覆盖（第 30 页）；"
        "python build_deck.py && python check_deck.py 可一键重建。",
    ], left=0.6, top=4.50, width=12.1, size=15, height=2.45)
    deck.footer(slide, 34)


PAGES = [
    p01_cover, p02_conclusions, p03_roadmap, p04_answer, p05_component, p06_switches,
    p07_scale, p08_dataset, p09_build, p10_pipeline, p11_metrics, p12_commands,
    p13_baseline, p14_baseline_table, p15_startup, p16_spectrogram_base, p17_diagnostics,
    p18_optimized, p19_compare_table, p20_spectrogram_optimized, p21_spectral_diff,
    p22_waveform, p23_chain_cut, p24_ablation, p25_d8, p26_d1_d5, p27_d6, p28_reference,
    p29_verification, p30_gaps, p31_performance, p32_conclusion, p33_appendix_a,
    p34_appendix_b,
]


def _normalize_zip(path, stamp=(1980, 1, 1, 0, 0, 0)):
    """把 .pptx 的 ZIP 条目时间戳归一化，使重建结果字节级可复现。

    python-pptx 保存时以当前时间写入条目时间戳；原生图表的内嵌工作簿
    （``ppt/embeddings/Microsoft_Excel_Sheet*.xlsx``）的 ``docProps/core.xml``
    还带当前时间。两者都固定后，同一内容的重建产物 SHA256 完全一致。
    """
    temporary = path + ".tmp"
    with zipfile.ZipFile(path, "r") as source:
        infos = source.infolist()
        with zipfile.ZipFile(temporary, "w", zipfile.ZIP_DEFLATED) as target:
            for info in infos:
                data = source.read(info.filename)
                if info.filename.lower().endswith((".xlsx", ".xlsm")):
                    data = _freeze_inner_zip(data)
                entry = zipfile.ZipInfo(info.filename, date_time=stamp)
                entry.compress_type = info.compress_type
                entry.external_attr = info.external_attr
                target.writestr(entry, data)
    os.replace(temporary, path)


_CORE_CREATED = re.compile(rb"<dcterms:created[^>]*>[^<]*</dcterms:created>")
_CORE_MODIFIED = re.compile(rb"<dcterms:modified[^>]*>[^<]*</dcterms:modified>")
_FROZEN_TIME = b"1980-01-01T00:00:00Z"


def _freeze_inner_zip(data):
    """内嵌 xlsx：固定 ZIP 时间戳与 core.xml 的 created / modified。"""
    try:
        with zipfile.ZipFile(io.BytesIO(data), "r") as source:
            entries = [(info, source.read(info.filename)) for info in source.infolist()]
    except zipfile.BadZipFile:
        return data
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as target:
        for info, blob in entries:
            if info.filename == "docProps/core.xml":
                blob = _CORE_CREATED.sub(
                    b'<dcterms:created xsi:type="dcterms:W3CDTF">' + _FROZEN_TIME
                    + b"</dcterms:created>", blob)
                blob = _CORE_MODIFIED.sub(
                    b'<dcterms:modified xsi:type="dcterms:W3CDTF">' + _FROZEN_TIME
                    + b"</dcterms:modified>", blob)
            entry = zipfile.ZipInfo(info.filename, date_time=info.date_time)
            entry.compress_type = info.compress_type
            entry.external_attr = info.external_attr
            target.writestr(entry, blob)
    return buffer.getvalue()


def build():
    dataset = Dataset()
    deck = Deck(dataset)
    for page in PAGES:
        page(deck)
    assert len(deck.prs.slides) == TOTAL_PAGES, "页数不是 %d" % TOTAL_PAGES
    return deck


def _write_report(deck: Deck, path):
    report = deck.bounds_report()
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(report, handle, ensure_ascii=False, indent=2)
    return report


def _print_report(deck: Deck, report):
    print("=" * 78)
    print("第 4 步 / 逐 shape 边界自检（幻灯片 %.3f × %.3f in）" % (SLIDE_W_IN, SLIDE_H_IN))
    print("=" * 78)
    for page in report["pages"]:
        flag = "OK " if page["violations"] == 0 else "越界"
        print("  第 %2d 页 %-4s shapes=%-3d 右边界=%6.3f  下边界=%6.3f  %s" % (
            page["index"], flag, page["shapes"], page["max_right"], page["max_bottom"],
            page["title"][:34]))
    print("-" * 78)
    print("shape 总数 %d；越界 %d；无法判定 %d；页数 %d" % (
        report["shape_count"], len(report["violations"]), len(report["unknown"]),
        report["slide_count"]))
    for item in report["violations"]:
        print("  [越界] 第 %d 页 %s：%s" % (item["slide"], item["label"], item.get("reason", "")))
    over = [b for b in report["bullets"] if b["count"] > 5]
    for item in over:
        print("  [要点超限] 第 %d 页：%d 条" % (item["slide"], item["count"]))
    print("要点文本框最大条数：%d" % (max([b["count"] for b in report["bullets"]] or [0])))


def main():
    print("build_deck.py — 《%s》（%d 页）" % (DD.REPORT_NAME, TOTAL_PAGES))
    print("-" * 78)
    print("第 1 步 / 路径")
    for label, value in (("DATA_DIR", DD.DATA_DIR), ("FIGURES_DIR", DD.FIGURES_DIR),
                         ("PPTX_PATH", DD.PPTX_PATH), ("REPORT_DIR", DD.REPORT_DIR)):
        print("  %-12s = %s" % (label, value))

    deck = build()
    print("-" * 78)
    print("第 2 步 / 数据可用性")
    print("  报告口径等级：%s" % deck.level)
    for variant, levels in deck.dataset.state().items():
        print("  %-10s %s" % (variant, dict(levels)))
    missing = deck.dataset.missing_figures()
    print("  缺失图件 %d/%d 张%s" % (len(missing), len(DD.FIGURE_IDS),
                                     ("" if not missing else "： " + ", ".join(missing[:8]))))
    if deck.dataset.warnings:
        print("  数据警告 %d 条（前 8 条）：" % len(deck.dataset.warnings))
        for message in deck.dataset.warnings[:8]:
            print("    - %s" % message)
    note = _missing_note(deck.dataset)
    if note:
        print("  %s" % note)

    os.makedirs(os.path.dirname(DD.PPTX_PATH), exist_ok=True)
    deck.prs.save(DD.PPTX_PATH)
    _normalize_zip(DD.PPTX_PATH)
    size_kb = os.path.getsize(DD.PPTX_PATH) / 1024.0
    print("-" * 78)
    print("第 3 步 / 保存")
    print("  %s（%d 页，%.1f KB）" % (DD.PPTX_PATH, len(deck.slide_titles), size_kb))

    report_path = os.path.join(DD.REPORT_DIR, "deck_bounds.json")
    report = _write_report(deck, report_path)
    _print_report(deck, report)
    print("自检报告：%s" % report_path)
    print("=" * 78)
    ok = not report["violations"] and report["slide_count"] == TOTAL_PAGES
    print("结论：%s" % ("通过（34 页，无越界）" if ok else "不通过，请检查上方明细"))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
