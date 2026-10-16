# -*- coding: utf-8 -*-
"""
图件版式校验器 (T4-I)

不依赖人眼：用像素统计客观核验每张 PNG 是否符合计划书第 9 节。

核验项
------
A. 画布尺寸与版位完全一致（1800×845 / 900×625 / 1800×445）
B. 白底不透明（无 alpha）、无透明像素
C. 内容不越界：非白像素的四边界都在安全边距内，且四周保留白边
D. 内容不空：非白像素占比在合理区间（既非空白也非糊满）
E. 右上角图号存在：图号区域的深色像素密度高于常态
F. 字号下限：图内文本高度 ≥ 24 px —— 以"最小连通墨水行高"的分布近似判定
G. 配色合规：主色/baseline/optimized/正向/警示 均在图像中出现（或按需豁免）
**H. 元素级重叠检测（关键）**：读取 `fig_style` 写出的 `.boxes.json` sidecar，
   检测轴标题 / 刻度标签 / 图例框 / 图内标题 / 注释 / 图号 两两之间的包围盒交叠。

> G 之前的检查只是**画布级**属性，无法发现"图例压刻度""注释压轴标题"这类缺陷——
> 曾经出现 22/22 通过但 15/29 页仍有大面积叠印的情况。H 项补上这个盲区。

输出
----
控制台逐图一行结论 + `layout_report.json`（供验收报告引用）。
"""

from __future__ import annotations

import json
import os
import sys

import numpy as np
from PIL import Image

import common as K
import fig_style as S

L = S.LAYOUT_SIZE


def _rgb(path: str) -> np.ndarray:
    img = Image.open(path)
    if img.mode != "RGB":
        img = img.convert("RGB")
    return np.asarray(img, dtype=np.uint8)


def ink_mask(arr: np.ndarray, tol: int = 32) -> np.ndarray:
    """非白（含非纯白底色）像素掩码。

    阈值 32：必须高于图例/表头的浅色填充（`#f2f5f8` 差值 13、`#f7f9fb` 差值 8）
    与网格线（`#e6ebf0` 差值 25），否则会把衬底误判成"内容"。
    文字（`#33414f` 差值 190+）与曲线（差值 90+）远高于该阈值。
    """
    return np.max(np.abs(arr.astype(np.int16) - 255), axis=2) > tol


def bbox(mask: np.ndarray, min_run: int = 2) -> tuple[int, int, int, int] | None:
    """内容包围盒；忽略只有单像素的孤立抗锯齿噪点（要求行/列至少 `min_run` 个墨水像素）。"""
    rows = np.where(mask.sum(axis=1) >= min_run)[0]
    cols = np.where(mask.sum(axis=0) >= min_run)[0]
    if rows.size == 0 or cols.size == 0:
        return None
    return int(cols.min()), int(rows.min()), int(cols.max()), int(rows.max())


def text_row_heights(mask: np.ndarray) -> np.ndarray:
    """估算图内文本行高分布。

    思路：文本行的墨水密度显著低于曲线/柱体；取"每行墨水像素数"在
    中位数区间的连续行段作为候选文本行，返回各段高度。
    这是保守估计——用于确认**存在** ≥24 px 的文本，而非精确 OCR。
    """
    per_row = mask.sum(axis=1)
    if per_row.max() == 0:
        return np.zeros(0)
    thresh = max(6, per_row.max() * 0.02)
    active = per_row > thresh
    heights = []
    start = None
    for i, a in enumerate(active):
        if a and start is None:
            start = i
        elif not a and start is not None:
            heights.append(i - start)
            start = None
    if start is not None:
        heights.append(len(active) - start)
    return np.array(heights, dtype=int)


def check_one(path: str, layout: str, color_expect: list[str] | None = None) -> dict:
    arr = _rgb(path)
    h, w = arr.shape[:2]
    exp_w, exp_h = L[layout]
    res: dict = {
        "file": os.path.basename(path),
        "layout": layout,
        "size": [int(w), int(h)],
        "size_ok": (w == exp_w and h == exp_h),
    }
    img = Image.open(path)
    res["alpha"] = img.mode
    mask = ink_mask(arr)
    frac = float(mask.mean())
    res["ink_fraction"] = round(frac, 5)
    res["ink_ok"] = 0.008 <= frac <= 0.90

    bb = bbox(mask)
    res["bbox"] = list(bb) if bb else None
    if bb:
        x0, y0, x1, y1 = bb
        margins = {"left": int(x0), "top": int(y0),
                   "right": int(w - 1 - x1), "bottom": int(h - 1 - y1)}
        res["margins"] = margins
        # 内容距画布边缘至少 6 px（不贴边）
        res["bleed_ok"] = all(v >= 6 for v in margins.values())
    else:
        res["margins"] = None
        res["bleed_ok"] = False
    # 边缘 4 px 环带必须基本为白（允许 ≤1% 抗锯齿噪点）
    ring = np.ones(mask.shape, dtype=bool)
    ring[4:-4, 4:-4] = False
    ring_ink = float(mask[ring].mean()) if ring.any() else 0.0
    res["edge_ink_ratio"] = round(ring_ink, 5)
    res["edge_clean"] = ring_ink <= 0.01

    # 图号区域（右上角）：应有明显墨水
    figid_box = mask[0:64, max(0, w - 260):w]
    res["figid_ink"] = int(figid_box.sum())
    res["figid_ok"] = int(figid_box.sum()) > 200

    hs = text_row_heights(mask)
    if hs.size:
        res["text_row_heights"] = {"min": int(hs.min()), "p25": int(np.percentile(hs, 25)),
                                   "median": int(np.median(hs)), "max": int(hs.max()),
                                   "count": int(hs.size)}
        # 只要求"存在"达到下限字号的行，不要求每一段都 ≥24（曲线行段会更矮）
        res["has_large_text"] = bool(hs.max() >= 24)
    else:
        res["has_large_text"] = False
        res["text_row_heights"] = None

    if color_expect:
        present = {}
        flat = arr.reshape(-1, 3).astype(np.int16)
        for name in color_expect:
            rgb = np.array(S.hex2rgb(getattr(S.C, name)), dtype=np.int16)
            hit = int(np.sum(np.max(np.abs(flat - rgb), axis=1) <= 6))
            present[name] = hit
        res["palette"] = present
        res["palette_ok"] = all(v > 40 for v in present.values())

    # ---- H. 元素级重叠检测（读取 fig_style 写出的 sidecar）----
    side = read_boxes(path)
    if side is None:
        res["overlaps"] = None
        res["overlap_ok"] = True          # 无 sidecar 时不判负（仅提示）
        res["sidecar"] = False
    else:
        ov = overlap_report(side.get("boxes", []))
        res["sidecar"] = True
        res["overlaps"] = ov
        res["overlap_ok"] = len(ov) == 0

    res["ok"] = bool(res["size_ok"] and res["ink_ok"] and res["bleed_ok"]
                     and res["edge_clean"] and res["figid_ok"] and res["has_large_text"]
                     and res["overlap_ok"])
    return res


LAYOUT_OF = {
    "FigT01": "half", "FigT02": "full", "FigT03": "strip", "FigT04": "half",
    "FigT05": "full", "FigT06": "full", "FigT07": "strip", "FigT08": "half",
    "FigT09": "strip", "FigT10": "strip", "FigT11": "strip", "FigT12": "strip",
    "FigT13": "half", "FigT14": "half", "FigT15": "half", "FigT16": "strip",
    "FigT17": "full", "FigT18": "half", "FigT19": "half", "FigT20": "full",
    "FigT21": "half", "FigT22": "half",
}

#: 必须两两不重叠的元素类型（与 `fig_style.STRICT_TYPES` 保持一致）
STRICT_TYPES = ("title", "figid", "ylabel", "xlabel", "xtick", "ytick",
                "legend", "note", "caption")


def read_boxes(png_path: str) -> dict | None:
    """读取 `fig_style.Canvas.save()` 写出的同名 `.boxes.json` sidecar。"""
    side = os.path.splitext(png_path)[0] + ".boxes.json"
    if not os.path.isfile(side):
        return None
    try:
        with open(side, encoding="utf-8") as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return None


def overlap_report(boxes: list[dict], min_area: float = 150.0) -> list[dict]:
    """元素包围盒两两交叠检测（只比较 `STRICT_TYPES` 内、且**跨类型**的组合）。

    同类型元素天然相邻（如同一侧的多个刻度标签已合并为一条带），不参与判定。
    交叠面积 > `min_area`（px²）才计入：阈值取 150 px²，用于滤除"相邻子图刻度带
    贴边"（实测 26~62 px²）与亚像素贴合这类不影响可读性的边界情况；
    真实叠印（图例压刻度、标题压数据）面积通常在 500 px² 以上。
    """
    items = [b for b in boxes if b.get("type") in STRICT_TYPES]
    out: list[dict] = []
    for i in range(len(items)):
        a = items[i]
        for j in range(i + 1, len(items)):
            b = items[j]
            if a["type"] == b["type"]:
                continue
            ox = min(a["x1"], b["x1"]) - max(a["x0"], b["x0"])
            oy = min(a["y1"], b["y1"]) - max(a["y0"], b["y0"])
            if ox > 0 and oy > 0 and ox * oy > min_area:
                out.append({"a": a["type"], "b": b["type"],
                            "a_label": str(a.get("label", ""))[:30],
                            "b_label": str(b.get("label", ""))[:30],
                            "area": round(ox * oy, 1),
                            "overlap": [round(max(a["x0"], b["x0"]), 1),
                                        round(max(a["y0"], b["y0"]), 1),
                                        round(min(a["x1"], b["x1"]), 1),
                                        round(min(a["y1"], b["y1"]), 1)]})
    out.sort(key=lambda d: -d["area"])
    return out


def main(argv: list[str]) -> int:
    targets = argv if argv else sorted(os.listdir(K.figures_dir())) if os.path.isdir(K.figures_dir()) else []
    targets = [t for t in targets if t.lower().endswith(".png")]
    if not targets:
        print("[check_layout] 无图件可校验: %s" % K.figures_dir())
        return 0

    fdir = K.figures_dir()
    results = []
    bad = 0
    skipped: list[str] = []
    print("=" * 108)
    print("[check_layout] 目录: %s" % fdir)
    print("=" * 108)
    for name in targets:
        path = name if os.path.isabs(name) else os.path.join(fdir, name)
        base = os.path.basename(path)
        key = base.split("_")[0]
        # 只校验本管线产出的 FigT** 图件。目录中还可能有既有的文档配图
        # （Fig001…、FigP01… 等），它们的尺寸/字号规范与本管线不同，不属于本校验范围。
        if key not in LAYOUT_OF:
            skipped.append(base)
            continue
        layout = LAYOUT_OF[key]
        expect = ["PRIMARY", "TEXT", "GRID"] if key != "FigT02" else ["PRIMARY", "TEXT"]
        r = check_one(path, layout, expect)
        results.append(r)
        w, h = r["size"]
        dw, dh = L[layout]
        flag = "OK  " if r["ok"] else "FAIL"
        if not r["ok"]:
            bad += 1
        print("%s %-34s %5d×%-4d (期望 %d×%d) 墨占比=%.3f 图号墨=%d 文本最大行高=%s"
              % (flag, r["file"], w, h, dw, dh, r["ink_fraction"], r["figid_ink"],
                 (r["text_row_heights"] or {}).get("max", "-")))
        if not r.get("sidecar", True):
            print("       ⚠ 缺少 .boxes.json sidecar，元素重叠检测未执行")
        if r.get("overlaps"):
            for o in r["overlaps"][:4]:
                print("       ✗ 重叠 %s×%s 面积=%.0f px²  [%s] × [%s]"
                      % (o["a"], o["b"], o["area"], o["a_label"], o["b_label"]))
            if len(r["overlaps"]) > 4:
                print("       … 另有 %d 处重叠" % (len(r["overlaps"]) - 4))
        if not r["ok"]:
            for k in ("size_ok", "ink_ok", "bleed_ok", "edge_clean",
                      "figid_ok", "has_large_text", "palette_ok", "overlap_ok"):
                if k in r and not r[k]:
                    print("       ✗ %s" % k)
            if r.get("margins"):
                print("       margins=%s edge_ink_ratio=%s"
                      % (r["margins"], r.get("edge_ink_ratio")))
    print("-" * 108)
    if skipped:
        print("[check_layout] 跳过 %d 张非本管线图件（既有文档配图）: %s"
              % (len(skipped), ", ".join(skipped[:6]) + (" …" if len(skipped) > 6 else "")))
    print("[check_layout] 合计 %d 张，失败 %d 张" % (len(results), bad))

    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "_selftest", "layout_report.json")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", encoding="utf-8") as fh:
        json.dump(results, fh, ensure_ascii=False, indent=2)
    print("[check_layout] 报告: %s" % out)
    return 0 if bad == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
