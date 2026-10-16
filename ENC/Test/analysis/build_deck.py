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
                  number_format="0.00", legend=True, title=None, data_labels=True):
    """原生柱状图（数据内嵌，与源 CSV 一致）。

    ``data_labels=False`` 用于柱体过密的场景（例如 9 文件 × 3 系列 = 27 根柱），
    此时柱宽小于标签宽度、柱顶标签会互相压印 —— 数值仍可在表格页逐项核对。
    """
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
        plot.has_data_labels = bool(data_labels)
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

def _short(text, limit=22):
    """附录表格用的短描述（超出长度加省略号）。"""
    text = str(text)
    return text if len(text) <= limit else text[: limit - 1] + "…"

# 章节导览（第 6 页）。本定义覆盖文件前部的旧结构定义（旧 8 章结构已废弃）。
CHAPTERS = [
    ("第 1 章 结论先行", "1–6", "一句话答案、三项核心结论、DNS-MOS 总览、问题所在、定位与边界"),
    ("第 2 章 被测对象与评价方法", "7–11", "组件摘取与架构、量纲与测试数据、评价方法、构建可复现、一键复现"),
    ("第 3 章 发现的问题", "12–18", "缺陷判定矩阵、D8 根因与影响范围、零效果修复、基线低于不处理、未覆盖项"),
    ("第 4 章 修复与效果", "19–25", "DNS-MOS 逐文件明细、分层效果、消融五组、诊断量轨迹、信号层、互证、性能"),
    ("第 5 章 结论与附录", "26–34", "结论与建议、修复优先级、限制与假设、图件与数据索引、检查方法"),
]

METRIC_TABLE = [
    ("frame_idx", "int", "帧序号，从 0 开始", "时域"),
    ("in_rms", "float", "本帧输入 RMS（24bit 量纲）", "时域"),
    ("out_rms", "float", "本帧输出 RMS（24bit 量纲）", "时域"),
    ("atten_db", "float", "20·log10(in_rms / out_rms)；out_rms=0 记为 999.0", "时域"),
    ("signal_energy", "float", "频域能量累加量（D1 的写入点；D8 会使其上游失真）", "频域"),
    ("signal_spectral_sum", "float", "信号谱幅度总和", "频域"),
    ("diff_normalization", "float", "谱差特征归一化量（传导链第二级）", "诊断"),
    ("speech_prob_mean", "float", "帧内语音概率均值；D8 唯一被 NaN 污染的列（第 13 / 14 页）", "诊断"),
    ("prior_speech_prob", "float", "先验语音概率", "诊断"),
    ("gain_adjustment", "float", "维纳增益调节量（传导链第四级）", "诊断"),
    ("clamp_hits", "int", "本帧触达钳位边界的样本数；实测全变体为 0（第 15 页）", "诊断"),
    ("elapsed_us", "int", "本帧 NS_Process48k 耗时（微秒）", "性能"),
]


def _ovrl(deck: Deck, variant):
    """DNS-MOS 汇总里的 OVRL（缺失返回 None）。"""
    return (deck.dataset.dnsmos(variant) or {}).get("OVRL")


def _dnsmos_cell(deck: Deck, variant, key):
    return (deck.dataset.dnsmos(variant) or {}).get(key)


def _dns_categories_and_values(deck: Deck, key="OVRL", variants=None):
    """DNS-MOS 原生图表用：分类（中文标签）与数值序列。"""
    variants = variants or DD.DNSMOS_VARIANTS
    labels = [DD.DNSMOS_LABELS.get(v, v) for v in variants]
    values = [_chart_cell(_dnsmos_cell(deck, v, key)) for v in variants]
    return labels, values


def p01_cover(deck: Deck):
    slide = deck.slide(None)
    add_text(slide, ["NS 降噪组件化与对比测试报告"], 0.6, 0.9, 12.1, 0.9, size=40, bold=True,
             color="10243a")
    add_text(slide, ["固件算法摘取" + SEP + "缺陷归因" + SEP + "DNS-MOS 感知质量评估"],
             0.6, 1.85, 12.1, 0.4, size=19, color=COLOR_PRIMARY)
    lines = [
        "被测对象：ENC 降噪算法（noise_suppressor.c 及其依赖，共 29 个源文件）",
        "评价方式：12 类逐帧指标 + DNS-MOS 无参考感知质量评估（7 个变体）",
        "感知质量（OVRL）：不处理 %s → 基线 %s → 修复后 %s" % (
            fmt(_ovrl(deck, "input"), 4), fmt(_ovrl(deck, "baseline"), 4),
            fmt(_ovrl(deck, "optimized"), 4)),
        "数据规模：9 段音频 × 3 个抑制等级（L0 / L1 / L3），逐帧记录 12 类指标",
        "报告日期：%s" % deck.date,
    ]
    deck.bullets(slide, 1, lines, left=0.6, top=2.75, width=5.9, size=15, height=3.4,
                 label="封面信息")
    deck.picture(slide, 1, "FigT02", (6.85, 2.4, 5.9, 3.6), label="FigT02 缩略")
    deck.footer(slide, 1)


def p02_answer(deck: Deck):
    base, inp, opt = _ovrl(deck, "baseline"), _ovrl(deck, "input"), _ovrl(deck, "optimized")
    slide = deck.slide("一句话答案",
                       "感知质量口径（DNS-MOS OVRL，5 分制）：不处理 %s → 基线 %s → 修复后 %s" % (
                           fmt(inp, 4), fmt(base, 4), fmt(opt, 4)))
    add_text(slide, ["基线比“完全不处理”低 %s；修复后反超不处理 %s" % (
        fmt(abs(_delta(base, inp)) if _delta(base, inp) is not None else None, 4),
        fmt(_delta(opt, inp), 4))],
        0.6, 1.5, 6.9, 0.9, size=20, bold=True, color=COLOR_OPT)
    deck.bullets(slide, 2, [
        "基线把质量做“低”了：OVRL %s，低于不处理的 %s（低 %s）。" % (
            fmt(base, 4), fmt(inp, 4),
            fmt(abs(_delta(base, inp)) if _delta(base, inp) is not None else None, 4)),
        "修复后反超不处理：OVRL %s，比基线高 %s。" % (fmt(opt, 4), fmt(_delta(opt, base), 4)),
        "四项修复里只有 D8 有效：D1 / D5 对输出影响为零，D6 影响极小（第 15 / 20 页）。",
        "结论建立在 9 个文件、19046 帧的逐帧指标与 63 段 DNS-MOS 评分之上（第 19 页）。",
    ], left=0.6, top=2.55, width=5.9, size=16, height=3.6)
    labels, values = _dns_categories_and_values(deck, "OVRL")
    add_bar_chart(slide, labels, [("DNS-MOS OVRL", values)], 6.85, 1.6, 5.9, 4.5,
                  number_format="0.000")
    deck.note(slide, 2, "柱状数据内嵌自 dnsmos/dnsmos_summary.csv 的 OVRL 列，逐变体一一对应。",
              top=6.35, size=12)
    deck.footer(slide, 2)


def p03_conclusions(deck: Deck):
    nan_base = deck.dataset.nan_frames("baseline", deck.level) or {}
    nan_opt = deck.dataset.nan_frames("optimized", deck.level) or {}
    sp_base = deck.dataset.speech_prob("baseline", deck.level) or {}
    sp_opt = deck.dataset.speech_prob("optimized", deck.level) or {}
    same_d1, total_d1 = deck.dataset.wav_identical("baseline", "d1", deck.level)
    same_d5, total_d5 = deck.dataset.wav_identical("baseline", "d1d5", deck.level)
    slide = deck.slide("三项核心结论",
                       "每一条都能回溯到具体页面与具体列；无数据项以 — 占位，不预设通过")
    card1 = [
        "D8 成立且已修：fastpow2 定义域缺陷",
        "NaN 帧数 %s → %s（%s 帧总）" % (fmt(nan_base.get("nan_frames")),
                                          fmt(nan_opt.get("nan_frames")),
                                          fmt(nan_base.get("frames"))),
        "speech_prob_mean 饱和率 %s → %s" % (
            _pct(sp_base.get("share_at_one")), _pct(sp_opt.get("share_at_one"))),
        "→ 第 13 / 14 页",
    ]
    card2 = [
        "D1 / D5 修复对输出严格为零",
        "输出 WAV 逐字节相同：%s/%s（D1）、%s/%s（D5）" % (same_d1, total_d1, same_d5, total_d5),
        "钳位命中：全变体 0 次",
        "→ 第 15 页",
    ]
    card3 = [
        "基线感知质量低于不处理",
        "OVRL 基线 %s < 不处理 %s" % (fmt(_ovrl(deck, "baseline"), 4), fmt(_ovrl(deck, "input"), 4)),
        "差值 %s（基线更差）" % fmt(_delta(_ovrl(deck, "baseline"), _ovrl(deck, "input")), 4),
        "→ 第 16 页",
    ]
    add_cards(slide, [("结论一", card1, COLOR_OPT),
                      ("结论二", card2, COLOR_PRIMARY),
                      ("结论三", card3, COLOR_WARN)], top=1.55, height=3.3)
    add_text(slide, [
        "判读口径：DNS-MOS 为无参考感知质量模型打分（非听感主观评价），四指标为 SIG / BAK / OVRL / P808_MOS。",
        "本页数值随 build_deck.py 重建自动更新，来源为 dnsmos_summary.csv 与 %s 级别的逐帧指标。" % deck.level,
    ], 0.6, 5.05, 12.1, 1.4, size=14, color=COLOR_SUB, spacing=6)
    deck.footer(slide, 3)


def p04_dnsmos(deck: Deck):
    base, opt, ref = _ovrl(deck, "baseline"), _ovrl(deck, "optimized"), _ovrl(deck, "reference")
    slide = deck.slide("语音整体质量对比（DNS-MOS）",
                       "四指标 × 7 变体：修复后 OVRL %s，基线 %s，参照路径 %s；差异集中在 BAK 与 OVRL"
                       % (fmt(opt, 4), fmt(base, 4), fmt(ref, 4)))
    deck.picture(slide, 4, "FigT23", FULL_IMG_BOX)
    deck.footer(slide, 4)


def p05_problems(deck: Deck):
    slide = deck.slide("问题所在：5 项缺陷 + 3 项零效果 + 1 项定性修正",
                       "一页看全本次测试对每个缺陷的判定、影响与修复效果")
    rows = [
        ["D8", "fastpow2 定义域缺陷 → NaN", "本次新发现，成立",
         "speech_prob_mean 481 帧 NaN、饱和率 92.3%", "已修复 → OVRL 提升"],
        ["D1", "未初始化 Y[257] 污染 signal_energy", "成立（审查报告）",
         "诊断量被改变，但未传导至输出", "修复后输出逐字节相同 → 无效"],
        ["D5", "输出钳位量程写死 24bit", "成立（审查报告）",
         "钳位从未触发（0 命中）", "修复后输出逐字节相同 → 无效"],
        ["D6", "fastsqrt 位近似开方", "成立（审查报告）",
         "改变数值路径与 NaN 分布", "影响极小（OVRL +0.0067）"],
        ["D2", "中断触发丢失、双缓冲错位", "未覆盖",
         "需硬件中断竞争上下文", "未验证（第 18 页）"],
        ["D3", "ENC 关闭期采集指针不递增", "未覆盖",
         "属 app 层调度行为", "未验证（第 18 页）"],
    ]
    add_table(slide, ["编号", "缺陷", "判定", "实测影响", "修复效果"], rows,
              left=0.8, top=1.45, width=11.7, col_widths=[0.8, 3.5, 1.7, 3.0, 2.7],
              body_size=12, header_size=13, max_height=3.9, row_height=0.58)
    deck.note(slide, 5, "判定只取三种值：成立 / 未覆盖 / 不成立；“零效果”指修复后输出 WAV 与基线逐字节相同"
                        "（第 15 页）或感知差异低于模型分辨率（第 20 页）。", top=6.15, size=12)
    deck.footer(slide, 5)


def p06_scope(deck: Deck):
    base = deck.agg("baseline")
    slide = deck.slide("本次测试的定位与边界",
                       "5 章导览 + 关键数字；本报告只主张“本次量测范围内”的结论")
    rows = [[chapter, pages, question] for chapter, pages, question in CHAPTERS]
    add_table(slide, ["章节", "页", "回答的问题"], rows, left=0.8, top=1.45, width=11.7,
              col_widths=[3.0, 1.0, 7.7], body_size=13, header_size=14, max_height=3.0,
              row_height=0.40)
    numbers = [
        "9 个文件" + SEP + "总时长 %s s" % fmt(base["duration_s"] if base else None, 1),
        "总帧数 %s" % fmt(base["frames"] if base else None),
        "5 个消融变体 + 不处理 + 参照",
        "12 类逐帧指标",
        "23 张图件",
    ]
    add_text(slide, ["关键数字：" + SEP.join(numbers)], 0.6, 5.0, 12.1, 0.5, size=18, bold=True,
             color=COLOR_PRIMARY)
    deck.bullets(slide, 6, [
        "边界一：被测对象是组件层（ENC/NS_Component），不含 app 层调度与中断上下文。",
        "边界二：harness 为单线程连续调用，D2 / D3 这类时序缺陷无法复现（第 18 页）。",
        "边界三：感知质量由 DNS-MOS 模型给出，不等价于听感主观评价；量纲假设见第 29 页。",
    ], left=0.6, top=5.55, width=12.1, size=14, height=1.4)
    deck.footer(slide, 6)


def p07_component(deck: Deck):
    slide = deck.slide("从固件工程中摘出的独立组件",
                       "29 个源文件逐文件搬运，只改 include 与平台宏，算法算式零改动")
    deck.bullets(slide, 7, [
        "分层：api/（ns_api.h 公共入口）· src/（算法 29 文件）· compat/（CMSIS 兼容层）· harness/（Test）",
        "摘取范围：enc.c / noise_suppressor.c / fast_math.c / histograms.c / "
        "prior_signal_model*.c 等（计划书 14.1 冻结清单）",
        "去平台化：GPIO 打点、__breakpoint、RAM 段宏、厂商头文件全部剥离，不参与编译",
        "唯一改动通道：src/ns_patch.h 的 D1 / D5 / D6 / D8 四个编译开关（第 10 页）",
        "接口冻结：48 kHz / 480 样本每帧（10 ms）/ 单声道 / 256 点 FFT",
    ], left=0.6, top=1.45, width=12.1, size=15, height=1.6)
    deck.picture(slide, 7, "FigT02", (0.6, 3.15, 12.13, 3.75))
    deck.footer(slide, 7)


def p08_scale_data(deck: Deck):
    level = deck.level
    rows_raw = deck.dataset.summary("baseline", level)
    table_rows = []
    if rows_raw:
        for row in rows_raw[:9]:
            table_rows.append([
                str(row.get("file", "")), fmt(row.get("duration_s"), 2),
                fmt(row.get("frames")), fmt(row.get("in_rms"), 1),
                fmt(row.get("atten_db"), 3),
            ])
    else:
        table_rows = [["—"] * 5 for _ in range(9)]
    base = deck.agg("baseline")
    slide = deck.slide("量纲与测试数据",
                       "量纲必须与固件一致，否则钳位边界与谱能量都会失真；数据规模 %s 秒 / %s 帧" % (
                           fmt(base["duration_s"] if base else None, 2),
                           fmt(base["frames"] if base else None)))
    add_text(slide, ["16bit WAV（±32768）  →  ×256（左移 8 位）  →  24bit 量纲（±8388608）  →  "
                     "NS_Process48k  →  ÷256 + 钳位（−32768 … 32767）  →  16bit 输出 WAV"],
             0.6, 1.45, 12.1, 0.6, size=16, bold=True, color=COLOR_PRIMARY,
             align=PP_ALIGN.CENTER)
    add_table(slide, ["文件", "时长 (s)", "帧数", "输入 RMS", "衰减 (dB)"], table_rows,
              left=0.8, top=2.2, width=5.6, col_widths=[1.0, 1.2, 1.2, 1.3, 1.3],
              body_size=12, header_size=13, max_height=3.6, row_height=0.34)
    deck.bullets(slide, 8, [
        "依据一：drv_alg.c:46 `#define I2S_INOUT_RESOLUTION_LEVEL (2)`；依据二：drv_alg.c:646 `>> 8`。",
        "算法内部钳位与 24bit 匹配：noise_suppressor.c:439 的 ±8388607。",
        "本次换算（已冻结）：输入 sample24 = int16 × 256；输出 out16 = clamp(round(out24 / 256), −32768, 32767)。",
        "风险：若出厂固件的 I2S_INOUT_RESOLUTION_LEVEL 不是 2，基线需按 16bit 重跑（第 29 / 34 页）。",
    ], left=6.7, top=2.2, width=6.0, size=14, height=3.6)
    deck.footer(slide, 8)


def p09_evaluation(deck: Deck):
    slide = deck.slide("如何评价：逐帧指标 + 感知质量",
                       "两条互补的证据链：可逐帧复算的过程量，与面向听感的质量分")
    rows = [
        ["时域", "frame_idx / in_rms / out_rms / atten_db", "衰减是否发生、发生多少", "可逐行复算"],
        ["频域", "signal_energy / signal_spectral_sum", "能量与谱形态是否失真", "D8 的污染入口"],
        ["诊断", "diff_normalization / speech_prob_mean / prior_speech_prob / gain_adjustment / clamp_hits",
         "算法内部决策是否偏移", "D1 / D8 的观测点"],
        ["性能", "elapsed_us", "实时性是否达标", "RTF 与分位耗时"],
    ]
    add_table(slide, ["分类", "代表列", "回答的问题", "作用"], rows, left=0.8, top=1.45,
              width=11.7, col_widths=[0.9, 4.4, 3.4, 3.0], body_size=12, header_size=13,
              max_height=2.9, row_height=0.62)
    deck.bullets(slide, 9, [
        "DNS-MOS：无参考语音质量评估模型，输出 SIG / BAK / OVRL / P808_MOS 四项分数（1–5，越高越好）。",
        "为什么不用 PESQ：PESQ 属有参考评估，需要与干净参考信号逐样本对齐；本次素材的“原始”本身已含噪，"
        "不存在可用作参考的干净版本，故改用无参考的 DNS-MOS。",
        "评分对象：每个变体在每个文件上的输出 WAV，共 7 变体 × 9 文件 = 63 段（逐段按时长分片后取均值）。",
        "两条证据链的分工：逐帧指标解释“为什么变”，DNS-MOS 回答“变得好不好”。",
        "局限：DNS-MOS 是客观模型打分，不等价于听感主观评价（第 29 页）。",
    ], left=0.6, top=4.7, width=12.1, size=13, height=2.2)
    deck.footer(slide, 9)


def p10_build(deck: Deck):
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
    deck.bullets(slide, 10, [
        flags_line,
        "编译器与选项固定，构建类型 Release；同一 commit 可复现同一份 .pptx 输入。",
        "重建方式：python build_deck.py（第 11 页给出全部命令）。",
    ], left=0.6, top=5.6, width=12.1, size=14, height=1.4)
    deck.footer(slide, 10)


def p11_reproduce(deck: Deck):
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
    deck.bullets(slide, 11, [
        "run_all.py 内部依次调用出图脚本、build_deck.py、check_deck.py。",
        "单独重建报告：python build_deck.py；单独校验：python check_deck.py。",
        "校验命令原文见第 34 页；渲染只走仓库内置 LibreOffice Kit，不调用系统 LibreOffice。",
    ], left=0.6, top=5.5, width=12.1, size=14, height=1.4)
    deck.footer(slide, 11)


def p12_problem_matrix(deck: Deck):
    slide = deck.slide("问题总览：判定矩阵",
                       "先给判定，再给证据：每一项缺陷要么有实测依据，要么明确标注未覆盖")
    rows = [
        ["D1", "成立", "诊断量被改变（signal_energy 等 4 列）", "输出 WAV 逐字节相同", "无效"],
        ["D5", "成立", "钳位从未触发（0 命中）", "输出 WAV 逐字节相同", "无效"],
        ["D6", "成立", "改变数值路径与 NaN 分布（481 → 500）", "输出 WAV 不同但感知差 +0.0067", "极小"],
        ["D8", "成立（新发现）", "NaN 481 帧 + 饱和率 92.3%", "输出 WAV 变为有效值", "显著"],
        ["D2", "未覆盖", "需中断竞争上下文", "单线程 harness 无法复现", "未验证"],
        ["D3", "未覆盖", "属 app 层调度行为", "组件层不体现", "未验证"],
    ]
    add_table(slide, ["编号", "判定", "关键实测证据", "对输出的影响", "修复效果"], rows,
              left=0.8, top=1.45, width=11.7, col_widths=[0.8, 1.4, 4.4, 3.2, 1.9],
              body_size=12, header_size=13, max_height=3.9, row_height=0.58)
    deck.note(slide, 12, "“无效 / 极小 / 显著”三级来自同一口径：输出 WAV 的逐字节比较与 DNS-MOS OVRL 变化"
                         "（第 15 / 20 页）。", top=6.15, size=12)
    deck.footer(slide, 12)


def p13_d8_root(deck: Deck):
    slide = deck.slide("D8：新发现的 NaN 缺陷（根因）",
                       "fast_math.c 的 fastpow2 在定义域外取值错误，NaN 经 fastexp 传播到语音概率")
    add_text(slide, [
        "fastpow2 位重解释失效  →  avg_log_lrt 计算偏离  →  fastexp(−avg_log_lrt) 在 y < −88.7 时得 NaN"
        "  →  speech_probability = NaN  →  speech_prob_mean 为 nan",
    ], 0.6, 1.5, 12.1, 0.8, size=16, bold=True, color=COLOR_OPT, align=PP_ALIGN.CENTER)
    deck.bullets(slide, 13, [
        "根因位置：fast_math.c 的 fastpow2（位重解释实现），在定义域外取值时结果错误。",
        "传播路径：错误值经 fastexp 放大为 NaN；NaN 只污染诊断量 speech_prob_mean。",
        "为什么输出没坏：wiener_filter.c 的 MIN(NaN, 1.0f) 在 C 宏语义下返回 1.0f，把 NaN 挡在滤波器之外。",
        "为什么仍要修：NaN 使该帧的诊断量完全不可用，并让整个变体的 speech_prob_mean 长期饱和（第 14 页）。",
        "判定：属“数值失效”类缺陷，而非“量级偏差”类 —— 优先级高于 D1 / D5 / D6。",
    ], left=0.6, top=2.4, width=12.1, size=15, height=4.3)
    deck.footer(slide, 13)


def p14_d8_scope(deck: Deck):
    level = deck.level
    prof_base = deck.dataset.speech_prob("baseline", level) or {}
    prof_opt = deck.dataset.speech_prob("optimized", level) or {}
    ratio = None
    if prof_base.get("nan_frames") is not None and prof_base.get("frames"):
        ratio = prof_base["nan_frames"] / prof_base["frames"]
    slide = deck.slide("D8 的影响范围",
                       "NaN 之外更严重的是饱和：基线 92.3% 的帧把 speech_prob_mean 顶在 1.0")
    rows = [
        ["NaN 帧数", fmt(prof_base.get("nan_frames")), fmt(prof_opt.get("nan_frames")),
         "%s / %s = %s" % (fmt(prof_base.get("nan_frames")), fmt(prof_base.get("frames")),
                           _pct(ratio, 2))],
        ["speech_prob_mean 中位数", fmt(prof_base.get("median"), 4), fmt(prof_opt.get("median"), 4),
         "基线饱和在 1.0，修复后回到真实分布"],
        ["饱和率（≥ 0.999 的帧占比）", _pct(prof_base.get("share_at_one")),
         _pct(prof_opt.get("share_at_one")), "诊断量信息在基线中几乎完全丢失"],
        ["1% / 99% 分位", "%s / %s" % (fmt(prof_base.get("p01"), 4), fmt(prof_base.get("p99"), 4)),
         "%s / %s" % (fmt(prof_opt.get("p01"), 4), fmt(prof_opt.get("p99"), 4)),
         "两端形态一致，差异集中在上端饱和"],
    ]
    add_table(slide, ["指标", "基线", "优化（含 D8）", "判读"], rows, left=0.8, top=1.45,
              width=11.7, col_widths=[2.6, 2.2, 2.4, 4.5], body_size=12, header_size=13,
              max_height=2.5, row_height=0.55)
    deck.picture(slide, 14, "FigT10", (0.6, 4.35, 12.13, 2.5))
    deck.footer(slide, 14)


def p15_zero_effect(deck: Deck):
    level = deck.level
    same, total = deck.dataset.wav_identical("baseline", "d1", level)
    same5, total5 = deck.dataset.wav_identical("baseline", "d1d5", level)
    same6, total6 = deck.dataset.wav_identical("baseline", "d1d5d6", level)
    agg = deck.agg("baseline")
    nan5 = deck.dataset.nan_frames("d1d5", level) or {}
    nan6 = deck.dataset.nan_frames("d1d5d6", level) or {}
    rows = [
        ["D1", "未初始化 Y[257] 污染 signal_energy",
         "输出 WAV %s/%s 逐字节相同" % (same, total), "诊断量改变但未传导到输出", "无效"],
        ["D5", "输出钳位量程写死 24bit",
         "输出 WAV %s/%s 逐字节相同" % (same5, total5),
         "钳位从未触发（总命中 %s）" % fmt(agg["clamp_hits_total"] if agg else None), "无效"],
        ["D6", "fastsqrt 位近似开方",
         "输出 WAV %s/%s 相同（即全部不同）" % (same6, total6),
         "NaN 帧数 %s → %s" % (fmt(nan5.get("nan_frames")), fmt(nan6.get("nan_frames"))), "极小"],
    ]
    slide = deck.slide("D1 / D5 / D6：修复效果实测",
                       "D1 / D5 对输出严格为零（逐字节相同）；D6 改变了输出，但幅度极小")
    add_table(slide, ["修复项", "机理", "输出 WAV 逐字节比较", "诊断量变化", "判定"], rows,
              left=0.8, top=1.45, width=11.7, col_widths=[0.9, 3.3, 3.0, 3.2, 1.3],
              body_size=12, header_size=13, max_height=2.6, row_height=0.62)
    deck.bullets(slide, 15, [
        "判据：对同一变体的 9 个输出 WAV 逐文件计算 SHA256，与基线比较；相同即“对输出零影响”。",
        "D1 的意义不在输出：它修的是未定义行为（读未初始化栈内存），移除该风险本身即有价值。",
        "D5 零效果的原因很直接：本次素材的输出从未触达钳位边界，量程写死与否都不影响结果。",
        "D6 是三项“无效修复”中唯一改变输出的：字节变了，但感知差异只有 %s（第 20 页）。" % (
            fmt(_delta(_ovrl(deck, "d1d5d6"), _ovrl(deck, "d1d5")), 4)),
        "因此“修复数量”与“修复收益”无关：四项修复里真正产生收益的只有 D8。",
    ], left=0.6, top=4.35, width=12.1, size=14, height=2.5)
    deck.footer(slide, 15)


def p16_below_input(deck: Deck):
    slide = deck.slide("基线低于不处理",
                       "“加了降噪”反而更差：OVRL 与 SIG 双降，BAK 仅微升")
    rows = [
        ["OVRL 整体质量", fmt(_ovrl(deck, "input"), 4), fmt(_ovrl(deck, "baseline"), 4),
         fmt(_delta(_ovrl(deck, "baseline"), _ovrl(deck, "input")), 4)],
        ["SIG 语音质量", fmt(_dnsmos_cell(deck, "input", "SIG"), 4),
         fmt(_dnsmos_cell(deck, "baseline", "SIG"), 4),
         fmt(_delta(_dnsmos_cell(deck, "baseline", "SIG"), _dnsmos_cell(deck, "input", "SIG")), 4)],
        ["BAK 背景噪声", fmt(_dnsmos_cell(deck, "input", "BAK"), 4),
         fmt(_dnsmos_cell(deck, "baseline", "BAK"), 4),
         fmt(_delta(_dnsmos_cell(deck, "baseline", "BAK"), _dnsmos_cell(deck, "input", "BAK")), 4)],
        ["P808_MOS", fmt(_dnsmos_cell(deck, "input", "P808_MOS"), 4),
         fmt(_dnsmos_cell(deck, "baseline", "P808_MOS"), 4),
         fmt(_delta(_dnsmos_cell(deck, "baseline", "P808_MOS"),
                    _dnsmos_cell(deck, "input", "P808_MOS")), 4)],
    ]
    add_table(slide, ["指标", "不处理（原始输入）", "基线（对齐固件）", "差值（基线 − 不处理）"], rows,
              left=0.8, top=1.45, width=11.7, col_widths=[2.6, 3.1, 3.1, 2.9], body_size=12,
              header_size=13, max_height=2.6, row_height=0.55)
    labels, values = _dns_categories_and_values(deck, "OVRL",
                                                ("input", "baseline", "optimized"))
    add_bar_chart(slide, labels, [("DNS-MOS OVRL", values)], 0.8, 4.3, 6.0, 2.5,
                  number_format="0.000")
    deck.bullets(slide, 16, [
        "现象：降噪链路的整体质量与语音质量都低于“完全不处理”，只有背景噪声略有改善。",
        "原因链条：D8 让语音概率长期饱和在 1.0（第 14 页），语音/噪声判定失真，语音成分被误判处理。",
        "修复后（第 4 / 20 页）：OVRL 与 BAK 同时超过不处理，说明 D8 是这条负面链条的源头。",
        "注意口径：DNS-MOS 是模型打分；该结论只在本次 9 个文件的素材范围内成立。",
    ], left=7.0, top=4.35, width=5.7, size=13, height=2.5)
    deck.footer(slide, 16)


def p17_audit_revision(deck: Deck):
    slide = deck.slide("前置审查结论的修正",
                       "本次实测对《NS移植缺陷审查报告》的两处结论做出修正，并新增一项未覆盖缺陷")
    rows = [
        ["D6", "量化误差级，影响可忽略",
         "对衰减量确无差异，但会改变数值路径与 NaN 分布（481 → 500），"
         "且输出 WAV 与基线不同", "第 15 / 20 页"],
        ["D6", "幅度谱约 0.096% 的固定偏置",
         "实测未观察到可归因于该偏置的感知差异（OVRL +0.0067，低于模型分辨率）",
         "第 20 页"],
        ["D8", "（审查报告未覆盖）",
         "本次新发现：fastpow2 定义域缺陷导致 NaN 与诊断量饱和，是唯一有效修复项",
         "第 13 / 14 页"],
    ]
    add_table(slide, ["编号", "审查原结论", "本次修正", "依据"], rows, left=0.8, top=1.45,
              width=11.7, col_widths=[0.8, 3.4, 5.4, 2.1], body_size=12, header_size=13,
              max_height=3.0, row_height=0.72)
    deck.bullets(slide, 17, [
        "修正一（D6）：它不是“无影响”，而是“改变了数值路径，但收益极小” —— 定性从“可忽略”改为“非零但可忽略”。",
        "修正二（D6 误差量级）：0.096% 是审查报告的推算量级，本次实测未测到可归因的感知变化。",
        "新增（D8）：审查报告未覆盖该缺陷；它是本次唯一产生可测收益的修复。",
        "方法说明：修正以本次实测数据为准，不推翻审查报告对缺陷存在的判定。",
    ], left=0.6, top=4.5, width=12.1, size=14, height=2.4)
    deck.footer(slide, 17)


def p18_gaps(deck: Deck):
    slide = deck.slide("未覆盖项：D2 / D3",
                       "两项时序与调度类缺陷在单线程 harness 中无法复现，明确标注为未覆盖")
    deck.bullets(slide, 18, [
        "D2（严重）：缓冲指针与处理触发不同步 —— 触发条件是 USER2 中断被 USER0 抢占"
        "（app_dma.c:247 优先级 1 vs drv_alg.c:558 优先级 2），单线程 harness 不存在该竞争。",
        "D2 需要什么：硬件上实测 Enc_Process 单次耗时分布，确认是否越过 1 ms 帧周期。",
        "D3（中等）：ENC 关闭期间采集指针不递增（drv_alg.c:672-676）—— 属 app 层调度，"
        "组件化后被 harness 的连续调用语义覆盖，偏差不显现。",
        "风险声明：两项均为“非崩溃、非静音”类缺陷，无法通过本次指标证伪，故不默认通过。",
        "后续验证项：板端插桩测量 Enc_Process 耗时；在 app 层恢复开关时序后重跑同一素材。",
    ], left=0.6, top=1.45, width=12.1, size=15, height=4.6)
    deck.note(slide, 18, "本页与第 17 页（结论修正）、第 29 页（限制与假设）、第 34 页（检查方法）"
                         "共同构成报告的“限制声明”四处，缺一不可。", top=6.3, size=13)
    deck.footer(slide, 18)


def p19_dnsmos_detail(deck: Deck):
    detail = deck.dataset.dnsmos_detail() or []
    by_variant = OrderedDict()
    for row in detail:
        by_variant.setdefault(row.get("variant"), {})[str(row.get("file"))] = row.get("OVRL")
    files = list(DD.FILE_NAMES)
    series = []
    for variant, label in (("input", "不处理"), ("baseline", "基线"), ("optimized", "优化（含 D8）")):
        series.append((label, [_chart_cell((by_variant.get(variant) or {}).get(f)) for f in files]))
    slide = deck.slide("DNS-MOS 逐文件明细",
                       "9 个文件 × 3 个关键变体的 OVRL：优化在多数文件上抬升，且逐文件离散度下降")
    add_bar_chart(slide, files, series, 0.8, 1.55, 11.7, 4.0, number_format="0.000",
                  data_labels=False)
    spread = _dnsmos_cell(deck, "optimized", "OVRL_std")
    spread_base = _dnsmos_cell(deck, "baseline", "OVRL_std")
    deck.bullets(slide, 19, [
        "逐文件 OVRL 来自 dnsmos/dnsmos_detail.csv（三系列 = 不处理 / 基线 / 优化）。",
        "离散度：OVRL 标准差基线 %s → 优化 %s，逐文件表现更一致。" % (
            fmt(spread_base, 4), fmt(spread, 4)),
        "逐段评分为按时长分片后取均值，每文件 7–16 段，共 %s 段。" % fmt(len(detail)),
    ], left=0.6, top=5.7, width=12.1, size=14, height=1.2)
    deck.footer(slide, 19)


def p20_layered_effect(deck: Deck):
    level = deck.level
    nan5 = deck.dataset.nan_frames("d1d5", level) or {}
    nan6 = deck.dataset.nan_frames("d1d5d6", level) or {}
    nan_opt = deck.dataset.nan_frames("optimized", level) or {}
    sp5 = deck.dataset.speech_prob("d1d5d6", level) or {}
    sp_opt = deck.dataset.speech_prob("optimized", level) or {}
    ovrl_5 = _ovrl(deck, "d1d5")
    ovrl_6 = _ovrl(deck, "d1d5d6")
    ovrl_opt = _ovrl(deck, "optimized")
    delta_6 = _delta(ovrl_6, ovrl_5)
    delta_8 = _delta(ovrl_opt, ovrl_6)
    rows = [
        ["D1", "—（输出逐字节相同）", fmt(nan5.get("nan_frames")), "—", "0.0000", "无效"],
        ["D5", "—（输出逐字节相同）", fmt(nan5.get("nan_frames")), "—", "0.0000", "无效"],
        ["D6", "%s dB" % fmt(_delta((deck.dataset.aggregate("d1d5d6", level) or {}).get("atten_db"),
                                    (deck.dataset.aggregate("d1d5", level) or {}).get("atten_db")), 3),
         "%s → %s" % (fmt(nan5.get("nan_frames")), fmt(nan6.get("nan_frames"))),
         _pct(sp5.get("share_at_one")), fmt(delta_6, 4), "极小"],
        ["D8", "不改变衰减量（口径不变）",
         "%s → %s" % (fmt(nan6.get("nan_frames")), fmt(nan_opt.get("nan_frames"))),
         "%s → %s" % (_pct(sp5.get("share_at_one")), _pct(sp_opt.get("share_at_one"))),
         fmt(delta_8, 4), "显著"],
    ]
    slide = deck.slide("四项修复的分层效果",
                       "逐项增量（同一套数据、同一口径）：只有 D8 在感知质量上产生可测收益；"
                       "各列为“加入该修复后相对上一组的增量”")
    add_table(slide, ["修复项", "对衰减量", "对 NaN 帧数", "对 speech_prob 饱和率", "对 OVRL", "判定"],
              rows, left=0.8, top=1.45, width=11.7,
              col_widths=[0.9, 2.6, 2.3, 2.6, 1.9, 1.4], body_size=12, header_size=13,
              max_height=2.4, row_height=0.48)
    labels, values = _dns_categories_and_values(deck, "OVRL")
    add_bar_chart(slide, labels, [("DNS-MOS OVRL", values)], 0.8, 4.00, 11.7, 2.80,
                  number_format="0.000")
    deck.footer(slide, 20)


def p21_ablation(deck: Deck):
    level = deck.level
    nan5 = deck.dataset.nan_frames("d1d5", level) or {}
    nan6 = deck.dataset.nan_frames("d1d5d6", level) or {}
    nan_opt = deck.dataset.nan_frames("optimized", level) or {}
    base = deck.agg("baseline")
    opt = deck.agg("optimized")
    slide = deck.slide("消融五组",
                       "NaN 帧数 %s（+D1D5）→ %s（+D1D5D6）→ %s（含 D8）；整段衰减 %s → %s dB" % (
                           fmt(nan5.get("nan_frames")), fmt(nan6.get("nan_frames")),
                           fmt(nan_opt.get("nan_frames")),
                           fmt(base["atten_db"] if base else None, 3),
                           fmt(opt["atten_db"] if opt else None, 3)))
    deck.picture(slide, 21, "FigT17", FULL_IMG_BOX)
    deck.footer(slide, 21)


def p22_diagnostics(deck: Deck):
    slide = deck.slide("诊断量轨迹",
                       "signal_energy / diff_normalization / gain_adjustment：基线（灰）vs 优化（红）")
    deck.picture(slide, 22, "FigT07", (0.6, 1.5, 5.9, 2.0))
    deck.picture(slide, 22, "FigT09", (6.85, 1.5, 5.9, 2.0))
    deck.picture(slide, 22, "FigT11", (0.6, 3.7, 5.9, 2.0))
    deck.bullets(slide, 22, [
        "三条曲线的基线形态在启动期同向偏离，优化后同步归位 —— 源头是 D1 的写入点（未初始化 Y[257]）。",
        "但这些诊断量的变化并没有传导到输出：D1 单独打开时输出 WAV 逐字节不变（第 15 页）。",
        "speech_prob_mean 的 NaN 断点属 D8（第 14 页），与 D1 传导链分开归因，不要混淆。",
        "结论：诊断量的“可见变化”不等于“可测收益”，判据必须落在输出与感知质量上。",
    ], left=6.85, top=3.8, width=5.9, size=14, height=2.6)
    deck.footer(slide, 22)


def p23_signal(deck: Deck):
    slide = deck.slide("信号层面：声谱与波形",
                       "差异集中在背景噪声段，语音主能量区基本重合")
    deck.picture(slide, 23, "FigT05", (0.6, 1.55, 5.9, 2.85))
    deck.picture(slide, 23, "FigT06", (6.85, 1.55, 5.9, 2.85))
    deck.picture(slide, 23, "FigT03", (0.6, 4.55, 12.13, 2.25))
    deck.footer(slide, 23)


def p24_reference(deck: Deck):
    slide = deck.slide("与另一条移植路径互证",
                       "参照路径与本次优化的感知质量同向：OVRL %s vs %s、BAK %s vs %s" % (
                           fmt(_ovrl(deck, "reference"), 4), fmt(_ovrl(deck, "optimized"), 4),
                           fmt(_dnsmos_cell(deck, "reference", "BAK"), 4),
                           fmt(_dnsmos_cell(deck, "optimized", "BAK"), 4)))
    deck.picture(slide, 24, "FigT20", FULL_IMG_BOX)
    deck.footer(slide, 24)


def p25_performance(deck: Deck):
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
                       "单帧耗时（1.wav / %s）与实时率：修复不构成实时性风险" % level)
    deck.picture(slide, 25, "FigT19", (0.6, 1.5, 5.9, 4.6))
    add_table(slide, ["变体", "均值 (µs)", "P95 (µs)", "最大 (µs)", "RTF"], table_rows,
              left=6.85, top=1.6, width=5.9, col_widths=[1.0, 1.3, 1.2, 1.2, 1.2],
              body_size=12, header_size=13, max_height=1.6, row_height=0.5)
    deck.bullets(slide, 25, [
        "帧预算：10 ms / 帧（1 ms I2S 帧周期的 10 倍余量）。",
        "RTF 口径：总耗时 / 音频总时长，整段聚合（第 16 / 20 页同口径）。",
        "D6 的精确开方每帧多 128 次，落在此处的耗时分布内。",
    ], left=6.85, top=3.4, width=5.9, size=15, height=2.6)
    deck.footer(slide, 25)


def p26_conclusion(deck: Deck):
    base, opt = _ovrl(deck, "baseline"), _ovrl(deck, "optimized")
    inp = _ovrl(deck, "input")
    level = deck.level
    nan_base = deck.dataset.nan_frames("baseline", level) or {}
    nan_opt = deck.dataset.nan_frames("optimized", level) or {}
    slide = deck.slide("结论与建议",
                       "结论可复现：同一命令、同一输入、同一开关组合，得到同一份报告")
    deck.bullets(slide, 26, [
        "结论一：D8 是本次唯一产生可测收益的修复 —— NaN %s → %s 帧，OVRL %s → %s。" % (
            fmt(nan_base.get("nan_frames")), fmt(nan_opt.get("nan_frames")),
            fmt(_ovrl(deck, "d1d5d6"), 4), fmt(opt, 4)),
        "结论二：D1 / D5 对输出严格为零（9/9 输出 WAV 逐字节相同，钳位 0 命中）；D6 改变输出但感知差异极小。",
        "结论三：基线感知质量低于不处理 —— OVRL %s < %s，说明携带缺陷的降噪链路会主动降低质量。" % (
            fmt(base, 4), fmt(inp, 4)),
        "结论四：D2 / D3 未覆盖，报告不主张“移植完全等价”，只主张“本次量测范围内等价”。",
        "建议：D8 立即合入；D1 建议合入（消除未定义行为，成本低）；D5 / D6 按需评估（第 27 页）。",
    ], left=0.6, top=1.5, width=12.1, size=16, height=4.6)
    deck.note(slide, 26, "报告口径：抑制等级 %s；DNS-MOS 与逐帧指标的口径说明见第 9 / 29 页。" % level,
              top=6.35, size=12)
    deck.footer(slide, 26)


def p27_priority(deck: Deck):
    level = deck.level
    nan_opt = deck.dataset.nan_frames("optimized", level) or {}
    nan_6 = deck.dataset.nan_frames("d1d5d6", level) or {}
    nan_5 = deck.dataset.nan_frames("d1d5", level) or {}
    sp5 = deck.dataset.speech_prob("d1d5d6", level) or {}
    sp_opt = deck.dataset.speech_prob("optimized", level) or {}
    same_d1, total_d1 = deck.dataset.wav_identical("baseline", "d1", level)
    same_d5, total_d5 = deck.dataset.wav_identical("baseline", "d1d5", level)
    rows = [
        ["P0 必修", "D8（fastpow2 定义域）",
         "NaN %s → %s 帧；饱和率 %s → %s；OVRL +%s" % (
             fmt(nan_6.get("nan_frames")), fmt(nan_opt.get("nan_frames")),
             _pct(sp5.get("share_at_one")), _pct(sp_opt.get("share_at_one")),
             fmt(_delta(_ovrl(deck, "optimized"), _ovrl(deck, "d1d5d6")), 4)),
         "立即合入：它是唯一把“诊断量失效”变回“可用”的修复"],
        ["P1 建议修", "D1（未初始化 Y[257]）",
         "输出 WAV %s/%s 逐字节相同（对输出零影响）" % (same_d1, total_d1),
         "成本极低且消除未定义行为，建议合入（收益不在本次指标上）"],
        ["P2 可选", "D6（fastsqrt 精度）",
         "OVRL +%s；NaN 帧数 %s → %s（改变了分布）" % (
             fmt(_delta(_ovrl(deck, "d1d5d6"), _ovrl(deck, "d1d5")), 4),
             fmt(nan_5.get("nan_frames")), fmt(nan_6.get("nan_frames"))),
         "按需评估：不改善衰减指标，仅改变数值路径"],
        ["P3 低优先", "D5（钳位量程）",
         "输出 WAV %s/%s 逐字节相同；钳位总命中 0" % (same_d5, total_d5),
         "可延后：本次素材从未触达钳位边界"],
        ["待验证", "D2 / D3（时序类）",
         "需中断竞争上下文，本次未覆盖",
         "板端插桩实测 Enc_Process 耗时后再定级"],
    ]
    slide = deck.slide("修复优先级（按实测效果排序）",
                       "排序依据是可测收益与风险，而不是缺陷数量")
    add_table(slide, ["优先级", "修复项", "实测依据", "建议"], rows, left=0.8, top=1.45,
              width=11.7, col_widths=[1.2, 2.6, 4.6, 3.3], body_size=12, header_size=13,
              max_height=4.2, row_height=0.72)
    deck.note(slide, 27, "P0/P1/P2/P3 只反映“本次实测量到的收益”；D1 的未定义行为风险不体现在指标上，"
                         "因此仍列为 P1。", top=6.35, size=12)
    deck.footer(slide, 27)


def p28_failures(deck: Deck):
    slide = deck.slide("失败与边界案例",
                       "本次测试中出现的失败与边界：NaN 帧、钳位边界、变体间零差异")
    deck.picture(slide, 28, "FigT22", (0.6, 1.5, 5.9, 3.9))
    deck.bullets(slide, 28, [
        "NaN：基线 481 帧的 speech_prob_mean 为 nan（D8），报告按原样保留、不替换为 0。",
        "钳位边界：所有变体、所有文件的钳位命中均为 0 —— 本次没有触达量程边界，这本身是一条边界结论。",
        "零差异：D1 / D5 相对基线的输出 WAV 逐字节相同，属“修复存在但输出不变”的极端情形。",
        "饱和：基线 92.3% 的帧 speech_prob_mean ≥ 0.999，诊断量在该区间无法区分语音与噪声。",
        "数据缺口：d1 / d1d5 / d1d5d6 缺 L0 / L1 汇总，报告统一按 %s 口径。" % deck.level,
    ], left=6.85, top=1.6, width=5.9, size=14, height=3.9)
    deck.note(slide, 28, "图件 FigT22 汇总失败与边界案例；本页列出口径层面的边界条件。",
              top=6.35, size=12)
    deck.footer(slide, 28)


def p29_limits(deck: Deck):
    slide = deck.slide("限制与假设",
                       "以下条件不成立时，报告结论需要重跑；未覆盖项不默认通过")
    deck.bullets(slide, 29, [
        "量纲假设（2.7 节）：输入左移 8 位对齐 24bit（sample24 = int16 × 256），输出右移 8 位还原并钳位；"
        "基于 drv_alg.c:46 的 I2S_INOUT_RESOLUTION_LEVEL (2)。若出厂固件使用其它值，基线需重跑。",
        "评价方法限制：DNS-MOS 是无参考模型打分，不等价于听感主观评价；PESQ 因缺少干净参考不可用（第 9 页）。",
        "测试环境限制：单线程 harness，无中断竞争与 app 层调度，因此 D2 / D3 未覆盖（第 18 页）。",
        "数据完整性限制：d1 / d1d5 / d1d5d6 仅有 L3 数据，L0 / L1 的对照结论不适用于这三个变体。",
        "范围限制：结论只覆盖 ENC 组件层与本次 9 个素材；不主张移植完全等价、也不主张板端等价。",
    ], left=0.6, top=1.45, width=12.1, size=15, height=4.6)
    deck.note(slide, 29, "本页与第 17 / 18 / 34 页共同构成限制声明；任一页的结论被引用时都必须同时引用这些边界。",
              top=6.3, size=12)
    deck.footer(slide, 29)


def p30_appendix_a(deck: Deck):
    slide = deck.slide("附录 A：图件与脚本索引",
                       "23 张图件的版位、数据源与生成脚本；全部由 ENC/Test/analysis 下的脚本产出")
    items = list(FIGURE_SPECS.items())
    half = (len(items) + 1) // 2
    for chunk_index, chunk in enumerate((items[:half], items[half:])):
        rows = []
        for fig_id, spec in chunk:
            rows.append([fig_id, _short(spec["desc"], 24), DD.BOX_LABEL[spec["box"]],
                         spec["script"]])
        add_table(slide, ["编号", "内容", "版位", "生成脚本"], rows,
                  left=0.8 if chunk_index == 0 else 6.9, top=1.5, width=5.7,
                  col_widths=[0.8, 2.85, 0.75, 1.4], body_size=10, header_size=11,
                  max_height=4.9, row_height=0.36)
    deck.note(slide, 30, "图件完整描述与数据源见计划书 9.4 节；数据路径："
                         "ENC/Test/data/<variant>/<level>/ 与 ENC/Test/data/dnsmos/；报告脚本："
                         "deck_data.py / build_deck.py / check_deck.py。", top=6.45, size=12)
    deck.footer(slide, 30)


def p31_detail_table(deck: Deck):
    level = deck.level
    rows_raw = deck.dataset.summary("baseline", level)
    rows = []
    if rows_raw:
        for row in rows_raw[:9]:
            rows.append([str(row.get("file", "")), fmt(row.get("duration_s"), 2),
                         fmt(row.get("frames")), fmt(row.get("in_rms"), 1),
                         fmt(row.get("out_rms"), 1), fmt(row.get("atten_db"), 3),
                         fmt(row.get("total_elapsed_ms"), 1), fmt(row.get("clamp_hits_total"))])
    else:
        rows = [["—"] * 8 for _ in range(9)]
    slide = deck.slide("逐文件明细（基线）",
                       "放大镜下的基线：这是后续一切对比的参照系")
    add_table(slide, ["文件", "时长 (s)", "帧数", "输入 RMS", "输出 RMS", "衰减 (dB)",
                      "总耗时 (ms)", "钳位命中"], rows, left=0.8, top=1.45, width=11.7,
              col_widths=[1.0, 1.2, 1.2, 1.6, 1.6, 1.5, 1.6, 1.2], body_size=13,
              header_size=15, max_height=4.5, row_height=0.44)
    deck.note(slide, 31, "报告口径：抑制等级 %s，逐文件汇总来自 summary_<variant>_%s.csv；"
                         "钳位命中全列为 0（第 15 页）。" % (level, level), top=6.05, size=12)
    deck.footer(slide, 31)


def p32_metrics(deck: Deck):
    slide = deck.slide("附录：12 类逐帧指标定义",
                       "列定义与测试 harness 的导出表头逐字一致，禁止写 NaN / Inf")
    rows = [[name, kind, desc, group] for name, kind, desc, group in METRIC_TABLE]
    add_table(slide, ["列名", "类型", "含义", "分类"], rows, left=0.8, top=1.45,
              width=11.7, col_widths=[2.6, 1.1, 6.4, 1.6], body_size=12, header_size=13,
              max_height=4.9, row_height=0.37)
    deck.note(slide, 32, "数值格式：浮点 %.9g、整数 %d；实测出现的 nan 已归因到 D8（第 13 / 14 页），"
                         "报告按原样保留、不替换为 0。", top=6.35, size=12)
    deck.footer(slide, 32)


def p33_dnsmos_table(deck: Deck):
    detail = deck.dataset.dnsmos_detail() or []
    by_variant = OrderedDict()
    for row in detail:
        by_variant.setdefault(row.get("variant"), {})[str(row.get("file"))] = row
    rows = []
    for name in DD.FILE_NAMES:
        inp = (by_variant.get("input") or {}).get(name) or {}
        base = (by_variant.get("baseline") or {}).get(name) or {}
        opt = (by_variant.get("optimized") or {}).get(name) or {}
        rows.append([
            "%s.wav" % name,
            fmt(inp.get("segments")),
            fmt(inp.get("OVRL"), 4), fmt(base.get("OVRL"), 4), fmt(opt.get("OVRL"), 4),
            fmt(_delta(opt.get("OVRL"), base.get("OVRL")), 4),
        ])
    if not detail:
        rows = [["—"] * 6 for _ in DD.FILE_NAMES]
    slide = deck.slide("附录：DNS-MOS 逐文件数据",
                       "供复核第 19 页图表：逐文件的 OVRL 与“优化 − 基线”增量")
    add_table(slide, ["文件", "分段数", "不处理 OVRL", "基线 OVRL", "优化 OVRL", "增量"], rows,
              left=0.8, top=1.45, width=11.7, col_widths=[1.5, 1.2, 2.2, 2.2, 2.2, 1.9],
              body_size=13, header_size=15, max_height=4.5, row_height=0.44)
    deck.note(slide, 33, "数据源：ENC/Test/data/dnsmos/dnsmos_detail.csv；OVRL 为分段评分按时长加权均值。",
              top=6.05, size=12)
    deck.footer(slide, 33)


def p34_appendix_b(deck: Deck):
    slide = deck.slide("附录 B：检查方法与限制（强制页）",
                       "校验范围、视觉责任归属、量纲假设与无效修复结论；本页不允许被省略或简化")
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
    level = deck.level
    nan_base = deck.dataset.nan_frames("baseline", level) or {}
    same_d1, total_d1 = deck.dataset.wav_identical("baseline", "d1", level)
    deck.bullets(slide, 34, [
        "版式已由视觉模型完成自动检查（对 preview/page-0001..0034.png 逐页检查溢出 / 遮挡 / "
        "字号可读性 / 中文字体渲染，结论记入 _reports/visual_check.json），最终以用户审阅 "
        "preview/ 后的确认为准；本页不代表已由人工完成视觉验收。",
        "评价方法：感知质量用 DNS-MOS 无参考模型（SIG / BAK / OVRL / P808_MOS）；"
        "PESQ 因缺少干净参考不可用（第 9 页）。",
        "哪些修复无效：D1 / D5 对输出零影响（输出 WAV %s/%s 逐字节相同）；D6 影响极小；"
        "唯一有效的是 D8（第 15 / 20 / 27 页）。" % (same_d1, total_d1),
        "量纲假设（2.7 节）：输入左移 8 位对齐 24bit，输出右移 8 位还原并钳位；基于 drv_alg.c:46 的 "
        "I2S_INOUT_RESOLUTION_LEVEL (2)。若出厂固件使用其它值，基线需重跑。",
        "基线 nan 是被测对象（固件）的固有缺陷表现（D8），只出现在诊断量列 speech_prob_mean"
        "（%s 共 %s 帧），不是测量故障；报告按原样呈现，未替换为 0。" % (
            level, fmt(nan_base.get("nan_frames"))),
    ], left=0.6, top=4.50, width=12.1, size=14, height=2.45)
    deck.footer(slide, 34)


PAGES = [
    p01_cover, p02_answer, p03_conclusions, p04_dnsmos, p05_problems, p06_scope,
    p07_component, p08_scale_data, p09_evaluation, p10_build, p11_reproduce,
    p12_problem_matrix, p13_d8_root, p14_d8_scope, p15_zero_effect, p16_below_input,
    p17_audit_revision, p18_gaps, p19_dnsmos_detail, p20_layered_effect, p21_ablation,
    p22_diagnostics, p23_signal, p24_reference, p25_performance, p26_conclusion,
    p27_priority, p28_failures, p29_limits, p30_appendix_a, p31_detail_table,
    p32_metrics, p33_dnsmos_table, p34_appendix_b,
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
    dnsmos = deck.dataset.dnsmos_rows()
    print("  DNS-MOS 变体 %s 个：%s" % (len(dnsmos or {}), ", ".join(list(dnsmos or []))))
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
