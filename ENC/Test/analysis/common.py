# -*- coding: utf-8 -*-
"""
EmbeddedNS-Portable · 分析管线公共层 (T4-I)

职责
----
1. 路径解析：仓库根 / 数据目录 / 图件目录 / 原始音频目录
2. 读取层：metrics_<name>.csv、summary_<variant>_<level>.csv、WAV（输入 2ch / 输出 1ch）
3. 数值原语：RMS、dB 换算、STFT、分频带能量、LTAS

设计约束
--------
* **只读**：本模块不写任何输入数据；写操作只发生在绘图输出目录。
* 容错：列缺失/文件缺失给出明确异常或 None，绝不让 KeyError 穿透到调用方。
* 所有读入 audio 统一为 **int16 量纲的 float32**（与仓库 `floatS16` 约定一致：
  样本范围 [-32768, 32767]，不做 [-1,1] 归一化）。

单位说明（计划书 6.2）
--------------------
* 输入为 2 声道：视觉/频谱分析统一取 **左声道**（`read_input_left`），字面契约即如此。
* 输出为 1 声道。
* `atten_db` = 20*log10(in_rms/out_rms)，`out_rms == 0` 时记为 999.0（哨兵值）。
"""

from __future__ import annotations

import csv
import os
import struct
import wave

import numpy as np

# --------------------------------------------------------------------------
# 常量
# --------------------------------------------------------------------------

#: 正式输入音频目录（9 个双声道 48 kHz 原始样本）
INPUT_AUDIO = "Test_Audio/AudioSample-48000hz"

#: 测试变体与抑制等级
#: 消融链为 **5 组**：baseline → +D1 → +D1D5 → +D1D5D6 → +D1D5D6D8(optimized)
VARIANTS = ("baseline", "d1", "d1d5", "d1d5d6", "optimized")
ABLATION_ORDER = ("baseline", "d1", "d1d5", "d1d5d6", "optimized")
LEVELS = ("L0", "L1", "L3")
PRIMARY_VARIANT = "baseline"
COMPARE_VARIANT = "optimized"
PRIMARY_LEVEL = "L3"

#: `patch_flags` 位定义（计划书 6.4）：bit0=D1 bit1=D5 bit2=D6 bit3=D8
PATCH_FLAGS = {"baseline": 0, "d1": 1, "d1d5": 3, "d1d5d6": 7, "optimized": 15, "reference": 0}

#: 变体 → PPT 用中文标签
VARIANT_LABEL = {
    "baseline": "基线 baseline",
    "d1": "+D1",
    "d1d5": "+D1D5",
    "d1d5d6": "+D1D5D6",
    "optimized": "优化 optimized",
    "reference": "参照 ENC_Core",
}

#: 帧长（样本）：48000 Hz / 100 帧 = 10 ms 一帧
FRAME_SAMPLES = 480
SAMPLE_RATE = 48000

#: 计划书 6.2 固定列（顺序即契约）
METRICS_COLUMNS = (
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

#: 计划书 6.3 固定列
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

_METRICS_INT_COLUMNS = frozenset({"frame_idx", "clamp_hits", "elapsed_us"})
_SUMMARY_STR_COLUMNS = frozenset({"file"})
_SUMMARY_INT_COLUMNS = frozenset({"frames"})

#: `atten_db` 哨兵：out_rms == 0 时的记录值
ATTEN_DB_SENTINEL = 999.0

_EPS = 1e-12


# --------------------------------------------------------------------------
# 路径解析
# --------------------------------------------------------------------------

def repo_root() -> str:
    """仓库根目录绝对路径（本文件位于 <repo>/ENC/Test/analysis/common.py）。"""
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(here, "..", "..", ".."))


def data_dir() -> str:
    """测量产物根目录。

    默认 `<repo>/ENC/Test/data`。自测时通过环境变量重定向：
      * `NS_DATA_DIR=<dir>` —— 直接指定数据根目录（优先级最高）
      * `NS_SELFTEST=1`     —— 重定向到 `<analysis>/_selftest/data`
    """
    override = os.environ.get("NS_DATA_DIR", "").strip()
    if override:
        return os.path.abspath(override)
    if os.environ.get("NS_SELFTEST", "").strip() not in ("", "0", "false", "False"):
        return os.path.join(os.path.dirname(os.path.abspath(__file__)), "_selftest", "data")
    return os.path.join(repo_root(), "ENC", "Test", "data")


def doc_dir() -> str:
    """文档目录：<repo>/ENC/Document"""
    return os.path.join(repo_root(), "ENC", "Document")


def figures_dir() -> str:
    """图件输出目录。

    默认 `<repo>/ENC/Document/figures`。自测时通过环境变量重定向：
      * `NS_FIG_OUT=<dir>` —— 直接指定输出目录（优先级最高）
      * `NS_SELFTEST=1`   —— 重定向到 `<analysis>/_selftest/figures`
    """
    override = os.environ.get("NS_FIG_OUT", "").strip()
    if override:
        return os.path.abspath(override)
    if os.environ.get("NS_SELFTEST", "").strip() not in ("", "0", "false", "False"):
        return os.path.join(os.path.dirname(os.path.abspath(__file__)), "_selftest", "figures")
    return os.path.join(doc_dir(), "figures")


def input_audio_dir() -> str:
    """原始 WAV 目录：<repo>/Test_Audio/AudioSample-48000hz"""
    return os.path.join(repo_root(), *INPUT_AUDIO.split("/"))


def analysis_dir() -> str:
    """本分析脚本目录：<repo>/ENC/Test/analysis"""
    return os.path.dirname(os.path.abspath(__file__))


def test_file_names() -> list[str]:
    """测试文件名（去扩展名），按数值排序：['1', ..., '9']。

    以 `Test_Audio/AudioSample-48000hz` 实际内容为准（跳过 `output_ns/` 等子目录）；
    该目录不可用时退回计划书 2.6 的固定 1..9。
    """
    d = input_audio_dir()
    names: list[str] = []
    if os.path.isdir(d):
        for entry in os.listdir(d):
            if not entry.lower().endswith(".wav"):
                continue
            if not os.path.isfile(os.path.join(d, entry)):
                continue
            names.append(entry[:-4])
    if not names:
        return [str(i) for i in range(1, 10)]
    return sorted(names, key=_name_sort_key)


def _name_sort_key(name: str):
    """数字名按数值排，其余按字典序，保证 ['1','2','10'] 而非 ['1','10','2']。"""
    return (0, int(name)) if name.isdigit() else (1, name)


def metrics_path(variant: str, level: str, name: str) -> str:
    """`<DATA_DIR>/<variant>/<level>/metrics_<name>.csv`"""
    return os.path.join(data_dir(), variant, level, "metrics_%s.csv" % name)


def summary_path(variant: str, level: str) -> str:
    """`<DATA_DIR>/<variant>/<level>/summary_<variant>_<level>.csv`"""
    return os.path.join(data_dir(), variant, level, "summary_%s_%s.csv" % (variant, level))


def wav_path(variant: str, level: str, name: str) -> str:
    """`<DATA_DIR>/<variant>/<level>/<name>.wav`"""
    return os.path.join(data_dir(), variant, level, "%s.wav" % name)


def build_info_path(variant: str) -> str:
    """`<DATA_DIR>/<variant>/build_info.txt`"""
    return os.path.join(data_dir(), variant, "build_info.txt")


def ensure_figures_dir() -> str:
    """确保图件输出目录存在并返回它。"""
    d = figures_dir()
    os.makedirs(d, exist_ok=True)
    return d


def exists_metrics(variant: str, level: str, name: str) -> bool:
    """metrics CSV 是否存在（非空文件）。"""
    p = metrics_path(variant, level, name)
    return os.path.isfile(p) and os.path.getsize(p) > 0


def exists_summary(variant: str, level: str) -> bool:
    p = summary_path(variant, level)
    return os.path.isfile(p) and os.path.getsize(p) > 0


def exists_wav(variant: str, level: str, name: str) -> bool:
    p = wav_path(variant, level, name)
    return os.path.isfile(p) and os.path.getsize(p) > 0


# --------------------------------------------------------------------------
# CSV 读取
# --------------------------------------------------------------------------

def _require_columns(header: list[str], required, path: str) -> None:
    missing = [c for c in required if c not in header]
    if missing:
        raise ValueError(
            "CSV 列缺失: %s\n  缺少: %s\n  实际表头: %s"
            % (path, ", ".join(missing), ", ".join(header))
        )


def read_metrics(variant: str, level: str, name: str) -> dict:
    """读取逐帧指标 CSV。

    返回 `{列名: numpy 数组}`，数组 dtype：`frame_idx`/`clamp_hits`/`elapsed_us`
    为 int64，其余为 float64。列缺失时抛 ValueError（含路径与实际表头），
    文件缺失时抛 FileNotFoundError。
    """
    path = metrics_path(variant, level, name)
    if not os.path.isfile(path):
        raise FileNotFoundError("metrics CSV 不存在: %s" % path)

    with open(path, "r", newline="", encoding="utf-8-sig") as fh:
        reader = csv.reader(fh)
        try:
            header = [c.strip() for c in next(reader)]
        except StopIteration:
            raise ValueError("metrics CSV 为空文件: %s" % path) from None
        _require_columns(header, METRICS_COLUMNS, path)
        rows = [r for r in reader if r and any(c.strip() for c in r)]

    if not rows:
        raise ValueError("metrics CSV 无数据行: %s" % path)

    width = len(header)
    norm: list[list[str]] = []
    for r in rows:
        if len(r) < width:
            r = list(r) + [""] * (width - len(r))
        norm.append(r[:width])

    out: dict[str, np.ndarray] = {}
    for idx, col in enumerate(header):
        raw = [r[idx].strip() for r in norm]
        try:
            if col in _METRICS_INT_COLUMNS:
                out[col] = np.array([int(float(v)) if v else 0 for v in raw], dtype=np.int64)
            else:
                out[col] = np.array([_to_float(v) for v in raw], dtype=np.float64)
        except ValueError as exc:
            raise ValueError("metrics CSV 列 `%s` 存在非数值内容: %s (%s)" % (col, path, exc)) from None
    return out


def _to_float(text: str) -> float:
    """空串按 0.0；`nan`/`inf` 保留其字面值（FigT22 需要检出它们）。"""
    if text == "":
        return 0.0
    return float(text)


def read_summary(variant: str, level: str) -> list[dict]:
    """读取汇总 CSV，返回每文件一行的 dict 列表（按 `file` 自然序）。"""
    path = summary_path(variant, level)
    if not os.path.isfile(path):
        raise FileNotFoundError("summary CSV 不存在: %s" % path)

    with open(path, "r", newline="", encoding="utf-8-sig") as fh:
        reader = csv.reader(fh)
        try:
            header = [c.strip() for c in next(reader)]
        except StopIteration:
            raise ValueError("summary CSV 为空文件: %s" % path) from None
        _require_columns(header, SUMMARY_COLUMNS, path)
        rows = [r for r in reader if r and any(c.strip() for c in r)]

    result: list[dict] = []
    for r in rows:
        if len(r) < len(header):
            r = list(r) + [""] * (len(header) - len(r))
        row: dict = {}
        for idx, col in enumerate(header):
            text = r[idx].strip()
            if col in _SUMMARY_STR_COLUMNS:
                row[col] = text
            elif col in _SUMMARY_INT_COLUMNS:
                row[col] = int(float(text)) if text else 0
            else:
                row[col] = _to_float(text)
        result.append(row)
    result.sort(key=lambda d: _name_sort_key(str(d.get("file", ""))))
    return result


def summary_map(variant: str, level: str) -> dict[str, dict]:
    """`read_summary` 的按 `file` 索引版本；文件缺失时返回空 dict（不抛异常）。"""
    try:
        return {str(r["file"]): r for r in read_summary(variant, level)}
    except (FileNotFoundError, ValueError):
        return {}


def read_build_info(variant: str) -> dict[str, str]:
    """读取 `build_info.txt` 的 `key = value` 行；缺失返回空 dict。"""
    path = build_info_path(variant)
    info: dict[str, str] = {}
    if not os.path.isfile(path):
        return info
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if "=" not in line:
                continue
            key, _, val = line.partition("=")
            key = key.strip()
            if key:
                info[key] = val.split("#")[0].strip()
    return info


# --------------------------------------------------------------------------
# WAV 读取
# --------------------------------------------------------------------------

def _wav_params(path: str) -> tuple[int, int, int, int]:
    """返回 (采样率, 声道数, 位深, 帧数)。"""
    with wave.open(path, "rb") as wf:
        sampwidth = wf.getsampwidth()
        return wf.getframerate(), wf.getnchannels(), sampwidth * 8, wf.getnframes()


def read_wav_mono(path: str) -> tuple[np.ndarray, int]:
    """读取 WAV 并降为单声道。

    返回 `(samples, sample_rate)`，`samples` 为 **float32、int16 量纲**（[-32768, 32767]）。
    多声道按通道算术平均降混；仅支持 16 bit PCM。
    """
    with wave.open(path, "rb") as wf:
        nch = wf.getnchannels()
        sampwidth = wf.getsampwidth()
        rate = wf.getframerate()
        nframes = wf.getnframes()
        raw = wf.readframes(nframes)

    if sampwidth != 2:
        raise ValueError("仅支持 16 bit PCM，实际 %d bit: %s" % (sampwidth * 8, path))

    data = np.frombuffer(raw, dtype="<i2")
    if nch > 1:
        usable = (data.size // nch) * nch
        data = data[:usable].reshape(-1, nch).mean(axis=1)
    return data.astype(np.float32), rate


def read_input_left(name: str) -> tuple[np.ndarray, int]:
    """读取原始 WAV 的**左声道**，返回 `(samples, sample_rate)`（float32 / int16 量纲）。"""
    path = os.path.join(input_audio_dir(), "%s.wav" % name)
    with wave.open(path, "rb") as wf:
        nch = wf.getnchannels()
        sampwidth = wf.getsampwidth()
        rate = wf.getframerate()
        raw = wf.readframes(wf.getnframes())

    if sampwidth != 2:
        raise ValueError("仅支持 16 bit PCM，实际 %d bit: %s" % (sampwidth * 8, path))

    data = np.frombuffer(raw, dtype="<i2")
    if nch > 1:
        usable = (data.size // nch) * nch
        data = data[:usable].reshape(-1, nch)[:, 0]
    return data.astype(np.float32), rate


def read_input_mono(name: str) -> tuple[np.ndarray, int]:
    """读取原始 WAV 并降为单声道（用于与单声道输出做同口径频谱比较）。"""
    return read_wav_mono(os.path.join(input_audio_dir(), "%s.wav" % name))


# --------------------------------------------------------------------------
# 数值原语
# --------------------------------------------------------------------------

def rms(x: np.ndarray) -> float:
    """整体 RMS。"""
    a = np.asarray(x, dtype=np.float64)
    if a.size == 0:
        return 0.0
    return float(np.sqrt(np.mean(a * a)))


def frame_rms(x: np.ndarray, frame_len: int = FRAME_SAMPLES) -> np.ndarray:
    """逐帧 RMS（末帧不足则忽略）。"""
    a = np.asarray(x, dtype=np.float64)
    n = a.size // frame_len
    if n == 0:
        return np.zeros(0, dtype=np.float64)
    return np.sqrt(np.mean(a[: n * frame_len].reshape(n, frame_len) ** 2, axis=1))


def atten_db(in_rms, out_rms) -> float:
    """20*log10(in/out)；out<=0 时返回哨兵 999.0（与计划书 6.2 一致）。"""
    if out_rms <= 0:
        return ATTEN_DB_SENTINEL
    if in_rms <= 0:
        return 0.0
    return float(20.0 * np.log10(in_rms / out_rms))


def to_db(x, floor: float = _EPS):
    """幅度 → dB（带下限保护，避免 log10(0)）。"""
    a = np.asarray(x, dtype=np.float64)
    a = np.maximum(np.abs(a), floor)
    return 20.0 * np.log10(a)


def norm_deviation(x: np.ndarray) -> np.ndarray:
    """归一化偏差 `|a - mean| / (peak + eps)`（非负，0..1 量级）。"""
    a = np.asarray(x, dtype=np.float64)
    if a.size == 0:
        return a
    denom = float(np.max(np.abs(a))) + _EPS
    return np.abs(a - float(np.mean(a))) / denom


def hann_window(n: int) -> np.ndarray:
    """周期汉宁窗（与 scipy.signal.get_window('hann', n, fftbins=True) 同口径）。"""
    if n <= 1:
        return np.ones(n, dtype=np.float64)
    k = np.arange(n, dtype=np.float64)
    return 0.5 - 0.5 * np.cos(2.0 * np.pi * k / n)


def stft_mag(
    x: np.ndarray,
    n_fft: int = 512,
    hop: int | None = None,
    block_frames: int = 256,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """短时傅里叶变换（幅度谱）。

    参数
    ----
    n_fft : 512（计划书 9.4 固定）
    hop   : 默认为 `n_fft // 2`（即 50% 重叠）
    block_frames : 分块帧数，控制峰值内存；**分块不改变等价于全量 STFT 的结果**

    返回 `(t, f, mag)`：
      * `t`   帧中心时间（秒）
      * `f`   `rfftfreq` 单边频率轴（Hz）
      * `mag` 幅度谱，shape = (帧数, n_fft//2+1)
    """
    a = np.asarray(x, dtype=np.float64).ravel()
    hop = n_fft // 2 if hop is None else int(hop)
    if a.size < n_fft:
        return (np.zeros(0), np.fft.rfftfreq(n_fft, 1.0 / SAMPLE_RATE),
                np.zeros((0, n_fft // 2 + 1), dtype=np.float64))

    n_frames = 1 + (a.size - n_fft) // hop
    frames_this = max(1, int(block_frames))
    pad = n_fft - hop
    padded = np.concatenate([a, np.zeros(pad, dtype=np.float64)])
    win = hann_window(n_fft)

    # 所有帧在 padded 里的起始偏移
    starts = np.arange(n_frames, dtype=np.int64) * hop
    mag = np.empty((n_frames, n_fft // 2 + 1), dtype=np.float64)
    for begin in range(0, n_frames, frames_this):
        end = min(begin + frames_this, n_frames)
        idx = starts[begin:end][:, None] + np.arange(n_fft)[None, :]
        mag[begin:end] = np.abs(np.fft.rfft(padded[idx] * win, axis=1))

    f = np.fft.rfftfreq(n_fft, 1.0 / SAMPLE_RATE)
    # 窗中心时间：帧起点 + n_fft/2
    t = (starts + n_fft / 2.0) / float(SAMPLE_RATE)
    return t, f, mag


def band_energy_db(x: np.ndarray, edges=(0.0, 4000.0, 8000.0, 12000.0, 16000.0, 24000.0),
                   n_fft: int = 512, rate: int = SAMPLE_RATE) -> np.ndarray:
    """分频带能量（dB），用于 FigT13。

    谱估计：对信号做 50% 重叠、汉宁窗、`n_fft` 点 STFT，累加各帧功率谱。
    各频带能量 = 带内功率谱之和；返回 `10*log10(E)`。
    """
    _, f, mag = stft_mag(x, n_fft=n_fft)
    psd = (mag ** 2).sum(axis=0)
    out = np.empty(len(edges) - 1, dtype=np.float64)
    for i in range(len(edges) - 1):
        lo, hi = float(edges[i]), float(edges[i + 1])
        sel = (f >= lo) & (f < hi) if i < len(edges) - 2 else (f >= lo) & (f <= hi)
        e = float(psd[sel].sum()) if np.any(sel) else 0.0
        out[i] = 10.0 * np.log10(e + _EPS)
    return out


def ltas_db(x: np.ndarray, n_fft: int = 512, rate: int = SAMPLE_RATE,
            n_points: int = 241) -> tuple[np.ndarray, np.ndarray]:
    """长时平均谱 LTAS，用于 FigT14。

    返回 `(freq_hz, level_db)`：对信号做 50% 重叠 STFT 后逐频点取平均功率谱，
    再重采样到 `n_points` 个对数等距频点（20 Hz .. 20 kHz）。
    """
    _, f, mag = stft_mag(x, n_fft=n_fft)
    if mag.size == 0:
        grid = np.logspace(np.log10(20.0), np.log10(20000.0), n_points)
        return grid, np.full(n_points, -np.inf)
    psd = (mag ** 2).mean(axis=0)
    grid = np.logspace(np.log10(20.0), np.log10(min(20000.0, rate / 2.0)), n_points)
    level = np.interp(grid, f, 10.0 * np.log10(psd + _EPS))
    return grid, level


def downmix_envelope(x: np.ndarray, n_cols: int) -> tuple[np.ndarray, np.ndarray]:
    """把长信号按列聚成 (min, max) 包络，用于高密度波形绘制。

    前 `n_cols * k` 个样本按列分组卷积式的 min/max；剩余尾部追加为最后一列。
    """
    a = np.asarray(x, dtype=np.float64).ravel()
    n_cols = max(1, int(n_cols))
    if a.size == 0:
        z = np.zeros(n_cols, dtype=np.float64)
        return z, z.copy()
    if a.size <= n_cols:
        pad = n_cols - a.size
        lo = np.concatenate([a, np.full(pad, a[-1])])
        hi = lo.copy()
        return lo, hi
    k = a.size // n_cols
    body = a[: k * n_cols].reshape(n_cols, k)
    lo = body.min(axis=1)
    hi = body.max(axis=1)
    tail = a[k * n_cols:]
    if tail.size:
        lo[-1] = min(lo[-1], float(tail.min()))
        hi[-1] = max(hi[-1], float(tail.max()))
    return lo, hi


def box_stats(values: np.ndarray, whis: float = 1.5) -> dict:
    """箱线图统计：中位数/四分位/须端/离群点（线性插值分位）。"""
    v = np.asarray(values, dtype=np.float64).ravel()
    v = v[np.isfinite(v)]
    if v.size == 0:
        return {"n": 0, "q1": 0.0, "med": 0.0, "q3": 0.0, "whislo": 0.0,
                "whishi": 0.0, "fliers": np.zeros(0), "mean": 0.0}
    q1, med, q3 = np.percentile(v, [25.0, 50.0, 75.0])
    iqr = float(q3 - q1)
    lo_fence = float(q1 - whis * iqr)
    hi_fence = float(q3 + whis * iqr)
    inside = v[(v >= lo_fence) & (v <= hi_fence)]
    whislo = float(inside.min()) if inside.size else float(v.min())
    whishi = float(inside.max()) if inside.size else float(v.max())
    fliers = v[(v < lo_fence) | (v > hi_fence)]
    return {"n": int(v.size), "q1": float(q1), "med": float(med), "q3": float(q3),
            "whislo": whislo, "whishi": whishi, "fliers": fliers,
            "mean": float(v.mean())}


def nan_report(m: dict, columns=None) -> dict:
    """统计各列的 NaN/Inf 帧数。

    生产数据允许出现 `nan` 字面量（D8 缺陷：`fastpow2` 在定义域外返回 NaN）。
    **绝不做 `nan_to_num` 静默替换**——那会伪造数据；图件必须显式呈现 NaN 位置。
    返回 `{列名: {"nan": n, "inf": n, "invalid": n}}`（invalid = nan+inf）。
    """
    cols = list(columns) if columns else list(METRICS_COLUMNS)
    out: dict = {}
    n = 0
    for c in cols:
        if c not in m:
            continue
        a = np.asarray(m[c], dtype=np.float64)
        n = max(n, int(a.size))
        n_nan = int(np.sum(np.isnan(a)))
        n_inf = int(np.sum(np.isinf(a)))
        out[c] = {"nan": n_nan, "inf": n_inf, "invalid": n_nan + n_inf}
    out["_frames"] = n
    return out


def nan_frame_mask(m: dict, columns=None) -> np.ndarray:
    """任一指定列在该帧为 NaN/Inf 的布尔掩码。"""
    cols = list(columns) if columns else list(METRICS_COLUMNS)
    n = int(np.asarray(m[METRICS_COLUMNS[0]]).size)
    mask = np.zeros(n, dtype=bool)
    for c in cols:
        if c not in m:
            continue
        a = np.asarray(m[c], dtype=np.float64)
        mask |= np.isnan(a) | np.isinf(a)
    return mask


def runs_of(mask: np.ndarray, min_len: int = 1) -> list[tuple[int, int]]:
    """把布尔掩码压成连续 True 的区间列表 `[(start, end_inclusive), ...]`。"""
    a = np.asarray(mask, dtype=bool).ravel()
    out: list[tuple[int, int]] = []
    start = None
    for i, v in enumerate(a):
        if v and start is None:
            start = i
        elif not v and start is not None:
            if i - start >= min_len:
                out.append((start, i - 1))
            start = None
    if start is not None and a.size - start >= min_len:
        out.append((start, a.size - 1))
    return out


def scale_mix_warning(columns: dict, rel_span: float = 6.0) -> list[str]:
    """检测"同轴多量级混画"风险。

    参数
    ----
    columns : `{标签: 数组}`，即计划画在同一线性轴上的所有序列
    rel_span : 各序列有效量级跨度的比值阈值（十进制数量级），超过则告警

    返回告警文本列表（空表示安全）。判据：
      * 某两序列的 `log10(max|v|)` 相差超过 `rel_span` 个数量级
      * 或某序列内部非有限值占比超过 30%

    背景：`diff_normalization` 实测为 1e10 量级、`speech_prob_mean` 为 0–1 量级，
    两者同轴线性混画会把小量级序列压成贴地直线——图能出，但内容是坏的。
    """
    warns: list[str] = []
    mags: dict[str, float] = {}
    for label, arr in columns.items():
        a = np.asarray(arr, dtype=np.float64)
        fin = a[np.isfinite(a)]
        if fin.size == 0:
            warns.append("%s: 无非有限以外的有效值（全 NaN/Inf）" % label)
            continue
        peak = float(np.max(np.abs(fin)))
        mags[label] = math.log10(peak) if peak > 0 else -300.0
        bad = 1.0 - fin.size / max(1, a.size)
        if bad > 0.30:
            warns.append("%s: 非有限值占比 %.1f%%" % (label, bad * 100.0))
    if len(mags) >= 2:
        hi = max(mags.values())
        lo = min(mags.values())
        if hi - lo > rel_span:
            big = max(mags, key=lambda k: mags[k])
            small = min(mags, key=lambda k: mags[k])
            warns.append("量级跨度 %.1f 个数量级：`%s`(1e%.0f) 与 `%s`(1e%.0f) 不宜同轴线性混画"
                         % (hi - lo, big, mags[big], small, mags[small]))
    return warns


def fmt_num(v: float, digits: int = 3) -> str:
    """紧凑数值格式化（控制台/图上标签共用），避免出现 `1e-17` 之类噪声。"""
    if v is None:
        return "n/a"
    if isinstance(v, (int, np.integer)):
        return str(int(v))
    fv = float(v)
    if not np.isfinite(fv):
        return "nan" if np.isnan(fv) else ("inf" if fv > 0 else "-inf")
    if fv == 0:
        return "0"
    a = abs(fv)
    if a >= 1e6 or a < 1e-4:
        return "%.3g" % fv
    if a >= 100:
        return "%.0f" % fv
    if a >= 10:
        return "%.1f" % fv
    return ("%%.%df" % digits) % fv


# --------------------------------------------------------------------------
# 自检（直接运行本文件时的轻量冒烟）
# --------------------------------------------------------------------------

if __name__ == "__main__":
    print("[common] repo_root     =", repo_root())
    print("[common] data_dir      =", data_dir())
    print("[common] figures_dir   =", figures_dir())
    print("[common] doc_dir       =", doc_dir())
    print("[common] input_audio   =", input_audio_dir())
    print("[common] test files    =", test_file_names())
    print("[common] metrics_path  =", metrics_path("baseline", "L3", "1"))
    print("[common] summary_path  =", summary_path("optimized", "L3"))
    print("[common] exists_metrics=", exists_metrics("baseline", "L3", "1"))
    print("[common] struct sanity =", struct.calcsize("<h"))
