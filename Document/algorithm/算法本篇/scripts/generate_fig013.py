"""
生成第 8 章维纳滤波器的 DD 闭环数据流图。

输出:
  figures/Fig013_DD回路数据流.png

依赖: matplotlib
运行: python scripts/generate_fig013.py
"""
import os

import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch

# 中文字体
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

C_INPUT = '#E8F5E9'      # 浅绿 - 当前帧输入
C_HIST = '#FFF3E0'       # 浅橙 - 历史通道（缓存/重建）
C_CURR = '#E3F2FD'       # 浅蓝 - 当前通道（新证据）
C_FUSE = '#FFFDE7'       # 浅黄 - DD 融合
C_GAIN = '#F3E5F5'       # 浅紫 - 增益
C_APPLY = '#E0F2F1'      # 浅青 - 应用
C_WB = '#ECEFF1'         # 浅灰 - 回写
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


def generate_fig013():
    fig, ax = plt.subplots(figsize=(12.5, 13))
    ax.set_xlim(0, 12.5)
    ax.set_ylim(-0.4, 17.2)
    ax.axis('off')

    # --- 顶部：当前帧输入 ---
    draw_box(ax, 2.4, 15.0, 7.7, 1.2,
             '当前帧输入：观测幅度谱 $A_x(t,k)$（FFT）  +  噪声谱 $\\hat{A}_d(t,k)$（第 6 章）',
             C_INPUT, fontsize=10.5)

    # --- 左列：历史通道 ---
    ax.text(2.9, 14.0, '历史通道（旧判决，权重 98%）', fontsize=10.5,
            fontweight='bold', color='#E65100', ha='center')
    draw_box(ax, 0.5, 11.9, 4.8, 1.7,
             '上一帧缓存的三个“原料”\n$A_x^{prev}$（观测谱）  $\\hat{A}_d^{prev}$（噪声谱）\n$G_{prev}$（最终采用的增益）',
             C_HIST, fontsize=9.5)
    draw_box(ax, 0.5, 9.0, 4.8, 1.9,
             '重建上一帧输出的 SNR：\n$\\hat{A}_s^{prev}=A_x^{prev}\\cdot G_{prev}$（≈干净语音）\n'
             '$\\xi_{prev}=\\hat{A}_s^{prev}\\,/\\,\\hat{A}_d^{prev}$',
             C_HIST, fontsize=9.5, bold=True)
    draw_arrow(ax, 2.9, 11.9, 2.9, 11.1)

    # --- 右列：当前通道 ---
    ax.text(9.6, 14.0, '当前通道（新证据，权重 2%）', fontsize=10.5,
            fontweight='bold', color='#1565C0', ha='center')
    draw_box(ax, 7.2, 9.0, 4.8, 1.9,
             '后验 SNR（每帧现算）：\n$\\gamma=\\max\\left(A_x/\\hat{A}_d-1,\\;0\\right)$\n“观测超出噪声底多少”',
             C_CURR, fontsize=9.5, bold=True)
    draw_arrow(ax, 9.6, 15.0, 9.6, 13.2)
    draw_arrow(ax, 9.6, 13.2, 9.6, 11.1)

    # --- DD 融合 ---
    draw_box(ax, 2.7, 6.4, 7.1, 1.6,
             'DD 融合：$\\xi = 0.98\\cdot\\xi_{prev} + 0.02\\cdot\\gamma$\n'
             '98% 信自己的旧判决，2% 留给新证据“点火”',
             C_FUSE, fontsize=10.5, bold=True)
    draw_arrow(ax, 2.9, 9.0, 4.6, 8.2)
    draw_arrow(ax, 9.6, 9.0, 7.9, 8.2)

    # --- 增益 ---
    draw_box(ax, 2.7, 3.9, 7.1, 1.5,
             '增益：$G=\\max\\left(\\xi/(\\alpha+\\xi),\\;G_{min}\\right)$\n'
             '启动期（前 50 帧）再与 $G_{init}$ 线性混合（8.8 节）',
             C_GAIN, fontsize=10.5)
    draw_arrow(ax, 6.2, 6.4, 6.2, 5.6)

    # --- 应用 ---
    draw_box(ax, 2.7, 1.6, 7.1, 1.3,
             '应用：输出谱 $= G\\times$ 输入谱 → 逆 FFT 合成降噪语音',
             C_APPLY, fontsize=10.5)
    draw_arrow(ax, 6.2, 3.9, 6.2, 3.1)

    # --- 回写 ---
    draw_box(ax, 2.7, -0.3, 7.1, 1.1,
             '帧末回写缓存：$A_x\\to A_x^{prev}$、$\\hat{A}_d\\to\\hat{A}_d^{prev}$、$G\\to G_{prev}$',
             C_WB, fontsize=10)
    draw_arrow(ax, 6.2, 1.6, 6.2, 1.0)

    # --- 闭环：回写 → 缓存（左侧大回环，虚线） ---
    draw_arrow(ax, 2.7, 0.25, 0.25, 12.7, dashed=True, rad=-0.35)
    ax.text(-0.15, 6.6, '成为下一帧的“历史”（跨帧闭环）', fontsize=10,
            color='#E65100', rotation=90, va='center', fontweight='bold')

    # 输入 → 历史通道说明（输入本帧也参与回写与 γ 计算）
    draw_arrow(ax, 4.0, 15.0, 2.9, 13.6, dashed=False)
    ax.text(2.6, 14.55, '（读取的是上一帧存的值）', fontsize=8.5, color='#757575',
            ha='center')

    ax.text(6.25, 16.8, 'Decision-Directed 闭环：上一帧的判决指导本帧的估计',
            ha='center', va='center', fontsize=13.5, fontweight='bold',
            color='#212121')

    fig.savefig(out_path('Fig013_DD回路数据流.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('done: Fig013_DD回路数据流.png')


if __name__ == '__main__':
    generate_fig013()
