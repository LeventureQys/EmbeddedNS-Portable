"""
生成 5.2 节“调制定理”的直观解释图。

输出到“算法附录/figures”和“算法本篇/figures”:
    Fig001_余弦调制如何搬移频谱.png
    Fig002_同一原型生成三个子带滤波器.png
    Fig003_三个卷积如何重排为共享滤波与DCT.png
    Fig004_DCT矩阵如何读取三路模式.png

依赖: numpy, matplotlib
运行: python scripts/generate_modulation_figures.py
"""

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import FancyBboxPatch


SAMPLE_RATE = 48_000
FILTER_LENGTH = 97
FFT_SIZE = 16_384

BLUE = "#1976D2"
ORANGE = "#F57C00"
GREEN = "#388E3C"
PURPLE = "#7B1FA2"
GRAY = "#607D8B"

plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "Noto Sans SC", "SimHei"]
plt.rcParams["axes.unicode_minus"] = False


def design_lowpass(cutoff_hz):
    sample_index = np.arange(FILTER_LENGTH) - (FILTER_LENGTH - 1) / 2
    impulse_response = 2 * cutoff_hz / SAMPLE_RATE * np.sinc(
        2 * cutoff_hz / SAMPLE_RATE * sample_index
    )
    impulse_response *= np.hamming(FILTER_LENGTH)
    impulse_response /= impulse_response.sum()
    return sample_index, impulse_response


def frequency_response(impulse_response, whole=False):
    spectrum = np.fft.fft(impulse_response, FFT_SIZE)
    frequency_hz = np.fft.fftfreq(FFT_SIZE, d=1 / SAMPLE_RATE)
    if whole:
        order = np.argsort(frequency_hz)
        return frequency_hz[order] / 1000, spectrum[order]
    positive = frequency_hz >= 0
    return frequency_hz[positive] / 1000, spectrum[positive]


def magnitude_db(spectrum, reference=1.0):
    return 20 * np.log10(np.maximum(np.abs(spectrum) / reference, 1e-5))


def style_frequency_axis(axis, x_limits, y_limits=(-80, 5)):
    axis.set_xlim(*x_limits)
    axis.set_ylim(*y_limits)
    axis.set_xlabel("频率 / kHz")
    axis.set_ylabel("幅度 / dB")
    axis.grid(True, alpha=0.22)
    axis.axhline(0, color="#B0BEC5", linewidth=0.8)


def generate_spectrum_shift_figure(output_dir):
    sample_index, prototype = design_lowpass(3_200)
    carrier_hz = 12_000
    carrier = np.cos(2 * np.pi * carrier_hz / SAMPLE_RATE * sample_index)
    modulated = 2 * prototype * carrier

    frequency_khz, prototype_spectrum = frequency_response(prototype, whole=True)
    _, modulated_spectrum = frequency_response(modulated, whole=True)

    positive_copy = np.interp(
        frequency_khz - carrier_hz / 1000,
        frequency_khz,
        np.abs(prototype_spectrum),
        left=0,
        right=0,
    )
    negative_copy = np.interp(
        frequency_khz + carrier_hz / 1000,
        frequency_khz,
        np.abs(prototype_spectrum),
        left=0,
        right=0,
    )

    figure, axes = plt.subplots(3, 1, figsize=(12, 10), constrained_layout=True)

    axes[0].plot(frequency_khz, magnitude_db(prototype_spectrum), color=BLUE, linewidth=2)
    axes[0].fill_between(
        frequency_khz,
        -80,
        magnitude_db(prototype_spectrum),
        where=np.abs(frequency_khz) <= 3.2,
        color=BLUE,
        alpha=0.15,
    )
    axes[0].set_title("① 原型低通 H(f)：频谱位于 0 Hz 附近")
    axes[0].annotate(
        "原型形状",
        xy=(0, 0),
        xytext=(5, -18),
        arrowprops={"arrowstyle": "->", "color": GRAY},
        color=GRAY,
    )
    style_frequency_axis(axes[0], (-24, 24))

    axes[1].stem(
        [-carrier_hz / 1000, carrier_hz / 1000],
        [0, 0],
        linefmt=ORANGE,
        markerfmt="o",
        basefmt=" ",
        bottom=-80,
    )
    axes[1].vlines(
        [-carrier_hz / 1000, carrier_hz / 1000], -80, 0, color=ORANGE, linewidth=2
    )
    axes[1].text(-12, 2, "−12 kHz", ha="center", color=ORANGE)
    axes[1].text(12, 2, "+12 kHz", ha="center", color=ORANGE)
    axes[1].set_title("② 余弦载波 cos(2π·12kHz·n/fs)：包含 +12 kHz 和 −12 kHz 两条谱线")
    style_frequency_axis(axes[1], (-24, 24))

    reference = np.max(np.abs(prototype_spectrum))
    axes[2].plot(
        frequency_khz,
        magnitude_db(positive_copy, reference),
        color=GREEN,
        linestyle="--",
        linewidth=1.4,
        label="搬到 +12 kHz 的副本",
    )
    axes[2].plot(
        frequency_khz,
        magnitude_db(negative_copy, reference),
        color=PURPLE,
        linestyle="--",
        linewidth=1.4,
        label="搬到 −12 kHz 的副本",
    )
    axes[2].plot(
        frequency_khz,
        magnitude_db(modulated_spectrum, reference),
        color=ORANGE,
        linewidth=2.2,
        label="两份副本相加后的带通",
    )
    axes[2].set_title("③ h(n) × 2cos(2π·12kHz·n/fs)：低通形状被复制并搬到 ±12 kHz")
    axes[2].annotate(
        "在正频率半轴看，\n它就是 8–16 kHz 附近的带通",
        xy=(12, 0),
        xytext=(16, -22),
        arrowprops={"arrowstyle": "->", "color": GRAY},
        color=GRAY,
        ha="center",
    )
    axes[2].legend(loc="lower center", ncol=3, fontsize=9)
    style_frequency_axis(axes[2], (-24, 24))

    figure.suptitle("余弦调制的本质：时域相乘，频域产生两份平移副本", fontsize=16, fontweight="bold")
    output_path = output_dir / "Fig001_余弦调制如何搬移频谱.png"
    figure.savefig(output_path, dpi=160, facecolor="white")
    plt.close(figure)


def generate_filter_bank_figure(output_dir):
    sample_index, prototype = design_lowpass(4_000)
    center_frequencies_hz = [4_000, 12_000, 20_000]
    colors = [BLUE, ORANGE, GREEN]
    labels = [
        "Band 0：中心 4 kHz，覆盖约 0–8 kHz",
        "Band 1：中心 12 kHz，覆盖约 8–16 kHz",
        "Band 2：中心 20 kHz，覆盖约 16–24 kHz",
    ]

    frequency_khz, _ = frequency_response(prototype)
    figure, axes = plt.subplots(2, 1, figsize=(12, 8.5), constrained_layout=True)

    axes[0].plot(sample_index, prototype, color=GRAY, linewidth=2, label="同一个原型 h(n)")
    for center_hz, color in zip(center_frequencies_hz, colors):
        carrier = np.cos(2 * np.pi * center_hz / SAMPLE_RATE * sample_index)
        axes[0].plot(
            sample_index,
            carrier * np.max(np.abs(prototype)),
            color=color,
            linewidth=1,
            alpha=0.7,
            label=f"{center_hz // 1000} kHz 余弦（缩放显示）",
        )
    axes[0].set_title("时域：原型系数分别乘以 4、12、20 kHz 的余弦")
    axes[0].set_xlabel("相对采样点 n")
    axes[0].set_ylabel("幅度")
    axes[0].grid(True, alpha=0.22)
    axes[0].legend(ncol=4, fontsize=9, loc="upper center")

    for center_hz, color, label in zip(center_frequencies_hz, colors, labels):
        modulated_filter = 2 * prototype * np.cos(
            2 * np.pi * center_hz / SAMPLE_RATE * sample_index
        )
        _, spectrum = frequency_response(modulated_filter)
        axes[1].plot(frequency_khz, magnitude_db(spectrum), color=color, linewidth=2, label=label)

    for boundary_khz in [0, 8, 16, 24]:
        axes[1].axvline(boundary_khz, color="#90A4AE", linestyle=":", linewidth=1.2)
    axes[1].text(4, 3, "Band 0", ha="center", color=BLUE, fontweight="bold")
    axes[1].text(12, 3, "Band 1", ha="center", color=ORANGE, fontweight="bold")
    axes[1].text(20, 3, "Band 2", ha="center", color=GREEN, fontweight="bold")
    axes[1].set_title("频域：形状基本相同，只是中心频率不同")
    axes[1].legend(loc="lower center", ncol=3, fontsize=9)
    style_frequency_axis(axes[1], (0, 24))

    figure.suptitle("一个低通原型 + 三个余弦 = 三个分析子带滤波器", fontsize=16, fontweight="bold")
    output_path = output_dir / "Fig002_同一原型生成三个子带滤波器.png"
    figure.savefig(output_path, dpi=160, facecolor="white")
    plt.close(figure)


def draw_box(axis, center_x, center_y, width, height, text, color, font_size=11):
    box = FancyBboxPatch(
        (center_x - width / 2, center_y - height / 2),
        width,
        height,
        boxstyle="round,pad=0.02,rounding_size=0.025",
        linewidth=1.8,
        edgecolor=color,
        facecolor=color,
        alpha=0.12,
    )
    axis.add_patch(box)
    axis.text(center_x, center_y, text, ha="center", va="center", fontsize=font_size)


def draw_arrow(axis, start, end, color=GRAY):
    axis.annotate(
        "",
        xy=end,
        xytext=start,
        arrowprops={"arrowstyle": "->", "color": color, "linewidth": 1.8},
    )


def generate_rearrangement_figure(output_dir):
    figure, axes = plt.subplots(2, 1, figsize=(13, 8.5), constrained_layout=True)

    top = axes[0]
    top.set_title("重排前：三个子带各做一次完整卷积", fontsize=14, fontweight="bold")
    top.text(0.06, 0.5, "$x(n)$", ha="center", va="center", fontsize=13)
    branch_y = [0.78, 0.5, 0.22]
    branch_colors = [BLUE, ORANGE, GREEN]
    branch_labels = ["4 kHz 余弦", "12 kHz 余弦", "20 kHz 余弦"]
    for branch_index, (position_y, color, carrier_label) in enumerate(
        zip(branch_y, branch_colors, branch_labels)
    ):
        draw_arrow(top, (0.09, 0.5), (0.17, position_y))
        draw_box(
            top,
            0.34,
            position_y,
            0.31,
            0.16,
            f"$h(n)\\cos(\\omega_{branch_index}n)$\n48-tap 卷积",
            color,
        )
        draw_arrow(top, (0.5, position_y), (0.59, position_y))
        draw_box(top, 0.65, position_y, 0.1, 0.14, "↓3", GRAY)
        draw_arrow(top, (0.71, position_y), (0.81, position_y))
        top.text(0.84, position_y, f"$y_{branch_index}(m)$", va="center", fontsize=12)
        top.text(0.47, position_y + 0.095, carrier_label, color=color, ha="right", fontsize=9)
    top.text(
        0.93,
        0.5,
        "同一个 $h(n)$\n被重复计算 3 次",
        ha="center",
        va="center",
        color="#C62828",
        fontsize=12,
        fontweight="bold",
    )
    top.set_xlim(0, 1)
    top.set_ylim(0, 1)
    top.axis("off")

    bottom = axes[1]
    bottom.set_title("重排后：先共享原型滤波，再用小矩阵区分子带", fontsize=14, fontweight="bold")
    bottom.text(0.04, 0.5, "$x(n)$", ha="center", va="center", fontsize=13)
    draw_arrow(bottom, (0.07, 0.5), (0.13, 0.5))
    draw_box(bottom, 0.22, 0.5, 0.17, 0.28, "按相位分路\n并 ↓3", PURPLE)
    draw_arrow(bottom, (0.31, 0.5), (0.38, 0.5))
    draw_box(
        bottom,
        0.49,
        0.5,
        0.21,
        0.34,
        "原型 $h(n)$ 的\n多相 FIR\n（只算一遍）",
        BLUE,
        12,
    )
    draw_arrow(bottom, (0.6, 0.5), (0.67, 0.5))
    bottom.text(0.64, 0.6, "$e_0,e_1,e_2$", ha="center", fontsize=11)
    draw_box(bottom, 0.75, 0.5, 0.13, 0.34, "$\\mathbf{C}$\n3×3 DCT", ORANGE, 12)
    draw_arrow(bottom, (0.82, 0.5), (0.9, 0.5))
    bottom.text(0.94, 0.5, "$y_0,y_1,y_2$", ha="center", va="center", fontsize=12)
    bottom.text(
        0.49,
        0.17,
        "负责共同的滤波形状",
        ha="center",
        color=BLUE,
        fontsize=11,
        fontweight="bold",
    )
    bottom.text(
        0.75,
        0.17,
        "负责区分低 / 中 / 高频",
        ha="center",
        color=ORANGE,
        fontsize=11,
        fontweight="bold",
    )
    bottom.set_xlim(0, 1)
    bottom.set_ylim(0, 1)
    bottom.axis("off")

    figure.suptitle("5.3 的核心不是新增算法，而是交换计算顺序", fontsize=17, fontweight="bold")
    output_path = output_dir / "Fig003_三个卷积如何重排为共享滤波与DCT.png"
    figure.savefig(output_path, dpi=160, facecolor="white")
    plt.close(figure)


def generate_dct_pattern_figure(output_dir):
    dct_matrix = np.array(
        [
            [2, 2, 2],
            [np.sqrt(3), 0, -np.sqrt(3)],
            [1, -2, 1],
        ]
    )
    row_names = ["低频探测器", "中频探测器", "高频探测器"]
    row_colors = [BLUE, ORANGE, GREEN]

    figure = plt.figure(figsize=(13, 8.5), constrained_layout=True)
    grid = figure.add_gridspec(2, 3, height_ratios=[1.05, 1])
    matrix_axis = figure.add_subplot(grid[0, :])
    image = matrix_axis.imshow(dct_matrix, cmap="RdBu_r", vmin=-2, vmax=2, aspect="auto")
    for row in range(3):
        for column in range(3):
            value = dct_matrix[row, column]
            if np.isclose(value, np.sqrt(3)):
                label = "$\\sqrt{3}$"
            elif np.isclose(value, -np.sqrt(3)):
                label = "$-\\sqrt{3}$"
            else:
                label = f"${int(value)}$"
            matrix_axis.text(column, row, label, ha="center", va="center", fontsize=16)
    matrix_axis.set_xticks([0, 1, 2], ["$e_0$", "$e_1$", "$e_2$"])
    matrix_axis.set_yticks([0, 1, 2], row_names)
    matrix_axis.set_title("矩阵的每一行，都是观察三路 $e_0,e_1,e_2$ 的一种加权方式", fontsize=14)
    matrix_axis.set_xlabel("共享多相 FIR 输出")
    figure.colorbar(image, ax=matrix_axis, shrink=0.75, label="加权系数")

    normalized_patterns = dct_matrix / np.max(np.abs(dct_matrix), axis=1, keepdims=True)
    explanations = [
        "三路同号就相加\n→ 一致模式得到最大输出",
        "比较第一路和第三路\n→ 渐变模式得到输出",
        "两边相加、中间反相\n→ 交替模式得到最大输出",
    ]
    for row in range(3):
        axis = figure.add_subplot(grid[1, row])
        axis.axhline(0, color="#90A4AE", linewidth=1)
        axis.bar([0, 1, 2], normalized_patterns[row], color=row_colors[row], alpha=0.78)
        axis.set_xticks([0, 1, 2], ["$e_0$", "$e_1$", "$e_2$"])
        axis.set_ylim(-1.25, 1.25)
        axis.set_title(row_names[row], color=row_colors[row], fontweight="bold")
        axis.text(1, -1.17, explanations[row], ha="center", va="bottom", fontsize=10)
        axis.grid(axis="y", alpha=0.2)
        if row == 0:
            axis.set_ylabel("归一化权重")

    figure.suptitle("DCT 小矩阵在做什么：用三种权重读取三路之间的模式", fontsize=17, fontweight="bold")
    output_path = output_dir / "Fig004_DCT矩阵如何读取三路模式.png"
    figure.savefig(output_path, dpi=160, facecolor="white")
    plt.close(figure)


def main():
    script_dir = Path(__file__).resolve().parent
    output_dirs = [
        script_dir.parent / "figures",
        script_dir.parent.parent / "算法本篇" / "figures",
    ]
    for output_dir in output_dirs:
        output_dir.mkdir(parents=True, exist_ok=True)
        generate_spectrum_shift_figure(output_dir)
        generate_filter_bank_figure(output_dir)
        generate_rearrangement_figure(output_dir)
        generate_dct_pattern_figure(output_dir)
        print(f"已生成图片目录: {output_dir}")


if __name__ == "__main__":
    main()
