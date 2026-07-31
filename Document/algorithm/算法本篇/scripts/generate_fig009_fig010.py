"""
生成第 4 节三分频器的两张数据流图（替换原 ASCII 字符图）。

输出:
  figures/Fig009_三分频概念结构.png       (4.1 节: 滤波+降采样的概念结构)
  figures/Fig010_分析与综合数据流.png     (4.4/4.5 节: 多相FIR+DCT 的实际数据流)

依赖: matplotlib
运行: python scripts/generate_fig009_fig010.py
"""
import os

import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch

# 中文字体
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

C_INPUT = '#E8F5E9'      # 浅绿 - 输入输出
C_FILTER = '#E3F2FD'     # 浅蓝 - 滤波
C_DOWN = '#FFF3E0'       # 浅橙 - 采样率变换
C_DCT = '#FFFDE7'        # 浅黄 - DCT 矩阵
C_BAND = '#F3E5F5'       # 浅紫 - 子带
C_EDGE = '#37474F'
C_ARROW = '#455A64'


def out_path(filename):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    out_dir = os.path.join(script_dir, '..', 'figures')
    os.makedirs(out_dir, exist_ok=True)
    return os.path.join(out_dir, filename)


def draw_box(ax, x, y, w, h, text, color, fontsize=10.5, bold=False):
    box = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.07",
                         facecolor=color, edgecolor=C_EDGE, linewidth=1.4)
    ax.add_patch(box)
    weight = 'bold' if bold else 'normal'
    ax.text(x + w / 2, y + h / 2, text, ha='center', va='center',
            fontsize=fontsize, fontweight=weight)


def draw_arrow(ax, x1, y1, x2, y2, text='', text_dy=0.28):
    ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                arrowprops=dict(arrowstyle='->', color=C_ARROW, lw=1.7))
    if text:
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        ax.text(mx, my + text_dy, text, fontsize=9, color='#616161',
                ha='center', va='center')


def generate_fig009():
    """4.1 节概念结构: 每条支路先带通滤波、再 3:1 降采样"""
    fig, ax = plt.subplots(figsize=(12, 5))
    ax.set_xlim(0, 13)
    ax.set_ylim(0, 7.2)
    ax.axis('off')

    rows = [
        (5.4, '滤出 0–8 kHz', 'Band 0 (16 kHz)'),
        (3.4, '滤出 8–16 kHz', 'Band 1 (16 kHz)'),
        (1.4, '滤出 16–24 kHz', 'Band 2 (16 kHz)'),
    ]

    # 输入框（垂直居中于三行）
    draw_box(ax, 0.3, 3.15, 2.6, 1.5, '48 kHz 输入\n$x(n)$', C_INPUT,
             fontsize=11.5, bold=True)

    for y, filt, band in rows:
        draw_box(ax, 4.4, y, 3.0, 1.1, filt, C_FILTER, fontsize=10.5)
        draw_box(ax, 8.2, y, 1.2, 1.1, r'$\downarrow 3$', C_DOWN, fontsize=12, bold=True)
        draw_box(ax, 10.2, y, 2.5, 1.1, band, C_BAND, fontsize=10.5, bold=True)
        draw_arrow(ax, 2.9, 3.9, 4.4, y + 0.55)
        draw_arrow(ax, 7.4, y + 0.55, 8.2, y + 0.55)
        draw_arrow(ax, 9.4, y + 0.55, 10.2, y + 0.55)

    ax.text(5.9, 6.9, '先带通滤波、再 3:1 降采样：避免混叠（概念结构，非实际计算顺序）',
            ha='center', va='center', fontsize=12.5, fontweight='bold', color='#212121')
    ax.text(5.9, 0.55, '注：直接每 3 个采样取 1 个会混叠，必须先滤出各自负责的频段',
            ha='center', va='center', fontsize=9.5, color='#757575', style='italic')

    fig.savefig(out_path('Fig009_三分频概念结构.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('已生成: Fig009_三分频概念结构.png')


def generate_fig010():
    """4.4/4.5 节: 分析侧与综合侧的实际数据流（多相 FIR + DCT）"""
    fig, axes = plt.subplots(2, 1, figsize=(13, 9))

    # --- 上半: 分析侧 ---
    ax = axes[0]
    ax.set_xlim(0, 14)
    ax.set_ylim(0, 7.4)
    ax.axis('off')

    draw_box(ax, 0.3, 3.15, 2.1, 1.5, '48 kHz 输入\n$x(n)$', C_INPUT,
             fontsize=11, bold=True)

    rows = [(5.4, '相位 0'), (3.4, '相位 1'), (1.4, '相位 2')]
    for i, (y, phase) in enumerate(rows):
        draw_box(ax, 3.6, y, 2.4, 1.1, f'{phase}，' + r'$\downarrow 3$', C_DOWN, fontsize=10.5)
        draw_box(ax, 6.8, y, 2.4, 1.1, '短多相 FIR\n(16 kHz 运行)', C_FILTER, fontsize=9.5)
        draw_arrow(ax, 2.4, 3.9, 3.6, y + 0.55)
        draw_arrow(ax, 6.0, y + 0.55, 6.8, y + 0.55)
        draw_arrow(ax, 9.2, y + 0.55, 10.4, 4.2 - i * 0.6)
        ax.text(9.7, y + 0.55 + 0.32, f'$e_{i}$', fontsize=10.5, color='#616161',
                ha='center', va='center')

    draw_box(ax, 10.4, 2.85, 1.6, 1.5, '3×3\nDCT', C_DCT, fontsize=11, bold=True)
    draw_box(ax, 12.3, 2.85, 1.5, 1.5, 'Band\n0 / 1 / 2', C_BAND, fontsize=10.5, bold=True)
    draw_arrow(ax, 12.0, 3.6, 12.3, 3.6)

    ax.text(4.8, 6.9, '【共享原型滤波】', ha='center', va='center',
            fontsize=10.5, color='#1565C0', fontweight='bold')
    ax.text(11.2, 6.9, '【区分三个频段】', ha='center', va='center',
            fontsize=10.5, color='#F9A825', fontweight='bold')
    ax.text(7.0, 0.5, '分析侧：三条线为三类输入采样相位；共享多相 FIR 负责“滤”，末端 DCT 矩阵负责“分”',
            ha='center', va='center', fontsize=10, color='#757575', style='italic')
    ax.set_title('分析侧数据流', fontsize=13, fontweight='bold', pad=2)

    # --- 下半: 综合侧 ---
    ax = axes[1]
    ax.set_xlim(0, 14)
    ax.set_ylim(0, 5.2)
    ax.axis('off')

    yc = 2.2
    draw_box(ax, 0.4, yc, 1.7, 1.4, 'Band\n0 / 1 / 2', C_BAND, fontsize=10.5, bold=True)
    draw_box(ax, 3.0, yc, 1.5, 1.4, r'$\mathbf{C}^{T}$', C_DCT, fontsize=13, bold=True)
    draw_box(ax, 5.4, yc, 3.0, 1.4, '三路多相综合 FIR\n(16 kHz 运行)', C_FILTER, fontsize=10)
    draw_box(ax, 9.3, yc, 2.4, 1.4, r'$\uparrow 3$ 并交错合并', C_DOWN, fontsize=10.5)
    draw_box(ax, 12.4, yc, 1.4, 1.4, '48 kHz\n输出', C_INPUT, fontsize=10.5, bold=True)

    draw_arrow(ax, 2.1, yc + 0.7, 3.0, yc + 0.7)
    draw_arrow(ax, 4.5, yc + 0.7, 5.4, yc + 0.7)
    draw_arrow(ax, 8.4, yc + 0.7, 9.3, yc + 0.7)
    draw_arrow(ax, 11.7, yc + 0.7, 12.4, yc + 0.7)

    ax.text(7.0, 0.6, '综合侧：DCT 正交，逆变换只需转置矩阵 $\\mathbf{C}^{T}$；综合 FIR 负责选出 ↑3 产生的正确频谱副本',
            ha='center', va='center', fontsize=10, color='#757575', style='italic')
    ax.set_title('综合侧数据流（分析侧的镜像）', fontsize=13, fontweight='bold', pad=2)

    fig.suptitle('三分频器的实际数据流：多相 FIR + DCT 调制', fontsize=14,
                 fontweight='bold', y=0.98)
    fig.tight_layout(rect=[0, 0, 1, 0.96])

    fig.savefig(out_path('Fig010_分析与综合数据流.png'),
                dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print('已生成: Fig010_分析与综合数据流.png')


if __name__ == '__main__':
    generate_fig009()
    generate_fig010()
