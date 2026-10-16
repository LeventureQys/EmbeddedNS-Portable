# -*- coding: utf-8 -*-
"""
T4-I 自测总控

流程
----
1. 生成合成数据（`_selftest/data`；另生成 `_selftest/data_inject` 用于 FigT22 检出分支）
2. 用**干净数据**跑全部 `fig_*.py`（经 `run_all.py --skip-deck`），图输出到 `_selftest/figures`
3. 对每张 PNG 做像素尺寸 + 版式校验（`check_layout.py`）
4. 复跑一次，比较 PNG 内容哈希，确认**幂等**
5. 用注入异常的数据再跑一次「异常检测类」图件，验证 FigT22 两条分支都会出图
6. 输出自测报告 `_selftest/selftest_report.txt` 与 `_selftest/per_figure.json`

全程不触碰 `ENC/Test/data/` 与 `ENC/Document/figures/` 正式目录。

用法：`python self_test.py`
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import subprocess
import sys
import time

import common as K
import fig_style as S

HERE = os.path.dirname(os.path.abspath(__file__))
SELFTEST = os.path.join(HERE, "_selftest")
DATA = os.path.join(SELFTEST, "data")
DATA_INJECT = os.path.join(SELFTEST, "data_inject")
FIGS = os.path.join(SELFTEST, "figures")
FIGS_INJECT = os.path.join(SELFTEST, "figures_inject")

PY = sys.executable

#: 逐图期望版位（与 check_layout.LAYOUT_OF 一致）
EXPECT_LAYOUT = {
    "FigT01": "half", "FigT02": "full", "FigT03": "strip", "FigT04": "half",
    "FigT05": "full", "FigT06": "full", "FigT07": "strip", "FigT08": "half",
    "FigT09": "strip", "FigT10": "strip", "FigT11": "strip", "FigT12": "strip",
    "FigT13": "half", "FigT14": "half", "FigT15": "half", "FigT16": "strip",
    "FigT17": "full", "FigT18": "half", "FigT19": "half", "FigT20": "full",
    "FigT21": "half", "FigT22": "half",
}

#: 允许「数据缺失 → SKIP」的图件（合成数据完备时应当全部产出）
OPTIONAL = {"FigT20"}

LINE = "=" * 100


def _env(data_root: str, fig_out: str) -> dict:
    env = dict(os.environ)
    env["PYTHONIOENCODING"] = "utf-8"
    env["PYTHONUTF8"] = "1"
    env["NS_DATA_DIR"] = data_root
    env["NS_FIG_OUT"] = fig_out
    env.pop("NS_SELFTEST", None)
    return env


def run(argv: list[str], env: dict, log: list[str], tag: str) -> tuple[int, float, str]:
    t0 = time.perf_counter()
    proc = subprocess.run(argv, cwd=HERE, env=env, capture_output=True, text=True,
                          encoding="utf-8", errors="replace")
    dt = time.perf_counter() - t0
    out = (proc.stdout or "") + (proc.stderr or "")
    log.append("\n" + "-" * 100)
    log.append("[%s] %s   退出码=%d  耗时=%.2f s" % (tag, " ".join(argv[1:]), proc.returncode, dt))
    log.append("-" * 100)
    log.append(out.rstrip())
    return proc.returncode, dt, out


def tree_snapshot(root: str) -> dict:
    """目录树快照：{相对路径: sha256前16位}（用于证明未触碰正式产物）。"""
    out: dict = {}
    if not os.path.isdir(root):
        return out
    for base, _dirs, files in os.walk(root):
        for f in files:
            p = os.path.join(base, f)
            try:
                with open(p, "rb") as fh:
                    out[os.path.relpath(p, root).replace("\\", "/")] = \
                        hashlib.sha256(fh.read()).hexdigest()[:16]
            except OSError:
                out[os.path.relpath(p, root).replace("\\", "/")] = "unreadable"
    return out


def hashes(figdir: str) -> dict:
    out = {}
    if not os.path.isdir(figdir):
        return out
    for f in sorted(os.listdir(figdir)):
        if f.lower().endswith(".png"):
            with open(os.path.join(figdir, f), "rb") as fh:
                out[f] = hashlib.sha256(fh.read()).hexdigest()[:16]
    return out


def fig_key(filename: str) -> str:
    m = re.match(r"(FigT\d\d)", filename)
    return m.group(1) if m else filename


def _probe_nan_read(data_root: str) -> dict:
    """子进程中验证 `common.read_metrics` 对 `nan` 字面量的容错（不得崩溃、不得替换为 0）。"""
    code = (
        "import os,sys,numpy as np;"
        "sys.path.insert(0, r'%s');"
        "os.environ['NS_DATA_DIR']=r'%s';"
        "import common as K;"
        "m=K.read_metrics('baseline','L3','1');"
        "a=m['speech_prob_mean'];"
        "n=int(np.sum(np.isnan(a)));"
        "z=int(np.sum(a==0));"
        "rep=K.nan_report(m);"
        "print('NANCOUNT',n);print('ZEROCOUNT',z);print('REPORT',rep['speech_prob_mean'])"
        % (HERE, data_root)
    )
    proc = subprocess.run([PY, "-c", code], cwd=HERE, capture_output=True, text=True,
                          encoding="utf-8", errors="replace")
    out = (proc.stdout or "") + (proc.stderr or "")
    nan_count = None
    zero_count = None
    for line in out.splitlines():
        if line.startswith("NANCOUNT "):
            nan_count = int(line.split()[1])
        elif line.startswith("ZEROCOUNT "):
            zero_count = int(line.split()[1])
    return {"ok": proc.returncode == 0 and (nan_count or 0) > 0,
            "code": proc.returncode, "nan_count": nan_count, "zero_count": zero_count,
            "raw": out.strip()[-400:]}


def _probe_contamination(data_root: str) -> list[str]:
    """探测正式数据目录里是否混入**本自测**的合成产物。

    注意：正式 `data/` 与 `data/reference/` 是并行任务（T1/T2/T6）的真实测量产物，
    文件名与主流程一致（含 `summary_reference_L3.csv`），因此不能按文件名判定。
    唯一可靠特征是本生成器写入 `build_info.txt` 的 `SELFTEST-SYNTHETIC` 标记。
    """
    found: list[str] = []
    if not os.path.isdir(data_root):
        return found
    for base, _dirs, files in os.walk(data_root):
        for f in files:
            if f != "build_info.txt":
                continue
            p = os.path.join(base, f)
            rel = os.path.relpath(p, data_root).replace("\\", "/")
            try:
                with open(p, "r", encoding="utf-8", errors="replace") as fh:
                    if "SELFTEST-SYNTHETIC" in fh.read():
                        found.append(rel)
            except OSError:
                pass
    return found


def contaminations_hint(before: dict, after: dict) -> list[str]:
    """正式图件目录的新增文件（供人工判断是否由别人写入）。"""
    return sorted(k for k in after if k not in before)


def _diff_snap(before: dict, after: dict) -> dict:
    """快照差异：{相对路径: 'added'|'removed'|'modified'}。"""
    out: dict = {}
    for k in set(before) | set(after):
        if k not in before:
            out[k] = "added"
        elif k not in after:
            out[k] = "removed"
        elif before[k] != after[k]:
            out[k] = "modified"
    return out


def main() -> int:
    # 控制台编码兜底：自测报告含 ✓/✗/× 等符号，在 GBK(936) 控制台下 `print`
    # 会抛 UnicodeEncodeError 把自测直接打断（表现为"自测失败"而非"图件不合格"）。
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(errors="replace")
        except (AttributeError, OSError, ValueError):
            pass

    log: list[str] = []
    log.append(LINE)
    log.append("T4-I 自测报告  %s" % time.strftime("%Y-%m-%d %H:%M:%S"))
    log.append("  分析目录 : %s" % HERE)
    log.append("  数据目录 : %s  (K.data_dir()=%s)" % (DATA, K.data_dir()))
    log.append("  图件目录 : %s  (K.figures_dir()=%s)" % (FIGS, K.figures_dir()))
    log.append("  Python   : %s" % PY)
    log.append(LINE)

    results: dict = {"steps": [], "figures": [], "idempotent": None}

    # ---------------- 0) 前置守卫：确认不会写正式目录 ----------------
    formal_data = os.path.join(K.repo_root(), "ENC", "Test", "data")
    formal_figs = os.path.join(K.repo_root(), "ENC", "Document", "figures")
    #: 正式产物完整快照（含 CSV/WAV 与 PNG），结束时逐一比对
    snap_data_before = tree_snapshot(formal_data)
    snap_figs_before = tree_snapshot(formal_figs)
    env_clean = _env(DATA, FIGS)
    assert env_clean["NS_DATA_DIR"] == DATA and env_clean["NS_FIG_OUT"] == FIGS
    print("[self_test] 正式数据目录（只读保护）: %s  文件 %d 个"
          % (formal_data, len(snap_data_before)))
    print("[self_test] 正式图件目录（只读保护）: %s  文件 %d 个"
          % (formal_figs, len(snap_figs_before)))

    # ---------------- 1) 生成合成数据 ----------------
    print("\n[1/6] 生成合成数据 …")
    code, dt, out = run([PY, "make_selftest_data.py"], _env(DATA, FIGS), log, "GEN-CLEAN")
    if code != 0:
        log.append("\n** 合成数据生成失败，自测中止 **")
        _write(log, results, ok=False)
        return 1
    m = re.search(r"生成文件数\s*:\s*(\d+)", out)
    print("      干净数据集：%s 个 WAV+metrics" % (m.group(1) if m else "?"))
    results["steps"].append({"name": "make_selftest_data(clean)", "code": code, "seconds": dt})

    print("[1b/6] 生成注入异常数据集 …")
    code_i, dt_i, _ = run([PY, "make_selftest_data.py", "--inject", "--out", DATA_INJECT],
                          _env(DATA_INJECT, FIGS_INJECT), log, "GEN-INJECT")
    results["steps"].append({"name": "make_selftest_data(inject)", "code": code_i, "seconds": dt_i})

    # ---------------- 2) 出图（干净数据） ----------------
    print("\n[2/6] 用干净数据跑全部 fig_*.py …")
    code, dt_all, out_all = run([PY, "run_all.py", "--skip-deck"], env_clean, log, "RUN-ALL")
    results["steps"].append({"name": "run_all.py --skip-deck", "code": code, "seconds": dt_all})
    if code != 0:
        log.append("\n** run_all.py 未通过，仍继续做逐图校验以便定位 **")
    for line in out_all.splitlines():
        if "退出码=" in line or "[artifacts]" in line or "流水线" in line:
            print("      " + line.strip())

    # ---------------- 3) 逐图校验 ----------------
    print("\n[3/6] 版式与像素尺寸校验 …")
    code, dt, out = run([PY, "check_layout.py"], env_clean, log, "CHECK-LAYOUT")
    results["steps"].append({"name": "check_layout.py", "code": code, "seconds": dt})
    for line in out.splitlines():
        if line.startswith(("OK", "FAIL", "[check_layout] 合计")):
            print("      " + line)

    report_path = os.path.join(SELFTEST, "layout_report.json")
    layout_rows = []
    if os.path.isfile(report_path):
        with open(report_path, encoding="utf-8") as fh:
            layout_rows = json.load(fh)
    by_file = {r["file"]: r for r in layout_rows}

    # 逐图汇总（尺寸 / 版位 / 校验结论）
    h1 = hashes(FIGS)
    for fname in sorted(h1):
        key = fig_key(fname)
        row = by_file.get(fname, {})
        want = S.LAYOUT_SIZE.get(EXPECT_LAYOUT.get(key, "full"))
        results["figures"].append({
            "file": fname, "figure": key,
            "layout": EXPECT_LAYOUT.get(key, "?"),
            "size": row.get("size"), "expected_size": list(want) if want else None,
            "size_ok": row.get("size_ok"), "layout_ok": row.get("ok"),
            "ink_fraction": row.get("ink_fraction"),
            "text_max_row_height": (row.get("text_row_heights") or {}).get("max"),
            "sha256_16": h1[fname],
        })

    # ---------------- 4) 幂等复跑 ----------------
    print("\n[4/6] 幂等性复跑 …")
    code2, dt2, _ = run([PY, "run_all.py", "--skip-deck"], env_clean, log, "RUN-ALL-2nd")
    results["steps"].append({"name": "run_all.py 2nd pass", "code": code2, "seconds": dt2})
    h2 = hashes(FIGS)
    same = h1 == h2
    diff = sorted(set(h1) ^ set(h2)) + [k for k in h1 if k in h2 and h1[k] != h2[k]]
    results["idempotent"] = {"identical": same, "differing": diff,
                             "count": len(h1), "second_pass_code": code2}
    print("      两次运行 PNG 内容哈希一致: %s（%d 张）" % ("是" if same else "否 → %s" % diff, len(h1)))

    # ---------------- 5) 异常数据分支（FigT22 检出 + FigT10 NaN 呈现） ----------------
    print("\n[5/6] 注入异常数据 → 验证 FigT22 检出分支 与 FigT10 的 NaN 显式呈现 …")
    code3, dt3, out3 = run([PY, "run_all.py", "--skip-deck", "--only", "10,22"],
                           _env(DATA_INJECT, FIGS_INJECT), log, "RUN-INJECT")
    results["steps"].append({"name": "run_all.py --only 10,22 (inject)", "code": code3, "seconds": dt3})
    inj_msg = ""
    for line in out3.splitlines():
        if line.strip().startswith(("未检出异常", "检出")):
            inj_msg = line.strip()
            print("      " + inj_msg)
    nan10 = ""
    for line in out3.splitlines():
        if "NaN/Inf 帧已用竖带标出" in line:
            nan10 = line.strip()
            print("      " + nan10)
    inj_nan_counts = []
    for line in out3.splitlines():
        s = line.strip()
        if s.startswith("baseline ") and "NaN/Inf:" in s:
            inj_nan_counts.append(s)
            print("      " + s)
    results["inject_branch"] = {
        "code": code3, "message": inj_msg, "nan_note_t10": nan10,
        "nan_counts": inj_nan_counts,
        "png_t22_exists": os.path.isfile(
            os.path.join(FIGS_INJECT, "FigT22_失败与边界案例.png")),
        "png_t10_exists": os.path.isfile(
            os.path.join(FIGS_INJECT, "FigT10_speech_prob_mean轨迹.png")),
    }
    # 注入数据下 FigT22 必须检出异常；且不得抛异常（退出码 0）
    if not inj_msg.startswith("检出"):
        print("      [!] 注入异常数据后 FigT22 未检出异常，检测逻辑可能有误")

    # NaN 读取健壮性：common.py 必须把 nan 字面量读成 NaN 而非崩溃
    nan_probe = _probe_nan_read(DATA_INJECT)
    results["nan_read"] = nan_probe
    print("      common.py 读取含 nan 的 CSV: %s（baseline/speech_prob_mean NaN=%s）"
          % ("正常" if nan_probe["ok"] else "**失败**", nan_probe.get("nan_count")))

    # 干净数据下 FigT22 应当报「未检出异常」
    clean_msg = ""
    for line in out_all.splitlines():
        if line.strip().startswith(("未检出异常", "检出")):
            clean_msg = line.strip()
    results["clean_branch"] = {"message": clean_msg}
    print("      干净数据分支: %s" % (clean_msg or "(未捕获)"))

    # ---------------- 6) 正式目录未被触碰 ----------------
    print("\n[6/6] 正式目录保护检查 …")
    # 正式 data/ 由并行测量任务（T1/T2/T6）写入；正式 figures/ 由 T4-II 与人工运行写入。
    # 二者都**合法**，因此本自测只判定一件事：有没有混入**本自测**的合成数据特征。
    contamination = _probe_contamination(formal_data)
    snap_figs_after = tree_snapshot(formal_figs)
    d_figs = _diff_snap(snap_figs_before, snap_figs_after)
    # 只把"被我覆盖/删除的既有正式图件"视为越界；新增文件属其它任务的正常产出
    overwritten = {k: v for k, v in d_figs.items() if v in ("modified", "removed")}
    touched_figs = {k: v for k, v in d_figs.items() if v == "added"}
    untouched = (not contamination) and (not overwritten)
    results["formal_untouched"] = {
        "contamination": contamination,
        "overwritten": overwritten,
        "added_by_others": sorted(touched_figs),
        "figure_files": len(snap_figs_after),
        "data_note": "正式 data/ 由并行测量任务写入，仅做合成数据特征探测",
        "figures_note": "正式 figures/ 新增文件属其它任务产出，不计越界；覆盖/删除才算",
    }
    print("      正式数据目录合成数据特征: %s"
          % ("无（未被本自测污染）" if not contamination else "**发现 %s**" % contamination))
    print("      正式图件目录被覆盖/删除  : %s（%d 个文件）"
          % ("无" if not overwritten else "**%s**" % overwritten, len(snap_figs_after)))
    if touched_figs:
        print("      其它任务新增的正式图件（不计越界）: %s" % sorted(touched_figs))

    # ---------------- 结论 ----------------
    n_fig = len(results["figures"])
    bad = [f for f in results["figures"] if not (f["size_ok"] and f["layout_ok"])]
    missing = [k for k in EXPECT_LAYOUT if k not in {f["figure"] for f in results["figures"]}]
    ok = (code == 0 and code2 == 0 and not bad and not missing and same
          and results["inject_branch"]["png_t22_exists"]
          and results["inject_branch"]["png_t10_exists"]
          and results["nan_read"]["ok"] and untouched)
    log.append("\n" + LINE)
    log.append("结论")
    log.append(LINE)
    log.append("  生成图件      : %d 张" % n_fig)
    log.append("  版式校验失败  : %d 张 %s" % (len(bad), [f["file"] for f in bad] if bad else ""))
    log.append("  缺失图件      : %s" % (missing or "无"))
    log.append("  幂等          : %s" % ("是" if same else "否"))
    log.append("  FigT22 检出分支: %s" % results["inject_branch"]["message"])
    log.append("  FigT22 干净分支: %s" % results["clean_branch"]["message"])
    log.append("  FigT10 NaN 标注: %s" % (results["inject_branch"]["nan_note_t10"] or "(无)"))
    log.append("  NaN 字面量读取  : %s (NaN=%s, 归零计数=%s)"
               % ("正常" if results["nan_read"]["ok"] else "失败",
                  results["nan_read"].get("nan_count"), results["nan_read"].get("zero_count")))
    log.append("  正式产物未被触碰: %s"
               % ("是" if untouched else "否 %s%s" % (contamination, overwritten)))
    log.append("  整体          : %s" % ("通过" if ok else "未通过"))

    print("\n" + LINE)
    print("自测结论: %s（图件 %d 张，失败 %d 张，缺失 %s，正式目录%s被触碰）"
          % ("通过" if ok else "未通过", n_fig, len(bad), missing or "无",
             "未" if untouched else "**已**"))
    print(LINE)
    _write(log, results, ok=ok)
    return 0 if ok else 1


def _write(log: list[str], results: dict, ok: bool) -> None:
    os.makedirs(SELFTEST, exist_ok=True)
    txt = os.path.join(SELFTEST, "selftest_report.txt")
    with open(txt, "w", encoding="utf-8") as fh:
        fh.write("\n".join(log) + "\n")
    js = os.path.join(SELFTEST, "selftest_report.json")
    with open(js, "w", encoding="utf-8") as fh:
        json.dump({"ok": ok, **results}, fh, ensure_ascii=False, indent=2)
    print("[self_test] 报告: %s" % txt)
    print("[self_test] 报告: %s" % js)


if __name__ == "__main__":
    sys.exit(main())
