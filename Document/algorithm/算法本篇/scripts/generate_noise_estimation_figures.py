"""
生成第 6 节“噪声功率谱估计”的教学示意图。

输出：
  figures/Fig002_低分位噪声跟踪原理.png
  figures/Fig003_噪声估计器协同机制.png
  figures/Fig004_在线分位数游标如何更新.png
  figures/Fig005_白噪声与粉噪声的频谱特征.png
  figures/Fig006_启动参数噪声模型处理流程.png

依赖：numpy、matplotlib
运行：python scripts/generate_noise_estimation_figures.py
"""
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DejaVu Sans"]
plt.rcParams["axes.unicode_minus"] = False

COLORS = {
    "observed": "#78909C",
    "noise": "#1976D2",
    "quantile": "#D32F2F",
    "speech": "#F9A825",
    "parametric": "#7B1FA2",
    "final": "#00897B",
    "grid": "#CFD8DC",
}


def smooth(values, width):
    kernel = np.ones(width) / width
    return np.convolve(values, kernel, mode="same")


def save_figure(fig, filename):
    output_dir = Path(__file__).resolve().parent.parent / "figures"
    output_dir.mkdir(parents=True, exist_ok=True)
    output_path = output_dir / filename
    fig.savefig(output_path, dpi=170, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    print(f"已生成: {output_path}")


def generate_quantile_principle_figure():
    rng = np.random.default_rng(12)
    frames = np.arange(300)
    true_noise = 0.95 + 0.08 * np.sin(2 * np.pi * frames / 170)
    observed = true_noise * np.exp(rng.normal(0, 0.13, frames.size))

    speech_mask = np.zeros(frames.size, dtype=bool)
    for start, end, strength in [(28, 60, 2.7), (91, 132, 4.0), (165, 207, 3.1), (235, 276, 4.5)]:
        speech_mask[start:end] = True
        envelope = np.sin(np.linspace(0, np.pi, end - start)) ** 0.7
        observed[start:end] += strength * envelope * (0.8 + 0.35 * rng.random(end - start))

    rolling_quantile = np.empty_like(observed)
    rolling_mean = np.empty_like(observed)
    window = 55
    for frame in frames:
        first = max(0, frame - window + 1)
        history = observed[first:frame + 1]
        rolling_quantile[frame] = np.quantile(history, 0.25)
        rolling_mean[frame] = np.mean(history)

    log_values = np.log(observed)
    threshold = np.quantile(log_values, 0.25)

    fig, axes = plt.subplots(2, 1, figsize=(13, 9), gridspec_kw={"height_ratios": [2.2, 1]})
    axis = axes[0]
    axis.fill_between(frames, 0, observed, where=speech_mask, color=COLORS["speech"], alpha=0.12,
                      label="语音活跃区间")
    axis.plot(frames, observed, color=COLORS["observed"], linewidth=1.0, alpha=0.72,
              label="观测幅度 |X(k)|")
    axis.plot(frames, true_noise, color=COLORS["noise"], linewidth=2.2, label="真实噪声底（示意）")
    axis.plot(frames, rolling_quantile, color=COLORS["quantile"], linewidth=2.0,
              label="滑动 25% 分位数")
    axis.plot(frames, smooth(rolling_mean, 9), color="#5E35B1", linewidth=1.7, linestyle="--",
              label="滑动均值（易被语音抬高）")
    axis.set_title("固定频率 bin 随时间的观测：低分位比均值更接近噪声底", fontsize=15, pad=12)
    axis.set_xlabel("帧序号（每帧 10 ms）")
    axis.set_ylabel("线性幅度（归一化）")
    axis.set_xlim(0, frames[-1])
    axis.set_ylim(bottom=0)
    axis.grid(alpha=0.35, color=COLORS["grid"])
    axis.legend(ncol=2, frameon=False, loc="upper left")

    axis = axes[1]
    axis.hist(log_values, bins=36, color="#B0BEC5", edgecolor="white", density=True)
    axis.axvline(threshold, color=COLORS["quantile"], linewidth=2.2,
                 label=f"25% 分位点 = {threshold:.2f}")
    axis.axvspan(log_values.min(), threshold, color=COLORS["quantile"], alpha=0.10)
    axis.annotate("左侧低值主要由\n噪声主导帧贡献", xy=(threshold - 0.18, 0.45),
                  xytext=(threshold - 0.95, 0.9), arrowprops={"arrowstyle": "->", "color": "#455A64"},
                  fontsize=11, ha="center")
    axis.set_title("同一批观测在 log 域的分布", fontsize=13)
    axis.set_xlabel("log |X(k)|")
    axis.set_ylabel("概率密度")
    axis.grid(axis="y", alpha=0.3, color=COLORS["grid"])
    axis.legend(frameon=False)

    fig.suptitle("低分位噪声估计的直觉", fontsize=18, fontweight="bold", y=1.01)
    fig.tight_layout()
    save_figure(fig, "Fig002_低分位噪声跟踪原理.png")


def generate_estimator_coordination_figure():
    rng = np.random.default_rng(21)
    frames = np.arange(260)
    true_noise = np.where(frames < 145, 1.0, 1.75)
    true_noise += 0.04 * np.sin(2 * np.pi * frames / 80)
    speech_probability = 0.04 + 0.04 * rng.random(frames.size)
    for start, end in [(35, 72), (102, 136), (168, 208), (225, 250)]:
        speech_probability[start:end] = 0.75 + 0.2 * np.sin(np.linspace(0, np.pi, end - start))

    observed = true_noise * np.exp(rng.normal(0, 0.10, frames.size))
    observed += speech_probability * (1.8 + 0.5 * rng.random(frames.size))

    quantile = np.empty_like(observed)
    for frame in frames:
        first = max(0, frame - 70)
        quantile[frame] = np.quantile(observed[first:frame + 1], 0.25)

    parametric = np.full(frames.size, np.mean(observed[:12]) * 0.72)
    parametric += 0.02 * np.sin(2 * np.pi * frames / 100)
    mixed = quantile.copy()
    startup = frames < 50
    weight = frames[startup] / 50.0
    mixed[startup] = weight * quantile[startup] + (1 - weight) * parametric[startup]

    tracked = np.empty_like(observed)
    tracked[0] = mixed[0]
    for frame in frames[1:]:
        gamma = 0.99 if speech_probability[frame] > 0.2 else 0.90
        candidate = ((1 - speech_probability[frame]) * observed[frame]
                     + speech_probability[frame] * tracked[frame - 1])
        update = gamma * tracked[frame - 1] + (1 - gamma) * candidate
        tracked[frame] = min(update, 0.90 * tracked[frame - 1] + 0.10 * candidate)
        tracked[frame] = max(0.75 * mixed[frame] + 0.25 * tracked[frame], 0.05)

    fig, axes = plt.subplots(3, 1, figsize=(13, 10), sharex=True,
                             gridspec_kw={"height_ratios": [1.55, 0.75, 0.85]})
    axis = axes[0]
    axis.plot(frames, observed, color=COLORS["observed"], alpha=0.48, linewidth=1.0, label="观测幅度")
    axis.plot(frames, true_noise, color=COLORS["noise"], linewidth=2.3, label="真实噪声底（示意）")
    axis.plot(frames, parametric, color=COLORS["parametric"], linestyle="--", linewidth=1.8,
              label="参数化模型")
    axis.plot(frames, quantile, color=COLORS["quantile"], linewidth=1.8, label="分位数估计")
    axis.plot(frames, tracked, color=COLORS["final"], linewidth=2.4, label="最终门控跟踪结果")
    axis.axvspan(0, 50, color=COLORS["parametric"], alpha=0.08)
    axis.axvline(50, color="#7E57C2", linestyle=":", linewidth=1.5)
    axis.text(25, axis.get_ylim()[1] * 0.92, "前 50 帧：模型渐退", color="#6A1B9A",
              ha="center", va="top", fontsize=10)
    axis.set_ylabel("线性幅度")
    axis.set_title("启动混合与语音概率门控：既要稳，也要能跟随噪声变化", fontsize=15)
    axis.grid(alpha=0.3, color=COLORS["grid"])
    axis.legend(ncol=3, frameon=False, loc="upper left")

    axis = axes[1]
    axis.fill_between(frames, 0, speech_probability, color=COLORS["speech"], alpha=0.55)
    axis.axhline(0.2, color="#C62828", linestyle="--", linewidth=1.4, label="门限 0.2")
    axis.text(258, 0.23, "高于门限：γ = 0.99，近似冻结", color="#C62828", ha="right", fontsize=10)
    axis.set_ylim(0, 1.05)
    axis.set_ylabel("语音概率")
    axis.grid(alpha=0.3, color=COLORS["grid"])

    axis = axes[2]
    offsets = [0, 67, 133]
    for row, offset in enumerate(offsets):
        active = ((frames + offset) % 200) / 200
        axis.plot(frames, row + active * 0.65, linewidth=2, label=f"估计器 {row + 1}")
        resets = frames[((frames + offset) % 200) == 0]
        axis.scatter(resets, np.full(resets.size, row), marker="v", s=45, color="#D32F2F", zorder=3)
    axis.set_yticks([0.32, 1.32, 2.32], ["组 1", "组 2", "组 3"])
    axis.set_ylim(-0.25, 3.0)
    axis.set_xlabel("帧序号（每帧 10 ms）")
    axis.set_ylabel("错峰周期")
    axis.set_title("3 组估计器错峰运行；红色三角表示周期重置", fontsize=12)
    axis.grid(axis="x", alpha=0.3, color=COLORS["grid"])

    fig.tight_layout()
    save_figure(fig, "Fig003_噪声估计器协同机制.png")


def generate_online_quantile_update_figure():
    rng = np.random.default_rng(37)
    sample_count = 220
    frames = np.arange(sample_count)
    noise_dominated = rng.lognormal(mean=0.0, sigma=0.12, size=sample_count)
    speech_active = rng.random(sample_count) < 0.42
    observed = noise_dominated.copy()
    observed[speech_active] += rng.uniform(1.2, 3.8, speech_active.sum())
    log_observed = np.log(observed)
    true_quantile = np.quantile(log_observed, 0.25)

    estimate = np.empty(sample_count)
    estimate[0] = 0.85
    step_size = 0.065
    for frame in range(1, sample_count):
        previous = estimate[frame - 1]
        if log_observed[frame] > previous:
            estimate[frame] = previous + 0.25 * step_size
        else:
            estimate[frame] = previous - 0.75 * step_size

    fig = plt.figure(figsize=(13, 9))
    grid = fig.add_gridspec(2, 2, height_ratios=[1, 1.55], hspace=0.35, wspace=0.28)

    axis = fig.add_subplot(grid[0, 0])
    axis.axvline(0, color="#455A64", linewidth=2)
    axis.scatter([0], [0], s=115, color=COLORS["quantile"], zorder=4)
    axis.annotate("当前游标 $\\hat q$", xy=(0, 0), xytext=(-0.9, 0.55),
                  arrowprops={"arrowstyle": "->", "color": COLORS["quantile"]}, fontsize=11)
    axis.scatter([1.0], [0], s=95, color=COLORS["observed"], zorder=4)
    axis.annotate("新样本在右边", xy=(1.0, 0), xytext=(0.55, 0.62),
                  arrowprops={"arrowstyle": "->", "color": COLORS["observed"]}, fontsize=11)
    axis.annotate("向右小走 $0.25\\mu$", xy=(0.25, -0.18), xytext=(0, -0.58),
                  arrowprops={"arrowstyle": "->", "color": COLORS["final"], "linewidth": 2},
                  color=COLORS["final"], fontsize=12, ha="center")
    axis.set_xlim(-1.25, 1.35)
    axis.set_ylim(-0.8, 0.85)
    axis.set_title("样本高于游标：轻轻向上追", fontsize=13, fontweight="bold")
    axis.set_xlabel("log 幅度（越往右越大）")
    axis.set_yticks([])
    axis.grid(axis="x", alpha=0.25, color=COLORS["grid"])

    axis = fig.add_subplot(grid[0, 1])
    axis.axvline(0, color="#455A64", linewidth=2)
    axis.scatter([0], [0], s=115, color=COLORS["quantile"], zorder=4)
    axis.annotate("当前游标 $\\hat q$", xy=(0, 0), xytext=(0.55, 0.55),
                  arrowprops={"arrowstyle": "->", "color": COLORS["quantile"]}, fontsize=11)
    axis.scatter([-1.0], [0], s=95, color=COLORS["noise"], zorder=4)
    axis.annotate("新样本在左边", xy=(-1.0, 0), xytext=(-0.62, 0.62),
                  arrowprops={"arrowstyle": "->", "color": COLORS["noise"]}, fontsize=11)
    axis.annotate("向左大走 $0.75\\mu$", xy=(-0.75, -0.18), xytext=(0, -0.58),
                  arrowprops={"arrowstyle": "->", "color": COLORS["quantile"], "linewidth": 2},
                  color=COLORS["quantile"], fontsize=12, ha="center")
    axis.set_xlim(-1.35, 1.25)
    axis.set_ylim(-0.8, 0.85)
    axis.set_title("样本低于游标：明显向下退", fontsize=13, fontweight="bold")
    axis.set_xlabel("log 幅度（越往左越小）")
    axis.set_yticks([])
    axis.grid(axis="x", alpha=0.25, color=COLORS["grid"])

    axis = fig.add_subplot(grid[1, :])
    axis.scatter(frames, log_observed, s=13, color=COLORS["observed"], alpha=0.45,
                 label="逐帧到来的 log 幅度样本")
    axis.plot(frames, estimate, color=COLORS["quantile"], linewidth=2.3,
              label="在线游标 $\\hat q$（只保存当前值）")
    axis.axhline(true_quantile, color=COLORS["noise"], linestyle="--", linewidth=2,
                 label="保存全部样本并排序得到的 25% 分位点")
    axis.axvspan(0, 55, color=COLORS["speech"], alpha=0.08)
    axis.text(27, axis.get_ylim()[1] * 0.88, "开始时快速靠近", ha="center", color="#8D6E00")
    axis.annotate("游标在目标附近上下小幅摆动",
                  xy=(165, estimate[165]), xytext=(125, 0.72),
                  arrowprops={"arrowstyle": "->", "color": "#455A64"}, fontsize=11)
    axis.set_xlim(0, sample_count - 1)
    axis.set_xlabel("帧序号：样本一个接一个到来")
    axis.set_ylabel("log 幅度")
    axis.set_title("长期效果：不断比较、移动，在线游标逐渐逼近真实 25% 分位点", fontsize=14)
    axis.grid(alpha=0.3, color=COLORS["grid"])
    axis.legend(frameon=False, ncol=3, loc="upper right")

    fig.suptitle("6.3 在线分位数更新：用一个会移动的游标代替保存历史并排序",
                 fontsize=18, fontweight="bold", y=0.98)
    save_figure(fig, "Fig004_在线分位数游标如何更新.png")


def generate_white_pink_characteristics_figure():
    rng = np.random.default_rng(52)
    sample_rate = 16_000
    sample_count = 16_384
    time = np.arange(sample_count) / sample_rate

    white = rng.normal(0, 1, sample_count)
    frequency = np.fft.rfftfreq(sample_count, 1 / sample_rate)
    white_spectrum = np.fft.rfft(white)
    pink_shaping = np.ones_like(frequency)
    pink_shaping[1:] = 1 / np.sqrt(frequency[1:] / frequency[1])
    pink_spectrum = white_spectrum * pink_shaping
    pink = np.fft.irfft(pink_spectrum, sample_count)
    pink /= np.std(pink)

    window = np.hanning(sample_count)
    white_amplitude = np.abs(np.fft.rfft(white * window))
    pink_amplitude = np.abs(np.fft.rfft(pink * window))
    smoothing_width = 81
    white_amplitude = smooth(white_amplitude, smoothing_width)
    pink_amplitude = smooth(pink_amplitude, smoothing_width)
    white_amplitude /= np.median(white_amplitude[50:])
    pink_amplitude /= np.median(pink_amplitude[50:])

    fig, axes = plt.subplots(2, 2, figsize=(13, 9))
    visible_samples = 700
    axes[0, 0].plot(time[:visible_samples] * 1000, white[:visible_samples],
                    color="#546E7A", linewidth=0.8)
    axes[0, 0].set_title("白噪声时域：相邻样本快速、无规则变化", fontweight="bold")
    axes[0, 0].set_xlabel("时间 / ms")
    axes[0, 0].set_ylabel("幅度")
    axes[0, 0].grid(alpha=0.25, color=COLORS["grid"])

    axes[0, 1].plot(time[:visible_samples] * 1000, pink[:visible_samples],
                    color=COLORS["parametric"], linewidth=0.9)
    axes[0, 1].set_title("粉噪声时域：低频更强，波形起伏更缓慢", fontweight="bold")
    axes[0, 1].set_xlabel("时间 / ms")
    axes[0, 1].set_ylabel("幅度")
    axes[0, 1].grid(alpha=0.25, color=COLORS["grid"])

    positive = frequency >= 62.5
    axes[1, 0].plot(frequency[positive] / 1000, 20 * np.log10(white_amplitude[positive] + 1e-6),
                    color="#546E7A", linewidth=1.7, label="白噪声：各频段近似平坦")
    axes[1, 0].plot(frequency[positive] / 1000, 20 * np.log10(pink_amplitude[positive] + 1e-6),
                    color=COLORS["parametric"], linewidth=1.8, label="粉噪声：频率越高幅度越低")
    axes[1, 0].set_title("线性频率轴：一条平，一条向右下降", fontweight="bold")
    axes[1, 0].set_xlabel("频率 / kHz")
    axes[1, 0].set_ylabel("相对幅度 / dB")
    axes[1, 0].grid(alpha=0.3, color=COLORS["grid"])
    axes[1, 0].legend(frameon=False)

    bin_index = np.arange(5, 129)
    beta = 0.72
    amplitude = 16 / np.power(bin_index, beta)
    flat = np.full_like(amplitude, np.median(amplitude), dtype=float)
    axes[1, 1].plot(np.log(bin_index), np.log(flat), color="#546E7A", linewidth=2,
                    label="白模型：斜率 0")
    axes[1, 1].plot(np.log(bin_index), np.log(amplitude), color=COLORS["parametric"], linewidth=2.2,
                    label=f"粉模型：斜率 $-\\beta=-{beta}$")
    axes[1, 1].scatter(np.log(bin_index[::10]), np.log(amplitude[::10]), s=20,
                       color=COLORS["speech"], zorder=3, label="待拟合的 log 频谱点（示意）")
    axes[1, 1].set_title("log-log 坐标：粉噪声幂律曲线变成直线", fontweight="bold")
    axes[1, 1].set_xlabel("$\\log k$")
    axes[1, 1].set_ylabel("$\\log A(k)$")
    axes[1, 1].grid(alpha=0.3, color=COLORS["grid"])
    axes[1, 1].legend(frameon=False)

    fig.suptitle("白噪声与粉噪声：算法真正利用的是频谱形状差异",
                 fontsize=18, fontweight="bold")
    fig.tight_layout(rect=[0, 0, 1, 0.96])
    save_figure(fig, "Fig005_白噪声与粉噪声的频谱特征.png")


def generate_startup_parametric_flow_figure():
    rng = np.random.default_rng(74)
    bins = np.arange(129)
    fitting_bins = np.arange(5, 129)
    true_curve = 7.5 / np.power(np.maximum(bins, 5), 0.68)
    observed = true_curve * np.exp(rng.normal(0, 0.18, bins.size))
    observed += 0.15

    log_bin = np.log(fitting_bins)
    log_observed = np.log(observed[fitting_bins])
    slope, intercept = np.polyfit(log_bin, log_observed, 1)
    beta = np.clip(-slope, 0, 1)
    numerator = np.exp(intercept)
    pink_model = numerator / np.power(np.maximum(bins, 5), beta)
    white_model = np.full(bins.size, np.mean(observed))

    frames = np.arange(51)
    quantile_weight = frames / 50
    model_weight = 1 - quantile_weight

    fig = plt.figure(figsize=(13, 10))
    grid = fig.add_gridspec(3, 2, height_ratios=[1.0, 1.2, 0.95], hspace=0.42, wspace=0.27)

    axis = fig.add_subplot(grid[0, :])
    axis.axis("off")
    box_style = {"boxstyle": "round,pad=0.5", "facecolor": "#E3F2FD", "edgecolor": "#1976D2"}
    steps = [
        (0.08, "当前帧\n129 个幅度 bin"),
        (0.29, "同时计算\n平坦均值 + log-log 拟合"),
        (0.52, "跨前 50 帧累计\n降低单帧随机波动"),
        (0.73, "生成参数谱\n白模型或粉模型"),
        (0.92, "与在线分位数谱\n渐变混合"),
    ]
    for position, text in steps:
        axis.text(position, 0.52, text, ha="center", va="center", fontsize=11, bbox=box_style)
    for first, second in zip(steps[:-1], steps[1:]):
        axis.annotate("", xy=(second[0] - 0.08, 0.52), xytext=(first[0] + 0.08, 0.52),
                      arrowprops={"arrowstyle": "->", "color": "#455A64", "linewidth": 1.8})
    axis.text(0.5, 0.93, "每一帧都走一遍；参数模型只在 0–49 帧参与输出",
              ha="center", fontsize=13, fontweight="bold")

    axis = fig.add_subplot(grid[1, 0])
    axis.scatter(bins, observed, s=14, color=COLORS["observed"], alpha=0.55, label="当前帧观测")
    axis.plot(bins, white_model, color="#546E7A", linestyle="--", linewidth=2,
              label="白模型：全频率同一高度")
    axis.plot(bins, pink_model, color=COLORS["parametric"], linewidth=2.3,
              label=f"粉模型：$A/k^{{{beta:.2f}}}$")
    axis.axvspan(0, 4, color=COLORS["speech"], alpha=0.12, label="不参与拟合，按 bin 5 外推")
    axis.set_title("步骤 1–3：用频谱整体形状拟合一条平滑曲线", fontweight="bold")
    axis.set_xlabel("频率 bin $k$")
    axis.set_ylabel("线性幅度")
    axis.grid(alpha=0.25, color=COLORS["grid"])
    axis.legend(frameon=False, fontsize=9)

    axis = fig.add_subplot(grid[1, 1])
    axis.scatter(log_bin, log_observed, s=15, color=COLORS["observed"], alpha=0.5,
                 label="bin 5–128 的 log 观测")
    axis.plot(log_bin, intercept + slope * log_bin, color=COLORS["parametric"], linewidth=2.3,
              label=f"最小二乘直线，$\\beta={beta:.2f}$")
    axis.annotate("斜率越负\n低频相对越强", xy=(3.6, intercept + slope * 3.6),
                  xytext=(2.5, np.max(log_observed) - 0.2),
                  arrowprops={"arrowstyle": "->", "color": "#455A64"}, fontsize=11)
    axis.set_title("为什么取 log：$A/k^\\beta$ 变成可拟合直线", fontweight="bold")
    axis.set_xlabel("$\\log k$")
    axis.set_ylabel("$\\log A_x(k)$")
    axis.grid(alpha=0.25, color=COLORS["grid"])
    axis.legend(frameon=False, fontsize=9)

    axis = fig.add_subplot(grid[2, :])
    axis.fill_between(frames, 0, model_weight, color=COLORS["parametric"], alpha=0.45,
                      label="参数模型权重 $(50-t)/50$")
    axis.fill_between(frames, model_weight, model_weight + quantile_weight,
                      color=COLORS["quantile"], alpha=0.38,
                      label="分位数估计权重 $t/50$")
    axis.axvline(50, color="#455A64", linestyle="--", linewidth=1.5)
    axis.text(1, 0.88, "刚启动：主要相信参数曲线", color="#6A1B9A", fontsize=11)
    axis.text(49, 0.88, "第 50 帧：完全交给分位数", ha="right", color="#B71C1C", fontsize=11)
    axis.set_xlim(0, 50)
    axis.set_ylim(0, 1.03)
    axis.set_xlabel("已分析帧数 $t$（每帧 10 ms）")
    axis.set_ylabel("混合权重")
    axis.set_title("步骤 4：参数模型逐帧退出，不发生硬切换", fontweight="bold")
    axis.grid(alpha=0.25, color=COLORS["grid"])
    axis.legend(frameon=False, ncol=2, loc="lower center")

    fig.suptitle("6.5 启动参数模型：先用少量参数画出噪声大致轮廓，再交棒给分位数估计",
                 fontsize=17, fontweight="bold")
    fig.subplots_adjust(left=0.08, right=0.97, bottom=0.07, top=0.91)
    save_figure(fig, "Fig006_启动参数噪声模型处理流程.png")


if __name__ == "__main__":
    generate_quantile_principle_figure()
    generate_estimator_coordination_figure()
    generate_online_quantile_update_figure()
    generate_white_pink_characteristics_figure()
    generate_startup_parametric_flow_figure()
