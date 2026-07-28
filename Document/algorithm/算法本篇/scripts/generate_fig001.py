"""
生成算法总体处理流程图
输出: figures/Fig001_总体处理流程.png
依赖: matplotlib
运行: python scripts/generate_fig001.py
"""
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
import numpy as np
import os

# 中文字体
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei']
plt.rcParams['axes.unicode_minus'] = False

fig, ax = plt.subplots(1, 1, figsize=(14, 18))
ax.set_xlim(0, 14)
ax.set_ylim(0, 22)
ax.axis('off')

# 颜色方案
C_INPUT = '#E8F5E9'      # 浅绿 - 输入输出
C_FILTER = '#E3F2FD'     # 浅蓝 - 滤波器组
C_MAIN = '#FFF3E0'       # 浅橙 - 主处理链路
C_UPPER = '#F3E5F5'      # 浅紫 - 高频段
C_DECISION = '#FFFDE7'   # 浅黄 - 决策
C_EDGE = '#37474F'       # 边框色
C_ARROW = '#455A64'      # 箭头色

def draw_box(ax, x, y, w, h, text, color, fontsize=11, bold=False, edgecolor=C_EDGE):
    """绘制圆角矩形框"""
    box = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.1",
                         facecolor=color, edgecolor=edgecolor, linewidth=1.5)
    ax.add_patch(box)
    weight = 'bold' if bold else 'normal'
    ax.text(x + w/2, y + h/2, text, ha='center', va='center',
            fontsize=fontsize, fontweight=weight, wrap=True)
    return (x + w/2, y, x + w/2, y + h)  # (cx, ybot, cx, ytop)

def draw_arrow(ax, x1, y1, x2, y2, text='', color=C_ARROW):
    """绘制箭头"""
    ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                arrowprops=dict(arrowstyle='->', color=color, lw=1.8))
    if text:
        mx, my = (x1+x2)/2, (y1+y2)/2
        ax.text(mx + 0.15, my, text, fontsize=9, color='#616161', va='center')

# === 布局 ===
cx = 7.0  # 中心 x

# 1. 输入
draw_box(ax, cx-2.5, 20.5, 5, 1.0, '输入: 480 samples @ 48kHz\n(10ms 帧)', C_INPUT, fontsize=12, bold=True)

# 2. 三分频器 Analysis
draw_box(ax, cx-2.5, 18.8, 5, 1.0, '三分频器 (Analysis)\n多相 FIR + DCT 调制', C_FILTER, fontsize=11, bold=True)
draw_arrow(ax, cx, 20.5, cx, 19.8)

# 3. 三个分支标注
# Band 0 - 主链路
draw_box(ax, 1.0, 16.5, 4.5, 1.0, 'Band 0 (0–8 kHz)\n160 samples', C_MAIN, fontsize=10, bold=True)
# Band 1
draw_box(ax, 6.2, 16.5, 3.2, 1.0, 'Band 1\n(8–16 kHz)', C_UPPER, fontsize=10)
# Band 2
draw_box(ax, 10.0, 16.5, 3.2, 1.0, 'Band 2\n(16–24 kHz)', C_UPPER, fontsize=10)

# 从三分频器到三个 band 的箭头
draw_arrow(ax, cx-1.0, 18.8, 3.25, 17.5)
draw_arrow(ax, cx, 18.8, 7.8, 17.5)
draw_arrow(ax, cx+1.0, 18.8, 11.6, 17.5)

# 4. Band 0 主处理链路 (纵向排列)
steps = [
    ('扩展帧 + 加窗\n(96点overlap, 256点FFT)', 14.8),
    ('FFT → 幅度谱', 13.4),
    ('噪声功率谱估计\n(分位数跟踪 + IIR平滑)', 11.8),
    ('语音存在概率\n(LRT + 平坦度 + 频谱差异)', 10.2),
    ('维纳滤波增益 G(k)\n(Decision-Directed SNR)', 8.6),
    ('IFFT + 综合窗 + OLA', 7.2),
]

for text, y in steps:
    draw_box(ax, 1.0, y, 4.5, 1.1, text, C_MAIN, fontsize=9.5)

# 步骤间箭头
for i in range(len(steps)-1):
    y_top = steps[i][1]
    y_bot = steps[i+1][1] + 1.1
    draw_arrow(ax, 3.25, y_top, 3.25, y_bot)

# 从 Band 0 到第一步
draw_arrow(ax, 3.25, 16.5, 3.25, 15.9)

# 5. 高频段处理
draw_box(ax, 6.2, 13.0, 7.0, 1.2, '时域标量增益\n(由 Band 0 高频端语音概率 + 滤波器增益推导)', C_UPPER, fontsize=10)
draw_arrow(ax, 7.8, 16.5, 9.7, 14.2)
draw_arrow(ax, 11.6, 16.5, 9.7, 14.2)

# 从 Band 0 主链路到高频段 (虚线箭头表示控制关系)
ax.annotate('', xy=(6.2, 13.6), xytext=(5.5, 12.35),
            arrowprops=dict(arrowstyle='->', color='#9E9E9E', lw=1.5, linestyle='dashed'))
ax.text(5.6, 13.1, '增益\n决策', fontsize=8, color='#757575', ha='center')

# 延迟补偿
draw_box(ax, 6.2, 11.2, 7.0, 1.0, '延迟补偿 (24 samples) + 增益施加', C_UPPER, fontsize=10)
draw_arrow(ax, 9.7, 13.0, 9.7, 12.2)

# 6. 增益修正
draw_box(ax, 1.0, 5.6, 4.5, 1.0, '增益修正\n(能量比 + 语音概率平滑)', C_DECISION, fontsize=9.5)
draw_arrow(ax, 3.25, 7.2, 3.25, 6.6)

# 7. 三分频器 Synthesis
draw_box(ax, cx-2.5, 3.8, 5, 1.0, '三分频器 (Synthesis)\n多相 FIR + DCT 调制', C_FILTER, fontsize=11, bold=True)

# Band 0 输出到综合
draw_arrow(ax, 3.25, 5.6, cx-1.0, 4.8)
# 高频段输出到综合
draw_arrow(ax, 9.7, 11.2, cx+1.5, 4.8)

# 8. 输出
draw_box(ax, cx-2.5, 2.2, 5, 1.0, '输出: 480 samples @ 48kHz', C_INPUT, fontsize=12, bold=True)
draw_arrow(ax, cx, 3.8, cx, 3.2)

# 9. 右侧标注: 启动阶段
ax.text(12.5, 9.0, '启动策略:\n• 前50帧: 参数化噪声模型\n• 前200帧: 分位数收敛\n• >200帧: 增益修正启用',
        fontsize=9, color='#616161', va='center',
        bbox=dict(boxstyle='round,pad=0.4', facecolor='#FAFAFA', edgecolor='#BDBDBD', linewidth=0.8))

# 标题
ax.text(cx, 21.8, '频域维纳滤波降噪算法 — 总体处理流程', ha='center', va='center',
        fontsize=14, fontweight='bold', color='#212121')

plt.tight_layout()

# 输出
script_dir = os.path.dirname(os.path.abspath(__file__))
out_dir = os.path.join(script_dir, '..', 'figures')
os.makedirs(out_dir, exist_ok=True)
out_path = os.path.join(out_dir, 'Fig001_总体处理流程.png')
plt.savefig(out_path, dpi=150, bbox_inches='tight', facecolor='white')
plt.close()
print(f"已生成: {os.path.abspath(out_path)}")
