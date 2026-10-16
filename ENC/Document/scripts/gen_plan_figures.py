# -*- coding: utf-8 -*-
"""
NS 组件化与测试 - 并行开发计划图生成脚本 (Pillow)

运行: python gen_plan_figures.py
输出: ../figures/FigP01_并行任务依赖图.png
      ../figures/FigP02_组件架构.png
"""

import os

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
FIGDIR = os.path.normpath(os.path.join(HERE, "..", "figures"))
os.makedirs(FIGDIR, exist_ok=True)

FR = "C:/Windows/Fonts/msyh.ttc"
FB = "C:/Windows/Fonts/msyhbd.ttc"
_fc = {}


def F(size, bold=False):
    k = (size, bold)
    if k not in _fc:
        _fc[k] = ImageFont.truetype(FB if bold else FR, size)
    return _fc[k]


def tw(d, t, f):
    b = d.textbbox((0, 0), t, font=f)
    return b[2] - b[0], b[3] - b[1]


def title(d, t, sub, W):
    f = F(26, True)
    w, _ = tw(d, t, f)
    d.text(((W - w) / 2, 20), t, font=f, fill="#10243a")
    fs = F(15)
    ws, _ = tw(d, sub, fs)
    d.text(((W - ws) / 2, 56), sub, font=fs, fill="#5a6b7c")


def node(d, x, y, w, h, tid, name, sub, fill, edge, tc="#12314d"):
    d.rounded_rectangle([x, y, x + w, y + h], radius=9, fill=fill,
                        outline=edge, width=2)
    ft = F(15, True)
    fs = F(12)
    d.text((x + 10, y + 7), tid, font=ft, fill=edge)
    ty = y + 28
    for ln in name.split("\n"):
        d.text((x + 10, ty), ln, font=F(14, True), fill=tc)
        ty += 19
    ty += 2
    for ln in sub.split("\n"):
        d.text((x + 10, ty), ln, font=fs, fill="#4a5866")
        ty += 16


def arrow(d, pts, color="#2c6fa8", lw=2, head=9):
    import math
    for i in range(len(pts) - 1):
        d.line([pts[i], pts[i + 1]], fill=color, width=lw)
    (x1, y1), (x2, y2) = pts[-2], pts[-1]
    a = math.atan2(y2 - y1, x2 - x1)
    d.polygon([(x2, y2),
               (x2 - head * math.cos(a - 0.42), y2 - head * math.sin(a - 0.42)),
               (x2 - head * math.cos(a + 0.42), y2 - head * math.sin(a + 0.42))],
              fill=color)


# ================================================================ FigP01

def fig_plan():
    W, H = 2020, 940
    img = Image.new("RGB", (W, H), "#ffffff")
    d = ImageDraw.Draw(img)
    title(d, "FigP01  NS 组件化与测试 - 并行任务依赖图",
          "13 个任务节点: 串行节点 6 个 (P0/P2/P4/T9/P6) + 可并行工作包 7 个; "
          "Phase 1 四路并行, Phase 3 两路并行, Phase 5 两路并行", W)

    COLW = 238
    X0 = 34
    LANE = [186, 300, 414, 528]
    BH = 100
    BW = COLW - 26

    def cx(col):
        return X0 + col * COLW

    phases = [("Phase 0", "骨架"), ("Phase 1", "并行构建"), ("Phase 2", "基线"),
              ("Phase 3", "并行改"), ("Phase 4", "复测"), ("Phase 5", "并行出料"),
              ("Phase 6", "校验"), ("Phase 7", "交付")]
    for i, (p, t) in enumerate(phases):
        x = cx(i)
        d.rectangle([x, 96, x + COLW - 4, 132], fill="#dbe4ee",
                    outline="#8fa6bb", width=1)
        w1, _ = tw(d, p, F(14, True))
        d.text((x + (COLW - 4 - w1) / 2, 100), p, font=F(14, True), fill="#10243a")
        w2, _ = tw(d, t, F(12))
        d.text((x + (COLW - 4 - w2) / 2, 116), t, font=F(12), fill="#4a5866")

    GREEN = ("#eaf7ec", "#3d7f33")
    BLUE = ("#eaf2fb", "#2c6fa8")
    AMBER = ("#fff6e5", "#c98a12")
    RED = ("#fdecea", "#c0392b")
    GREY = ("#eef1f6", "#5a6b7c")
    PURPLE = ("#f3eefb", "#6b4fa8")

    # (col, lane, id, name, sub, color)
    nodes = [
        (0, 1, "P0", "骨架与契约", "目录 / API / CMake\n类型桩 / 契约冻结", BLUE),
        (1, 0, "T1", "CMSIS 兼容层", "arm_rfft_fast_f32\n复刻格式与缩放", GREEN),
        (1, 1, "T2", "算法源码摘取", "27 文件去平台化\n算法零改动", GREEN),
        (1, 2, "T3", "测试 harness", "WAV I/O / 帧循环\n量纲转换 / CSV", GREEN),
        (1, 3, "T4", "分析绘图管线", "指标计算\nPPT 素材图件", GREEN),
        (2, 1, "P2", "基线测量", "9 个音频\n改动前输出", AMBER),
        (3, 0, "T5", "修复 / 优化版", "D1 / D5 / D6\n消融开关", RED),
        (3, 2, "T6", "交叉参照构建", "Project/ENC_Core\nOoura 路径", GREY),
        (4, 1, "P4", "优化后测量", "9 个音频\n优化后输出", AMBER),
        (5, 2, "T7", "PPT 素材图件", "波形 / 声谱 / 统计\n差异图 20+ 张", GREEN),
        (5, 1, "T8", "演示文稿组装", "python-pptx\n34 页 / 16:9", PURPLE),
        (6, 1, "T9", "结构校验与预览", "check_office.py\nLibreOffice 渲染", GREEN),
        (7, 1, "P6", "验收与交付", "自检清单\n交付归档", BLUE),
    ]

    pos = {}
    for col, lane, tid, name, sub, col_ in nodes:
        x, y = cx(col), LANE[lane]
        node(d, x, y, BW, BH, tid, name, sub, col_[0], col_[1])
        pos[tid] = (x, y, x + BW, y + BH)

    def R(tid):
        return pos[tid]

    def link(a, b, color="#2c6fa8"):
        x1, y1, x2, y2 = R(a)
        x3, y3, x4, y4 = R(b)
        sx = x2
        sy = (y1 + y2) / 2
        ex = x3
        ey = (y3 + y4) / 2
        mid = (sx + ex) / 2
        arrow(d, [(sx, sy), (mid, sy), (mid, ey), (ex, ey)], color=color)

    link("P0", "T1")
    link("P0", "T2")
    link("P0", "T3")
    link("P0", "T4")
    link("T1", "P2")
    link("T2", "P2")
    link("T3", "P2")
    link("P2", "T5", "#c0392b")
    link("P2", "T6", "#7a7a7a")
    link("T5", "P4", "#c0392b")
    link("T6", "P4", "#7a7a7a")
    link("P4", "T7")
    link("P4", "T8")
    link("T4", "T7", "#3d7f33")
    link("T7", "T8", "#6b4fa8")
    link("T8", "T9", "#3d7f33")
    link("T9", "P6")

    # 图例
    ly = H - 96
    d.rounded_rectangle([34, ly, W - 34, ly + 70], radius=10, fill="#f7f9fb",
                        outline="#8fa6bb", width=1)
    items = [(BLUE, "串行节点"), (GREEN, "可并行 / 校验"),
             (AMBER, "测量节点"), (RED, "修复优化"),
             (PURPLE, "PPT 组装"), (GREY, "可降级跳过")]
    x = 60
    for c, t in items:
        d.rectangle([x, ly + 16, x + 16, ly + 32], fill=c[0], outline=c[1])
        d.text((x + 24, ly + 15), t, font=F(13), fill="#33414f")
        x += tw(d, t, F(13))[0] + 58
    d.text((60, ly + 42),
           "关键路径: P0 → T1/T2/T3 → P2 → T5 → P4 → T8 → T9 → P6。"
           "T4 与 T6 不在关键路径上, 可延迟或降级而不阻塞交付。",
           font=F(13), fill="#4a5866")

    img.save(os.path.join(FIGDIR, "FigP01_并行任务依赖图.png"))
    print("FigP01 ok")


# ================================================================ FigP02

def fig_arch():
    W, H = 1680, 860
    img = Image.new("RGB", (W, H), "#ffffff")
    d = ImageDraw.Draw(img)
    title(d, "FigP02  NS 组件架构与测试数据流",
          "摘取原则: 算法源码零改动, 平台依赖经 port 层隔离; "
          "修复项以编译期开关切换, 保证同一份源码产出基线与优化两个变体", W)

    # 左侧: 组件
    x0, y0, cw = 46, 120, 720
    d.rounded_rectangle([x0, y0, x0 + cw, y0 + 620], radius=12,
                        fill="#fbfcfd", outline="#2c6fa8", width=2)
    d.text((x0 + 16, y0 + 12), "ENC/NS_Component  (新建, 自包含)",
           font=F(17, True), fill="#10243a")

    rows = [
        ("api/ns_api.h", "InitNS / FreeNS / NS_Process_48kAudio\n与 ENC_V.h 同形, 增加变体与状态查询", "#eaf2fb", "#2c6fa8"),
        ("src/  (24 文件)", "noise_suppressor / wiener_filter / noise_estimator /\n"
                            "signal_model* / speech_* / quantile_* / histograms /\n"
                            "three_band_filter_bank / suppression_params / fast_math",
         "#eaf7ec", "#3d7f33"),
        ("src/ns_patch.h", "D1  Y[257] 补零\nD5  钳位与量纲绑定\nD6  fastsqrt → sqrtf (可选)",
         "#fdecea", "#c0392b"),
        ("port/arm_math_compat", "arm_rfft_fast_instance_f32\narm_rfft_fast_init_f32 / arm_rfft_fast_f32\n"
                                 "复刻 CMSIS 输入输出格式与 1/fftLen 缩放", "#fff6e5", "#c98a12"),
        ("port/platform_stub.h", "__RAM_CODE / __RAM3 / __ALIGNED → 空宏\n"
                                 "UserFunction.h / UserRetype.h (MIN MAX ABS) / marco.h", "#eef1f6", "#5a6b7c"),
        ("CMakeLists.txt", "静态库 ns_core (C99)\n两个配置: BASELINE / OPTIMIZED", "#eaf2fb", "#2c6fa8"),
    ]
    y = y0 + 46
    for name, desc, fill, edge in rows:
        h = 36 + desc.count("\n") * 18 + 18
        d.rounded_rectangle([x0 + 16, y, x0 + cw - 16, y + h], radius=8,
                            fill=fill, outline=edge, width=1)
        d.text((x0 + 28, y + 8), name, font=F(14, True), fill=edge)
        yy = y + 28
        for ln in desc.split("\n"):
            d.text((x0 + 28, yy), ln, font=F(12), fill="#3d4b59")
            yy += 18
        y += h + 8

    # 右侧: 测试
    rx0, ry0, rw = 830, 120, 804
    d.rounded_rectangle([rx0, ry0, rx0 + rw, ry0 + 620], radius=12,
                        fill="#fbfcfd", outline="#3d7f33", width=2)
    d.text((rx0 + 16, ry0 + 12), "ENC/Test  (新建, 测试与出料)",
           font=F(17, True), fill="#10243a")

    steps = [
        ("输入", "Test_Audio/AudioSample-48000hz/*.wav\n16bit / 2ch / 48kHz / 9 个文件 / 190 s"),
        ("量纲变换", "取左声道 → 采样值 << 8 → 24bit 量纲 (±8388608)\n与固件 I2S_INOUT_RESOLUTION_LEVEL=2 一致"),
        ("帧切分", "480 样本/帧; 帧数与固件 enc_in 块对齐"),
        ("被测对象", "ns_core_BASELINE  /  ns_core_OPTIMIZED\n(可选) Project/ENC_Core 交叉参照"),
        ("输出还原", ">> 8 → 钳位 → 16bit 单声道 WAV"),
        ("指标导出", "逐帧 RMS / 能量 / 耗时 / 钳位命中数\nmetrics_<variant>.csv"),
        ("分析与出料", "Python(Pillow): 波形 / 声谱 / 轨迹 / LTAS / 差异图 → figures/*.png\n"
                       "python-pptx: 34 页 16:9 报告 → .pptx\n"
                       "check_office + LibreOffice Kit → 校验报告 + preview/"),
    ]
    yy = ry0 + 46
    for i, (name, desc) in enumerate(steps):
        h = 34 + desc.count("\n") * 17 + 16
        d.rounded_rectangle([rx0 + 16, yy, rx0 + rw - 16, yy + h], radius=8,
                            fill="#eef7ee", outline="#7aa872", width=1)
        d.text((rx0 + 28, yy + 7), name, font=F(14, True), fill="#2f6b28")
        y2 = yy + 26
        for ln in desc.split("\n"):
            d.text((rx0 + 28, y2), ln, font=F(12), fill="#3d4b59")
            y2 += 17
        if i < len(steps) - 1:
            ax = rx0 + 40
            arrow(d, [(ax, yy + h), (ax, yy + h + 8)], color="#7aa872", lw=2)
        yy += h + 10

    d.text((46, H - 66),
           "数据流: 原始 WAV → (量纲) → harness → ns_core → (还原) → 输出 WAV + CSV → Python 图件 → PPT → 校验预览。",
           font=F(14), fill="#33414f")
    d.text((46, H - 42),
           "基线与优化版共用同一 harness 与同一输入, 唯一变量是编译期开关, 保证 A/B 对比只反映算法改动。",
           font=F(14), fill="#33414f")

    img.save(os.path.join(FIGDIR, "FigP02_组件架构.png"))
    print("FigP02 ok")


if __name__ == "__main__":
    fig_plan()
    fig_arch()
    print("plan figures ->", FIGDIR)
