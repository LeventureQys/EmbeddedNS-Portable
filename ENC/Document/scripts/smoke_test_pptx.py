# -*- coding: utf-8 -*-
"""
PPT 工具链冒烟测试

目的: 在计划冻结前验证 python-pptx 生成链路、结构校验脚本、LibreOffice 渲染
      三条路径在本环境可用, 覆盖后续测试报告需要的四类元素:
      中文文本 / 嵌入 PNG / 原生图表 / 原生表格。

运行:
    python smoke_test_pptx.py
输出:
    _smoke_out/smoke.pptx, _smoke_out/smoke_asset.png
"""

import os

from PIL import Image, ImageDraw, ImageFont
from pptx import Presentation
from pptx.chart.data import CategoryChartData
from pptx.enum.chart import XL_CHART_TYPE
from pptx.util import Inches, Pt

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "_smoke_out")
os.makedirs(OUT, exist_ok=True)

CN = "C:/Windows/Fonts/msyh.ttc"


def make_asset(path):
    """生成一张带中文标签的 900x625 素材图, 验证 Pillow 出图链路"""
    img = Image.new("RGB", (900, 625), "#ffffff")
    d = ImageDraw.Draw(img)
    f_title = ImageFont.truetype(CN, 28)
    f_axis = ImageFont.truetype(CN, 20)
    d.text((30, 24), "FigT00 冒烟素材 (900x625)", font=f_title, fill="#10243a")
    d.rectangle([80, 100, 860, 520], outline="#8fa6bb", width=2)
    for i, y in enumerate((520, 415, 310, 205, 100)):
        d.line([80, y, 860, y], fill="#e6ebf0", width=1)
        d.text((30, y - 10), "%.1f" % (i * 0.9), font=f_axis, fill="#5a6b7c")
    d.line([110, 470, 300, 300, 500, 360, 700, 150, 840, 130],
           fill="#c0392b", width=4, joint="curve")
    d.line([110, 490, 300, 330, 500, 390, 700, 190, 840, 170],
           fill="#7f8c8d", width=4, joint="curve")
    d.text((600, 480), "基线 / 优化", font=f_axis, fill="#33414f")
    img.save(path)


def main():
    asset = os.path.join(OUT, "smoke_asset.png")
    make_asset(asset)

    prs = Presentation()
    prs.slide_width = Inches(13.333)
    prs.slide_height = Inches(7.5)
    blank = prs.slide_layouts[6]

    # --- 第 1 页: 中文文本 + 内嵌图片 ---
    s1 = prs.slides.add_slide(blank)
    tb = s1.shapes.add_textbox(Inches(0.6), Inches(0.35), Inches(12),
                               Inches(0.9))
    r = tb.text_frame.paragraphs[0].add_run()
    r.text = "冒烟测试：中文标题与正文渲染"
    r.font.size = Pt(30)
    r.font.bold = True
    r.font.name = "微软雅黑"

    body = s1.shapes.add_textbox(Inches(0.6), Inches(1.3), Inches(5.0),
                                 Inches(2.4))
    tf = body.text_frame
    tf.text = "结构校验：ZIP/XML 完整性与内部关系"
    for t in ("渲染预览：LibreOffice Kit 导出 PDF 与关键页 PNG",
              "数值一致：原生图表数据须与源 CSV 对齐",
              "版式规范：16:9，图内最小字号满足投影可读"):
        tf.add_paragraph().text = t
    for p in tf.paragraphs:
        for run in p.runs:
            run.font.size = Pt(17)
            run.font.name = "微软雅黑"

    s1.shapes.add_picture(asset, Inches(5.9), Inches(1.3), width=Inches(6.8))

    # --- 第 2 页: 原生可编辑图表 ---
    s2 = prs.slides.add_slide(blank)
    t2 = s2.shapes.add_textbox(Inches(0.6), Inches(0.35), Inches(12),
                               Inches(0.8))
    r2 = t2.text_frame.paragraphs[0].add_run()
    r2.text = "原生图表：能量衰减对比"
    r2.font.size = Pt(28)
    r2.font.name = "微软雅黑"

    data = CategoryChartData()
    data.categories = ["1.wav", "2.wav", "3.wav", "4.wav", "5.wav"]
    data.add_series("基线", (0.85, 0.80, 0.87, 2.04, 1.21))
    data.add_series("优化", (1.24, 1.16, 1.29, 2.41, 1.63))
    gf = s2.shapes.add_chart(XL_CHART_TYPE.COLUMN_CLUSTERED, Inches(0.8),
                             Inches(1.4), Inches(11.7), Inches(5.4), data)
    chart = gf.chart
    chart.has_legend = True
    chart.has_title = False
    try:
        chart.plots[0].has_data_labels = True
        chart.plots[0].data_labels.number_format = "0.00"
        chart.plots[0].data_labels.number_format_is_linked = False
    except Exception as e:  # 数据标签在部分版本下需显式支持
        print("   [warn] data labels:", e)

    # --- 第 3 页: 原生表格 ---
    s3 = prs.slides.add_slide(blank)
    t3 = s3.shapes.add_textbox(Inches(0.6), Inches(0.35), Inches(12),
                               Inches(0.8))
    r3 = t3.text_frame.paragraphs[0].add_run()
    r3.text = "原生表格：逐文件明细"
    r3.font.size = Pt(28)
    r3.font.name = "微软雅黑"

    tbl = s3.shapes.add_table(4, 4, Inches(0.8), Inches(1.4), Inches(11.7),
                              Inches(3.2)).table
    hdr = ["文件", "时长 (s)", "基线衰减 (dB)", "优化衰减 (dB)"]
    body_rows = [["1.wav", "18.62", "0.85", "1.24"],
                 ["2.wav", "21.06", "0.80", "1.16"],
                 ["3.wav", "16.96", "0.87", "1.29"]]
    for j, h in enumerate(hdr):
        cell = tbl.cell(0, j)
        cell.text = h
        for p in cell.text_frame.paragraphs:
            for run in p.runs:
                run.font.size = Pt(15)
                run.font.bold = True
                run.font.name = "微软雅黑"
    for i, row in enumerate(body_rows, start=1):
        for j, v in enumerate(row):
            cell = tbl.cell(i, j)
            cell.text = v
            for p in cell.text_frame.paragraphs:
                for run in p.runs:
                    run.font.size = Pt(14)
                    run.font.name = "微软雅黑"

    # --- 第 4 页: 附录页（含检查方法说明，对应计划第 34 页的强制项）---
    s4 = prs.slides.add_slide(blank)
    t4 = s4.shapes.add_textbox(Inches(0.6), Inches(0.35), Inches(12),
                               Inches(0.8))
    r4 = t4.text_frame.paragraphs[0].add_run()
    r4.text = "附录 B：检查方法与限制"
    r4.font.size = Pt(28)
    r4.font.name = "微软雅黑"
    b4 = s4.shapes.add_textbox(Inches(0.6), Inches(1.4), Inches(12),
                               Inches(4.5))
    tf4 = b4.text_frame
    tf4.text = "本页为冒烟测试占位，正式报告中此处注明结构校验与视觉检查的责任归属。"
    for p in tf4.paragraphs:
        for run in p.runs:
            run.font.size = Pt(18)
            run.font.name = "微软雅黑"

    out = os.path.join(OUT, "smoke.pptx")
    prs.save(out)
    print("生成:", out)
    print("幻灯片数:", len(prs.slides.__iter__.__self__._sldIdLst))


if __name__ == "__main__":
    main()
