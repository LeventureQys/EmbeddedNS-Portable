# -*- coding: utf-8 -*-
"""check_deck.py — 报告校验与渲染预览（T4-II 产出，对应计划书 T9）。

一键校验
--------
    python check_deck.py

三步（命令原文与计划书 T9 一致；执行次序见下"次序说明"）
--------------------------------------------------------
1. 结构校验：
   ``<python> <office-skills>/scripts/check_office.py <pptx> --out <preview>/checks.json
   --count 34 --contains 结论 --contains 附录``
2. 渲染 PDF：
   ``<node> <cli> convert --input <pptx> --output <preview>/NS报告.pdf``
3. 渲染全部 34 页 PNG：
   ``<node> <cli> render --input <pptx> --output-dir <preview> --pages 1,2,...,34 --dpi 144``

次序说明（实测约束）
--------------------
``render`` 的 ``--output-dir`` 语义是 **fresh directory**（cli.js:1798 直接
``mkdir``，目录已存在会 EEXIST）。因此本脚本的内部执行次序为 3 → 2 → 1：
先由 render 建立 preview 目录并产出 PNG + manifest.json，再写 PDF 与 checks.json。
三条命令的参数与产物位置与计划书完全一致，只有调用次序不同。

判定
----
- ``checks.json``：``verdict == "pass"``、各项 ``status == "pass"``、``summary.slides == 34``
- ``manifest.json``：``pageCount == 34``、``missingFonts == []``、PNG 文件 34 张
- ``preview/NS报告.pdf`` 存在且非空
任一不满足则退出码 1（0 = 全部通过，2 = 前置条件缺失）。

环境变量与 ``deck_data.py`` 一致（``NSDECK_*``）；另支持 ``NSDECK_ALLOW_CLEAN=1``
在 preview 目录不在预期位置时仍允许清理。
"""

from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys

import deck_data as DD

try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")
except Exception:  # pragma: no cover
    pass

TOTAL_PAGES = 34
CHECK_OFFICE = r"D:\Devtools\DSH\resources\runtime\office-skills\scripts\check_office.py"
NODE = r"D:\Devtools\DSH\resources\runtime\primary-runtime\dependencies\node\bin\node.exe"
LIBREOFFICE_CLI = (r"D:\Devtools\DSH\resources\app.asar.unpacked\dsh\node_modules"
                   r"\@deepseek-ai\libreoffice-kit\lib\cli.js")
PREVIEW_PDF_NAME = "NS报告.pdf"


def _rule(char="-"):
    print(char * 78)


def _run(command):
    """执行外部命令，返回 (returncode, stdout+stderr)。"""
    print("  命令：%s" % " ".join('"%s"' % part if " " in part else part for part in command))
    completed = subprocess.run(command, capture_output=True, timeout=1800)
    output = (completed.stdout or b"").decode("utf-8", errors="replace")
    error = (completed.stderr or b"").decode("utf-8", errors="replace")
    text = (output + ("\n" + error if error.strip() else "")).strip()
    print("  退出码：%d" % completed.returncode)
    return completed.returncode, text


def _tail(text, lines=14):
    rows = [row for row in text.splitlines() if row.strip()]
    for row in rows[-lines:]:
        print("    | %s" % row[:300])


def _clean_preview(preview_dir, doc_dir):
    """render 要求 fresh 目录：按白名单规则清理既有 preview。"""
    if not os.path.isdir(preview_dir):
        return
    real = os.path.abspath(preview_dir)
    parent = os.path.dirname(real)
    name = os.path.basename(real).lower()
    allowed = os.environ.get("NSDECK_ALLOW_CLEAN") == "1"
    expected = name.startswith("preview") and os.path.abspath(parent) == os.path.abspath(doc_dir)
    if not (allowed or expected):
        raise RuntimeError(
            "拒绝清理非预期目录（既不是 <OUT_DIR>/preview，也未设置 NSDECK_ALLOW_CLEAN=1）：%s" % real)
    shutil.rmtree(real)
    print("  已清理既有 preview 目录：%s" % real)


def _read_json(path):
    if not os.path.isfile(path):
        return None
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def _edge_ink_check(preview_dir, png_files, margin_px=6, inset_px=1, threshold=200):
    """附加检查（不是视觉验收）：渲染 PNG 四边 ``margin_px`` 内是否出现深色墨迹。

    内容越出幻灯片边界时，渲染图的最外圈会出现墨迹；正常留白应恒为纯白。
    实测 LibreOffice Kit 会在页面最外 1 像素画整圈边框（灰度 149，再往里即纯白
    255），因此判定环带在横纵两个方向都从 ``inset_px`` 起算，避免把渲染器固有
    边框误判为越界。
    局限：只能发现“贴边/越界”这一类风险，不能替代人工视觉审阅。
    """
    try:
        from PIL import Image
    except ImportError:
        print("  [跳过] 未安装 Pillow，无法做渲染边距自检")
        return None
    results = []
    for name in png_files:
        path = os.path.join(preview_dir, name)
        try:
            with Image.open(path) as image:
                gray = image.convert("L")
                width, height = gray.size
                inset = inset_px
                bands = [
                    gray.crop((inset, inset, width - inset, inset + margin_px)),
                    gray.crop((inset, height - inset - margin_px, width - inset, height - inset)),
                    gray.crop((inset, inset, inset + margin_px, height - inset)),
                    gray.crop((width - inset - margin_px, inset, width - inset, height - inset)),
                ]
                darkest = min(band.getextrema()[0] for band in bands)
        except Exception as error:  # 渲染产物异常时不中断整体校验
            results.append({"file": name, "error": str(error)})
            continue
        results.append({"file": name, "size": [width, height], "min_gray_at_edge": darkest,
                        "clean": darkest >= threshold})
    return results


def _report(summary):
    path = os.path.join(DD.REPORT_DIR, "deck_check_summary.json")
    os.makedirs(DD.REPORT_DIR, exist_ok=True)
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(summary, handle, ensure_ascii=False, indent=2)
    print("校验摘要：%s" % path)


def main():
    print("check_deck.py — 结构校验 + 渲染预览（%d 页）" % TOTAL_PAGES)
    _rule("=")
    print("PPTX        ：%s" % DD.PPTX_PATH)
    print("PREVIEW_DIR ：%s" % DD.PREVIEW_DIR)
    _rule()

    for label, path in (("python", sys.executable), ("check_office", CHECK_OFFICE),
                        ("node", NODE), ("libreoffice cli", LIBREOFFICE_CLI)):
        if not os.path.isfile(path):
            print("前置条件缺失：%s 不存在 → %s" % (label, path))
            return 2
    if not os.path.isfile(DD.PPTX_PATH):
        print("前置条件缺失：报告不存在，请先运行 build_deck.py → %s" % DD.PPTX_PATH)
        return 2

    checks_path = os.path.join(DD.PREVIEW_DIR, "checks.json")
    pdf_path = os.path.join(DD.PREVIEW_DIR, PREVIEW_PDF_NAME)
    manifest_path = os.path.join(DD.PREVIEW_DIR, "manifest.json")
    pages_arg = ",".join(str(index) for index in range(1, TOTAL_PAGES + 1))
    summary = {"pptx": DD.PPTX_PATH, "preview_dir": DD.PREVIEW_DIR, "steps": [], "verdict": "fail"}

    # ---------------------------------------------------------- 第 1 步：PNG
    print("第 1 步 / 渲染全部 %d 页 PNG（render 要求 fresh 目录）" % TOTAL_PAGES)
    try:
        _clean_preview(DD.PREVIEW_DIR, DD.DOC_DIR)
    except RuntimeError as error:
        print("  %s" % error)
        return 2
    render_command = [NODE, LIBREOFFICE_CLI, "render", "--input", DD.PPTX_PATH,
                      "--output-dir", DD.PREVIEW_DIR, "--pages", pages_arg, "--dpi", "144"]
    code, output = _run(render_command)
    summary["steps"].append({"id": "render_png", "command": render_command, "exit_code": code})
    _tail(output)
    if code != 0:
        print("渲染 PNG 失败，终止。")
        summary["steps"][-1]["output_tail"] = output.splitlines()[-6:]
        _report(summary)
        return 1

    png_files = sorted(name for name in os.listdir(DD.PREVIEW_DIR)
                       if name.startswith("page-") and name.endswith(".png"))

    # ---------------------------------------------------------- 第 2 步：PDF
    print()
    print("第 2 步 / 渲染 PDF")
    convert_command = [NODE, LIBREOFFICE_CLI, "convert", "--input", DD.PPTX_PATH,
                       "--output", pdf_path]
    code, output = _run(convert_command)
    summary["steps"].append({"id": "convert_pdf", "command": convert_command, "exit_code": code})
    _tail(output, 6)
    pdf_ok = code == 0 and os.path.isfile(pdf_path) and os.path.getsize(pdf_path) > 0

    # ---------------------------------------------------------- 第 3 步：结构校验
    print()
    print("第 3 步 / 结构校验（check_office.py）")
    check_command = [sys.executable, CHECK_OFFICE, DD.PPTX_PATH, "--out", checks_path,
                     "--count", str(TOTAL_PAGES), "--contains", "结论", "--contains", "附录"]
    code, output = _run(check_command)
    summary["steps"].append({"id": "check_office", "command": check_command, "exit_code": code})
    _tail(output, 8)
    report = _read_json(checks_path)

    # ---------------------------------------------------------- 附加：渲染边距自检
    print()
    print("附加检查 / 渲染边距自检（内容越界风险；不是视觉验收）")
    print("  环带口径：距页边 1–7 px（渲染器固有最外 1 px 整圈边框已排除，实测灰度 149）")
    edge = _edge_ink_check(DD.PREVIEW_DIR, png_files)
    if edge:
        dirty = [item for item in edge if item.get("clean") is False]
        errored = [item for item in edge if "error" in item]
        sizes = sorted({tuple(item["size"]) for item in edge if "size" in item})
        print("  渲染尺寸        : %s" % (sizes or "未知"))
        print("  边缘留白达标    : %d / %d 页" % (len(edge) - len(dirty) - len(errored), len(edge)))
        for item in dirty[:10]:
            print("    [疑似贴边] %s 边缘最暗灰度=%d" % (item["file"], item["min_gray_at_edge"]))
        for item in errored[:5]:
            print("    [读取失败] %s：%s" % (item["file"], item["error"]))
    else:
        dirty, errored = [], []

    # ---------------------------------------------------------- 判定
    print()
    _rule("=")
    print("校验结果汇总")
    _rule("=")
    problems = []

    verdict = (report or {}).get("verdict")
    slides = ((report or {}).get("summary") or {}).get("slides")
    checks = (report or {}).get("checks") or []
    print("  checks.json verdict      : %s" % verdict)
    print("  checks.json slides       : %s" % slides)
    for item in checks:
        print("    - %-9s %s%s" % (item.get("id"), item.get("status"),
                                   ("（%s）" % item.get("text")) if item.get("text") else ""))
    if report is None:
        problems.append("checks.json 未生成")
    else:
        if verdict != "pass":
            problems.append("verdict != pass（%s）" % verdict)
        if slides != TOTAL_PAGES:
            problems.append("页数不是 %d（实际 %s）" % (TOTAL_PAGES, slides))
        failed = [item.get("id") for item in checks if item.get("status") != "pass"]
        if failed:
            problems.append("校验项未通过：%s" % ", ".join(str(f) for f in failed))

    manifest = _read_json(manifest_path) or {}
    missing_fonts = manifest.get("missingFonts")
    page_count = manifest.get("pageCount")
    print("  manifest.json pageCount  : %s" % page_count)
    print("  manifest.json missingFonts: %s" % (missing_fonts if missing_fonts is not None else "（缺失）"))
    print("  PNG 文件数                : %d（期望 %d）" % (len(png_files), TOTAL_PAGES))
    print("  PDF                       : %s" % (
        "%s（%.1f KB）" % (pdf_path, os.path.getsize(pdf_path) / 1024.0) if pdf_ok else "缺失或为空"))
    if not manifest:
        problems.append("manifest.json 未生成")
    else:
        if page_count != TOTAL_PAGES:
            problems.append("manifest pageCount 不是 %d（实际 %s）" % (TOTAL_PAGES, page_count))
        if missing_fonts:
            problems.append("missingFonts 非空：%s" % missing_fonts)
    if len(png_files) != TOTAL_PAGES:
        problems.append("PNG 数量不是 %d" % TOTAL_PAGES)
    if not pdf_ok:
        problems.append("PDF 渲染失败")

    # 视觉检查结论（由具备图像输入能力的视觉模型在渲染完成后逐页检查并落盘）
    visual_path = os.path.join(DD.REPORT_DIR, "visual_check.json")
    visual = _read_json(visual_path)
    if visual:
        visual_pages = visual.get("pages_checked")
        visual_problems = visual.get("problems") or []
        print("  视觉检查（视觉模型）      : 已执行，检查页数 %s，问题 %d 项"
              % ("%s" % visual_pages if visual_pages is not None else "未标注", len(visual_problems)))
        for item in visual_problems[:6]:
            print("    - %s" % (item if isinstance(item, str) else json.dumps(item, ensure_ascii=False)))
    else:
        print("  视觉检查（视觉模型）      : 待执行 —— 渲染产物已就绪（%d 张 PNG），"
              "结论应写入 %s" % (len(png_files), visual_path))

    summary.update({
        "verdict": "pass" if not problems else "fail",
        "checks_json": report,
        "manifest": {k: v for k, v in manifest.items() if k != "images"},
        "png_count": len(png_files),
        "pdf": {"path": pdf_path, "bytes": os.path.getsize(pdf_path) if pdf_ok else 0},
        "edge_check": edge,
        "visual_check": (dict(visual, status=visual.get("status", "present")) if visual
                         else {"status": "pending", "expected_path": visual_path}),
        "problems": problems,
    })
    _report(summary)

    _rule("=")
    if problems:
        print("结论：不通过")
        for problem in problems:
            print("  [问题] %s" % problem)
        return 1
    print("结论：通过（verdict=pass，页数=%d，missingFonts 为空，PDF + %d 张 PNG 已生成）"
          % (TOTAL_PAGES, len(png_files)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
