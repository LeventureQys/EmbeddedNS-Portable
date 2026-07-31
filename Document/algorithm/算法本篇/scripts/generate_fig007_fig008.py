"""
生成 6.5.5 节"最终噪声谱组成"相关的两张图。

输出:
  figures/Fig007_参数噪声模型决策流程.png   (白/粉二选一决策流程图)
  figures/Fig008_启动期噪声谱的组成.png     (参数谱与分位数谱渐变混合)

依赖: numpy、matplotlib
运行: python scripts/generate_fig007_fig008.py
"""
import os

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import FancyBboxPatch, Polygon

# 中文字体
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

C_INPUT = '#E8F5E9'      # 浅绿 - 输入输出
C_WHITE = '#E3F2FD'      # 浅蓝 - 白模型支路
C_PINK = '#FCE4EC'       # 浅粉 - 粉模型支路
C_DECISION = '#FFFDE7'   # 浅黄 - 决策
C_MERGE = '#F3E5F5'      # 浅紫 - 参数谱
C_FINAL = '#E0F2F1'      # 浅青 - 最终输出
C_EDGE = '#37474F'
C_ARROW = '#455A64'

COLORS = {
    "true": "#1976D2",
    "quantile": "#D32F2F",
    "parametric": "#7B1FA2",
    "blend": "#00897B",
}


def out_path(filename):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    out_dir = os.path.join(script_dir, '..', 'figures')
    os.makedirs(out_dir, exist_ok=True)
    return os.path.join(out_dir, filename)


def draw_box(ax, x, y, w, h, text, color, fontsize=10.5, bold=False):
    box = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.08",
                         facecolor=color, edgecolor=C_EDGE, linewidth=1.4)
    ax.add_patch(box)
    weight = 'bold' if bold else 'normal'
    ax.text(x + w / 2, y + h / 2, text, ha='center', va='center',
            fontsize=fontsize, fontweight=weight)


def draw_diamond(ax, cx, cy, w, h, text, fontsize=10.5):
    pts = [(cx, cy + h / 2), (cx + w / 2, cy), (cx, cy - h / 2), (cx - w / 2, cy)]
    ax.add_patch(Polygon(pts, closed=True, facecolor=C_DECISION,
                         edgecolor=C_EDGE, linewidth=1.4))
    ax.text(cx, cy, text, ha='center', va='center', fontsize=fontsize, fontweight='bold')


def draw_arrow(ax, x1, y1, x2, y2, text='', text_dx=0.15, text_dy=0.0):
    ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                arrowprops=dict(arrowstyle='->', color=C_ARROW, lw=1.7))
    if text:
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        ax.text(mx + text_dx, my + text_dy, text, fontsize=9.5,
                color='#616161', va='center', fontweight='bold')


def generate_fig007():
    fig, ax = plt.subplots(figsize=(12, 13))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 16.2)
    ax.axis('off')

    cx = 6.0

    # 输入
    draw_box(ax, cx - 2.6, 14.0, 5.2, 1.0,
             '当前帧幅度谱 $A_x(t,k)$，129 个 bin', C_INPUT, fontsize=11.5, bold=True)

    # 分成两条累计支路
    # 白模型支路（左）
    draw_box(ax, 0.6, 11.6, 4.6, 1.3,
             '白模型候选\n全频段求平均 → ×过减因子 α', C_WHITE, fontsize=10.5)
    draw_box(ax, 0.6, 9.6, 4.6, 1.3,
             '累计到 white_noise_level_\n（只有 1 个状态量：总高度）', C_WHITE, fontsize=10.5)
    # 粉模型支路（右）
    draw_box(ax, 6.8, 11.6, 4.6, 1.3,
             '粉模型候选\nbin 5–128 做 log-log 最小二乘拟合', C_PINK, fontsize=10.5)
    draw_box(ax, 6.8, 9.6, 4.6, 1.3,
             '截距/斜率 clamp 后累计到\npink_noise_numerator_ / pink_noise_exp_', C_PINK, fontsize=10)

    draw_arrow(ax, cx - 1.2, 14.0, 2.9, 12.9)
    draw_arrow(ax, cx + 1.2, 14.0, 9.1, 12.9)
    draw_arrow(ax, 2.9, 11.6, 2.9, 10.9)
    draw_arrow(ax, 9.1, 11.6, 9.1, 10.9)

    # 两条支路都是每帧执行的标注
    ax.text(cx, 12.25, '两条支路\n每帧都执行', ha='center', va='center', fontsize=9.5,
            color='#757575', style='italic',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#FAFAFA',
                      edgecolor='#BDBDBD', linewidth=0.8))

    # 决策菱形
    draw_diamond(ax, cx, 7.6, 5.2, 1.9, '累计斜率\npink_noise_exp_ > 0 ?', fontsize=10.5)
    draw_arrow(ax, 2.9, 9.6, cx - 1.6, 8.3)
    draw_arrow(ax, 9.1, 9.6, cx + 1.6, 8.3)

    # 两个结果分支
    draw_box(ax, 0.6, 4.9, 4.6, 1.5,
             '用白模型\nparametric[k] = white_noise_level_\n（一条水平线）', C_WHITE, fontsize=10)
    draw_box(ax, 6.8, 4.9, 4.6, 1.5,
             '用粉模型\nparametric[k] = $A_{acc}\\,/\\,k_{use}^{\\bar{\\beta}}$\n（一条幂律下降曲线）', C_PINK, fontsize=10)
    draw_arrow(ax, cx - 2.6, 7.6, 2.9, 6.4, text='否', text_dx=-0.5, text_dy=0.25)
    draw_arrow(ax, cx + 2.6, 7.6, 9.1, 6.4, text='是', text_dx=0.25, text_dy=0.25)

    # 汇合：参数谱
    draw_box(ax, cx - 3.1, 2.8, 6.2, 1.2,
             'parametric_noise_spectrum_[k]\n（任何时刻只保存其中一条曲线）', C_MERGE, fontsize=10.5, bold=True)
    draw_arrow(ax, 2.9, 4.9, cx - 1.4, 4.0)
    draw_arrow(ax, 9.1, 4.9, cx + 1.4, 4.0)

    # 最终混合
    draw_box(ax, cx - 3.6, 0.7, 7.2, 1.3,
             '与 25% 分位数谱按 $\\frac{50-t}{50} : \\frac{t}{50}$ 渐变混合\n→ noise_spectrum_（最终噪声谱，见图 Fig008）', C_FINAL, fontsize=10.5, bold=True)
    draw_arrow(ax, cx, 2.8, cx, 2.0)

    ax.text(cx, 15.8, '启动期参数噪声模型：白 / 粉二选一决策流程',
            ha='center', va='center', fontsize=13.5, fontweight='bold', color='#212121')

    fig.savefig(out_path('Fig007_参数噪声模型决策流程.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('已生成: Fig007_参数噪声模型决策流程.png')


def generate_fig008():
    rng = np.random.default_rng(7)
    k = np.arange(1, 130)

    # 真实噪声底：幂律下降 + 少量起伏（参数模型无法描述的细节）
    true_noise = 5.2 / k ** 0.42
    ripple = 1.0 + 0.16 * np.sin(k / 6.5) + 0.10 * np.sin(k / 2.3 + 1.2)
    true_noise = true_noise * ripple

    # 参数谱：对真实噪声的平滑幂律拟合（抓住轮廓、抹掉细节）
    k_use = np.maximum(k, 5)
    parametric = 5.4 / k_use ** 0.40

    def quantile_estimate(t):
        """模拟收敛中的分位数估计: t 越大越接近真实值、抖动越小"""
        conv = 1.0 - np.exp(-t / 18.0)          # 收敛程度 0→1
        bias = 0.45 + 0.55 * conv               # 未收敛时整体偏低
        jitter = rng.normal(0, 0.22 * (1.0 - 0.8 * conv), k.size)
        return true_noise * bias * np.exp(jitter)

    frames = np.arange(0, 61)
    w_q = np.clip(frames / 50.0, 0, 1)
    w_p = np.clip((50.0 - frames) / 50.0, 0, 1)

    fig = plt.figure(figsize=(13.5, 9))
    gs = fig.add_gridspec(2, 3, height_ratios=[1.0, 1.55], hspace=0.42, wspace=0.28)

    # --- 上排：权重渐变曲线 ---
    ax_w = fig.add_subplot(gs[0, :])
    ax_w.plot(frames, w_p, color=COLORS['parametric'], lw=2.4,
              label='参数模型权重 $(50-t)/50$')
    ax_w.plot(frames, w_q, color=COLORS['quantile'], lw=2.4,
              label='分位数估计权重 $t/50$')
    ax_w.axvline(25, color='#9E9E9E', ls=':', lw=1.3)
    ax_w.axvline(50, color='#9E9E9E', ls='--', lw=1.3)
    ax_w.text(25, 1.04, '交接中点 t=25', ha='center', fontsize=9.5, color='#616161')
    ax_w.text(50, 1.04, 't=50 参数模型退出', ha='center', fontsize=9.5, color='#616161')
    for t_mark in (5, 25, 45):
        ax_w.axvline(t_mark, color=COLORS['blend'], ls='-', lw=0.9, alpha=0.45)
        ax_w.text(t_mark, -0.13, f't={t_mark}', ha='center', fontsize=9,
                  color=COLORS['blend'], fontweight='bold')
    ax_w.set_xlim(0, 60)
    ax_w.set_ylim(0, 1.12)
    ax_w.set_xlabel('帧序号 t（10ms/帧）')
    ax_w.set_ylabel('混合权重')
    ax_w.set_title('前 50 帧内两路噪声估计的权重渐变', fontsize=12, fontweight='bold')
    ax_w.legend(loc='center right', fontsize=10)
    ax_w.grid(alpha=0.3)

    # --- 下排：三个时刻的频谱快照 ---
    for col, t in enumerate((5, 25, 45)):
        ax = fig.add_subplot(gs[1, col])
        q = quantile_estimate(t)
        blend = (t / 50.0) * q + ((50 - t) / 50.0) * parametric

        ax.plot(k, true_noise, color=COLORS['true'], lw=1.6, ls='--',
                label='真实噪声底（示意）')
        ax.plot(k, parametric, color=COLORS['parametric'], lw=1.8,
                label='参数谱（白/粉二选一）')
        ax.plot(k, q, color=COLORS['quantile'], lw=1.0, alpha=0.75,
                label='分位数估计（收敛中）')
        ax.plot(k, blend, color=COLORS['blend'], lw=2.4,
                label='混合结果 $\\hat{A}_d$')

        wq, wp = t / 50.0, (50 - t) / 50.0
        ax.set_title(f't = {t}：参数 {wp:.0%} + 分位数 {wq:.0%}',
                     fontsize=11, fontweight='bold', color=COLORS['blend'])
        ax.set_xlabel('频率 bin k')
        ax.set_xlim(1, 129)
        ax.set_ylim(0, 6.2)
        ax.grid(alpha=0.3)
        if col == 0:
            ax.set_ylabel('幅度（线性）')
            ax.legend(fontsize=8.5, loc='upper right')

    fig.suptitle('启动期最终噪声谱的组成：参数谱与分位数谱的渐变混合',
                 fontsize=13.5, fontweight='bold', y=0.99)

    fig.savefig(out_path('Fig008_启动期噪声谱的组成.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('已生成: Fig008_启动期噪声谱的组成.png')


if __name__ == '__main__':
    generate_fig007()
    generate_fig008()
