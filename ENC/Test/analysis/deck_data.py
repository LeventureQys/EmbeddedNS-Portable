# -*- coding: utf-8 -*-
"""deck_data.py — NS 对比测试报告 PPT 的数据装配层（T4-II 产出）。

职责
----
为 ``build_deck.py`` 提供"缺数据不崩溃"的读取能力：逐帧指标 CSV、汇总 CSV、
构建指纹 txt、16bit 单声道 WAV 波形，以及跨文件聚合统计。

本模块 **独立实现**，不 import ``common.py``（后者由并行任务 T4-I 维护，
两者不得互相耦合）。仅依赖标准库 + numpy。

路径契约（全部可用环境变量覆盖，默认指向正式交付路径）
------------------------------------------------------
============================  ================================================
环境变量                       默认值
============================  ================================================
``NSDECK_REPO_ROOT``          由本文件位置推导：``<repo>/ENC/Test/analysis/``
``NSDECK_DATA_DIR``           ``<repo>/ENC/Test/data``
``NSDECK_FIGURES_DIR``        ``<repo>/ENC/Document/figures``
``NSDECK_OUT_DIR``            ``<repo>/ENC/Document``
``NSDECK_PPTX``               ``<OUT_DIR>/NS组件化与对比测试报告.pptx``
``NSDECK_PREVIEW_DIR``        ``<OUT_DIR>/preview``
``NSDECK_REPORT_DIR``         ``<analysis>/_reports``（自检 JSON 落盘处）
``NSDECK_DATE``               报告日期字符串；留空则用构建指纹里的 build_time
============================  ================================================

数据契约（计划书 6.1–6.4）
--------------------------
- ``<DATA_DIR>/<variant>/<level>/metrics_<name>.csv``  逐帧指标，12 列
- ``<DATA_DIR>/<variant>/<level>/summary_<variant>_<level>.csv``  逐文件汇总，10 列
- ``<DATA_DIR>/<variant>/build_info.txt``  构建指纹
- ``<DATA_DIR>/<variant>/<level>/<name>.wav``  16bit/单声道/48kHz 输出音频

``variant`` ∈ {baseline, d1, d1d5, d1d5d6, optimized}；``level`` ∈ {L0, L1, L3}；
``name`` ∈ {1..9}。任何缺失一律返回 ``None`` 并追加 warning，绝不抛异常。

**nan 语义**：NaN 是被测对象（固件）的固有缺陷表现（D8：``fast_math.c`` 的
``fastpow2`` 定义域缺陷），不是测量故障。它只出现在**诊断量列
``speech_prob_mean``**（``avg_log_lrt`` 越界 → ``fastexp`` 得 NaN →
``speech_probability`` 为 NaN；``wiener_filter.c`` 的 ``MIN(NaN, 1.0f)`` 在 C 宏
语义下返回 ``1.0f``，把 NaN 挡在滤波器之外，故 ``atten_db`` / ``in_rms`` /
``out_rms`` 与输出音频全部正常）。读取层保留 nan 原值，格式化层如实输出 ``nan``，
统计层自动剔除并单独计数（``nan_count`` / ``nan_files`` /
``count_nan_frames``），**绝不伪造成 0**。
"""

from __future__ import annotations

import csv
import glob
import hashlib
import os
import sys
import wave
from collections import OrderedDict

try:
    import numpy as np
except ImportError:  # pragma: no cover - 环境正常时不会走到
    np = None

try:  # 保证在 Windows 控制台/管道下中文可打印
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")
except Exception:  # pragma: no cover
    pass

HERE = os.path.dirname(os.path.abspath(__file__))


def _env_path(name: str, default: str) -> str:
    """读环境变量做路径覆盖；空字符串视为未设置。"""
    value = os.environ.get(name)
    return os.path.abspath(value) if value else os.path.abspath(default)


# --------------------------------------------------------------- 路径契约
REPO_ROOT = _env_path("NSDECK_REPO_ROOT", os.path.join(HERE, "..", "..", ".."))
DATA_DIR = _env_path("NSDECK_DATA_DIR", os.path.join(REPO_ROOT, "ENC", "Test", "data"))
FIGURES_DIR = _env_path(
    "NSDECK_FIGURES_DIR", os.path.join(REPO_ROOT, "ENC", "Document", "figures")
)
DOC_DIR = _env_path("NSDECK_OUT_DIR", os.path.join(REPO_ROOT, "ENC", "Document"))
PPTX_NAME = "NS组件化与对比测试报告.pptx"
PPTX_PATH = _env_path("NSDECK_PPTX", os.path.join(DOC_DIR, PPTX_NAME))
PREVIEW_DIR = _env_path("NSDECK_PREVIEW_DIR", os.path.join(DOC_DIR, "preview"))
REPORT_DIR = _env_path("NSDECK_REPORT_DIR", os.path.join(HERE, "_reports"))
REPORT_NAME = "NS组件化与对比测试报告"

# --------------------------------------------------------------- 数据契约
# 消融五组。patch_flags 位定义：bit0=D1、bit1=D5、bit2=D6、bit3=D8（D8 为本次新发现）
VARIANTS = ("baseline", "d1", "d1d5", "d1d5d6", "optimized")
PATCH_FLAG_BITS = OrderedDict([("D1", 1), ("D5", 2), ("D6", 4), ("D8", 8)])
LEVELS = ("L0", "L1", "L3")
FILE_NAMES = tuple(str(i) for i in range(1, 10))

# --------------------------------------------------------------- DNS-MOS 感知质量
# 数据源：<DATA_DIR>/dnsmos/dnsmos_summary.csv（7 变体 × 4 指标均值）
#         <DATA_DIR>/dnsmos/dnsmos_detail.csv （逐文件明细）
DNSMOS_DIR = os.path.join(DATA_DIR, "dnsmos")
DNSMOS_SUMMARY_NAME = "dnsmos_summary.csv"
DNSMOS_DETAIL_NAME = "dnsmos_detail.csv"
DNSMOS_METRICS = ("SIG", "BAK", "OVRL", "P808_MOS")
DNSMOS_VARIANTS = ("input", "baseline", "d1", "d1d5", "d1d5d6", "optimized", "reference")
DNSMOS_LABELS = OrderedDict([
    ("input", "不处理（原始输入）"),
    ("baseline", "基线（对齐固件）"),
    ("d1", "基线 + D1"),
    ("d1d5", "基线 + D1 + D5"),
    ("d1d5d6", "基线 + D1 + D5 + D6"),
    ("optimized", "优化（含 D8）"),
    ("reference", "参照路径（ENC_Core）"),
])

METRIC_COLUMNS = (
    "frame_idx",
    "in_rms",
    "out_rms",
    "atten_db",
    "signal_energy",
    "signal_spectral_sum",
    "diff_normalization",
    "speech_prob_mean",
    "prior_speech_prob",
    "gain_adjustment",
    "clamp_hits",
    "elapsed_us",
)
METRIC_INT_COLUMNS = frozenset({"frame_idx", "clamp_hits", "elapsed_us"})

SUMMARY_COLUMNS = (
    "file",
    "duration_s",
    "frames",
    "total_elapsed_ms",
    "in_rms",
    "out_rms",
    "atten_db",
    "in_peak",
    "out_peak",
    "clamp_hits_total",
)

# 图件版位（计划书 9.2）：幻灯片占用英寸 + 画布像素
FIGURE_BOXES = {
    "full": {"w_in": 12.13, "h_in": 5.70, "px": (1800, 845)},
    "half": {"w_in": 5.90, "h_in": 4.10, "px": (900, 625)},
    "strip": {"w_in": 12.13, "h_in": 3.00, "px": (1800, 445)},
}
BOX_LABEL = {"full": "整页图", "half": "半页图", "strip": "条带图"}

# 22 个图件：文件名逐字冻结（计划书 9.4）。单一事实源，供 build_deck 与附录表共用。
FIGURE_SPECS = OrderedDict(
    [
        ("FigT01", dict(file="FigT01_九文件衰减汇总.png", box="half",
                        desc="9 个文件的时长与能量衰减汇总柱状图（基线 vs 优化双柱）",
                        source="各 summary_*.csv", script="fig_summary.py")),
        ("FigT02", dict(file="FigT02_组件架构与数据流.png", box="full",
                        desc="组件架构与数据流（标注真实文件路径）",
                        source="静态绘制（无数据源）", script="静态绘制")),
        ("FigT03", dict(file="FigT03_波形对比全段.png", box="strip",
                        desc="波形对比（1.wav 全段）：原始 / 基线 / 优化三行",
                        source="输入 WAV + 两份输出 WAV", script="fig_waveform.py")),
        ("FigT04", dict(file="FigT04_波形对比放大.png", box="half",
                        desc="波形对比（放大 0.5 s 片段，取 5–5.5 s）",
                        source="同上", script="fig_waveform.py")),
        ("FigT05", dict(file="FigT05_声谱三联.png", box="full",
                        desc="声谱图三联（原始 / 基线 / 优化）：512 点 STFT，汉宁窗，50% 重叠",
                        source="输入 WAV + 两份输出 WAV", script="fig_spectrogram.py")),
        ("FigT06", dict(file="FigT06_声谱差异.png", box="full",
                        desc="声谱差异图（基线谱 − 优化谱，dB）",
                        source="同上", script="fig_spectrogram.py")),
        ("FigT07", dict(file="FigT07_signal_energy轨迹.png", box="strip",
                        desc="signal_energy 轨迹（基线 vs 优化，对数纵轴）",
                        source="metrics_1.csv 两份", script="fig_trace.py")),
        ("FigT08", dict(file="FigT08_Y257反推.png", box="half",
                        desc="Y[257] 取值由 signal_energy 反推",
                        source="同上", script="fig_summary.py")),
        ("FigT09", dict(file="FigT09_diff_normalization轨迹.png", box="strip",
                        desc="diff_normalization 轨迹对比",
                        source="同上", script="fig_trace.py")),
        ("FigT10", dict(file="FigT10_speech_prob_mean轨迹.png", box="strip",
                        desc="speech_prob_mean 轨迹对比",
                        source="同上", script="fig_trace.py")),
        ("FigT11", dict(file="FigT11_gain_adjustment轨迹.png", box="strip",
                        desc="gain_adjustment 轨迹对比",
                        source="同上", script="fig_trace.py")),
        ("FigT12", dict(file="FigT12_逐帧atten_db轨迹.png", box="strip",
                        desc="逐帧 atten_db 轨迹（基线 vs 优化）",
                        source="同上", script="fig_trace.py")),
        ("FigT13", dict(file="FigT13_分频带衰减.png", box="half",
                        desc="分频带衰减柱状对比（0–4 / 4–8 / 8–12 / 12–16 / 16–24 kHz）",
                        source="输入与两份输出 WAV", script="fig_summary.py")),
        ("FigT14", dict(file="FigT14_LTAS对比.png", box="half",
                        desc="LTAS 对比（原始 / 基线 / 优化三条曲线）",
                        source="同上", script="fig_spectrogram.py")),
        ("FigT15", dict(file="FigT15_钳位命中统计.png", box="half",
                        desc="钳位命中统计（逐文件柱状 + 总命中）",
                        source="clamp_hits 列", script="fig_summary.py")),
        ("FigT16", dict(file="FigT16_启动期放大.png", box="strip",
                        desc="启动期（前 200 帧）放大视图",
                        source="metrics_1.csv", script="fig_trace.py")),
        ("FigT17", dict(file="FigT17_消融五组对比.png", box="full",
                        desc="消融五组对比（基线 / +D1 / +D1D5 / +D1D5D6 / +D1D5D6+D8）",
                        source="五份 summary_*.csv", script="fig_summary.py")),
        ("FigT18", dict(file="FigT18_atten_db箱线图.png", box="half",
                        desc="9 个文件的 atten_db 箱线图",
                        source="各 summary_*.csv", script="fig_summary.py")),
        ("FigT19", dict(file="FigT19_单帧耗时分布.png", box="half",
                        desc="单帧耗时分布直方图 + RTF 标注",
                        source="elapsed_us 列", script="fig_summary.py")),
        ("FigT20", dict(file="FigT20_参照形态对比.png", box="full",
                        desc="与 Project/ENC_Core 参照的形态对比（T6 未执行则跳过并说明）",
                        source="data/reference/", script="fig_reference.py")),
        ("FigT21", dict(file="FigT21_无语音段与语音段衰减.png", box="half",
                        desc="无语音段 vs 语音段的衰减对比（按 speech_prob_mean 分档）",
                        source="metrics_1.csv", script="fig_summary.py")),
        ("FigT22", dict(file="FigT22_失败与边界案例.png", box="half",
                        desc="失败与边界案例（如出现 NaN、异常衰减、钳位超限）",
                        source="全部 CSV", script="fig_summary.py")),
        ("FigT23", dict(file="FigT23_DNS-MOS感知质量对比.png", box="full",
                        desc="DNS-MOS 四指标 × 7 变体的感知质量对比",
                        source="dnsmos/dnsmos_summary.csv", script="fig_dnsmos.py")),
    ]
)

FIGURE_IDS = tuple(FIGURE_SPECS.keys())


# --------------------------------------------------------------- 基础工具
def is_nan(value):
    """判断 CSV 中的 ``nan`` 字面量。

    基线数据里的 nan 是**被测对象（固件）的固有缺陷表现**（D8：``fast_math.c``
    的 ``fastpow2`` 定义域缺陷），不是测量故障。报告按原样呈现，不做替换。
    """
    return isinstance(value, float) and value != value


def is_finite(value):
    """可用于统计与制图的有限数值（int / float，排除 nan 与 ±inf）。"""
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        return False
    if isinstance(value, float):
        return value == value and value not in (float("inf"), float("-inf"))
    return True


def _to_float(value):
    """字符串 → float；保留 nan / inf 字面量以便如实呈现，非法值返回 None。"""
    if value is None:
        return None
    text = str(value).strip()
    if not text:
        return None
    try:
        return float(text)
    except ValueError:
        return None


def _to_int(value):
    number = _to_float(value)
    if number is None or not is_finite(number):
        return None
    return int(round(number))


def fmt(value, digits=2, dash="—"):
    """统一数值格式化：None → 破折号；nan / inf 如实呈现，绝不伪造成 0。"""
    if value is None:
        return dash
    if isinstance(value, float):
        if value != value:
            return "nan"
        if value == float("inf"):
            return "inf"
        if value == float("-inf"):
            return "-inf"
        return ("{:.%df}" % digits).format(value)
    if isinstance(value, int):
        return str(value)
    return str(value)


def metrics_path(variant: str, level: str, name: str) -> str:
    return os.path.join(DATA_DIR, variant, level, "metrics_%s.csv" % name)


def summary_path(variant: str, level: str) -> str:
    return os.path.join(DATA_DIR, variant, level, "summary_%s_%s.csv" % (variant, level))


def build_info_path(variant: str) -> str:
    return os.path.join(DATA_DIR, variant, "build_info.txt")


def wav_path(variant: str, level: str, name: str) -> str:
    return os.path.join(DATA_DIR, variant, level, "%s.wav" % name)


def figure_path(fig_id: str) -> str:
    """图件路径。

    ``FigT23`` 为本次新增、文件名尚未冻结：精确名不存在时按 ``FigT23_*.png`` 兜底匹配，
    避免因命名差异退化成占位框。
    """
    spec = FIGURE_SPECS.get(fig_id)
    if spec is None:
        return os.path.join(FIGURES_DIR, fig_id)
    path = os.path.join(FIGURES_DIR, spec["file"])
    if not os.path.isfile(path):
        matches = sorted(glob.glob(os.path.join(FIGURES_DIR, "%s_*.png" % fig_id)))
        if matches:
            return matches[0]
    return path


def dnsmos_summary_path() -> str:
    return os.path.join(DNSMOS_DIR, DNSMOS_SUMMARY_NAME)


def dnsmos_detail_path() -> str:
    return os.path.join(DNSMOS_DIR, DNSMOS_DETAIL_NAME)


# --------------------------------------------------------------- 读取实现
def _read_metrics_file(path: str):
    """读逐帧指标 CSV → {列名: [值...]}；文件缺失/空/表头不符返回 None。"""
    if not os.path.isfile(path):
        return None
    with open(path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.reader(handle)
        try:
            header = next(reader)
        except StopIteration:
            return None
        header = [h.strip() for h in header]
        if tuple(header[: len(METRIC_COLUMNS)]) != METRIC_COLUMNS:
            return None
        columns = {name: [] for name in METRIC_COLUMNS}
        for row in reader:
            if not row:
                continue
            for index, name in enumerate(METRIC_COLUMNS):
                raw = row[index] if index < len(row) else ""
                columns[name].append(_to_int(raw) if name in METRIC_INT_COLUMNS else _to_float(raw))
    return columns


def _read_summary_file(path: str):
    """读汇总 CSV → [{列名: 值}...]。"""
    if not os.path.isfile(path):
        return None
    rows = []
    with open(path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.reader(handle)
        try:
            header = next(reader)
        except StopIteration:
            return None
        header = [h.strip() for h in header]
        if tuple(header[: len(SUMMARY_COLUMNS)]) != SUMMARY_COLUMNS:
            return None
        for raw_row in reader:
            if not raw_row:
                continue
            record = {}
            for index, name in enumerate(SUMMARY_COLUMNS):
                raw = raw_row[index] if index < len(raw_row) else ""
                if name == "file":
                    record[name] = str(raw).strip()
                elif name in ("frames", "clamp_hits_total"):
                    record[name] = _to_int(raw)
                else:
                    record[name] = _to_float(raw)
            rows.append(record)
    return rows


def _read_build_info_file(path: str):
    """读 ``key = value`` 形式的构建指纹（``#`` 起头为注释）。"""
    if not os.path.isfile(path):
        return None
    info = OrderedDict()
    with open(path, "r", encoding="utf-8-sig", errors="replace") as handle:
        for line in handle:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" not in line:
                continue
            key, _, value = line.partition("=")
            value = value.split("#", 1)[0].strip()
            info[key.strip()] = value
    return info or None


def _read_dnsmos_summary(path: str):
    """读 DNS-MOS 汇总 CSV → ``OrderedDict[variant] = {files, SIG, BAK, OVRL, P808_MOS, OVRL_std}``。

    表头固定为 ``variant,files,SIG,BAK,OVRL,P808_MOS,OVRL_std``；缺文件返回 None。
    """
    if not os.path.isfile(path):
        return None
    rows = OrderedDict()
    with open(path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.reader(handle)
        try:
            header = [h.strip() for h in next(reader)]
        except StopIteration:
            return None
        for raw in reader:
            if not raw:
                continue
            record = {}
            for index, name in enumerate(header):
                value = raw[index] if index < len(raw) else ""
                if name == "variant":
                    record["variant"] = value.strip()
                elif name == "files":
                    record["files"] = _to_int(value)
                else:
                    record[name] = _to_float(value)
            if record.get("variant"):
                rows[record["variant"]] = record
    return rows or None


def _read_dnsmos_detail(path: str):
    """读 DNS-MOS 逐文件 CSV → ``[{variant, file, dur_s, segments, SIG, BAK, OVRL, ...}]``。"""
    if not os.path.isfile(path):
        return None
    rows = []
    with open(path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.reader(handle)
        try:
            header = [h.strip() for h in next(reader)]
        except StopIteration:
            return None
        for raw in reader:
            if not raw:
                continue
            record = {}
            for index, name in enumerate(header):
                value = raw[index] if index < len(raw) else ""
                if name in ("variant", "file"):
                    record[name] = value.strip()
                elif name == "segments":
                    record[name] = _to_int(value)
                else:
                    record[name] = _to_float(value)
            rows.append(record)
    return rows or None


def read_wav_mono(path: str):
    if np is None or not os.path.isfile(path):
        return None
    try:
        with wave.open(path, "rb") as handle:
            channels = handle.getnchannels()
            width = handle.getsampwidth()
            frames = handle.getnframes()
            raw = handle.readframes(frames)
    except (wave.Error, OSError, EOFError):
        return None
    if width != 2:
        raise ValueError("仅支持 16bit WAV：%s (sampwidth=%d)" % (path, width))
    data = np.frombuffer(raw, dtype="<i2")
    if channels > 1:
        data = data.reshape(-1, channels)[:, 0]
    return data


# --------------------------------------------------------------- 统计
def aggregate(rows):
    """把 summary 的逐文件行聚合成整段指标。

    ``atten_db`` 按文件时长加权（各文件 RMS 量级不同，此处口径为"时长加权平均"，
    报告中必须保持该措辞）。RTF = 总耗时 / 音频总时长。

    ``atten_db`` 为 nan（D8 缺陷表现）的文件不计入加权平均，其文件名列入
    ``nan_files``，数量列入 ``nan_count``，供页面如实标注。
    """
    if not rows:
        return None
    duration = 0.0
    frames = 0
    elapsed_ms = 0.0
    clamp_total = 0
    weighted = 0.0
    weight_sum = 0.0
    in_rms_values = []
    out_rms_values = []
    in_peak_max = None
    nan_files = []
    for row in rows:
        name = str(row.get("file", "?"))
        d = row.get("duration_s")
        if not is_finite(d):
            d = 0.0
        duration += d
        frames += row.get("frames") or 0
        elapsed = row.get("total_elapsed_ms")
        elapsed_ms += elapsed if is_finite(elapsed) else 0.0
        clamp_total += row.get("clamp_hits_total") or 0
        atten = row.get("atten_db")
        if not is_finite(atten):
            nan_files.append(name)
        elif d > 0:
            weighted += atten * d
            weight_sum += d
        if is_finite(row.get("in_rms")):
            in_rms_values.append(row["in_rms"])
        if is_finite(row.get("out_rms")):
            out_rms_values.append(row["out_rms"])
        peak = row.get("in_peak")
        if is_finite(peak):
            in_peak_max = peak if in_peak_max is None else max(in_peak_max, peak)
    audio_ms = duration * 1000.0
    return {
        "files": len(rows),
        "duration_s": duration,
        "frames": frames,
        "total_elapsed_ms": elapsed_ms,
        "atten_db": (weighted / weight_sum) if weight_sum else None,
        "in_rms": (sum(in_rms_values) / len(in_rms_values)) if in_rms_values else None,
        "out_rms": (sum(out_rms_values) / len(out_rms_values)) if out_rms_values else None,
        "in_peak_max": in_peak_max,
        "clamp_hits_total": clamp_total,
        "rtf": (elapsed_ms / audio_ms) if audio_ms else None,
        "nan_count": len(nan_files),
        "nan_files": nan_files,
        "finite_files": len(rows) - len(nan_files),
        "rows": rows,
    }


def percentile(values, fraction):
    """线性插值分位数；忽略 None / nan / inf；空序列返回 None。"""
    data = sorted(v for v in values if is_finite(v))
    if not data:
        return None
    if len(data) == 1:
        return float(data[0])
    position = (len(data) - 1) * fraction
    low = int(position)
    high = min(low + 1, len(data) - 1)
    weight = position - low
    return float(data[low] * (1.0 - weight) + data[high] * weight)


def column_stats(columns, name, head=None):
    """对某一列给出 mean / min / max / head_mean（前 ``head`` 个样本均值）。

    nan / inf 一律剔除（D8 会让诊断量出现 nan），并单独计数到 ``nan_count``。
    """
    if not columns:
        return None
    raw = columns.get(name, [])
    values = [v for v in raw if is_finite(v)]
    if not values:
        return None
    stats = {
        "count": len(values),
        "nan_count": sum(1 for v in raw if is_nan(v)),
        "mean": sum(values) / len(values),
        "min": min(values),
        "max": max(values),
    }
    if head:
        head_values = [v for v in raw[:head] if is_finite(v)]
        stats["head_mean"] = (sum(head_values) / len(head_values)) if head_values else None
        stats["head_nan_count"] = sum(1 for v in raw[:head] if is_nan(v))
    return stats


def count_nan_frames(variant, level, column="speech_prob_mean"):
    """流式统计“含 nan 的帧数”（不缓存整份列，逐行判定后丢弃）。

    口径：``column`` 在该行为 nan 的行数，**限定在该 ``variant`` / ``level`` 目录下
    存在的全部 ``metrics_<name>.csv``**（不跨 level，避免 T3 期遗留的不完整目录把
    文件数搅乱）。默认列 ``speech_prob_mean`` 是 D8 唯一被污染的列 ——
    ``atten_db`` / ``in_rms`` / ``out_rms`` 在所有变体、所有 level 上均为 0 个 nan。

    返回 ``{"files": n, "frames": total, "nan_frames": n}``；无任何文件时返回 None。
    """
    files = 0
    frames = 0
    nan_frames = 0
    for name in FILE_NAMES:
        path = metrics_path(variant, level, name)
        if not os.path.isfile(path):
            continue
        with open(path, "r", encoding="utf-8-sig", newline="") as handle:
            reader = csv.reader(handle)
            try:
                header = [h.strip() for h in next(reader)]
            except StopIteration:
                continue
            if column not in header:
                continue
            index = header.index(column)
            files += 1
            for row in reader:
                if not row or index >= len(row):
                    continue
                frames += 1
                if "nan" in row[index].strip().lower():
                    nan_frames += 1
    if not files:
        return None
    return {"files": files, "frames": frames, "nan_frames": nan_frames}


def speech_prob_profile(variant, level, column="speech_prob_mean"):
    """流式扫描 ``speech_prob_mean``：nan 帧数 + 非 nan 值的分位数与分布特征。

    用于第 14 页「D8 的影响范围」：给出该列在污染帧之外的真实分布（中位、1%/99% 分位、
    以及贴近 1.0 的样本占比），全部为实测值，不写死任何数字。
    """
    values = []
    nan_frames = 0
    frames = 0
    files = 0
    for name in FILE_NAMES:
        path = metrics_path(variant, level, name)
        if not os.path.isfile(path):
            continue
        with open(path, "r", encoding="utf-8-sig", newline="") as handle:
            reader = csv.reader(handle)
            try:
                header = [h.strip() for h in next(reader)]
            except StopIteration:
                continue
            if column not in header:
                continue
            index = header.index(column)
            files += 1
            for row in reader:
                if not row or index >= len(row):
                    continue
                frames += 1
                text = row[index].strip()
                if "nan" in text.lower():
                    nan_frames += 1
                    continue
                try:
                    values.append(float(text))
                except ValueError:
                    continue
    if not files:
        return None
    values.sort()
    at_one = sum(1 for v in values if v >= 0.999)
    return {
        "files": files,
        "frames": frames,
        "nan_frames": nan_frames,
        "valid": len(values),
        "median": percentile(values, 0.5),
        "p01": percentile(values, 0.01),
        "p99": percentile(values, 0.99),
        "min": values[0] if values else None,
        "max": values[-1] if values else None,
        "share_at_one": (at_one / len(values)) if values else None,
    }


def _file_sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def wav_identical_count(variant_a, variant_b, level, names=FILE_NAMES):
    """比较两个变体在同 level 下输出 WAV 的 SHA256 → ``(相同数, 比较数)``。

    用于「D1 / D5 修复效果严格为零」的逐字节证据（第 15 页），为实测而非引用。
    """
    same = 0
    total = 0
    for name in names:
        path_a = wav_path(variant_a, level, name)
        path_b = wav_path(variant_b, level, name)
        if not (os.path.isfile(path_a) and os.path.isfile(path_b)):
            continue
        total += 1
        if _file_sha256(path_a) == _file_sha256(path_b):
            same += 1
    return same, total


# --------------------------------------------------------------- 数据集
class Dataset:

    def __init__(self):
        self.warnings = []
        self._summary = {}
        self._metrics = {}
        self._build_info = {}
        self._wav = {}
        self._nan_frames = {}
        self._dnsmos = None
        self._dnsmos_detail = None
        self._wav_identical = {}
        self._speech_prob = {}

    # --- 内部
    def _warn(self, message):
        if message not in self.warnings:
            self.warnings.append(message)

    # --- summary / metrics / build_info
    def summary(self, variant, level):
        key = (variant, level)
        if key not in self._summary:
            path = summary_path(variant, level)
            rows = _read_summary_file(path)
            if rows is None:
                self._warn("缺少汇总 CSV：%s" % path)
            self._summary[key] = rows
        return self._summary[key]

    def metrics(self, variant, level, name):
        key = (variant, level, name)
        if key not in self._metrics:
            path = metrics_path(variant, level, name)
            columns = _read_metrics_file(path)
            if columns is None:
                self._warn("缺少逐帧指标 CSV：%s" % path)
            self._metrics[key] = columns
        return self._metrics[key]

    def build_info(self, variant):
        if variant not in self._build_info:
            info = _read_build_info_file(build_info_path(variant))
            if info is None:
                self._warn("缺少构建指纹：%s" % build_info_path(variant))
            self._build_info[variant] = info
        return self._build_info[variant]

    def wav_mono(self, variant, level, name):
        key = (variant, level, name)
        if key not in self._wav:
            path = wav_path(variant, level, name)
            data = read_wav_mono(path)
            if data is None:
                self._warn("缺少输出 WAV：%s" % path)
            self._wav[key] = data
        return self._wav[key]

    # --- 聚合视图
    def aggregate(self, variant, level):
        return aggregate(self.summary(variant, level))

    def nan_frames(self, variant, level, column="speech_prob_mean"):
        """按变体统计逐帧 nan 数量（流式，结果缓存；供第 25 / 27 页的 D8 证据使用）。"""
        key = (variant, level, column)
        if key not in self._nan_frames:
            self._nan_frames[key] = count_nan_frames(variant, level, column)
        return self._nan_frames[key]

    # --- DNS-MOS 感知质量（7 变体：input / baseline / d1 / d1d5 / d1d5d6 / optimized / reference）
    def dnsmos_rows(self):
        """DNS-MOS 汇总表 → OrderedDict[variant] = {files, SIG, BAK, OVRL, P808_MOS, OVRL_std}。"""
        if self._dnsmos is None:
            rows = _read_dnsmos_summary(dnsmos_summary_path())
            if rows is None:
                self._warn("缺少 DNS-MOS 汇总：%s" % dnsmos_summary_path())
            self._dnsmos = rows
        return self._dnsmos

    def dnsmos(self, variant):
        rows = self.dnsmos_rows() or {}
        return rows.get(variant)

    def dnsmos_detail(self, variant=None):
        """DNS-MOS 逐文件明细；给定 ``variant`` 时只返回该变体的行。"""
        if self._dnsmos_detail is None:
            rows = _read_dnsmos_detail(dnsmos_detail_path())
            if rows is None:
                self._warn("缺少 DNS-MOS 明细：%s" % dnsmos_detail_path())
            self._dnsmos_detail = rows
        rows = self._dnsmos_detail or []
        if variant is None:
            return rows
        return [row for row in rows if row.get("variant") == variant]

    def wav_identical(self, variant_a, variant_b, level):
        """两个变体输出 WAV 的逐字节一致性 → ``(相同数, 比较数)``（结果缓存）。"""
        key = (variant_a, variant_b, level)
        if key not in self._wav_identical:
            self._wav_identical[key] = wav_identical_count(variant_a, variant_b, level)
        return self._wav_identical[key]

    def speech_prob(self, variant, level):
        """``speech_prob_mean`` 的 nan 帧数与有效值分布（结果缓存）。"""
        key = (variant, level)
        if key not in self._speech_prob:
            self._speech_prob[key] = speech_prob_profile(variant, level)
        return self._speech_prob[key]

    def summary_for_file(self, variant, level, name):
        rows = self.summary(variant, level)
        if not rows:
            return None
        for row in rows:
            if str(row.get("file", "")).strip() in (name, name + ".wav"):
                return row
        return None

    def available_levels(self, variant):
        return [level for level in LEVELS if self.summary(variant, level)]

    def primary_level(self, variant="baseline"):
        """主报告口径优先用 L3；缺失则退到 L1/L0。"""
        levels = self.available_levels(variant)
        if not levels:
            return "L3"
        for preferred in ("L3", "L1", "L0"):
            if preferred in levels:
                return preferred
        return levels[0]

    def missing_figures(self):
        return [fig_id for fig_id in FIGURE_IDS if not os.path.isfile(figure_path(fig_id))]

    def present_figures(self):
        return [fig_id for fig_id in FIGURE_IDS if os.path.isfile(figure_path(fig_id))]

    def report_date(self):
        """报告日期：NSDECK_DATE > 构建指纹 build_time > 占位串（保证幂等）。"""
        override = os.environ.get("NSDECK_DATE", "").strip()
        if override:
            return override
        for variant in VARIANTS:
            info = self.build_info(variant)
            if info and info.get("build_time"):
                return info["build_time"]
        return "（未记录构建时间）"

    def state(self):
        """数据可用性快照，用于启动日志与报告附录。"""
        snapshot = OrderedDict()
        for variant in VARIANTS:
            levels = OrderedDict()
            for level in LEVELS:
                rows = self.summary(variant, level)
                levels[level] = len(rows) if rows else 0
            snapshot[variant] = levels
        return snapshot


def load_dataset():
    return Dataset()


def main():
    """自检入口：打印路径契约与数据可用性快照。"""
    print("deck_data.py 路径契约")
    for label, value in (
        ("REPO_ROOT", REPO_ROOT),
        ("DATA_DIR", DATA_DIR),
        ("FIGURES_DIR", FIGURES_DIR),
        ("DOC_DIR", DOC_DIR),
        ("PPTX_PATH", PPTX_PATH),
        ("PREVIEW_DIR", PREVIEW_DIR),
        ("REPORT_DIR", REPORT_DIR),
    ):
        print("  %-12s = %s" % (label, value))
    print("图件定义：%d 个" % len(FIGURE_IDS))
    dataset = Dataset()
    print("summary 行数（variant → level → 文件数）：")
    for variant, levels in dataset.state().items():
        print("  %-10s %s" % (variant, dict(levels)))
    missing = dataset.missing_figures()
    print("缺失图件：%d 个%s" % (len(missing), ("" if not missing else " → " + ", ".join(missing))))
    print("警告：%d 条" % len(dataset.warnings))
    for message in dataset.warnings[:20]:
        print("  - %s" % message)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
