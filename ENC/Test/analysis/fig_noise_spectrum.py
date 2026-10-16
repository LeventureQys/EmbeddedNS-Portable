# -*- coding: utf-8 -*-
"""
FigT24 — 强噪声样本的幅频谱对比（背景噪声消去量）

为什么单独做这张图
------------------
报告原有的 FigT14（LTAS 对比）是**全段平均**谱，语音段能量远高于噪声段，
会把噪声段的抑制量淹没在语音能量里，看不出"噪声到底消了多少"。
本图改成**分段平均**：
  * 【噪声段】= `speech_prob_mean < 0.20` 的帧（算法的非语音判定）
  * 【语音段】= `speech_prob_mean >= 0.60` 的帧
分别求平均功率谱，从而把"噪声被压了多少"与"语音被伤了多少"分开来看。
两者的差值即为**选择性**（噪声衰减 − 语音衰减），这是降噪有效性的直接判据。

样本选择
--------
从 9 个测试文件中按「噪声段 RMS」与「SNR」筛出噪声最明显的两个：
  文件 4：噪声段 RMS 228459.6，SNR 6.21 dB，噪声底 107.2 dB
  文件 5：噪声段 RMS 173614.2，SNR 5.51 dB，噪声底 104.8 dB
（其余 7 个文件的 SNR 为 25.9–44.1 dB，噪声 RMS 低 2 个数量级）

方法
----
  * 帧长 480 样本（10 ms），**FFT 点数 = 480**，频率分辨率 100 Hz，输出 241 个频点（0–24 kHz）
  * 平均功率谱：对每帧 `|X|²/N²` 求算术平均后转 dB（标准 LTAS 口径）
  * **量纲统一**：原始输入（2ch，取左）与算法输出（1ch）**均按 int16 原值读入**，
    不做 ×256 —— 算法内部虽用 24bit 量纲，但输入输出两侧的换算正好抵消，
    在 16bit 量纲下三者可直接比较（编排者先前一次分析因只给输入乘 256 而得出
    不可能出现的「−55 dB 衰减」，已修正）

版式设计（v2 重做，逐条对应旧版的 9 处缺陷）
--------------------------------------------
① 旧版全局图例排在 `cv.top0 - 26`（y≈74），与左上子图标题（y≈86）逐字叠印。
   本版把纵向空间**显式分段预算**，四层结构自上而下互不侵占：
   图标题 → 上排图例条(y=62) → 上排子图标题(y=98) → 上排轴区(134–334) →
   上排刻度带(344–369) → 上排轴标题(中心 393.5) → 下排图例条(y=418) →
   下排子图标题(y=455) → 下排轴区(492–692) → 下排刻度带(702–727) →
   下排轴标题(中心 751.5) → 底部注释(798–829)。
   所有 y 坐标由 `LAYOUT_Y` 一处定义，改一处即整体一致。
② 旧版上排 x 刻度带（y 398–423）压住下排子图标题（y 386–426）、上排轴标题
   （y 434–462）落进下排轴区。本版两排之间留出 133 px（上排轴标题底 406 →
   下排子图标题顶 455），刻度带、轴标题、下排标题三者不再共享任何像素行。
③ 旧版柱体调用 `Axes.bars()`：该方法以**调用当时的 `ylim[0]`（即轴底）**为柱底，
   且第一次调用发生在 `set_ylim()` 之前（旧代码先 `bars()` 后 `set_ylim()`），
   于是两类柱基线相差 32 px、柱长与数值**反向**（衰减量小的语音段柱反而最长）。
   本版改用**矩形原语**：柱体一律从 `y = 0` 基线画到 `y = 数值`，
   柱长严格正比于 |衰减量|，负值柱向下延伸。
④ 数值标签在**全部柱体绘制完成之后**落笔（z 序最高），并加 3 px 白描边，
   负号绝不丢失；同组两柱中心距 69 px > 标签半宽 + 柱半宽，标签也不会被邻柱切掉。
⑤ 下排新增**自己的图例**（色块「噪声段衰减」/「语音段衰减」），居中排在下排标题
   之上、上排轴标题之下，与上排的线样图例在形式与位置上双重区分。
⑥ 基线与优化输出的逐频带差仅 0.00–0.20 dB（虚线被实线完全覆盖），本版不再声明
   第三条曲线：上排只画「原始输入 / 优化输出」两条，并在图内动态注明实测差值
   （0.20 dB 由数据算出，不是写死的文案），逐带差详值见下排柱状图。
⑦ 上下两排**各自共用同一 y 标尺**（下排取两样本最小衰减量、上排取两样本谱线的整体
   极值）：柱长与谱线高度是读者跨子图比较的直接依据，各按自身极值定标会让左右两个
   面板的比例尺相差 5%（谱图 20 dB = 57.3 px vs 54.3 px，0 线错开 13 px）到 20%
   （柱状图 12.8 px/dB vs 10.4 px/dB），文件 5 的衰减被压缩、幅度被低估。
⑧ 脚本末尾内置元素级重叠自检（`Canvas.overlaps`）与轴区越界自检，
   并把子图标题登记进包围盒表，避免"图内小标题不参与检测"的盲区。

版位：整页图 1800×845
独立运行：`python fig_noise_spectrum.py`
"""

from __future__ import annotations

import csv
import os
import sys
import wave

import numpy as np

import common as K
import fig_style as S
from fig_style import C

FRAME = 480
SR = 48000
NFFT = 480                          # 与帧长一致 → 分辨率 100 Hz，无需补零
FREQS = np.fft.rfftfreq(NFFT, 1.0 / SR)

SAMPLES = ["4", "5"]                # 噪声最明显的两个文件
NOISE_THR = 0.20                    # speech_prob_mean 低于此值判为非语音
SPEECH_THR = 0.60                   # 高于此值判为语音
BANDS = [(0, 4000), (4000, 8000), (8000, 12000), (12000, 16000), (16000, 24000)]
BAND_LABELS = ["0-4k", "4-8k", "8-12k", "12-16k", "16-24k"]

#: 上排：谱图的两条曲线（基线输出与其逐带差 ≤0.2 dB，画上去也看不见，
#: 故不单列，差值在底部注释里由实测值动态说明）
SPECTRA = (("原始输入", C.TEXT), ("优化输出", C.OPTIMIZED))

#: 下排：柱状图两个序列（颜色与 FigT13/FigT15 的语义色一致）
BAR_SERIES = (("噪声段衰减", C.POSITIVE), ("语音段衰减", C.WARN))

#: 纵向版式（px）。全部 y 坐标集中在此，缺陷①/② 的"逐层预算"由此保证：
#: 相邻两层之间至少留 6 px，任何两层都不共享像素行。
LAYOUT_Y = {
    "legend_top": 62.0,      # 上排图例条（文字顶边）
    "cap_top": 98.0,         # 上排子图标题顶边
    "ax1": (134.0, 334.0),   # 上排轴区
    "legend_bot": 418.0,     # 下排图例条（文字顶边）
    "cap_bot": 455.0,        # 下排子图标题顶边
    "ax2": (492.0, 692.0),   # 下排轴区
}

#: 下排柱状几何（数据坐标）
BAR_W = 0.30                # 柱宽（1 个类目 = 1.0）
BAR_OFF = 0.28              # 两序列中心距 = 2 × BAR_OFF
Y_TOP = 1.5                 # y 轴上界（数据全为负，0 线之上留一点白）
Y_BOT_PAD = 7.0             # y 轴下界 = 最小值 − 此值（给柱端数值标签留出空间）


def read_wav_int16(path):
    """按 int16 原值读单声道（多声道取第 0 声道）。"""
    with wave.open(path, "rb") as w:
        n, ch = w.getnframes(), w.getnchannels()
        raw = w.readframes(n)
    a = np.frombuffer(raw, dtype="<i2").astype(np.float64)
    return a[0::ch] if ch > 1 else a


def read_speech_prob(name, variant="baseline"):
    p = K.metrics_path(variant, K.PRIMARY_LEVEL, name)
    out = []
    with open(p, newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            t = row["speech_prob_mean"].strip().lower()
            out.append(float("nan") if t in ("nan", "inf", "-inf", "") else float(t))
    return np.array(out)


def power_spectrum(x, idx):
    """噪声段/语音段的平均功率谱，返回 dB 数组（241 点）。"""
    acc = np.zeros(NFFT // 2 + 1)
    n = 0
    for i in idx:
        seg = x[i * FRAME:(i + 1) * FRAME]
        if len(seg) < FRAME:
            continue
        X = np.fft.rfft(seg)
        acc += (X.real ** 2 + X.imag ** 2) / (FRAME ** 2)
        n += 1
    if n == 0:
        return None
    return 10.0 * np.log10(np.maximum(acc / n, 1e-30))


def band_db(psd):
    return np.array([10.0 * np.log10(np.mean(10.0 ** (psd[(FREQS >= lo) & (FREQS < hi)] / 10.0)))
                     for lo, hi in BANDS])


def load_sample(name):
    """返回 dict：三条信号的噪声段/语音段功率谱与频带值。"""
    spm = read_speech_prob(name)
    nfr = len(spm)
    noise = [i for i in range(nfr) if np.isfinite(spm[i]) and spm[i] < NOISE_THR]
    spch = [i for i in range(nfr) if np.isfinite(spm[i]) and spm[i] >= SPEECH_THR]
    if not noise or not spch:
        return None
    sigs = {
        "原始输入": read_wav_int16(os.path.join(K.input_audio_dir(), name + ".wav")),
        "基线输出": read_wav_int16(os.path.join(K.data_dir(), "baseline", K.PRIMARY_LEVEL, name + ".wav")),
        "优化输出": read_wav_int16(os.path.join(K.data_dir(), "optimized", K.PRIMARY_LEVEL, name + ".wav")),
    }
    out = {"name": name, "noise_frames": len(noise), "spch_frames": len(spch)}
    for seg, idxs in (("noise", noise), ("spch", spch)):
        for label, x in sigs.items():
            psd = power_spectrum(x, idxs)
            if psd is None:
                return None
            out[(seg, label)] = psd
            out[(seg, label, "band")] = band_db(psd)
    return out


# --------------------------------------------------------------------------
# 图内小标题登记（`S.caption()` 只落笔不登记包围盒 → 元素级重叠检测会漏掉
# "标题压刻度/压图例"这类缺陷，这里补登记）
# --------------------------------------------------------------------------

def put_caption(cv: S.Canvas, x: float, y: float, text: str, max_w: float,
                tag: str) -> None:
    f = S.font(S.FS_LABEL, True)
    w, h = S.text_size(cv.d, text, f)
    if w > max_w:
        print("[fig_noise_spectrum] 警告: 子图标题宽 %.0f px 超过面板宽 %.0f px，"
              "会被相邻面板压住: %s" % (w, max_w, text))
    S.caption(cv, x, y, text, color=C.TEXT, fnt=S.FS_LABEL)
    cv.box(S.EL_CAPTION, x, y, x + w, y + h, tag)


def strip_width(cv: S.Canvas, entries, fnt: int = S.FS_LABEL, gap: int = 26,
                sw: int = 40) -> float:
    """预算 `legend_strip` 的横向宽度（与其内部 `cx += sw + 12 + 文字宽 + gap` 同口径）。"""
    w = 0.0
    for k, (_kind, _color, txt) in enumerate(entries):
        w += sw + 12 + S.text_size(cv.d, str(txt), S.font(fnt))[0]
        if k < len(entries) - 1:
            w += gap
    return w


def put_strip(cv: S.Canvas, x: float, y: float, entries, tag: str) -> None:
    """`legend_strip` + 包围盒登记（同一原因）。"""
    right = S.legend_strip(cv, x, y, entries)
    _, h = S.text_size(cv.d, "汉", S.font(S.FS_LABEL))
    cv.box(S.EL_LEGEND, x, y, right, y + h, tag)


# --------------------------------------------------------------------------
# 下排柱状图：从 0 基线手工绘制矩形
# --------------------------------------------------------------------------

def draw_bars(ax: S.Axes, series, edges_cb=None) -> list[tuple]:
    """把 `series` 逐类目画成"0 基线 → 数值"的柱，再统一落数值标签。

    **为什么不用 `Axes.bars()`**：该方法以 `ylim[0]`（轴底）为柱底、且取**调用当时**
    的 ylim 换算像素；本图的衰减量全为负值，用它画出的柱长与数值大小反向
    （衰减量小的语音段反而最长），并因两次调用间 ylim 被改写而基线不齐。
    这里直接按数据坐标算像素矩形，语义与读者预期一致。

    返回标签包围盒列表，供越界自检。
    """
    zero_y = ax.py(0.0)
    n_band = len(series[0][1])
    boxes: list[tuple] = []
    # ---- 柱体（先画，压在网格线之上）----
    for k, (_label, vals, color) in enumerate(series):
        off = (2 * k - 1) * BAR_OFF
        for i in range(n_band):
            v = float(vals[i])
            cx = i + off
            xa = ax.px(cx - BAR_W / 2.0)
            xb = ax.px(cx + BAR_W / 2.0)
            yv = ax.py(v)
            ax.d.rectangle([xa, min(zero_y, yv), xb, max(zero_y, yv)], fill=color)
    # ---- 数值标签（后画 → z 序最高；白描边保证在柱体/网格之上完整可读）----
    for k, (_label, vals, _color) in enumerate(series):
        off = (2 * k - 1) * BAR_OFF
        for i in range(n_band):
            v = float(vals[i])
            cx = i + off
            txt = "%.1f" % v
            f = S.font(S.FS_MIN)
            w, h = S.text_size(ax.d, txt, f)
            xc = ax.px(cx)
            yv = ax.py(v) + 6.0             # 柱端外侧 6 px
            ax.d.text((xc - w / 2.0, yv), txt, font=f, fill=C.TEXT,
                      stroke_width=3, stroke_fill=C.BG)
            boxes.append((txt, xc - w / 2.0, yv, xc + w / 2.0, yv + h))
    return boxes


def fig_t24() -> str:
    data = []
    for name in SAMPLES:
        d = load_sample(name)
        if d is None:
            print("SKIP FigT24 文件 %s 的分档数据不足" % name)
            continue
        data.append(d)
    if not data:
        print("SKIP FigT24 无可用样本")
        return ""

    cv = S.Canvas("full", "FigT24", "强噪声样本的幅频谱对比：背景噪声消去量（噪声段 vs 语音段）")
    # 底部注释层收窄到 30 px：默认 100 px 会把 `panel(xlabel=...)` 的自动底线
    # （`y1_target`）抬到 643，把下排轴区（y1=692）反向压扁，两排高度就不等了。
    cv.set_bottom_reserve(30)

    y1a, y1b = LAYOUT_Y["ax1"]
    y2a, y2b = LAYOUT_Y["ax2"]
    top_rects = S.Axes.grid_rects((cv.pad_l, y1a, cv.W - cv.pad_r, y1b),
                                  nrow=1, ncol=2, gap_x=150)
    bot_rects = S.Axes.grid_rects((cv.pad_l, y2a, cv.W - cv.pad_r, y2b),
                                  nrow=1, ncol=2, gap_x=150)
    panel_w = top_rects[0][2] - top_rects[0][0]

    # ---- 上排图例条：独占「图标题之下、子图标题之上」的一行，与标题分离 ----
    put_strip(cv, cv.pad_l, LAYOUT_Y["legend_top"],
              [("line", col, lab) for lab, col in SPECTRA],
              "上排图例：原始输入、优化输出")

    # ---- 上排：噪声段平均功率谱 ----
    # 两个面板**共用同一 y 标尺**：并排子图的高低对比必须建立在同比例尺上
    # （各按自身 min/max 定标会让左右比例尺相差 5% 以上、0 线错开十余像素，
    # 读者交叉比较两条谱线的高低时会被误导）。定标必须**先于**落笔：`line()`
    # 用调用当时的 ylim 换算像素，范围不对会把曲线画到错的位置。
    spec_all = [np.concatenate([d[("noise", lab)] for lab, _c in SPECTRA]) for d in data]
    y_lo = float(min(float(a.min()) for a in spec_all)) - 4.0
    y_hi = float(max(float(a.max()) for a in spec_all)) + 6.0
    for i, d in enumerate(data):
        rect = top_rects[i]
        put_caption(cv, rect[0], LAYOUT_Y["cap_top"],
                    "文件 %s 噪声段平均功率谱（%d 帧非语音）" % (d["name"], d["noise_frames"]),
                    panel_w, "文件 %s 上排标题" % d["name"])
        ax = cv.panel(rect=rect, ylabel="功率谱 (dB)" if i == 0 else "",
                      xlabel="频率 (Hz)")
        ax.set_xlim(0.0, float(SR) / 2.0)
        ax.set_ylim(y_lo, y_hi)
        ax.spine()
        ax.grid(horizontal=True, vertical=False)
        for lab, col in SPECTRA:
            ax.line(FREQS, d[("noise", lab)], color=col, lw=S.LW_CURVE)
        ax.frame()

    # ---- 下排：各频带衰减量（柱长正比于 |衰减量|，从 0 基线起）----
    stats = []
    for d in data:
        stats.append((d,
                      d[("noise", "优化输出", "band")] - d[("noise", "原始输入", "band")],
                      d[("spch", "优化输出", "band")] - d[("spch", "原始输入", "band")]))
    # 两个面板**共用同一 y 标尺**：柱长是读者跨子图比较衰减幅度的直接依据，
    # 若各按自身 min 定标（左 12.8 px/dB、右 10.4 px/dB），文件 5 的柱会被压缩、
    # 衰减幅度被低估——同画布并排对比图必须共享标尺。
    y_bot = float(min(float(min(nb.min(), sb.min())) for _d, nb, sb in stats)) - Y_BOT_PAD

    # ---- 下排图例条：上排轴标题之下、下排子图标题之上（下排自己的图例）。
    #      **居中于整排**（左对齐会被读成上排图例的续行）。----
    leg2 = [("patch", col, lab) for lab, col in BAR_SERIES]
    put_strip(cv, (cv.pad_l + (cv.W - cv.pad_r) - strip_width(cv, leg2)) / 2.0,
              LAYOUT_Y["legend_bot"], leg2, "下排图例：噪声段衰减、语音段衰减")

    for i, (d, nb, sb) in enumerate(stats):
        rect = bot_rects[i]
        put_caption(cv, rect[0], LAYOUT_Y["cap_bot"],
                    "文件 %s · 衰减量与选择性（优化版相对原始输入）" % d["name"],
                    panel_w, "文件 %s 下排标题" % d["name"])
        ax = cv.panel(rect=rect, ylabel="衰减量 (dB)" if i == 0 else "",
                      xlabel="频带 (Hz)")
        ax.set_xlim(-0.6, len(BANDS) - 0.4)
        ax.set_ylim(y_bot, Y_TOP)
        ax.spine()
        ax.grid(horizontal=True, vertical=False, xlabels=BAND_LABELS)
        boxes = draw_bars(ax, ((BAR_SERIES[0][0], nb, BAR_SERIES[0][1]),
                               (BAR_SERIES[1][0], sb, BAR_SERIES[1][1])))
        ax.frame()
        # 越界自检：柱端标签必须整个落在轴区之内（缺陷④/⑦ 的客观判据）
        out = [(t, round(x0, 1), round(y0, 1), round(x1, 1), round(y1, 1))
               for t, x0, y0, x1, y1 in boxes
               if y1 > ax.y1 + 0.5 or y0 < ax.y0 - 0.5 or x0 < ax.x0 - 0.5 or x1 > ax.x1 + 0.5]
        print("  [自检] 文件 %s 下排：柱端标签 %d 个，越出轴区 %d 个%s"
              % (d["name"], len(boxes), len(out), ("：" + str(out[:4])) if out else ""))

    # ---- 控制台数值（与验收表逐位对齐；基线差值一并打印，供"两线重合"的判据）----
    dmax_base = 0.0
    for d, nb, sb in stats:
        bb = d[("noise", "基线输出", "band")] - d[("noise", "原始输入", "band")]
        bo = d[("spch", "基线输出", "band")] - d[("spch", "原始输入", "band")]
        dmax_base = max(dmax_base,
                        float(np.abs(bb - nb).max()), float(np.abs(bo - sb).max()))
        print("  文件 %s  噪声段衰减 均值 %+.2f dB  语音段衰减 均值 %+.2f dB  选择性 %+.2f dB"
              % (d["name"], nb.mean(), sb.mean(), (nb - sb).mean()))
        print("    逐频带 噪声段 %s dB" % " ".join("%+.1f" % v for v in nb))
        print("    逐频带 语音段 %s dB" % " ".join("%+.1f" % v for v in sb))
        print("    基线输出同口径（噪声段 %s / 语音段 %s），与优化输出逐带最大差 %.2f dB"
              % (" ".join("%+.1f" % v for v in bb), " ".join("%+.1f" % v for v in bo),
                 max(float(np.abs(bb - nb).max()), float(np.abs(bo - sb).max()))))

    # ---- 底部注释：文案随实测差值生成，并预检能否单行放下 ----
    max_w = cv.W - cv.pad_l - cv.pad_r
    notes = [
        "谱为噪声段帧平均（480 点 FFT）；柱为优化输出相对原始输入的带内电平差；"
        "基线输出与优化输出逐带差 ≤%.2f dB，故上排未单列。" % dmax_base,
        "谱为噪声段帧平均（480 点 FFT）；柱为优化输出相对原始输入的带内电平差；"
        "基线与优化输出逐带重合（≤%.2f dB），上排未单列。" % dmax_base,
        "柱为优化输出相对原始输入的带内电平差；基线与优化输出逐带重合，上排未单列。",
    ]
    for t in notes:
        if len(S.wrap_text(cv.d, t, S.font(S.FS_NOTE), max_w)) == 1:
            cv.note(t)
            break
        if len(S.wrap_text(cv.d, t, S.font(S.FS_MIN), max_w)) == 1:
            cv.note(t)
            break
    else:
        cv.note(notes[-1])

    # ---- 元素级重叠自检（阈值 150 px²，与 check_layout.py 的口径一致）----
    ov = cv.overlaps(min_area=150.0)
    print("  [自检] 元素包围盒重叠 %d 处%s"
          % (len(ov), "" if not ov else "："))
    for o in ov[:6]:
        print("     ✗ %s×%s 面积=%.0f px²  [%s] × [%s]"
              % (o["a"], o["b"], o["area"], o["a_label"], o["b_label"]))

    return cv.save("FigT24_强噪声样本幅频谱对比.png")


def main() -> int:
    S.warn_if_formal_output("fig_noise_spectrum.py")
    path = fig_t24()
    if path:
        print("[fig_noise_spectrum] 写出 %s" % path)
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
