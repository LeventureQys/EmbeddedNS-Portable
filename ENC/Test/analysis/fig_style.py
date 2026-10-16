# -*- coding: utf-8 -*-
"""
EmbeddedNS-Portable · 图件版式与绘图原语 (T4-I)

一切绘图使用 **Pillow**（环境无 matplotlib），numpy 仅做数值计算。

本模块冻结以下规范（计划书 9.2 / 9.3）
----------------------------------------
画布：整页 1800×845 / 半页 900×625 / 条带 1800×445
配色：主色 #2c6fa8，baseline #7f8c8d，optimized #c0392b，正向 #3d7f33，
      警示 #c98a12，网格 #e6ebf0，文字 #33414f，底色 #ffffff
字号：图标题 32 / 轴标题 28 / 图例 28 / 数据标签 24 / 刻度 24 / 图号 36
      **图内最小字号 24 px**
其他：白底不透明、单位写在轴标题内、右上角固定图号 `FigTxx`

对外导出
--------
`Canvas`            画布 + 标题 + 图号 + 保存
`Axes`              坐标轴（自动刻度/网格/线/点/柱/带/热力图/图例/表格/箱线）
`ColorMap`          热力图色带（顺序 / 发散）
`FontBox`           文本度量缓存

用法骨架::

    from fig_style import Canvas, C
    cv = Canvas("half", "FigT01", "九文件时长与能量衰减汇总")
    ax = cv.panel(rect=(cv.pad_l, cv.top0, cv.W - cv.pad_r, cv.H - cv.bottom0))
    ax.plot(x, y, color=C.BASELINE, label="baseline")
    ax.legend("tl")
    cv.save()
"""

from __future__ import annotations

import math
import os

import numpy as np
from PIL import Image, ImageDraw, ImageFont

import common as K

# --------------------------------------------------------------------------
# 规范常量
# --------------------------------------------------------------------------

#: 三种版位的画布尺寸（计划书 9.2）
LAYOUT_SIZE = {
    "full": (1800, 845),   # 整页图
    "half": (900, 625),    # 半页图
    "strip": (1800, 445),  # 条带图
}

#: 版位 → 幻灯片占位（英寸），供 /check_deck.py 交叉核对
LAYOUT_INCHES = {
    "full": (12.13, 5.70),
    "half": (5.90, 4.10),
    "strip": (12.13, 3.00),
}


class C:
    """配色（计划书 9.3）。属性名大写以便 `C.BASELINE` 直读。"""

    PRIMARY = "#2c6fa8"    # 主色 / 主链路
    BASELINE = "#7f8c8d"   # 基线 baseline
    OPTIMIZED = "#c0392b"  # 优化 optimized
    POSITIVE = "#3d7f33"   # 正向强调 / 一致
    WARN = "#c98a12"       # 警示 / 需注意
    GRID = "#e6ebf0"       # 网格线
    TEXT = "#33414f"       # 坐标轴 / 文字
    BG = "#ffffff"         # 底色

    #: 多序列时的补充色（仅在需要第 5+ 条曲线时使用，仍与主配色同族）
    EXTRA = ("#5b8fbf", "#8e6bb0", "#2f8f8a", "#a06a4a")

    #: 变体名 → 固定颜色（消融链保证跨图一致；顺序 = 消融推进顺序）
    VARIANT = {
        "baseline": BASELINE,
        "d1": "#5b8fbf",
        "d1d5": "#8e6bb0",
        "d1d5d6": "#2f8f8a",
        "optimized": OPTIMIZED,
        "原始": "#33414f",
        "reference": POSITIVE,
    }

    #: 变体显示名（PPT 中的中文标签）
    VARIANT_LABEL = {
        "baseline": "基线 baseline",
        "d1": "+D1",
        "d1d5": "+D1D5",
        "d1d5d6": "+D1D5D6",
        "optimized": "优化 optimized",
        "reference": "参照 ENC_Core",
    }

    #: NaN/Inf 帧的标记色（与警示色区分，用深红以示"缺陷"）
    NAN = "#8e2f2f"


#: 字体与字号
FONT_REG = "C:/Windows/Fonts/msyh.ttc"
FONT_BOLD = "C:/Windows/Fonts/msyhbd.ttc"

FS_TITLE = 32   # 图标题
FS_AXIS = 28    # 坐标轴标题
FS_LEGEND = 28  # 图例
FS_LABEL = 28   # 数据标签
FS_TICK = 28    # 刻度标签
FS_FIGID = 36   # 右上角图号
FS_NOTE = 28    # 图内注释
FS_MIN = 24     # 图内最小字号（硬约束下限，不得再小）

#: 线宽 / 图元尺寸（像素）
LW_AXIS = 2
LW_GRID = 1
LW_CURVE = 3
LW_AUX = 2
DASH = (10, 7)
MARKER = 5

#: 中文字体回退（msyh 缺失时的兜底，仅影响字形不影响尺寸约束）
_FALLBACKS = (
    "C:/Windows/Fonts/simhei.ttf",
    "C:/Windows/Fonts/simsun.ttc",
    "C:/Windows/Fonts/arial.ttf",
)

_font_cache: dict = {}
_font_warned = False


#: 元素类型（供重叠检测分类）
EL_TITLE = "title"        # 图标题
EL_FIGID = "figid"        # 右上角图号
EL_PLOT = "plot"          # 绘图区
EL_YLABEL = "ylabel"      # 纵轴标题
EL_XLABEL = "xlabel"      # 横轴标题
EL_XTICK = "xtick"        # x 刻度标签带
EL_YTICK = "ytick"        # y 刻度标签带
EL_LEGEND = "legend"      # 图例框
EL_NOTE = "note"          # 底部注释
EL_CAPTION = "caption"    # 图内小标题
EL_TABLE = "table"        # 表格
EL_COLORBAR = "colorbar"  # 色条

#: 必须两两不重叠的元素类型（"文字级"元素；绘图区与色条另按专门规则判定）
STRICT_TYPES = (EL_TITLE, EL_FIGID, EL_YLABEL, EL_XLABEL,
                EL_XTICK, EL_YTICK, EL_LEGEND, EL_NOTE, EL_CAPTION)


def font(size: int = FS_TICK, bold: bool = False) -> ImageFont.FreeTypeFont:
    """按字号加载中文字体（带缓存，线程无关的单进程缓存即可）。"""
    global _font_warned
    size = max(FS_MIN, int(size))
    key = (size, bool(bold))
    hit = _font_cache.get(key)
    if hit is not None:
        return hit
    for path in ((FONT_BOLD, FONT_REG) if bold else (FONT_REG, FONT_BOLD)) + _FALLBACKS:
        try:
            f = ImageFont.truetype(path, size)
            _font_cache[key] = f
            return f
        except OSError:
            continue
    if not _font_warned:
        print("[fig_style] 警告: 未能加载 msyh/回退字体，改用 Pillow 内置位图字体（中文可能显示为方块）")
        _font_warned = True
    f = ImageFont.load_default()
    _font_cache[key] = f
    return f


def text_size(draw: ImageDraw.ImageDraw, text: str, fnt) -> tuple[int, int]:
    """文本像素尺寸（不依赖锚点行为，跨 Pillow 版本稳定）。"""
    b = draw.textbbox((0, 0), text, font=fnt)
    return int(b[2] - b[0]), int(b[3] - b[1])


def draw_text_center(draw: ImageDraw.ImageDraw, x: float, y: float, text: str,
                     fnt, fill: str = C.TEXT) -> None:
    """以 `(x, y)` 为**中心**绘制文本。"""
    w, h = text_size(draw, text, fnt)
    draw.text((x - w / 2.0, y - h / 2.0), text, font=fnt, fill=fill)


def wrap_text(draw: ImageDraw.ImageDraw, text: str, fnt, max_w: float) -> list[str]:
    """把文本折成不超过 `max_w` 的行。

    中文逐字符断行；ASCII 单词（连续的 `[A-Za-z0-9._/+\\-]`）作为整体，超长时强制断开。
    """
    if max_w <= 20:
        return [str(text)]
    lines: list[str] = []
    for para in str(text).split("\n"):
        if not para:
            lines.append("")
            continue
        cur = ""
        token = ""
        for ch in para:
            if ch.isascii() and (ch.isalnum() or ch in "._/+-#%"):
                token += ch
                continue
            # 遇到非 ASCII 字符：先把待定单词并入当前行
            if token:
                if text_size(draw, cur + token, fnt)[0] <= max_w:
                    cur += token
                else:
                    if cur:
                        lines.append(cur)
                        cur = ""
                    t = token
                    while t and text_size(draw, t, fnt)[0] > max_w:
                        k = len(t)
                        while k > 1 and text_size(draw, t[:k], fnt)[0] > max_w:
                            k -= 1
                        lines.append(t[:k])
                        t = t[k:]
                    cur = t
                token = ""
            if text_size(draw, cur + ch, fnt)[0] <= max_w:
                cur += ch
            else:
                if cur:
                    lines.append(cur)
                cur = ch
        if token:
            if text_size(draw, cur + token, fnt)[0] <= max_w:
                cur += token
            else:
                if cur:
                    lines.append(cur)
                    cur = ""
                t = token
                while t and text_size(draw, t, fnt)[0] > max_w:
                    k = len(t)
                    while k > 1 and text_size(draw, t[:k], fnt)[0] > max_w:
                        k -= 1
                    lines.append(t[:k])
                    t = t[k:]
                cur = t
        lines.append(cur)
    return [ln for ln in lines] or [""]


# --------------------------------------------------------------------------
# 色带
# --------------------------------------------------------------------------

def hex2rgb(h: str) -> tuple[int, int, int]:
    h = h.lstrip("#")
    return int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16)


def lerp_rgb(a, b, t: float) -> tuple[int, int, int]:
    t = 0.0 if t < 0 else (1.0 if t > 1 else t)
    return (int(round(a[0] + (b[0] - a[0]) * t)),
            int(round(a[1] + (b[1] - a[1]) * t)),
            int(round(a[2] + (b[2] - a[2]) * t)))


def mix_white(h: str, t: float) -> tuple[int, int, int]:
    """把颜色朝白色插值 `t`（t=0 原色，t=1 纯白），用于生成浅色填充。"""
    return lerp_rgb(hex2rgb(h), (255, 255, 255), t)


class ColorMap:
    """256 级查找表（返回 `bytes`，直接喂给 `Image.frombytes`）。

    顺序色带骨架（暗→亮）:
        深蓝 #123a5c → 主色 #2c6fa8 → 青 #3f9ec4 → 绿 #7cc16b → 黄 #e8d05a → 白
    发散色带骨架（负→正）:
        蓝 #2c6fa8 → 浅灰 #f2f4f6 → 红 #c0392b
    """

    def __init__(self, stops, name: str = "map"):
        self.name = name
        xs = np.linspace(0.0, 1.0, 256)
        xs = xs * xs * (3.0 - 2.0 * xs)  # 轻微平滑，避免色带出现硬折
        cols = np.zeros((256, 3), dtype=np.uint8)
        n = len(stops) - 1
        for i, t in enumerate(xs):
            pos = float(t) * n
            k = min(int(pos), n - 1)
            frac = pos - k
            cols[i] = lerp_rgb(hex2rgb(stops[k]), hex2rgb(stops[k + 1]), frac)
        self.lut = cols

    def color(self, t: float) -> tuple[int, int, int]:
        i = int(round(max(0.0, min(1.0, t)) * 255.0))
        r, g, b = self.lut[i]
        return int(r), int(g), int(b)

    def apply(self, unit: np.ndarray) -> bytes:
        """`unit` ∈ [0,1] 的任意形状数组 → RGB 字节流（行优先）。"""
        idx = np.clip(np.rint(np.asarray(unit, dtype=np.float64) * 255.0), 0, 255).astype(np.uint8)
        rgb = self.lut[idx.reshape(-1)]
        return np.ascontiguousarray(rgb).tobytes()


SEQ = ColorMap(("#102a45", "#2c6fa8", "#4ba3c7", "#8ecf9a", "#eddc7a", "#ffffff"), "seq")
DIV = ColorMap(("#2c6fa8", "#8fb6d6", "#f2f4f6", "#e2a196", "#c0392b"), "div")


# --------------------------------------------------------------------------
# 通用刻度
# --------------------------------------------------------------------------

def nice_step(span: float, target: int = 6) -> float:
    """把区间分成 `target` 段附近、步长为 1/2/2.5/5/10 × 10^k 的整数刻度。"""
    if not np.isfinite(span) or span <= 0:
        return 1.0
    raw = span / max(1, int(target))
    mag = 10.0 ** math.floor(math.log10(raw))
    for m in (1.0, 2.0, 2.5, 5.0, 10.0):
        if raw <= m * mag:
            return m * mag
    return 10.0 * mag


def ticks_linear(lo: float, hi: float, target: int = 6) -> tuple[list[float], float]:
    """线性刻度值列表 + 步长。"""
    if not np.isfinite(lo) or not np.isfinite(hi) or hi <= lo:
        hi = lo + 1.0
    step = nice_step(hi - lo, target)
    start = math.floor(lo / step) * step
    stop = math.ceil(hi / step) * step
    n = int(round((stop - start) / step)) + 1
    if n > 64:  # 防御：极端跨度时收敛
        return [lo, hi], hi - lo
    vals = [start + i * step for i in range(n)]
    return vals, step


def ticks_log(lo: float, hi: float) -> list[float]:
    """对数刻度值列表（1/2/5 × 10^k，最少两个数量级）。"""
    lo = max(float(lo), 1e-12)
    hi = max(float(hi), lo * 10.0)
    e0 = math.floor(math.log10(lo))
    e1 = math.ceil(math.log10(hi))
    vals: list[float] = []
    for e in range(e0, e1 + 1):
        base = 10.0 ** e
        for m in (1.0, 2.0, 5.0):
            v = m * base
            if lo * 0.999 <= v <= hi * 1.001:
                vals.append(v)
    if len(vals) < 2:
        vals = [lo, hi]
    if len(vals) > 9:  # 太密则只保留 1×10^k
        vals = [v for v in vals if abs(math.log10(v) - round(math.log10(v))) < 1e-6] or vals
    return vals


def fmt_tick(v: float) -> str:
    """刻度标签格式化：短、无科学计数法噪声。"""
    a = abs(v)
    if a == 0:
        return "0"
    if a >= 1e6 or a < 1e-3:
        return "%.0e" % v
    if a >= 10000:
        # 频率轴常用：10000 → 10k
        return ("%gk" % (v / 1000.0)) if abs(v / 1000.0 - round(v / 1000.0)) < 1e-9 \
            else ("%.1fk" % (v / 1000.0))
    if a >= 1000:
        return "%.0f" % v
    if a >= 100:
        return "%.0f" % v
    if a >= 10:
        return ("%g" % round(v, 1))
    if a >= 1:
        return ("%g" % round(v, 2))
    return ("%g" % round(v, 3))


def fmt_val(v: float) -> str:
    """数据标签格式化。"""
    if not np.isfinite(v):
        return "—"
    a = abs(v)
    if a == 0:
        return "0"
    if a >= 1e5 or a < 1e-3:
        return "%.2g" % v
    if a >= 100:
        return "%.0f" % v
    if a >= 10:
        return "%.1f" % v
    if a >= 1:
        return "%.2f" % v
    return "%.3f" % v


def _fmt_tick_v(v: float) -> str:
    return fmt_tick(v)


# --------------------------------------------------------------------------
# 画布
# --------------------------------------------------------------------------

class Canvas:
    """一张图 = 画布 + 标题 + 图号 + 若干 `Axes`。"""

    def __init__(self, layout: str, fig_id: str, title: str):
        if layout not in LAYOUT_SIZE:
            raise ValueError("未知版位: %r（应为 %s）" % (layout, tuple(LAYOUT_SIZE)))
        self.layout = layout
        self.fig_id = fig_id
        self.title = title
        self.W, self.H = LAYOUT_SIZE[layout]
        self.img = Image.new("RGB", (self.W, self.H), C.BG)
        self.d = ImageDraw.Draw(self.img)
        #: 最近一次 `panel(xlabel=...)` 决定的轴标题中心 y（供 `note()` 对齐预算）
        self._xlabel_pos: int | None = None
        #: 最近一次 `panel(ylabel=...)` 决定的轴标题左 x
        self._ylabel_pos: int | None = None
        #: 已占用区域（注释避让用）：(x0, y0, x1, y1) 列表
        self._occupied: list[tuple[float, float, float, float]] = []
        #: 元素包围盒登记（重叠检测用）：[(type, x0, y0, x1, y1, label)]
        self._boxes: list[tuple] = []
        #: 「图例带」底边（轴区上界）；由 `set_legend_band()` 设定
        self._legend_band_bottom: int | None = None
        self._prep_margins()
        self._draw_header()

    # -- 元素登记与重叠检测 ----------------------------------------------
    def box(self, kind: str, x0: float, y0: float, x1: float, y1: float,
            label: str = "") -> None:
        """登记一个元素的包围盒（供 `overlaps()` 做元素级重叠检测）。"""
        self._boxes.append((kind, float(x0), float(y0), float(x1), float(y1), label))

    def overlaps(self, strict: tuple = STRICT_TYPES, min_area: float = 12.0) -> list[dict]:
        """检测已登记元素之间的包围盒交叠。

        仅比较 `strict` 中的元素类型两两之间；交叠面积 > `min_area`（px²）才计入，
        以容忍抗锯齿与相邻元素的亚像素贴合。返回冲突列表（按面积降序）。
        """
        items = [b for b in self._boxes if b[0] in strict]
        out: list[dict] = []
        for i in range(len(items)):
            k1, a0, b0, a1, b1, l1 = items[i]
            for j in range(i + 1, len(items)):
                k2, c0, d0, c1, d1, l2 = items[j]
                ox = min(a1, c1) - max(a0, c0)
                oy = min(b1, d1) - max(b0, d0)
                if ox > 0 and oy > 0 and ox * oy > min_area:
                    out.append({"a": k1, "b": k2, "a_label": l1, "b_label": l2,
                                "area": round(ox * oy, 1),
                                "box_a": [round(a0, 1), round(b0, 1), round(a1, 1), round(b1, 1)],
                                "box_b": [round(c0, 1), round(d0, 1), round(c1, 1), round(d1, 1)]})
        out.sort(key=lambda d: -d["area"])
        return out

    def boxes_json(self) -> list[dict]:
        """登记的元素包围盒序列化（写入 sidecar 供外部检测）。"""
        return [{"type": k, "x0": round(a, 1), "y0": round(b, 1),
                 "x1": round(c, 1), "y1": round(d, 1), "label": lb}
                for k, a, b, c, d, lb in self._boxes]

    # -- 占位管理 --------------------------------------------------------
    def occupy(self, box) -> None:
        """登记一块已绘制区域，供 `note()` 自动避让。"""
        self._occupied.append(tuple(float(v) for v in box))

    def text_bottom(self, kind: str) -> float | None:
        """返回指定类型元素已登记的底边 y（用于垂直预算串接）。"""
        vals = [b[4] for b in self._boxes if b[0] == kind]
        return max(vals) if vals else None

    def _free_for(self, y: int, total: int, x0: float | None = None,
                  x1: float | None = None) -> bool:
        """检查 `[y, y+total)` 在 x 区间内是否与已占区域冲突（含 6 px 间隙）。"""
        lo, hi = float(y) - 6, float(y + total) + 6
        bx0 = self.pad_l if x0 is None else float(x0)
        bx1 = (self.W - self.pad_r) if x1 is None else float(x1)
        for ox0, oy0, ox1, oy1 in self._occupied:
            if hi > oy0 and lo < oy1 and bx1 > ox0 and bx0 < ox1:
                return False
        return True

    # -- 版式 ------------------------------------------------------------
    def _prep_margins(self) -> None:
        """边距预算（像素）。

        `bottom0` 只留给「刻度标签 + 轴标题」之外的**注释层**——轴区自身在
        `panel(xlabel=...)` 时再向上让出 `_xlabel_reserve`（刻度带 + 轴标题高度），
        这样三层（轴区 / 刻度+轴标题 / 注释）互不侵占。
        """
        if self.layout == "strip":
            self.pad_l, self.pad_r = 258, 110
            self.top, self.bottom = 82, 106
        elif self.layout == "half":
            self.pad_l, self.pad_r = 214, 40
            self.top, self.bottom = 92, 96
        else:
            self.pad_l, self.pad_r = 258, 110
            self.top, self.bottom = 100, 100
        #: 绘图区可用高度（未扣除轴标题带）
        self.top0, self.bottom0 = self.top, self.bottom

    def _draw_header(self) -> None:
        """左上角图标题（32 粗体）+ 右上角图号（36 常规）。"""
        if self.title:
            ft = font(FS_TITLE, True)
            tw, th = text_size(self.d, self.title, ft)
            self.d.text((self.pad_l, 18), self.title, font=ft, fill=C.TEXT)
            self.box(EL_TITLE, self.pad_l, 16, self.pad_l + tw, 18 + th + 6, self.title[:24])
        ff = font(FS_FIGID, False)
        fw, fh = text_size(self.d, self.fig_id, ff)
        fx = self.W - self.pad_r - fw
        self.d.text((fx, 16), self.fig_id, font=ff, fill=C.PRIMARY)
        self.box(EL_FIGID, fx, 14, fx + fw, 16 + fh + 6, self.fig_id)

    # -- 绘图区 ----------------------------------------------------------
    def set_legend_band(self, rows: int = 2, fnt: int = FS_LEGEND) -> float:
        """在轴区上方预留「图例带」，返回该带的底边 y（应作为轴区上界）。

        图例放在轴区内**必然**遮挡曲线/柱体——放哪个角落都会压到数据，这是几何必然。
        因此单面板图统一把图例排到轴区之外、标题之下的一行里。

        `rows` 要按**最坏情况**给（默认 2 行）：图例项若干宽度不足而折行，框会变高；
        只按一行预留会让框向下溢出到轴区顶部，又变成遮挡。
        """
        _, h = text_size(self.d, "汉", font(fnt))
        rows = max(1, int(rows))
        self._legend_band_bottom = int(self.top0 + rows * (h + 8) + 16)
        return float(self._legend_band_bottom)

    def panel(self, rect=None, ylabel: str = "", xlabel: str = "",
              ylim=None, xlim=None, ylog: bool = False, xlog: bool = False,
              ytick_target: int = 0, xtick_target: int = 0) -> "Axes":
        """新建一个 `Axes`。

        `rect=(x0, y0, x1, y1)` 为**轴区域**像素范围（不含轴标题占位）。
        省略 `rect` 时占满整页可用区域。坐标轴标题占位由 `Axes` 自动从边缘挤入。
        """
        if rect is None:
            rect = (self.pad_l, self.top0, self.W - self.pad_r, self.H - self.bottom0)
        x0, y0, x1, y1 = rect
        if x0 >= x1 or y0 >= y1:
            raise ValueError("轴区域过小: %r" % (rect,))
        # 轴标题占位：把轴标题的绘制位置记录在 canvas 上，供 `Canvas.note()` 对齐预算，
        # 避免"轴标题与注释互相挤占"导致其中一方越出画布。
        if xlabel:
            # x 轴标题的位置**以画布底边为基准反推**。各层高度必须与绘制端一致：
            #   刻度标签带实际高度 = 10（`grid()` 的 y1+10 偏移）+ tick_h
            #   轴标题高度 = axis_h
            _, tick_h = text_size(self.d, "汉", font(FS_TICK))
            _, axis_h = text_size(self.d, "汉", font(FS_AXIS))
            band_h = tick_h + 12                     # 与 grid() 的偏移口径对齐
            label_cy = self.H - self.bottom0 - axis_h / 2.0 - 4
            y1_target = label_cy - axis_h / 2.0 - 10 - band_h
            # 仅在**不会造成倒置**时收紧 y1：多面板图传入的 rect 可能本就低于该底线
            # （各面板需共享同一底部结构），此时保持传入值，否则会把 y1 压到 y0 之下、
            # 生成倒置矩形（曾导致 FigT03 第三个面板落到第二个面板内部）。
            if y1 > y1_target and y1_target > y0 + 40:
                y1 = y1_target
            self._xlabel_pos = int(round(y1 + band_h + 10 + axis_h / 2.0))
            if not hasattr(self, "_xlabel_pos_list"):
                self._xlabel_pos_list = []
            self._xlabel_pos_list.append(self._xlabel_pos)
            self._xlabel_reserve = 0
        if ylabel:
            self._ylabel_pos = max(6, int(x0) - 106)
        if y0 >= y1:
            y0 = max(self.top0, y1 - 120.0)
        ax = Axes(self, (x0, y0, x1, y1), ylabel=ylabel, xlabel=xlabel,
                  ylim=ylim, xlim=xlim, ylog=ylog, xlog=xlog,
                  ytick_target=ytick_target, xtick_target=xtick_target)
        return ax

    # -- 输出 ------------------------------------------------------------
    def save(self, filename: str) -> str:
        """写入 `common.figures_dir()`（幂等覆盖）。

        同时写出同名 `.boxes.json` sidecar，记录各元素包围盒，供 `check_layout.py`
        做**元素级重叠检测**（画布级检查无法发现"图例压刻度"这类缺陷）。
        """
        d = K.ensure_figures_dir()
        path = os.path.join(d, filename)
        self.img.save(path, "PNG")
        try:
            import json
            side = os.path.splitext(path)[0] + ".boxes.json"
            with open(side, "w", encoding="utf-8") as fh:
                json.dump({"figure": self.fig_id, "layout": self.layout,
                           "size": [self.W, self.H], "boxes": self.boxes_json()},
                          fh, ensure_ascii=False, indent=1)
        except OSError:
            pass
        print("[fig_style] 写出 %s  (%d×%d, %d 字节)" %
              (path, self.W, self.H, os.path.getsize(path)))
        return path

    # -- 页脚注释 --------------------------------------------------------
    def note(self, text: str, y: int | None = None) -> None:
        """图底部注释（28 px），自动折行，并自动避让轴标题与其它已占区域。

        纵向约束（硬）：注释块必须**整体**落在
        `[轴标题下缘 + 8, 画布底 - 8]` 区间内，且不与任何已占区域相交。
        在满足约束的前提下**尽量贴底**；若空间不足则**自动略去**注释
        （改由标题/轴标题承载说明）——绝不叠印、绝不越出画布。
        """
        f = font(FS_NOTE)
        max_w = self.W - self.pad_l - self.pad_r
        lines = wrap_text(self.d, text, f, max_w)
        _, h = text_size(self.d, "汉", f)
        line_h = h + 6
        total = line_h * len(lines)

        hi_limit = self.H - 16                     # 留出字形下沿与抗锯齿余量
        lo_limit = 2                               # 注释块顶边的下限
        cur = self.text_bottom(EL_XLABEL) or self.text_bottom(EL_XTICK)
        if cur is not None:
            lo_limit = max(lo_limit, int(cur + 8))

        cands: list[int] = []
        if y is not None:
            cands.append(int(y))
        else:
            cands.append(hi_limit - total)         # 尽量贴底
            cands.append(lo_limit)                 # 退回轴标题正下方
        for cand in cands:
            if cand < lo_limit or cand + total > hi_limit:
                continue
            if not self._free_for(cand, total):
                continue
            for i, ln in enumerate(lines):
                self.d.text((self.pad_l, cand + i * line_h), ln, font=f, fill=C.TEXT)
            box = (float(self.pad_l), float(cand), float(self.W - self.pad_r),
                   float(cand + total))
            self.occupy(box)
            self.box(EL_NOTE, *box, "注释：%s" % text[:40])
            return
        avail = max(0, hi_limit - total - lo_limit)
        print("[fig_style] 提示: 底部注释放不下（需 %d px，可用 %d px），已略去注释"
              % (total, avail))


# --------------------------------------------------------------------------
# 坐标轴
# --------------------------------------------------------------------------

class Axes:
    """手绘坐标轴：自动刻度、网格、曲线、柱、带、热力图、图例、表格、箱线。"""

    def __init__(self, canvas: Canvas, rect, ylabel: str = "", xlabel: str = "",
                 ylim=None, xlim=None, ylog: bool = False, xlog: bool = False,
                 ytick_target: int = 0, xtick_target: int = 0):
        self.cv = canvas
        self.d = canvas.d
        self.rect = tuple(float(v) for v in rect)
        self.ylabel = ylabel
        self.xlabel = xlabel
        self.ylog = bool(ylog)
        self.xlog = bool(xlog)
        self._ylim = ylim
        self._xlim = xlim
        base_target = 4 if canvas.layout == "strip" else 6
        self.ytick_target = int(ytick_target or base_target)
        self.xtick_target = int(xtick_target or (9 if canvas.layout != "strip" else 12))

        #: 轴标题带（供热力图 / 表格精确摆放）
        self.x0, self.y0, self.x1, self.y1 = self.rect
        self.title_gap_y = 40 if self.ylabel else 0
        self.title_gap_x = 44 if self.xlabel else 0

        #: 已绘制图元范围（用于 `autorange`）
        self._data_bounds = None
        self._dirty = False
        self._xlim_fixed = xlim is not None
        self._ylim_fixed = ylim is not None
        self._legend_items: list[tuple] = []
        #: y 刻度标签的最大宽度（供 `ylabel` 横向定位与 `pad_l` 预算）
        self._tick_w = 0.0
        #: 各序列的**数据值**（供图例选位时估计数据密度，避免遮挡）
        self._data_paths: dict = {}

    # -- 尺寸 ------------------------------------------------------------
    @property
    def width(self) -> float:
        return self.x1 - self.x0

    @property
    def height(self) -> float:
        return self.y1 - self.y0

    @property
    def xlim(self) -> tuple[float, float]:
        if self._xlim is None:
            self.autoscale(x_only=True)
        return self._xlim if self._xlim is not None else (0.0, 1.0)

    @property
    def ylim(self) -> tuple[float, float]:
        if self._ylim is None:
            self.autoscale(y_only=True)
        return self._ylim if self._ylim is not None else (0.0, 1.0)

    def set_range(self, xlim=None, ylim=None) -> None:
        """显式定标（覆盖自动定标，且不被后续数据改写）。"""
        if xlim is not None:
            self._xlim = (float(xlim[0]), float(xlim[1]))
            self._xlim_fixed = True
        if ylim is not None:
            self._ylim = (float(ylim[0]), float(ylim[1]))
            self._ylim_fixed = True
        self._fix_inverted()

    def set_xlim(self, lo, hi) -> None:
        """显式设定 x 范围（此后不再被自动定标改写）。"""
        self._xlim = (float(lo), float(hi))
        self._xlim_fixed = True
        self._fix_inverted()

    def set_ylim(self, lo, hi) -> None:
        """显式设定 y 范围（此后不再被自动定标改写）。"""
        self._ylim = (float(lo), float(hi))
        self._ylim_fixed = True
        self._fix_inverted()

    def reset_data(self, keep_explicit: bool = True) -> None:
        """清空已登记的数据范围。

        `keep_explicit=False` 时同时解除显式定标，使 `autoscale()` 可完全重算
        （用于「先画一部分数据、中途增删序列」的复杂布局）。
        """
        self._data_bounds = None
        self._dirty = True
        if not keep_explicit:
            self._xlim = self._ylim = None
            self._xlim_fixed = self._ylim_fixed = False

    def _include(self, x, y) -> None:
        """登记数据范围（供 `autoscale`），并把轴标记为"需要重算"。"""
        xa = np.asarray(x, dtype=np.float64).ravel()
        ya = np.asarray(y, dtype=np.float64).ravel()
        if xa.size == 0 or ya.size == 0:
            return
        nx = xa.size
        if ya.size != nx:
            # 允许"少量 y 对多个 x"或反过来的广播（例：boxplot 传 2 个 x 端点、
            # 5 个 y 统计量）。长度不整除时按需重复，绝不因尺寸不匹配而放弃登记——
            # 那会导致 `ylim` 退化成先验范围，把图元画到画布之外。
            if ya.size == 1:
                ya = np.full(nx, float(ya[0]))
            elif nx % ya.size == 0 or ya.size > nx:
                ya = np.resize(ya, nx)
            else:
                n = min(nx, ya.size)
                xa, ya = xa[:n], ya[:n]
        m = np.isfinite(xa) & np.isfinite(ya)
        if not np.any(m):
            return
        xa, ya = xa[m], ya[m]
        lo_x, hi_x = float(xa.min()), float(xa.max())
        lo_y, hi_y = float(ya.min()), float(ya.max())
        if self.xlog:
            pos = xa[xa > 0]
            if pos.size:
                lo_x, hi_x = float(pos.min()), float(pos.max())
            else:
                lo_x, hi_x = 1e-9, 1.0
        if self.ylog:
            pos = ya[ya > 0]
            if pos.size:
                lo_y, hi_y = float(pos.min()), float(pos.max())
            else:
                lo_y, hi_y = 1e-9, 1.0
        if self._data_bounds is None:
            self._data_bounds = [lo_x, hi_x, lo_y, hi_y]
        else:
            b = self._data_bounds
            b[0] = min(b[0], lo_x)
            b[1] = max(b[1], hi_x)
            b[2] = min(b[2], lo_y)
            b[3] = max(b[3], hi_y)
        self._dirty = True

    def _autogrow(self) -> None:
        """若刚登记的数据超出当前定标范围，立即重算（只在已定标的前提下）。

        必须用内部状态判断，不可用 `self.xlim/ylim`（那会在未定标时触发先验定标）。
        这是"多条序列依次绘制"场景的关键：第 2..n 条序列的坐标换算必须建立在
        包含它们自身的数据范围之上，否则会被画到画布之外。
        """
        if self._data_bounds is None:
            return
        grow_x = (not self._xlim_fixed) and self._xlim is not None
        grow_y = (not self._ylim_fixed) and self._ylim is not None
        if not (grow_x or grow_y):
            return
        b = self._data_bounds
        need = False
        if grow_x:
            lo, hi = self._xlim
            if b[0] < lo - abs(lo) * 1e-9 or b[1] > hi + abs(hi) * 1e-9:
                need = True
        if grow_y:
            lo, hi = self._ylim
            if b[2] < lo - abs(lo) * 1e-9 or b[3] > hi + abs(hi) * 1e-9:
                need = True
        if need:
            self._dirty = True
            self.autoscale()

    def autoscale(self, pad: float = 0.06, x_only: bool = False, y_only: bool = False) -> None:
        """按已登记数据自动定标。

        重算条件：某维度未被**显式设定**（`set_xlim`/`set_ylim`），且其范围为空
        或自上次定标后有**新数据登记**（`_dirty`）。

        这一点很关键：`ax.line()` 的坐标换算会读取 `ylim`，若首次调用即定标，
        范围会被冻结在"只有第一条序列"的区间——后续序列将被推出画布。
        """
        b = self._data_bounds
        if b is None:
            if self._xlim is None:
                self._xlim = (0.0, 1.0)
            if self._ylim is None:
                self._ylim = (1e-9, 1.0) if self.ylog else (0.0, 1.0)
            self._fix_inverted()
            return
        if not y_only and not self._xlim_fixed and (self._xlim is None or self._dirty):
            if self.xlog:
                lo = max(b[0], 1e-9)
                hi = max(b[1], lo * 10.0)
                self._xlim = (lo / 1.6, hi * 1.6)
            else:
                span = b[1] - b[0]
                if span <= 0:
                    span = 1.0 if b[0] == 0 else abs(b[0]) * 0.1
                self._xlim = (b[0] - span * pad * 0.5, b[1] + span * pad * 0.5)
        if not x_only and not self._ylim_fixed and (self._ylim is None or self._dirty):
            if self.ylog:
                lo = max(b[2], 1e-9)
                hi = max(b[3], lo * 10.0)
                self._ylim = (lo / 2.0, hi * 2.0)
            else:
                span = b[3] - b[2]
                if span <= 0:
                    span = 1.0 if b[2] == 0 else abs(b[2]) * 0.2
                self._ylim = (b[2] - span * pad, b[3] + span * pad)
        self._dirty = False
        self._fix_inverted()

    def _fix_inverted(self) -> None:
        for attr in ("_xlim", "_ylim"):
            lim = getattr(self, attr)
            if lim is not None and lim[1] <= lim[0]:
                setattr(self, attr, (lim[0], lim[0] + 1.0))

    # -- 坐标变换 --------------------------------------------------------
    def px(self, x: float) -> float:
        lo, hi = self.xlim
        x = float(x)
        if self.xlog:
            if not np.isfinite(x) or x <= 0:
                return self.x0 - 1_000_000.0  # 屏幕外
            x = math.log10(x)
            lo, hi = math.log10(max(lo, 1e-12)), math.log10(max(hi, lo * 10))
        return self.x0 + (x - lo) / (hi - lo) * self.width

    def py(self, y: float) -> float:
        lo, hi = self.ylim
        y = float(y)
        if self.ylog:
            if not np.isfinite(y) or y <= 0:
                return self.y1 + 1_000_000.0  # 屏幕外
            y = math.log10(y)
            lo, hi = math.log10(max(lo, 1e-12)), math.log10(max(hi, lo * 10))
        return self.y1 - (y - lo) / (hi - lo) * self.height

    def _tick_values_y(self) -> list[float]:
        if self._ylim is None:
            return []
        if self.ylog:
            return ticks_log(self._ylim[0], self._ylim[1])
        want = max(2, self.ytick_target)
        return ticks_linear(self._ylim[0], self._ylim[1], want)[0]

    def _tick_values_x(self) -> list[float]:
        if self._xlim is None:
            return []
        if self.xlog:
            return ticks_log(self._xlim[0], self._xlim[1])
        lo, hi = self._xlim
        span = hi - lo
        # 类目轴（整数 + 跨度小）：逐个整数标注，步长按可用宽度自适应
        if span <= 12 and float(lo).is_integer() and (span - int(span)) < 1e-9:
            ilo, ihi = int(round(lo)), int(round(hi))
            n = ihi - ilo + 1
            step = 1
            # 标签粘连防护：按**实际字宽**估算每条可占像素，不够就加大步长
            room = self.width / max(1, n)
            need = max(
                text_size(self.d, s, font(FS_TICK))[0]
                for s in ("0000", "10000", "-0000")) + 16
            while room * step < need and step < n:
                step += 1
            return [float(v) for v in range(ilo, ihi + 1, step)]
        # 数值轴：刻度数按可用像素自适应（每刻度至少 96 px），避免数字串连
        want = max(2, min(int(self.xtick_target), max(2, int(self.width / 96.0))))
        return ticks_linear(lo, hi, want)[0]

    # -- 底层：网格 + 坐标轴 --------------------------------------------
    def grid(self, vertical: bool = True, horizontal: bool = True,
             xlabels: list | None = None) -> None:
        """绘制网格与刻度标签。

        **调用时机：数据绘制完成、`autoscale()` 之后**——先把曲线/柱体登记进范围，
        再定标，最后铺网格，刻度才不会与数据错位。
        `xlabels` 提供时按**等距类目**方式标注 x 轴（柱状图场景）：
        值 i（0..n-1）映射到第 i 个标签。
        """
        if self._xlim is None or self._ylim is None:
            # 尚无数据：按当前先验范围绘制（避免抛异常打断出图）
            self.autoscale()
        f = font(FS_TICK)
        # 水平网格 + y 刻度标签（整条刻度带合并登记，供重叠检测与 ylabel 定位）
        ytick_x0, ytick_x1 = self.x0, self.x0 - 12.0
        ytick_top, ytick_bot = self.y0, self.y1
        ytick_w = 0.0
        for v in self._tick_values_y():
            yp = self.py(v)
            if not (self.y0 - 1 <= yp <= self.y1 + 1):
                continue
            if horizontal:
                self.d.line([(self.x0, yp), (self.x1, yp)], fill=C.GRID, width=LW_GRID)
            txt = fmt_tick(v)
            w, h = text_size(self.d, txt, f)
            self.d.text((self.x0 - 12 - w, yp - h / 2.0), txt, font=f, fill=C.TEXT)
            ytick_w = max(ytick_w, float(w))
            ytick_x0 = min(ytick_x0, self.x0 - 12 - w)
            ytick_top = min(ytick_top, yp - h / 2.0)
            ytick_bot = max(ytick_bot, yp + h / 2.0)
        self._tick_w = ytick_w
        if ytick_bot > ytick_top:
            self.cv.box(EL_YTICK, ytick_x0, ytick_top, self.x0 - 4, ytick_bot,
                        "y 刻度标签带（最大宽 %.0f px）" % ytick_w)
        # 垂直网格 + x 刻度标签
        xtick_x0, xtick_x1 = self.x0, self.x1
        xtick_top, xtick_bot = self.y1 + 4, self.y1 + 4
        xtick_bbox = None
        for v in self._tick_values_x():
            xp = self.px(v)
            if not (self.x0 - 1 <= xp <= self.x1 + 1):
                continue
            if vertical:
                self.d.line([(xp, self.y0), (xp, self.y1)], fill=C.GRID, width=LW_GRID)
            label = self._label_for(v, xlabels)
            if label is None:
                continue
            w, h = text_size(self.d, label, f)
            self.d.text((xp - w / 2.0, self.y1 + 10), label, font=f, fill=C.TEXT)
            xtick_x0 = min(xtick_x0, xp - w / 2.0)
            xtick_x1 = max(xtick_x1, xp + w / 2.0)
            xtick_top = min(xtick_top, self.y1 + 10)
            xtick_bot = max(xtick_bot, self.y1 + 10 + h)
            if xtick_bbox is None:
                xtick_bbox = [xp - w / 2.0, xp + w / 2.0]
        if xtick_bot > xtick_top:
            # x 刻度带按轴区宽度裁剪：类目标签可能很宽（如「优化 optimized」），
            # 不裁剪会让登记的包围盒伸出轴区，干扰重叠检测与留白判定。
            self.cv.box(EL_XTICK, max(0.0, min(xtick_x0, self.x0)),
                        xtick_top, min(self.x1, max(xtick_x1, self.x1)),
                        xtick_bot, "x 刻度标签带")

    def _label_for(self, v: float, xlabels: list | None) -> str | None:
        if xlabels is None:
            return fmt_tick(v)
        idx = int(round(v))
        if idx < 0 or idx >= len(xlabels):
            return None
        return str(xlabels[idx])

    def frame(self) -> None:
        """坐标轴标题 + 轴线。

        调用时机：**数据绘制完成之后**（此时自动定标已确定，`py()` 才可信）；
        轴线另用 `spine()` 在数据之后补画。
        """
        if self.ylabel:
            # ylabel 的横向落点必须按**实际度量**得出，不能用固定留白：
            # 刻度文本可能是 `-24000` / `4.6e14`（宽），也可能是 `0`（窄）。
            # 可用宽度 = 轴区左界到画布左缘，减去刻度标签带与间隙；
            # 若竖排所需宽度放不下，逐级缩字号（下限 FS_MIN）；仍放不下则水平贴在轴外。
            _, tick_h = text_size(self.d, "汉", font(FS_TICK))
            tick_gap = self._tick_w + 26.0
            avail = self.x0 - tick_gap - 8.0
            tw_, th_ = text_size(self.d, self.ylabel, font(FS_AXIS))
            need = float(th_ + 8)          # 竖排后横向占用 = 原文本高度
            # 竖排：所需横向宽度 = 文本高度。文本过长时按**可用轴高截断**（加 `…`），
            # 而不是改成水平排版——水平排在轴外会压住刻度标签、甚至贴到画布左缘。
            max_text_w = max(60.0, (self.y1 - self.y0) - 6.0)
            label = self.ylabel
            f = font(FS_AXIS)
            while text_size(self.d, label, f)[0] > max_text_w and len(label) > 2:
                label = label[:-2] + "…"
            tw_, th_ = text_size(self.d, label, f)
            img = Image.new("RGB", (max(8, tw_ + 8), max(8, th_ + 8)), C.BG)
            ImageDraw.Draw(img).text((4, 4), label, font=f, fill=C.TEXT)
            rot = img.rotate(90, expand=True, fillcolor=C.BG)
            rw, rh = rot.size
            tick_left = self.x0 - (self._tick_w + 12.0)
            rx = tick_left - 18.0 - rw
            if rx < 4.0:
                # 仍放不下：缩到下限字号再试（保持竖排，绝不横向外溢）
                f = font(FS_MIN)
                tw_, th_ = text_size(self.d, label, f)
                img = Image.new("RGB", (max(8, tw_ + 8), max(8, th_ + 8)), C.BG)
                ImageDraw.Draw(img).text((4, 4), label, font=f, fill=C.TEXT)
                rot = img.rotate(90, expand=True, fillcolor=C.BG)
                rw, rh = rot.size
                rx = tick_left - 18.0 - rw
            rx = max(4.0, rx)
            ry = self.y0 + (self.y1 - self.y0) / 2.0 - rh / 2.0
            ry = max(self.cv.top0 - 30.0, min(ry, self.cv.H - rh - 6.0))
            self.cv.img.paste(rot, (int(round(rx)), int(round(ry))))
            self.cv.box(EL_YLABEL, rx, ry, rx + rw, ry + rh, label)
        if self.xlabel:
            # x 轴标题位置**只由 `Canvas.panel()` 预算**（以画布底边反推）。
            # 此处不得另算公式，否则两处不一致会把标题推出画布或压住刻度带。
            pos_list = getattr(self.cv, "_xlabel_pos_list", None)
            if pos_list:
                yc = pos_list.pop(0)
            else:
                _, tick_h = text_size(self.d, "汉", font(FS_TICK))
                _, axis_h = text_size(self.d, "汉", font(FS_AXIS))
                yc = int(round(self.y1 + tick_h + 8 + axis_h / 2.0))
            draw_text_center(self.d, self.cv.W / 2.0, yc, self.xlabel, font(FS_AXIS), C.TEXT)
            tw, th = text_size(self.d, self.xlabel, font(FS_AXIS))
            xl = ((self.cv.W - tw) / 2.0, yc - th / 2.0, (self.cv.W + tw) / 2.0, yc + th / 2.0)
            self.cv.box(EL_XLABEL, *xl, self.xlabel)
            self.cv.occupy(xl)
        if self.ylabel:
            self.cv.occupy((max(0, int(self.x0) - 90), self.y0,
                            max(0, int(self.x0) - 20), self.y1))

    def spine(self) -> None:
        """轴线（左 + 下）。推荐在**数据之前**调用，避免轴脊压住柱体/曲线。"""
        self.d.line([(self.x0, self.y0), (self.x0, self.y1)], fill=C.TEXT, width=LW_AXIS)
        self.d.line([(self.x0, self.y1), (self.x1, self.y1)], fill=C.TEXT, width=LW_AXIS)

    #: `spine` 的别名，语义更直白（画坐标轴骨架）
    axes_line = spine

    # -- 数据原语 --------------------------------------------------------
    def line(self, x, y, color: str = C.PRIMARY, lw: int = LW_CURVE,
             label: str | None = None, dash: bool = False, zorder: int = 3) -> None:
        """折线。`None`/NaN/Inf/非正值(y 对数轴) 处自动断线。"""
        xa = np.asarray(x, dtype=np.float64).ravel()
        ya = np.asarray(y, dtype=np.float64).ravel()
        if xa.size == 0 or ya.size == 0:
            return
        if ya.size != xa.size:
            if ya.size == 1:
                ya = np.full(xa.size, float(ya[0]))
            else:
                n = min(xa.size, ya.size)
                xa, ya = xa[:n], ya[:n]
        self._include(xa, ya)
        self._autogrow()
        self._data_paths[str(label or len(self._data_paths))] = ya
        pts = self._path(xa, ya)
        self._stroke(pts, color, lw, dash)
        if label:
            self._legend_items.append(("line", color, label, lw))

    def _path(self, xa: np.ndarray, ya: np.ndarray) -> list[list[tuple[float, float]]]:
        """把数组切成若干连续段（无效点断开）。"""
        ok = np.isfinite(xa) & np.isfinite(ya)
        if self.ylog:
            ok &= ya > 0
        if self.xlog:
            ok &= xa > 0
        segs: list[list[tuple[float, float]]] = []
        cur: list[tuple[float, float]] = []
        for i in range(xa.size):
            if ok[i]:
                cur.append((self.px(xa[i]), self.py(ya[i])))
            elif cur:
                if len(cur) > 1:
                    segs.append(cur)
                cur = []
        if len(cur) > 1:
            segs.append(cur)
        return segs

    def _stroke(self, segs, color: str, lw: int, dash: bool) -> None:
        for seg in segs:
            if dash:
                self._dashed_polyline(seg, color, lw)
            else:
                self.d.line(seg, fill=color, width=lw, joint="curve")

    def _dashed_polyline(self, pts, color: str, lw: int) -> None:
        on, off = DASH
        phase = 0.0
        drawing = True
        for a, b in zip(pts, pts[1:]):
            seg_len = math.hypot(b[0] - a[0], b[1] - a[1])
            if seg_len <= 0:
                continue
            t = 0.0
            while t < seg_len:
                limit = (on - phase) if drawing else (off - phase)
                t2 = min(seg_len, t + max(0.5, limit))
                if drawing:
                    p1 = (a[0] + (b[0] - a[0]) * t / seg_len, a[1] + (b[1] - a[1]) * t / seg_len)
                    p2 = (a[0] + (b[0] - a[0]) * t2 / seg_len, a[1] + (b[1] - a[1]) * t2 / seg_len)
                    self.d.line([p1, p2], fill=color, width=lw)
                if t2 - t >= limit - 1e-9:
                    drawing = not drawing
                    phase = 0.0
                else:
                    phase += t2 - t
                t = t2

    def envelope(self, x, lo=None, hi=None, color: str = C.PRIMARY, lw: int = 2,
                 label: str | None = None) -> None:
        """高密度波形的 (min,max) 包络柱（`common.downmix_envelope` 的输出）。"""
        if lo is None or hi is None:
            arr = np.asarray(x, dtype=np.float64).ravel()
            n = arr.size
            xs = np.arange(n, dtype=np.float64)
            self._include(xs, arr)
            self._autogrow()
            self._stroke(self._path(xs, arr), color, lw, False)
            return
        xs = np.asarray(x, dtype=np.float64).ravel()
        la = np.asarray(lo, dtype=np.float64).ravel()
        ha = np.asarray(hi, dtype=np.float64).ravel()
        n = min(xs.size, la.size, ha.size)
        xs, la, ha = xs[:n], la[:n], ha[:n]
        self._include(xs, la)
        self._include(xs, ha)
        self._autogrow()
        for i in range(n):
            if not (np.isfinite(la[i]) and np.isfinite(ha[i])):
                continue
            xp = self.px(xs[i])
            y_hi, y_lo = self.py(ha[i]), self.py(la[i])
            if y_lo - y_hi < 1.0:
                y_hi, y_lo = y_lo - 0.5, y_lo + 0.5
            self.d.line([(xp, y_hi), (xp, y_lo)], fill=color, width=lw)
        if label:
            self._legend_items.append(("line", color, label, lw))

    def bars(self, values, centers=None, width: float = 0.8, color: str = C.PRIMARY,
             edge: str | None = None, label: str | None = None, offset: float = 0.0,
             show_value: bool = False, value_font: int = FS_LABEL,
             outline_only: bool = False, hatch_dot: bool = False) -> None:
        """柱状图。`centers` 默认 0..n-1；`offset` 用于双柱并排。"""
        va = np.asarray(values, dtype=np.float64).ravel()
        if va.size == 0:
            return
        if centers is None:
            centers = np.arange(va.size, dtype=np.float64)
        ca = np.asarray(centers, dtype=np.float64).ravel()
        n = min(va.size, ca.size)
        f = font(value_font)
        # 顺序要求：**先登记数据**（并含 0 基线），再读取 `ylim`。
        # 若先读 `ylim`，首次调用时它还是 None，getter 会立即触发 `autoscale`，
        # 把范围冻结在 (0,1)，后续所有柱体都被画到画布之外。
        self._include(np.concatenate([ca[:n] - width / 2.0, ca[:n] + width / 2.0]),
                      np.concatenate([np.zeros(n), va[:n]]))
        self._autogrow()
        ybase = self.ylim[0]
        yb = self.py(ybase if not self.ylog else max(ybase, 1e-12))
        for i in range(n):
            v = va[i]
            if not np.isfinite(v) or (self.ylog and v <= 0):
                continue
            half = self.px(ca[i] + offset + width / 2.0) - self.px(ca[i] + offset)
            xa = self.px(ca[i] + offset) - half
            xb = self.px(ca[i] + offset) + half
            yt = self.py(v)
            box = [xa, min(yt, yb), xb, max(yt, yb)]
            if outline_only:
                self.d.rectangle(box, outline=color, width=LW_AUX)
            elif hatch_dot:
                self.d.rectangle(box, outline=color, width=1)
                step = 7
                yy = box[1] + step
                while yy < box[3]:
                    xx = box[0] + 3
                    while xx < box[2]:
                        self.d.point((xx, yy), fill=color)
                        xx += step
                    yy += step
            else:
                self.d.rectangle(box, fill=color,
                                 outline=edge if edge else color, width=LW_AXIS if edge else 0)
            if show_value:
                txt = fmt_val(v)
                w, h = text_size(self.d, txt, f)
                tx = (xa + xb) / 2.0 - w / 2.0
                ty = yt - h - 6 if yt <= yb else yb - h - 6
                if ty < self.y0 - h:
                    ty = self.y0 - h + 4
                self.d.text((tx, ty), txt, font=f, fill=C.TEXT)
        if label:
            self._legend_items.append(("patch", color, label, 0))

    def hline(self, y: float, color: str = C.WARN, lw: int = LW_AUX,
              dash: bool = True, label: str | None = None, annotate: str | None = None,
              annotate_font: int = FS_LABEL) -> None:
        """水平参考线（+ 可选右上标注）。"""
        yp = self.py(y)
        if not (self.y0 - 2 <= yp <= self.y1 + 2):
            return
        self._include(np.array([self.xlim[0], self.xlim[1]]), np.array([y, y]))
        if dash:
            self._dashed_polyline([(self.x0, yp), (self.x1, yp)], color, lw)
        else:
            self.d.line([(self.x0, yp), (self.x1, yp)], fill=color, width=lw)
        if annotate:
            f = font(annotate_font)
            w, h = text_size(self.d, annotate, f)
            self.d.text((self.x1 - w - 4, yp - h - 4), annotate, font=f, fill=color)
        if label:
            self._legend_items.append(("line", color, label, lw))

    def vline(self, x: float, color: str = C.WARN, lw: int = LW_AUX,
              dash: bool = True, label: str | None = None) -> None:
        """竖直参考线。先登记 x（避免触发先验定标），再换算像素。"""
        self._include(np.array([float(x)]), np.array([1.0]))
        xp = self.px(x)
        if not (self.x0 - 2 <= xp <= self.x1 + 2):
            return
        if dash:
            self._dashed_polyline([(xp, self.y0), (xp, self.y1)], color, lw)
        else:
            self.d.line([(xp, self.y0), (xp, self.y1)], fill=color, width=lw)
        if label:
            self._legend_items.append(("line", color, label, lw))

    def span(self, x_lo: float, x_hi: float, color: str = C.WARN, alpha: float = 0.14,
             label: str | None = None) -> None:
        """竖直半透明色带（无 alpha 通道，用与白色混合模拟）。"""
        xa, xb = self.px(x_lo), self.px(x_hi)
        if xb < xa:
            xa, xb = xb, xa
        self.img_blend_rect(xa, self.y0, xb, self.y1, mix_white(color, 1.0 - alpha))
        if label:
            self._legend_items.append(("patch", color, label, 0))

    def nan_bands(self, mask, color: str = C.NAN, min_px: float = 2.0,
                  label: str = "NaN/Inf 帧") -> int:
        """把 NaN/Inf 帧标成沿横轴的醒目竖带（不做静默丢点）。

        `mask` 为逐帧布尔数组（索引即帧序号）；长度 < 1 px 的相邻区间会合并，
        保证极稀疏的 NaN 也可见。返回标记的区间数。
        """
        a = np.asarray(mask, dtype=bool).ravel()
        if a.size == 0 or not a.any():
            return 0
        idx = np.flatnonzero(a)
        # 合并相邻帧为区间
        groups: list[tuple[int, int]] = []
        start = prev = int(idx[0])
        for v in idx[1:]:
            v = int(v)
            if v == prev + 1:
                prev = v
                continue
            groups.append((start, prev))
            start = prev = v
        groups.append((start, prev))

        marked = 0
        for g0, g1 in groups:
            xa, xb = self.px(float(g0) - 0.5), self.px(float(g1) + 0.5)
            if xb - xa < min_px:
                c = (xa + xb) / 2.0
                xa, xb = c - min_px / 2.0, c + min_px / 2.0
            # 夹紧到轴区：帧号贴边时（如末帧）px() 会算出轴外的坐标，
            # 不夹紧会让标记带覆盖到画布边缘之外，破坏留白。
            xa = max(xa, self.x0)
            xb = min(xb, self.x1)
            if xb <= xa:
                continue
            self.img_blend_rect(xa, self.y0, xb, self.y1, mix_white(color, 1.0 - 0.22))
            marked += 1
        if marked:
            self._legend_items.append(("patch", color,
                                       "%s（%d 帧 / %d 处）" % (label, int(a.sum()), marked), 0))
        return marked

    def img_blend_rect(self, x0, y0, x1, y1, rgb) -> None:
        self.d.rectangle([x0, y0, x1, y1], fill=rgb)

    def scatter(self, x, y, color: str = C.PRIMARY, r: int = MARKER, label: str | None = None,
                shape: str = "o") -> None:
        xa = np.asarray(x, dtype=np.float64).ravel()
        ya = np.asarray(y, dtype=np.float64).ravel()
        if xa.size == 0:
            return
        n = min(xa.size, ya.size)
        self._include(xa[:n], ya[:n])
        self._autogrow()
        for i in range(n):
            if not (np.isfinite(xa[i]) and np.isfinite(ya[i])):
                continue
            xp, yp = self.px(xa[i]), self.py(ya[i])
            if shape == "o":
                self.d.ellipse([xp - r, yp - r, xp + r, yp + r], fill=color)
            else:
                self.d.rectangle([xp - r, yp - r, xp + r, yp + r], fill=color)
        if label:
            self._legend_items.append(("marker", color, label, r))

    def text(self, x: float, y: float, s: str, color: str = C.TEXT,
             fnt: int = FS_LABEL, anchor: str = "lt", bold: bool = False) -> None:
        """按数据坐标放文本；`anchor` 为两字符 `lt/lm/lb/ct/cm/cb/rt/rm/rb`。"""
        f = font(fnt, bold)
        w, h = text_size(self.d, s, f)
        xp, yp = self.px(x), self.py(y)
        if anchor[0] == "c":
            xp -= w / 2.0
        elif anchor[0] == "r":
            xp -= w
        if anchor[1] == "t":
            pass
        elif anchor[1] == "m":
            yp -= h / 2.0
        else:
            yp -= h
        self.d.text((xp, yp), s, font=f, fill=color)

    def boxplot(self, stats: dict, center: float = 0.0, width: float = 0.5,
                color: str = C.PRIMARY, label: str | None = None,
                show_fliers: bool = True) -> None:
        """单个箱线（`common.box_stats` 的返回值）。"""
        w = self.px(center + width / 2.0) - self.px(center)
        xa, xb = self.px(center) - w, self.px(center) + w
        xm = self.px(center)
        self._include(np.array([center - width, center + width]),
                      np.array([stats["whislo"], stats["whishi"],
                                stats["q1"], stats["q3"], stats["med"]]))
        # 离群点也是数据的一部分：必须纳入值域，否则会被画到轴区之外、
        # 甚至越出画布（曾导致 FigT18/FigT21 顶部出现孤立标记）。
        fl = np.asarray(stats.get("fliers", []), dtype=np.float64)
        if show_fliers and fl.size:
            self._include(np.full(fl.size, float(center)), fl)
        self._autogrow()
        y_lo, y_hi = self.py(stats["whislo"]), self.py(stats["whishi"])
        y_q1, y_q3, y_med = self.py(stats["q1"]), self.py(stats["q3"]), self.py(stats["med"])
        # 须
        self.d.line([(xm, y_hi), (xm, y_lo)], fill=color, width=LW_AUX)
        self.d.line([(xa + (xb - xa) * 0.25, y_hi), (xb - (xb - xa) * 0.25, y_hi)],
                    fill=color, width=LW_AUX)
        self.d.line([(xa + (xb - xa) * 0.25, y_lo), (xb - (xb - xa) * 0.25, y_lo)],
                    fill=color, width=LW_AUX)
        # 箱
        self.d.rectangle([xa, min(y_q1, y_q3), xb, max(y_q1, y_q3)],
                         fill=mix_white(color, 0.82), outline=color, width=LW_AUX)
        # 中位线
        self.d.line([(xa, y_med), (xb, y_med)], fill=color, width=4)
        if show_fliers:
            for v in np.asarray(stats.get("fliers", [])).ravel()[:40]:
                if np.isfinite(v):
                    yp = self.py(float(v))
                    self.d.ellipse([xm - 3, yp - 3, xm + 3, yp + 3], fill=C.WARN)
        if label:
            self._legend_items.append(("patch", color, label, 0))

    def heatmap(self, data: np.ndarray, vmin: float, vmax: float,
                cmap: ColorMap = SEQ, ext: tuple | None = None,
                x_range: tuple | None = None, y_range: tuple | None = None) -> None:
        """热力图。

        `data` 为 (n_freq, n_freq 行=最低频在底) × (n_time)；`ext` 为像素范围
        `(x0, y0, x1, y1)`（省略时铺满轴区）；`x_range`/`y_range` 为对应的数据坐标范围，
        提供后本轴将定标为该范围（频谱图用 y 对数轴）。
        """
        arr = np.asarray(data, dtype=np.float64)
        if arr.ndim != 2 or arr.size == 0:
            return
        if ext is None:
            ext = (self.x0, self.y0, self.x1, self.y1)
        xa, yb, xb, ya = ext[0], ext[1], ext[2], ext[3]
        w = max(1, int(round(xb - xa)))
        h = max(1, int(round(ya - yb)))
        # 数据 → 像素：行=频率（低在底），列=时间（左起）
        src = np.ascontiguousarray(arr.T[::-1, :])
        if vmax <= vmin:
            vmax = vmin + 1.0
        unit = np.clip((src - vmin) / (vmax - vmin), 0.0, 1.0)
        small = Image.frombytes("RGB", (src.shape[1], src.shape[0]),
                                cmap.apply(unit)).resize((w, h), Image.BILINEAR)
        self.cv.img.paste(small, (int(round(xa)), int(round(yb))))
        if x_range is not None:
            self.set_xlim(*x_range)
        if y_range is not None:
            self.set_ylim(*y_range)

    def colorbar(self, cmap: ColorMap, vmin: float, vmax: float, rect,
                 ticks: int = 4, label: str = "dB") -> None:
        """右侧色条。`rect=(x0,y0,x1,y1)` 为色条本体像素范围（右侧留 96 px 放标签）。"""
        x0, y0, x1, y1 = [float(v) for v in rect]
        w = max(6, int(round(x1 - x0)))
        h = max(6, int(round(y1 - y0)))
        col = np.linspace(1.0, 0.0, h).reshape(h, 1)
        raw = cmap.apply(np.repeat(col, w, axis=1))
        self.cv.img.paste(Image.frombytes("RGB", (w, h), raw), (int(round(x0)), int(round(y0))))
        self.d.rectangle([x0, y0, x1, y1], outline=C.TEXT, width=LW_GRID)
        f = font(FS_TICK)
        for i in range(ticks + 1):
            t = i / float(ticks)
            val = vmin + (vmax - vmin) * t
            yp = y1 - t * (y1 - y0)
            self.d.line([(x1, yp), (x1 + 6, yp)], fill=C.TEXT, width=LW_GRID)
            self.d.text((x1 + 10, yp - 12), fmt_tick(val), font=f, fill=C.TEXT)
        if label:
            draw_text_center(self.d, (x0 + x1) / 2.0, y0 - 22, label, f, C.TEXT)

    # -- 表格 ------------------------------------------------------------
    def table(self, headers: list[str], rows: list[list[str]], rect=None,
              col_widths: list[float] | None = None, fnt: int = FS_LABEL,
              row_h: int | None = None, highlight: dict | None = None,
              zebra: bool = True) -> None:
        """在轴区域（或 `rect`）内绘制表格。`highlight[(r,c)] = "#c0392b"` 指定单元格文字色。"""
        if not headers:
            return
        x0, y0, x1, y1 = rect if rect else (self.x0, self.y0, self.x1, self.y1)
        n_rows = len(rows) + 1
        rh = int(row_h or min(52, max(30, (y1 - y0) / max(1, n_rows))))
        n_col = len(headers)
        if col_widths:
            total = float(sum(col_widths))
            widths = [(x1 - x0) * (w / total) for w in col_widths]
        else:
            widths = [(x1 - x0) / n_col] * n_col
        fh = font(fnt, True)
        fb = font(fnt)
        yy = y0
        # 表头
        self.d.rectangle([x0, yy, x1, yy + rh], fill=mix_white(C.PRIMARY, 0.82))
        cx = x0
        for j, htxt in enumerate(headers):
            self.fit_text(cx + 8, yy + rh / 2.0, htxt, fh, widths[j] - 16, C.TEXT, center=False)
            cx += widths[j]
        yy += rh
        for i, row in enumerate(rows):
            if yy + rh > y1 + 2:
                break
            if zebra and i % 2 == 1:
                self.d.rectangle([x0, yy, x1, yy + rh], fill="#f7f9fb")
            cx = x0
            for j in range(n_col):
                cell = row[j] if j < len(row) else ""
                col = (highlight or {}).get((i, j), C.TEXT)
                self.d.line([(cx, yy), (cx, yy + rh)], fill=C.GRID, width=LW_GRID)
                self.fit_text(cx + 8, yy + rh / 2.0, cell, fb, widths[j] - 16, col, center=False)
                cx += widths[j]
            self.d.line([(x0, yy + rh), (x1, yy + rh)], fill=C.GRID, width=LW_GRID)
            yy += rh
        self.d.rectangle([x0, y0, x1, yy], outline=C.TEXT, width=LW_GRID)

    def fit_text(self, x: float, y_center: float, s: str, fnt, max_w: float,
                 color: str = C.TEXT, center: bool = True) -> None:
        """写入文本；超宽时自动截断为 `…`（只改文本，不改字号，不越界）。

        `max_w` 过小时（例如图例框被夹得过窄）退化为单个 `…`，绝不绘制到框外。
        """
        if max_w <= 4:
            self.d.text((x, y_center), "…", font=fnt, fill=color)
            return
        w, h = text_size(self.d, s, fnt)
        shown = s
        if w > max_w:
            while shown and w > max_w:
                shown = shown[:-1]
                if not shown:
                    break
                w, h = text_size(self.d, shown + "…", fnt)
            shown = (shown + "…") if shown else "…"
            w, h = text_size(self.d, shown, fnt)
        xx = x + (max_w - w) / 2.0 if center else x
        # 硬约束：文本不得伸出画布（含 4 px 留白）
        xx = max(4.0, min(xx, self.cv.W - 4.0 - w))
        yy = y_center - h / 2.0
        yy = max(2.0, min(yy, self.cv.H - 2.0 - h))
        self.d.text((xx, yy), shown, font=fnt, fill=color)

    def note(self, s: str, color: str = C.TEXT, fnt: int = FS_NOTE,
             anchor: str = "lt") -> None:
        """轴内注释（数据坐标）。"""
        self.text(self.xlim[1] if anchor[0] == "r" else self.xlim[0],
                  self.ylim[1] if anchor[1] == "t" else self.ylim[0],
                  s, color=color, fnt=fnt, anchor=anchor)

    def note_px(self, x: float, y: float, s: str, color: str = C.TEXT,
                fnt: int = FS_NOTE, anchor: str = "lt", bold: bool = False,
                max_w: float | None = None) -> None:
        """轴内注释（像素坐标，相对整张画布），必要时自动截断。"""
        f = font(fnt, bold)
        if max_w is not None:
            self.fit_text(x, y, s, f, max_w, color, center=False)
            return
        w, h = text_size(self.d, s, f)
        xx = x - w / 2.0 if anchor[0] == "c" else (x - w if anchor[0] == "r" else x)
        yy = y if anchor[1] == "t" else (y - h / 2.0 if anchor[1] == "m" else y - h)
        self.d.text((xx, yy), s, font=f, fill=color)

    def pixel_note(self, x: float, y: float, s: str, color: str = C.TEXT,
                   fnt: int = FS_NOTE, bold: bool = False,
                   max_w: float | None = None, line_gap: int = 10) -> int:
        """画布像素坐标的**自动折行**注释块；返回占用行数。

        与 `note_px(max_w=...)`（截断）不同，本方法保留完整文本，适合结论性长句。
        """
        f = font(fnt, bold)
        avail = max_w if max_w is not None else (self.cv.W - x - self.cv.pad_r)
        lines = wrap_text(self.d, s, f, max(40.0, avail))
        _, h = text_size(self.d, "汉", f)
        step = h + line_gap
        yy = max(2.0, min(float(y), self.cv.H - step * len(lines) - 4))
        for i, ln in enumerate(lines):
            self.d.text((x, yy + i * step), ln, font=f, fill=color)
        return len(lines)

    # -- 图例 ------------------------------------------------------------
    def _data_vertical_balance(self) -> float:
        """数据在轴区的纵向重心（0=贴顶，1=贴底）；无数据时返回 0.5。

        用于自动选择图例落点：曲线整体偏下时把图例放上方空白，反之放下方。
        """
        b = self._data_bounds
        if b is None:
            return 0.5
        lo, hi = self.ylim
        if self.ylog:
            lo, hi = math.log10(max(lo, 1e-12)), math.log10(max(hi, lo * 10))
            d_lo = math.log10(max(b[2], 1e-12))
            d_hi = math.log10(max(b[3], d_lo * 10))
        else:
            d_lo, d_hi = b[2], b[3]
        if hi <= lo:
            return 0.5
        mid = (d_lo + d_hi) / 2.0
        return float(np.clip((hi - mid) / (hi - lo), 0.0, 1.0))

    def _data_density(self, x0: float, y0: float, x1: float, y1: float,
                      cols: int = 32, rows: int = 16) -> float:
        """估计给定矩形区域内折线数据的覆盖密度（0..1）。

        把区域划成网格，按列重采样每条曲线并标记其经过的格子。用于为图例挑选
        **遮挡最少**的角落——柱状图/折线图的空白位置差异很大，仅凭纵向重心不够。
        """
        if not self._data_paths:
            return 0.5
        hit = np.zeros((rows, cols), dtype=bool)
        w = max(1.0, x1 - x0)
        h = max(1.0, y1 - y0)
        for ys in self._data_paths.values():
            ya = np.asarray(ys, dtype=np.float64).ravel()
            ok = np.isfinite(ya)
            if ya.size < 2 or not ok.any():
                continue
            src = np.linspace(0.0, 1.0, ya.size)
            xs = np.linspace(0.0, 1.0, cols)
            interp = np.interp(xs, src[ok], ya[ok])
            for c in range(cols):
                px = x0 + (c + 0.5) / cols * w
                py = self.py(float(interp[c]))
                if not (x0 <= px <= x1 and y0 <= py <= y1):
                    continue
                r = int((py - y0) / h * rows)
                if 0 <= r < rows:
                    hit[r, c] = True
        return float(hit.mean()) if hit.size else 0.5

    def _best_legend_corner(self, box_w: float, box_h: float) -> str:
        """在四个角落中选数据密度最低者，返回 'tl'/'tr'/'bl'/'br'。

        坐标必须与 `legend()` 实际落点一致（同样夹紧在画布内），
        否则会把窗口移到轴外，数据密度恒为 0 而给出错误的"最优"角落。
        """
        cands = []
        for v in ("t", "b"):
            for hh in ("l", "r"):
                x0 = self.x0 + 12 if hh == "l" else self.x1 - box_w - 12
                y0 = self.y0 + 8 if v == "t" else self.y1 - box_h - 8
                x0 = max(self.x0 + 4.0, min(x0, self.x1 - box_w - 4.0))
                y0 = max(self.y0 + 2.0, min(y0, self.y1 - box_h - 2.0))
                cands.append((self._data_density(x0, y0, x0 + box_w, y0 + box_h), v + hh))
        cands.sort(key=lambda z: z[0])
        return cands[0][1]

    def legend(self, loc: str = "auto", fnt: int = FS_LEGEND, boxed: bool = True,
               entries=None, max_w: float | None = None, outside: bool = False,
               wrap: bool = True) -> None:
        """图例。

        `loc` ∈ {auto, tl, tr, bl, br, tc, bc}。`auto` 依据数据纵向重心选空白侧，
        避免图例框压住曲线/柱体（数据偏上则放下方，偏下则放上方）。
        `max_w` 给出时按**换行**适配（不截断成省略号）；`outside=True` 时横向移到轴区右侧之外。
        图例框会登记到元素表，供 `check_layout.py` 做重叠检测。
        """
        items = list(entries if entries is not None else self._legend_items)
        if not items:
            return
        auto_loc = (loc == "auto")
        pad, sw, gap, vgap = 14, 40, 14, 10
        avail_w = self.width * 0.96
        if max_w:
            avail_w = min(avail_w, float(max_w))
        size = int(fnt)

        # 排布探测：若各条目标签能并排放下，则用**横排一行**（高度 N 行 → 1 行），
        # 把对数据的遮挡面积压到最小；否则退回竖向多行。
        inline = False
        entry_w: list[float] = []
        if len(items) > 1:
            f = font(size)
            entry_w = [sw + gap + text_size(self.d, str(it[2]), f)[0] + vgap for it in items]
            if sum(entry_w) + pad * 2 - vgap <= avail_w:
                inline = True

        if inline:
            f = font(size)
            wrapped = [[str(it[2])] for it in items]
            box_w = pad * 2 + sum(entry_w) - vgap
            box_h = pad * 2 + size + 8
        else:
            while True:
                f = font(size)
                inner = max(24.0, avail_w - (pad * 2 + sw + gap) - 6)
                wrapped = [wrap_text(self.d, str(it[2]), f, inner) if wrap else [str(it[2])]
                           for it in items]
                tw_max = max(text_size(self.d, ln, f)[0] for ws in wrapped for ln in ws)
                n_lines = sum(len(ws) for ws in wrapped)
                box_w = pad * 2 + sw + gap + tw_max
                box_h = pad * 2 + n_lines * (size + 4) + (len(items) - 1) * vgap
                if box_h <= self.height * 0.85 or size <= FS_MIN:
                    break
                size = max(FS_MIN, size - 2)

        if auto_loc:
            if self.cv._legend_band_bottom is not None:
                # 已预留"图例带"：图例排在轴区之外、标题之下的专属一行，绝不压数据
                loc = "tl"
            else:
                # 否则按**实际数据密度**挑遮挡最少的角落（比仅看纵向重心可靠）
                loc = self._best_legend_corner(box_w, box_h) if not outside else "tr"
        if self.cv._legend_band_bottom is not None:
            y0 = max(4.0, float(self.cv._legend_band_bottom) - box_h - 6.0)
            x0 = self.x0
        elif loc[0] == "t":
            y0 = self.y0 + 8
        elif loc[0] == "b":
            y0 = self.y1 - box_h - 8
        else:
            y0 = self.y0 + 8
        if outside:
            x0 = self.x1 + 16
        elif loc[1] == "l":
            x0 = self.x0 + 12
        elif loc[1] == "r":
            x0 = self.x1 - box_w - 12
        else:
            x0 = (self.x0 + self.x1) / 2.0 - box_w / 2.0
        x0 = max(4.0, min(x0, self.cv.W - box_w - 4.0))
        y0 = max(4.0, min(y0, self.cv.H - box_h - 4.0))
        # 图例带内不得上侵标题：若框高超出带，向上夹紧只会撞标题，
        # 因此改为「顶边对齐带底」的方向下移（带已按行数预留足够高度）。
        if self.cv._legend_band_bottom is not None:
            y0 = max(float(self.cv.top0) + 2.0, y0)

        if boxed:
            self.d.rectangle([x0, y0, x0 + box_w, y0 + box_h],
                             fill=mix_white(C.PRIMARY, 0.97), outline="#cfd8e0", width=LW_GRID)
        if inline:
            # 横排：所有条目并排一行
            cx = x0 + pad
            yc = y0 + box_h / 2.0
            for (kind, color, txt, lw), ws in zip(
                    [(it[0], it[1], str(it[2]), it[3]) for it in items], wrapped):
                if kind == "line":
                    self.d.line([(cx, yc), (cx + sw, yc)], fill=color, width=max(LW_CURVE, lw))
                elif kind == "marker":
                    r = max(4, int(lw))
                    self.d.ellipse([cx + sw / 2.0 - r, yc - r, cx + sw / 2.0 + r, yc + r], fill=color)
                else:
                    self.d.rectangle([cx, yc - size / 2.0 + 3, cx + sw, yc + size / 2.0 - 3], fill=color)
                self.d.text((cx + sw + gap, yc - size / 2.0 - 2), ws[0], font=f, fill=C.TEXT)
                cx += sw + gap + text_size(self.d, ws[0], f)[0] + vgap
        else:
            yy = y0 + pad
            idx = 0
            for kind, color, txt, lw in [(it[0], it[1], str(it[2]), it[3]) for it in items]:
                ws = wrapped[idx]
                idx += 1
                first = yy + size / 2.0
                if kind == "line":
                    self.d.line([(x0 + pad, first), (x0 + pad + sw, first)], fill=color,
                                width=max(LW_CURVE, lw))
                elif kind == "marker":
                    r = max(4, int(lw))
                    self.d.ellipse([x0 + pad + sw / 2.0 - r, first - r,
                                    x0 + pad + sw / 2.0 + r, first + r], fill=color)
                else:
                    self.d.rectangle([x0 + pad, first - size / 2.0 + 3,
                                      x0 + pad + sw, first + size / 2.0 - 3], fill=color)
                for k, ln in enumerate(ws):
                    self.d.text((x0 + pad + sw + gap, yy + k * (size + 4)),
                                ln, font=f, fill=C.TEXT)
                yy += len(ws) * (size + 4) + vgap
        label = "、".join(str(it[2]) for it in items)[:48]
        self.cv.box(EL_LEGEND, x0, y0, x0 + box_w, y0 + box_h, "图例：%s" % label)
        self.cv.occupy((x0, y0, x0 + box_w, y0 + box_h))
        self._legend_items = []

    # -- 多面板辅助 ------------------------------------------------------
    @staticmethod
    def grid_rects(box, nrow: int, ncol: int, gap_x: float = 34, gap_y: float = 40,
                   x0_extend: bool = False):
        """把 `(x0,y0,x1,y1)` 均分为 `nrow×ncol` 子区域，返回按行优先的列表。

        `x0_extend=True` 时首列向左扩 `gap_x`，用于让多行共享同一条 y 轴刻度线。
        """
        x0, y0, x1, y1 = [float(v) for v in box]
        cw = ((x1 - x0) - gap_x * (ncol - 1)) / ncol
        ch = ((y1 - y0) - gap_y * (nrow - 1)) / nrow
        out = []
        for r in range(nrow):
            for c in range(ncol):
                rx0 = x0 + c * (cw + gap_x)
                rx1 = rx0 + cw
                if x0_extend and c == 0:
                    rx0 = x0 - gap_x
                ry0 = y0 + r * (ch + gap_y)
                out.append((rx0, ry0, rx1, ry0 + ch))
        return out


# --------------------------------------------------------------------------
# 通用装饰件（跨脚本复用）
# --------------------------------------------------------------------------

def caption(cv: Canvas, x: float, y: float, s: str, color: str = C.PRIMARY,
            bold: bool = True, fnt: int = FS_AXIS, anchor: str = "lt") -> None:
    """图内小标题（如三联图的各面板标题）。"""
    f = font(fnt, bold)
    w, h = text_size(cv.d, s, f)
    xx = x - w / 2.0 if anchor[0] == "c" else (x - w if anchor[0] == "r" else x)
    yy = y if anchor[1] == "t" else (y - h / 2.0 if anchor[1] == "m" else y - h)
    cv.d.text((xx, yy), s, font=f, fill=color)


def arrow(draw: ImageDraw.ImageDraw, pts, color: str = C.PRIMARY, lw: int = LW_AUX,
          head: int = 13) -> None:
    """折线箭头（架构图用）。"""
    for a, b in zip(pts, pts[1:]):
        draw.line([a, b], fill=color, width=lw)
    (x1, y1), (x2, y2) = pts[-2], pts[-1]
    ang = math.atan2(y2 - y1, x2 - x1)
    draw.polygon([(x2, y2),
                  (x2 - head * math.cos(ang - 0.42), y2 - head * math.sin(ang - 0.42)),
                  (x2 - head * math.cos(ang + 0.42), y2 - head * math.sin(ang + 0.42))],
                 fill=color)


def node_box(cv: Canvas, x: float, y: float, w: float, h: float, title: str,
             lines: list[str], tag: str = "", fill: str = C.PRIMARY,
             fnt_title: int = FS_AXIS, fnt_body: int = FS_LABEL,
             tag_fnt: int = FS_LABEL) -> None:
    """架构图节点：圆角框 + tag + 标题 + 若干行说明。

    文字严格裁剪在卡片内（超宽时按字符截断并追加 `…`），保证不被卡片边框裁切、
    也不会溢出到相邻卡片上。返回实际使用的正文行数。
    """
    d = cv.d
    d.rounded_rectangle([x, y, x + w, y + h], radius=10,
                        fill=mix_white(fill, 0.90), outline=fill, width=LW_AXIS)
    inner = w - 28.0
    ty = y + 10
    if tag:
        d.rounded_rectangle([x + 12, ty, x + 12 + 52, ty + fnt_title + 4], radius=6, fill=fill)
        draw_text_center(d, x + 12 + 26, ty + (fnt_title + 4) / 2.0, tag,
                         font(min(tag_fnt, fnt_title), True), C.BG)
        ty += fnt_title + 12
    ft = font(fnt_title, True)
    drawn = 0
    for ln in _clip_lines(d, title, ft, inner):
        if ty + fnt_title > y + h - 6:
            break
        d.text((x + 14, ty), ln, font=ft, fill=C.TEXT)
        ty += fnt_title + 6
        drawn += 1
    fb = font(fnt_body)
    for line in lines:
        for ln in _clip_lines(d, line, fb, inner):
            if ty + fnt_body > y + h - 6:
                return drawn
            d.text((x + 14, ty), ln, font=fb, fill=C.TEXT)
            ty += fnt_body + 6
            drawn += 1
    return drawn


def _clip_lines(draw: ImageDraw.ImageDraw, text: str, fnt, max_w: float) -> list[str]:
    """把文本按 `max_w` 截断成单行（超宽时以 `…` 结尾）；空串返回空列表。"""
    s = str(text)
    if not s:
        return []
    if text_size(draw, s, fnt)[0] <= max_w:
        return [s]
    while s:
        s = s[:-1]
        if not s:
            return ["…"]
        cand = s + "…"
        if text_size(draw, cand, fnt)[0] <= max_w:
            return [cand]
    return ["…"]


def iter_frames(n: int, hop: int = 1, limit: int | None = None):
    """轻量帧索引迭代器（图元密集时的辅助）。"""
    end = n if limit is None else min(n, limit)
    return range(0, end, hop)


def warn_if_formal_output(script: str = "") -> bool:
    """防误写守卫：默认输出到**正式**图件目录时给出显著提示。

    自测必须设置 `NS_FIG_OUT`（或 `NS_SELFTEST=1`）以重定向输出，
    否则会把合成数据的产物写进 `ENC/Document/figures/`。
    返回 True 表示当前写的是正式目录。
    """
    override = os.environ.get("NS_FIG_OUT", "").strip()
    selftest = os.environ.get("NS_SELFTEST", "").strip() not in ("", "0", "false", "False")
    if override or selftest:
        return False
    print("[fig_style] 注意: 未设置 NS_FIG_OUT / NS_SELFTEST，"
          "图件将写入正式目录 %s%s" % (K.figures_dir(), ("（%s）" % script) if script else ""))
    return True
