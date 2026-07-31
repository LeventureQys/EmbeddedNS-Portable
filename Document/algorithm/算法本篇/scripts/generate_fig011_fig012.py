"""
生成第 7、8 章（语音存在概率 / 维纳滤波器）的两张图。

输出:
  figures/Fig011_语音概率估计流程.png     (三特征→直方图→融合→后验的完整流程)
  figures/Fig012_维纳增益特性与DD平滑.png (增益曲线族 + Decision-Directed 平滑效果)

依赖: numpy、matplotlib
运行: python scripts/generate_fig011_fig012.py
"""
import os

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import FancyBboxPatch

# 中文字体
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

C_INPUT = '#E8F5E9'      # 浅绿 - 输入
C_FEAT = '#E3F2FD'       # 浅蓝 - 特征
C_HIST = '#FFF3E0'       # 浅橙 - 直方图/阈值
C_FUSE = '#FFFDE7'       # 浅黄 - 融合
C_OUT = '#F3E5F5'        # 浅紫 - 概率输出
C_USE = '#E0F2F1'        # 浅青 - 下游用途
C_EDGE = '#37474F'
C_ARROW = '#455A64'


def out_path(filename):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    out_dir = os.path.join(script_dir, '..', 'figures')
    os.makedirs(out_dir, exist_ok=True)
    return os.path.join(out_dir, filename)


def draw_box(ax, x, y, w, h, text, color, fontsize=10, bold=False):
    box = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.07",
                         facecolor=color, edgecolor=C_EDGE, linewidth=1.4)
    ax.add_patch(box)
    weight = 'bold' if bold else 'normal'
    ax.text(x + w / 2, y + h / 2, text, ha='center', va='center',
            fontsize=fontsize, fontweight=weight)


def draw_arrow(ax, x1, y1, x2, y2, text='', dashed=False, text_dx=0.15, text_dy=0.0):
    style = dict(arrowstyle='->', color=C_ARROW, lw=1.6)
    if dashed:
        style.update(color='#9E9E9E', linestyle='dashed', lw=1.4)
    ax.annotate('', xy=(x2, y2), xytext=(x1, y1), arrowprops=style)
    if text:
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        ax.text(mx + text_dx, my + text_dy, text, fontsize=9,
                color='#616161', va='center', ha='center')


def generate_fig011():
    fig, ax = plt.subplots(figsize=(13, 14))
    ax.set_xlim(0, 13)
    ax.set_ylim(-0.7, 17.4)
    ax.axis('off')

    # --- 输入行 ---
    draw_box(ax, 0.5, 15.2, 3.6, 1.2, '先验/后验 SNR\n（用上一帧增益推得）', C_INPUT, fontsize=9.5)
    draw_box(ax, 4.7, 15.2, 3.6, 1.2, '当前幅度谱\n$A_x(t,k)$', C_INPUT, fontsize=9.5)
    draw_box(ax, 8.9, 15.2, 3.6, 1.2, '保守噪声谱\n（6.7 节模板）', C_INPUT, fontsize=9.5)

    # --- 特征行 ---
    draw_box(ax, 0.5, 12.8, 3.6, 1.5, '特征 a：LRT 均值 $L$\n逐 bin 似然比平滑后\n再对 129 bin 取平均', C_FEAT, fontsize=9.5, bold=True)
    draw_box(ax, 4.7, 12.8, 3.6, 1.5, '特征 b：频谱平坦度 $F$\n几何均值 / 算术均值', C_FEAT, fontsize=9.5, bold=True)
    draw_box(ax, 8.9, 12.8, 3.6, 1.5, '特征 c：频谱差异 $D$\n对噪声模板回归后的\n残差方差（归一化）', C_FEAT, fontsize=9.5, bold=True)

    for cx_ in (2.3, 6.5, 10.7):
        draw_arrow(ax, cx_, 15.2, cx_, 14.3)
    # 保守噪声谱也参与特征 c
    draw_arrow(ax, 10.7, 15.2, 10.7, 14.3)

    # --- 直方图 ---
    draw_box(ax, 2.4, 10.4, 8.2, 1.3,
             '三个 1000-bin 直方图，累积 500 帧（5 秒）内的特征分布', C_HIST, fontsize=10.5)
    draw_arrow(ax, 2.3, 12.8, 4.5, 11.7)
    draw_arrow(ax, 6.5, 12.8, 6.5, 11.7)
    draw_arrow(ax, 10.7, 12.8, 8.5, 11.7)

    # --- 阈值提取 ---
    draw_box(ax, 2.4, 8.2, 8.2, 1.5,
             '每 500 帧提取一次模型参数：\n'
             '阈值 $\\theta_{lrt}$ / $\\theta_{flat}$ / $\\theta_{diff}$ + 三特征权重 $w_i$（被拒绝的特征权重置零）',
             C_HIST, fontsize=10)
    draw_arrow(ax, 6.5, 10.4, 6.5, 9.7)

    # --- tanh 指示函数 ---
    draw_box(ax, 0.5, 5.9, 3.6, 1.5, '$I_0$：tanh 映射\n$L$ 对比 $\\theta_{lrt}$', C_FUSE, fontsize=9.5)
    draw_box(ax, 4.7, 5.9, 3.6, 1.5, '$I_1$：tanh 映射（反向）\n$F$ 对比 $\\theta_{flat}$', C_FUSE, fontsize=9.5)
    draw_box(ax, 8.9, 5.9, 3.6, 1.5, '$I_2$：tanh 映射\n$D$ 对比 $\\theta_{diff}$', C_FUSE, fontsize=9.5)
    draw_arrow(ax, 4.0, 8.2, 2.3, 7.4)
    draw_arrow(ax, 6.5, 8.2, 6.5, 7.4)
    draw_arrow(ax, 9.0, 8.2, 10.7, 7.4)

    # --- 融合 ---
    draw_box(ax, 3.4, 3.7, 6.2, 1.4,
             '加权求和 $\\sum w_i I_i$ + 一阶平滑（步长 0.1）\n→ 全局先验语音概率 $P_{prior}$（每帧 1 个标量）',
             C_OUT, fontsize=10, bold=True)
    draw_arrow(ax, 2.3, 5.9, 4.7, 5.1)
    draw_arrow(ax, 6.5, 5.9, 6.5, 5.1)
    draw_arrow(ax, 10.7, 5.9, 8.9, 5.1)

    # --- 后验 ---
    draw_box(ax, 3.0, 1.6, 7.0, 1.4,
             '贝叶斯融合：$P_s(k)=\\dfrac{1}{1+\\frac{1-P_{prior}}{P_{prior}}e^{-\\bar{\\ell}(k)}}$（129 个 bin）',
             C_OUT, fontsize=10.5, bold=True)
    draw_arrow(ax, 6.5, 3.7, 6.5, 3.0)
    # 逐 bin 似然比来自特征 a 的中间量
    draw_arrow(ax, 0.9, 12.8, 0.9, 2.3, dashed=True)
    draw_arrow(ax, 0.9, 2.3, 3.0, 2.3, dashed=True)
    ax.text(1.35, 7.6, '逐 bin 平滑似然比 $\\bar{\\ell}(k)$', fontsize=9, color='#757575',
            rotation=90, va='center')

    # --- 下游用途 ---
    draw_box(ax, 0.4, -0.4, 3.7, 1.1, '噪声更新门控\n（6.6 节）', C_USE, fontsize=9.5)
    draw_box(ax, 4.7, -0.4, 3.6, 1.1, '高频段标量增益\n（第 10 章）', C_USE, fontsize=9.5)
    draw_box(ax, 8.9, -0.4, 3.7, 1.1, '整体缩放因子\n（第 9 章，用 $P_{prior}$）', C_USE, fontsize=9.5)
    draw_arrow(ax, 4.8, 1.6, 2.3, 0.7)
    draw_arrow(ax, 6.5, 1.6, 6.5, 0.7)
    draw_arrow(ax, 8.2, 1.6, 10.7, 0.7)

    ax.text(6.5, 17.0, '语音存在概率估计：三特征 → 直方图自适应阈值 → 贝叶斯后验',
            ha='center', va='center', fontsize=13.5, fontweight='bold', color='#212121')

    fig.savefig(out_path('Fig011_语音概率估计流程.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('已生成: Fig011_语音概率估计流程.png')


def generate_fig012():
    fig, axes = plt.subplots(1, 2, figsize=(13.5, 5.2))

    # --- 左：维纳增益曲线族 ---
    ax = axes[0]
    snr_db = np.linspace(-20, 20, 400)
    snr = 10 ** (snr_db / 10)          # 这里把先验 SNR 视作线性比值
    levels = [
        (0, 1.0, 0.5, '#1976D2'),
        (1, 1.0, 0.25, '#00897B'),
        (2, 1.1, 0.125, '#F9A825'),
        (3, 1.25, 0.09, '#D32F2F'),
    ]
    for lv, alpha, gmin, color in levels:
        gain = np.maximum(snr / (alpha + snr), gmin)
        ax.plot(snr_db, 20 * np.log10(gain), color=color, lw=2.0,
                label=f'级别 {lv}: $\\alpha$={alpha}, $G_{{min}}$={gmin}')
        ax.axhline(20 * np.log10(gmin), color=color, ls=':', lw=0.9, alpha=0.5)

    ax.set_xlabel('先验 SNR（dB）')
    ax.set_ylabel('增益（dB）')
    ax.set_title('维纳增益曲线：$G=\\max(\\xi/(\\alpha+\\xi),\\ G_{min})$',
                 fontsize=11.5, fontweight='bold')
    ax.set_ylim(-24, 2)
    ax.grid(alpha=0.3)
    ax.legend(fontsize=9, loc='lower right')

    # --- 右：DD 平滑效果（按真实递推：prior = 0.98·(幅度比·G_prev) + 0.02·post）---
    ax = axes[1]
    rng = np.random.default_rng(3)
    n_frames = 200
    frames = np.arange(n_frames)
    # 真实语音 SNR：噪声段为 0，中间一段语音
    true_snr = np.zeros(n_frames)
    true_snr[70:130] = 8.0 * np.sin(np.linspace(0, np.pi, 60)) ** 0.8

    # 观测幅度比 X/N：噪声段在 1 附近抖动，语音段抬高
    obs_ratio = np.maximum(1.0 + true_snr + rng.normal(0, 1.2, n_frames), 0.05)
    post_snr = np.maximum(obs_ratio - 1.0, 0.0)

    # 真实 Decision-Directed 递推（级别 1: alpha=1, Gmin=0.25）
    alpha, gmin = 1.0, 0.25
    prior = np.zeros(n_frames)
    gain = np.ones(n_frames)
    for t in range(1, n_frames):
        prev_estimate = obs_ratio[t - 1] * gain[t - 1]
        prior[t] = 0.98 * prev_estimate + 0.02 * post_snr[t]
        gain[t] = min(max(prior[t] / (alpha + prior[t]), gmin), 1.0)

    # “省事版”纯 IIR 平滑（直接复用上一帧的 xi，新观测只能走 2% 窄门）
    naive = np.zeros(n_frames)
    for t in range(1, n_frames):
        naive[t] = 0.98 * naive[t - 1] + 0.02 * post_snr[t]

    ax.plot(frames, post_snr, color='#90A4AE', lw=1.0, alpha=0.85,
            label='瞬时后验 SNR（逐帧抖动）')
    ax.plot(frames, true_snr, color='#1976D2', lw=1.8, ls='--',
            label='真实语音 SNR（示意）')
    ax.plot(frames, naive, color='#7B1FA2', lw=1.8, ls='-.',
            label='“省事版”纯 IIR（新观测只占 2%，爬升极慢）')
    ax.plot(frames, prior, color='#D32F2F', lw=2.2,
            label='DD 先验 SNR（完整观测乘 $G_{prev}$ 进入回路）')
    ax.axvspan(70, 130, color='#F9A825', alpha=0.10)
    ax.text(100, 11.0, '语音段', ha='center', fontsize=9.5, color='#B28900')

    ax.set_xlabel('帧序号 t（10ms/帧）')
    ax.set_ylabel('SNR（线性）')
    ax.set_title('Decision-Directed：噪声段抖动被压平，语音起始只慢 1–2 帧',
                 fontsize=11.5, fontweight='bold')
    ax.set_ylim(-0.5, 12)
    ax.grid(alpha=0.3)
    ax.legend(fontsize=8.5, loc='upper right')

    fig.suptitle('维纳滤波器的两个关键设计：增益曲线形状与先验 SNR 平滑',
                 fontsize=13, fontweight='bold', y=1.02)
    fig.tight_layout()

    fig.savefig(out_path('Fig012_维纳增益特性与DD平滑.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('已生成: Fig012_维纳增益特性与DD平滑.png')


if __name__ == '__main__':
    generate_fig011()
    generate_fig012()
