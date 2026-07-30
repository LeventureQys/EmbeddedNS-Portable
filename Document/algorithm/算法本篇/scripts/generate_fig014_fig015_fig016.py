"""
生成第 8 章（维纳滤波器）的三张补充图。

输出:
  figures/Fig014_增益公式拆解.png     (增益公式流水线：SNR→理想增益→α修正→Gmin地板)
  figures/Fig015_DD跨帧机制全景.png   (三面板：真实SNR / 逐帧增益热图 / DD vs 朴素IIR)
  figures/Fig016_DD噪声段vs语音段.png (对比噪声段/语音段回路中各量的数值变化)

依赖: numpy、matplotlib
运行: python scripts/generate_fig014_fig015_fig016.py
"""
import os

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

# 中文字体
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

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


def draw_arrow(ax, x1, y1, x2, y2, text='', dashed=False,
               text_dx=0.15, text_dy=0.0, rad=0.0):
    style = dict(arrowstyle='->', color=C_ARROW, lw=1.6,
                 connectionstyle=f'arc3,rad={rad}')
    if dashed:
        style.update(color='#E65100', linestyle='dashed', lw=1.8)
    ax.annotate('', xy=(x2, y2), xytext=(x1, y1), arrowprops=style)
    if text:
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        ax.text(mx + text_dx, my + text_dy, text, fontsize=9,
                color='#616161', va='center', ha='center')


# ============================================================
# Fig014: 增益公式拆解
# ============================================================
def generate_fig014():
    fig, ax = plt.subplots(figsize=(15, 6.5))
    ax.set_xlim(0, 15)
    ax.set_ylim(-0.5, 8)
    ax.axis('off')

    # 标题
    ax.text(7.5, 7.5, '增益公式拆解：从先验 SNR 到最终增益',
            ha='center', va='center', fontsize=14, fontweight='bold', color='#212121')

    # 四个阶段
    y_center = 4.0
    box_w = 2.8
    box_h = 2.2
    gap = 0.5

    x0 = 0.5
    x1 = x0 + box_w + gap
    x2 = x1 + box_w + gap
    x3 = x2 + box_w + gap

    # Stage 1: 先验 SNR → 理想增益
    draw_box(ax, x0, y_center - box_h / 2, box_w, box_h,
             '$\\xi$\n↓\n$G_{ideal}=\\frac{\\xi}{1+\\xi}$\n\n语音占比 → 保留比例',
             '#E3F2FD', fontsize=10, bold=True)

    # Arrow 1→2
    draw_arrow(ax, x0 + box_w, y_center, x1, y_center, text='', text_dy=0.35)
    ax.text((x0 + box_w + x1) / 2, y_center + 0.5, '引入 $\\alpha$', fontsize=9,
            ha='center', color='#E65100', fontweight='bold')

    # Stage 2: 过减修正
    draw_box(ax, x1, y_center - box_h / 2, box_w, box_h,
             '$G=\\frac{\\xi}{\\alpha+\\xi}$\n\n$\\alpha>1$: 故意高估噪声\n中低 SNR 曲线下移',
             '#FFF3E0', fontsize=10, bold=True)

    # Arrow 2→3
    draw_arrow(ax, x1 + box_w, y_center, x2, y_center, text='', text_dy=0.35)
    ax.text((x1 + box_w + x2) / 2, y_center + 0.5, '引入 $G_{min}$', fontsize=9,
            ha='center', color='#E65100', fontweight='bold')

    # Stage 3: 增益地板
    draw_box(ax, x2, y_center - box_h / 2, box_w, box_h,
             '$G=\\max(G,\\;G_{min})$\n\n不让任何 bin 彻底静音\n噪声段：回路阻尼器',
             '#F3E5F5', fontsize=10, bold=True)

    # Arrow 3→4
    draw_arrow(ax, x2 + box_w, y_center, x3, y_center)

    # Stage 4: 最终增益
    draw_box(ax, x3, y_center - box_h / 2, box_w, box_h,
             '$G(k) \\in [G_{min}, 1]$\n\n乘到频谱上:\n$\\hat{X}(k)=G(k)\\cdot X(k)$',
             '#E8F5E9', fontsize=10, bold=True)

    # 底部数值示例
    ax.text(7.5, 1.0, '数值示例（级别 1：$\\alpha=1.0,\\;G_{min}=0.25$）',
            ha='center', fontsize=10.5, fontweight='bold', color='#37474F')
    examples = [
        ('$\\xi=9$（强语音）', '$9/(1+9)=0.90$', '0.90', '几乎不动'),
        ('$\\xi=2$（中等）', '$2/(1+2)=0.67$', '0.67', '压掉 1/3'),
        ('$\\xi=0.1$（弱信号）', '$0.1/(1+0.1)=0.09$', '$\\max(0.09,0.25)=0.25$', '地板兜底'),
    ]
    for i, (xi_val, ideal, result, effect) in enumerate(examples):
        y = 0.1 - i * 0.45
        ax.text(1.5, y, xi_val, fontsize=9, ha='center', color='#455A64')
        ax.text(5.0, y, '$\\to$ ' + ideal, fontsize=9, ha='center', color='#455A64')
        ax.text(8.5, y, '$\\to$ ' + result, fontsize=9, ha='center', color='#D32F2F', fontweight='bold')
        ax.text(12.0, y, effect, fontsize=9, ha='center', color='#2E7D32')

    fig.savefig(out_path('Fig014_增益公式拆解.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('done: Fig014_增益公式拆解.png')


# ============================================================
# Fig015: DD 跨帧机制全景
# ============================================================
def generate_fig015():
    rng = np.random.default_rng(42)
    n_frames = 150
    frames = np.arange(n_frames)

    # 真实 SNR
    true_snr_linear = np.zeros(n_frames)
    true_snr_linear[50:100] = 6.0 * np.sin(np.linspace(0, np.pi, 50)) ** 0.8

    # 观测幅度比
    obs_ratio = np.maximum(1.0 + true_snr_linear + rng.normal(0, 1.0, n_frames), 0.1)
    post_snr = np.maximum(obs_ratio - 1.0, 0.0)

    # DD 递推（级别 1）
    alpha, gmin = 1.0, 0.25
    dd_prior = np.zeros(n_frames)
    dd_gain = np.ones(n_frames)
    for t in range(1, n_frames):
        prev_est = obs_ratio[t - 1] * dd_gain[t - 1]
        dd_prior[t] = 0.98 * prev_est + 0.02 * post_snr[t]
        dd_gain[t] = min(max(dd_prior[t] / (alpha + dd_prior[t]), gmin), 1.0)

    # 朴素 IIR
    naive = np.zeros(n_frames)
    for t in range(1, n_frames):
        naive[t] = 0.98 * naive[t - 1] + 0.02 * post_snr[t]

    fig, axes = plt.subplots(1, 3, figsize=(17, 5.5))

    # Panel 1: 真实 SNR 时序
    ax = axes[0]
    ax.plot(frames, 10 * np.log10(np.maximum(true_snr_linear, 0.001)),
            color='#1976D2', lw=2)
    ax.axvspan(50, 100, color='#F9A825', alpha=0.15)
    ax.axhline(0, color='#9E9E9E', ls=':', lw=0.8)
    ax.set_xlabel('帧序号 t（10ms/帧）')
    ax.set_ylabel('SNR（dB）')
    ax.set_title('(a) 每个 bin 的真实先验 SNR', fontweight='bold')
    ax.set_ylim(-30, 12)
    ax.grid(alpha=0.3)
    ax.text(75, 10, '语音段', ha='center', fontsize=9, color='#B28900')
    ax.text(25, -25, '纯噪声', ha='center', fontsize=9, color='#757575')

    # Panel 2: 增益热图
    ax = axes[1]
    n_bins = 64
    bin_gains = np.zeros((n_frames, n_bins))
    for b in range(n_bins):
        local_ratio = np.maximum(
            1.0 + true_snr_linear * (0.5 + 0.5 * np.random.default_rng(b + 1).random())
            + rng.normal(0, 0.8, n_frames), 0.1)
        local_post = np.maximum(local_ratio - 1.0, 0.0)
        p = np.zeros(n_frames)
        g = np.ones(n_frames)
        for t in range(1, n_frames):
            pe = local_ratio[t - 1] * g[t - 1]
            p[t] = 0.98 * pe + 0.02 * local_post[t]
            g[t] = min(max(p[t] / (alpha + p[t]), gmin), 1.0)
        bin_gains[:, b] = g

    im = ax.imshow(bin_gains.T, aspect='auto', origin='lower', cmap='RdYlGn',
                   extent=[0, 1500, 0, 8000], vmin=0, vmax=1)
    ax.set_xlabel('时间（ms）')
    ax.set_ylabel('频率（Hz）')
    ax.set_title('(b) 逐 bin 增益 $G(k)$ 热图', fontweight='bold')
    cbar = fig.colorbar(im, ax=ax, shrink=0.85)
    cbar.set_label('增益', fontsize=9)
    ax.axvspan(500, 1000, color='#F9A825', alpha=0.08)

    # Panel 3: DD vs 朴素
    ax = axes[2]
    ax.plot(frames, post_snr, color='#90A4AE', lw=0.8, alpha=0.7,
            label='后验 SNR $\\gamma$（逐帧抖动）')
    ax.plot(frames, true_snr_linear, color='#1976D2', lw=1.5, ls='--',
            label='真实 SNR')
    ax.plot(frames, naive, color='#7B1FA2', lw=1.8, ls='-.',
            label='朴素 IIR（爬升极慢）')
    ax.plot(frames, dd_prior, color='#D32F2F', lw=2.2,
            label='DD 先验 $\\xi$（快速跟踪）')
    ax.axvspan(50, 100, color='#F9A825', alpha=0.10)
    ax.set_xlabel('帧序号 t（10ms/帧）')
    ax.set_ylabel('SNR（线性）')
    ax.set_title('(c) DD 递推 vs 朴素 IIR 平滑', fontweight='bold')
    ax.set_ylim(-0.3, 8)
    ax.grid(alpha=0.3)
    ax.legend(fontsize=8, loc='upper right')

    fig.suptitle('Decision-Directed 全景：噪声段稳定、语音段快速响应的秘密',
                 fontsize=13.5, fontweight='bold', y=1.02)
    fig.tight_layout()
    fig.savefig(out_path('Fig015_DD跨帧机制全景.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('done: Fig015_DD跨帧机制全景.png')


# ============================================================
# Fig016: DD 在噪声段 vs 语音段的行为对比
# ============================================================
def generate_fig016():
    fig, axes = plt.subplots(1, 2, figsize=(14, 7))

    C_NOISE = '#E3F2FD'
    C_SPEECH = '#E8F5E9'
    C_FORMULA = '#FFFDE7'

    def draw_box_local(ax, x, y, w, h, text, color, fontsize=9.5, bold=False):
        box = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.06",
                             facecolor=color, edgecolor=C_EDGE, linewidth=1.3)
        ax.add_patch(box)
        weight = 'bold' if bold else 'normal'
        ax.text(x + w / 2, y + h / 2, text, ha='center', va='center',
                fontsize=fontsize, fontweight=weight)

    # --- 左：噪声段 ---
    ax = axes[0]
    ax.set_xlim(0, 10)
    ax.set_ylim(-0.5, 10)
    ax.axis('off')
    ax.set_title('噪声段：$G_{prev}$ 被压在地板上，回路自动阻尼',
                 fontweight='bold', fontsize=12, color='#1565C0')

    draw_box_local(ax, 1, 8.2, 8, 1.2,
                   '上一帧状态：$G_{prev}=G_{min}=0.25$（地板）',
                   C_NOISE, fontsize=10)

    draw_box_local(ax, 0.5, 5.8, 8, 1.6,
                   '历史项 = $(A_x^{prev}/\\hat{A}_d^{prev})\\times G_{prev}$\n'
                   '$\\approx 1.0 \\times 0.25 = 0.25$\n'
                   '（$G_{min}$ 把观测抖动衰减到 1/4）',
                   C_NOISE, fontsize=10, bold=True)

    draw_box_local(ax, 1, 3.6, 8, 1.4,
                   '新证据 = $\\gamma \\approx 0$（噪声段观测 ≈ 噪声底）\n'
                   '$0.02 \\times 0 = 0$',
                   C_FORMULA, fontsize=10)

    draw_box_local(ax, 1, 1.4, 8, 1.4,
                   '$\\xi = 0.98 \\times 0.25 + 0.02 \\times 0 = 0.245$\n'
                   '$G = 0.245/(1+0.245) = 0.197 \\to \\max(\\cdot, 0.25) = 0.25$',
                   C_FORMULA, fontsize=10.5, bold=True)

    draw_arrow(ax, 5, 8.2, 5, 7.4)
    draw_arrow(ax, 5, 5.8, 5, 5.0)
    draw_arrow(ax, 5, 3.6, 5, 2.8)

    ax.text(5, 0.3, '结论：$\\xi$ 稳定贴低位，$G$ 稳坐地板——无音乐噪声',
            ha='center', fontsize=10.5, fontweight='bold', color='#2E7D32')

    # --- 右：语音起始 ---
    ax = axes[1]
    ax.set_xlim(0, 10)
    ax.set_ylim(-0.5, 10)
    ax.axis('off')
    ax.set_title('语音起始：2% 新证据"点火"，下一帧历史项迅速接管',
                 fontweight='bold', fontsize=12, color='#E65100')

    draw_box_local(ax, 0.5, 8.2, 9, 1.2,
                   '第 71 帧：$G_{prev}=0.25$（还在地板上），$\\gamma=4.0$（语音突现）',
                   '#FFF3E0', fontsize=10)

    draw_box_local(ax, 0.5, 5.8, 9, 1.6,
                   '$\\xi = 0.98\\times(1.0\\times 0.25) + 0.02\\times 4.0$\n'
                   '$= 0.98\\times 0.25 + 0.08 = 0.245+0.08 = 0.33$\n'
                   '$G = 0.33/(1+0.33) = 0.248 \\to$ 仍在地板附近',
                   C_FORMULA, fontsize=10, bold=True)

    draw_box_local(ax, 0.5, 3.4, 9, 1.6,
                   '第 72 帧：$G_{prev}=0.25$，但 $A_x/\\hat{A}_d=5.0$（语音继续）\n'
                   '$\\xi = 0.98\\times(5.0\\times 0.25)+0.02\\times\\gamma$\n'
                   '$= 0.98\\times 1.25 + \\cdots = 1.225 + \\cdots \\approx 1.27$',
                   '#FFF3E0', fontsize=10, bold=True)

    draw_box_local(ax, 0.5, 1.2, 9, 1.4,
                   '$G = 1.27/(1+1.27) = 0.56$\n'
                   '增益从 0.25 跳到 0.56——已离开地板！',
                   C_FORMULA, fontsize=10.5, bold=True)

    draw_arrow(ax, 5, 8.2, 5, 7.4)
    draw_arrow(ax, 5, 5.8, 5, 5.0)
    draw_arrow(ax, 5, 3.4, 5, 2.6)

    ax.text(5, 0.1, '结论：仅 2 帧（20ms），增益已离开地板——人耳察觉不到的延迟',
            ha='center', fontsize=10.5, fontweight='bold', color='#2E7D32')

    fig.suptitle('Decision-Directed 的两种面孔：$G_{prev}$ 是关键开关',
                 fontsize=13.5, fontweight='bold', y=1.01)
    fig.tight_layout()
    fig.savefig(out_path('Fig016_DD噪声段vs语音段.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('done: Fig016_DD噪声段vs语音段.png')


if __name__ == '__main__':
    generate_fig014()
    generate_fig015()
    generate_fig016()
