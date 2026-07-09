# -*- coding: utf-8 -*-
"""
Generate all numbered figures for the Noise Suppressor teaching document.
Each figure starts with a number (01-08) for easy referencing.
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch
import numpy as np
import os

plt.rcParams.update({
    'font.family': 'sans-serif',
    'font.size': 10,
    'figure.dpi': 150,
    'savefig.bbox': 'tight',
    'savefig.pad_inches': 0.1,
})

OUT_DIR = os.path.dirname(os.path.abspath(__file__))

COLORS = {
    'input': '#4ECDC4',
    'process': '#FF6B6B',
    'output': '#45B7D1',
    'model': '#96CEB4',
    'estimate': '#FFEAA7',
    'decision': '#DDA0DD',
    'orange': '#FF9F43',
    'purple': '#A29BFE',
    'green': '#00B894',
    'red': '#E17055',
    'blue': '#74B9FF',
    'gray': '#B2BEC3',
    'dark': '#2D3436',
}


def draw_box(ax, x, y, w, h, text, color, fontsize=9, text_color='white'):
    box = FancyBboxPatch((x - w/2, y - h/2), w, h,
                          boxstyle="round,pad=0.1",
                          facecolor=color, edgecolor='#2D3436',
                          linewidth=1.2, alpha=0.92)
    ax.add_patch(box)
    ax.text(x, y, text, ha='center', va='center', fontsize=fontsize,
            fontweight='bold', color=text_color)


def draw_arrow(ax, x1, y1, x2, y2, color='#2D3436', lw=1.5):
    ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                arrowprops=dict(arrowstyle='->', color=color, lw=lw,
                                connectionstyle='arc3,rad=0'))


def draw_label(ax, x, y, text, fontsize=8, color='#2D3436', rotation=0):
    ax.text(x, y, text, ha='center', va='center', fontsize=fontsize,
            color=color, style='italic', rotation=rotation)


# ============================================================
# Figure 01: Overall Pipeline
# ============================================================
def fig01_overall_pipeline():
    fig, ax = plt.subplots(1, 1, figsize=(14, 6))
    ax.set_xlim(0, 14)
    ax.set_ylim(0, 6)
    ax.axis('off')
    ax.set_title("01 - Noise Suppressor Complete Pipeline", fontsize=14, fontweight='bold', pad=15)

    draw_box(ax, 1.0, 4.5, 1.8, 0.8, "Input Audio\n48kHz / 16kHz", COLORS['input'], 8)
    draw_label(ax, 1.0, 5.1, "Pre-processing", fontsize=8, color=COLORS['input'])
    draw_arrow(ax, 1.9, 4.5, 2.5, 4.5)
    draw_box(ax, 3.5, 4.5, 2.0, 0.9, "3-Band Filter Bank\nSplit -> Band 0/1/2", COLORS['process'], 8)

    draw_arrow(ax, 4.5, 4.5, 5.5, 4.5)
    draw_label(ax, 5.0, 4.8, "Band 0 only", fontsize=7, color='#636E72')

    draw_box(ax, 7.0, 4.5, 2.0, 0.8, "256-pt FFT\n+ Magnitude Spectrum", COLORS['blue'], 8)

    draw_arrow(ax, 8.0, 4.5, 9.0, 5.2)
    draw_box(ax, 11.0, 5.2, 2.2, 0.7, "Noise Estimator\n(Quantile+Parametric)", COLORS['estimate'], 7)
    draw_arrow(ax, 8.0, 4.5, 9.0, 4.5)
    draw_box(ax, 11.0, 4.5, 2.0, 0.8, "SNR Computation\n(Prior / Post SNR)", COLORS['orange'], 7)

    draw_arrow(ax, 8.0, 4.5, 9.0, 3.8)
    draw_box(ax, 11.0, 3.8, 2.2, 0.7, "Speech Probability\nEstimator", COLORS['decision'], 7)

    draw_arrow(ax, 11.0, 3.4, 11.0, 3.0)
    draw_arrow(ax, 11.0, 4.0, 11.0, 3.0)
    draw_box(ax, 11.0, 2.6, 2.2, 0.7, "Wiener Filter\n(Freq-domain Gain)", COLORS['red'], 7)

    draw_arrow(ax, 10.0, 2.6, 8.5, 2.6)
    draw_box(ax, 7.0, 2.6, 2.0, 0.8, "Apply Filter\n+ 256-pt IFFT", COLORS['blue'], 8)

    draw_arrow(ax, 6.0, 2.6, 5.0, 2.6)
    draw_box(ax, 3.5, 2.6, 2.0, 0.9, "Overlap-Add\n+Synthesis Window", COLORS['process'], 8)

    draw_arrow(ax, 2.5, 2.6, 1.5, 2.6)
    draw_box(ax, 3.5, 1.6, 2.0, 0.7, "Upper Band\nGain x Delay", COLORS['green'], 7)
    draw_arrow(ax, 3.5, 2.0, 3.5, 1.4)
    draw_arrow(ax, 3.5, 1.2, 1.5, 1.2)
    draw_box(ax, 1.0, 1.4, 1.5, 0.7, "3-Band\nSynthesis", COLORS['process'], 7)

    draw_arrow(ax, 1.0, 1.0, 1.0, 0.5)
    draw_box(ax, 1.0, 0.3, 1.8, 0.6, "Output Audio", COLORS['output'], 9)

    ax.axvline(x=2.2, ymin=0.05, ymax=0.95, color='#B2BEC3', linestyle='--', linewidth=1, alpha=0.5)
    ax.axvline(x=4.9, ymin=0.05, ymax=0.95, color='#B2BEC3', linestyle='--', linewidth=1, alpha=0.5)
    ax.axvline(x=9.5, ymin=0.05, ymax=0.95, color='#B2BEC3', linestyle='--', linewidth=1, alpha=0.5)
    ax.text(1.65, 5.75, "Pre-processing", fontsize=8, color='#636E72', ha='center')
    ax.text(7.2, 5.75, "Core NS Algorithm", fontsize=8, color='#636E72', ha='center')
    ax.text(12.5, 5.75, "Post-processing", fontsize=8, color='#636E72', ha='center')

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '01_overall_pipeline.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 01_overall_pipeline.png")


# ============================================================
# Figure 02: Three-Band Filter Bank Splitting
# ============================================================
def fig02_filterbank_splitting():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 5))
    fig.suptitle("02 - Three-Band Filter Bank (Analysis & Synthesis)", fontsize=13, fontweight='bold')

    ax1.set_xlim(0, 10)
    ax1.set_ylim(0, 10)
    ax1.axis('off')
    ax1.set_title("Analysis (Split)", fontsize=12, color=COLORS['red'])

    draw_box(ax1, 5, 9, 3.0, 0.7, "Input: 48kHz, 480 samples", COLORS['input'], 8)
    draw_label(ax1, 5, 9.6, "10 ms frame @ 48kHz", 7)

    draw_arrow(ax1, 5, 8.3, 3, 7.5)
    draw_arrow(ax1, 5, 8.3, 5, 7.5)
    draw_arrow(ax1, 5, 8.3, 7, 7.5)

    draw_box(ax1, 3, 6.8, 2.0, 0.7, "LPF\n0~8 kHz", COLORS['blue'], 7)
    draw_box(ax1, 5, 6.8, 2.0, 0.7, "BPF\n8~16 kHz", COLORS['orange'], 7)
    draw_box(ax1, 7, 6.8, 2.0, 0.7, "HPF\n16~24 kHz", COLORS['green'], 7)

    draw_arrow(ax1, 3, 6.1, 3, 5.5)
    draw_arrow(ax1, 5, 6.1, 5, 5.5)
    draw_arrow(ax1, 7, 6.1, 7, 5.5)

    draw_box(ax1, 3, 4.8, 2.0, 0.7, "Down-3 Decimate\n160 samples", COLORS['purple'], 7)
    draw_box(ax1, 5, 4.8, 2.0, 0.7, "Down-3 Decimate\n160 samples", COLORS['purple'], 7)
    draw_box(ax1, 7, 4.8, 2.0, 0.7, "Down-3 Decimate\n160 samples", COLORS['purple'], 7)

    draw_arrow(ax1, 3, 4.1, 1.5, 3.4)
    draw_arrow(ax1, 5, 4.1, 5, 3.4)
    draw_arrow(ax1, 7, 4.1, 8.5, 3.4)

    draw_box(ax1, 1.5, 2.8, 1.6, 0.6, "Band 0\n[0-8kHz]", COLORS['output'], 7)
    draw_box(ax1, 5, 2.8, 1.6, 0.6, "Band 1\n[8-16kHz]", COLORS['output'], 7)
    draw_box(ax1, 8.5, 2.8, 1.6, 0.6, "Band 2\n[16-24kHz]", COLORS['output'], 7)

    ax1.text(5, 1.5, "Band 0 processed by NS core\nBands 1&2 gain-scaled from Band 0 statistics",
             ha='center', fontsize=8, color='#636E72', style='italic',
             bbox=dict(boxstyle='round', facecolor='#F8F9FA', alpha=0.8))

    # Right: Synthesis
    ax2.set_xlim(0, 10)
    ax2.set_ylim(0, 10)
    ax2.axis('off')
    ax2.set_title("Synthesis (Merge)", fontsize=12, color=COLORS['green'])

    draw_box(ax2, 1.5, 9, 1.6, 0.6, "Band 0\n(processed)", COLORS['output'], 7)
    draw_box(ax2, 5, 9, 1.6, 0.6, "Band 1\n(gain x delayed)", COLORS['output'], 7)
    draw_box(ax2, 8.5, 9, 1.6, 0.6, "Band 2\n(gain x delayed)", COLORS['output'], 7)

    draw_arrow(ax2, 1.5, 8.4, 1.5, 7.8)
    draw_arrow(ax2, 5, 8.4, 5, 7.8)
    draw_arrow(ax2, 8.5, 8.4, 8.5, 7.8)

    draw_box(ax2, 1.5, 7.2, 1.6, 0.6, "Up-3 Interp.", COLORS['purple'], 7)
    draw_box(ax2, 5, 7.2, 1.6, 0.6, "Up-3 Interp.", COLORS['purple'], 7)
    draw_box(ax2, 8.5, 7.2, 1.6, 0.6, "Up-3 Interp.", COLORS['purple'], 7)

    draw_arrow(ax2, 1.5, 6.6, 1.5, 6.0)
    draw_arrow(ax2, 5, 6.6, 5, 6.0)
    draw_arrow(ax2, 8.5, 6.6, 8.5, 6.0)

    draw_box(ax2, 1.5, 5.3, 1.6, 0.7, "LPF\n0~8 kHz", COLORS['blue'], 7)
    draw_box(ax2, 5, 5.3, 1.6, 0.7, "BPF\n8~16 kHz", COLORS['orange'], 7)
    draw_box(ax2, 8.5, 5.3, 1.6, 0.7, "HPF\n16~24 kHz", COLORS['green'], 7)

    draw_arrow(ax2, 1.5, 4.6, 3, 3.8)
    draw_arrow(ax2, 5, 4.6, 4.5, 3.8)
    draw_arrow(ax2, 8.5, 4.6, 6.5, 3.8)
    draw_box(ax2, 5, 3.2, 3.0, 0.6, "Sum (Sigma)", COLORS['process'], 8)

    draw_arrow(ax2, 5, 2.6, 5, 2.1)
    draw_box(ax2, 5, 1.5, 3.0, 0.7, "Output: 48kHz\n480 samples", COLORS['input'], 8)

    ax2.text(5, 0.5, "DCT-modulated FIR filter bank (Harris)\n~9.5dB SNR perfect reconstruction",
             ha='center', fontsize=8, color='#636E72', style='italic',
             bbox=dict(boxstyle='round', facecolor='#F8F9FA', alpha=0.8))

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '02_filterbank_splitting.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 02_filterbank_splitting.png")


# ============================================================
# Figure 03: Overlap-Add (OLA) Illustration
# ============================================================
def fig03_overlap_add():
    fig, ax = plt.subplots(1, 1, figsize=(12, 5))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 6)
    ax.axis('off')
    fig.suptitle("03 - Overlap-Add (OLA) with 256-point FFT on 160-sample Frames", fontsize=13, fontweight='bold')

    for i, (start, color) in enumerate([
        (0.8, '#FF6B6B'),
        (3.5, '#4ECDC4'),
        (6.2, '#FFEAA7'),
        (8.9, '#A29BFE'),
    ]):
        rect = FancyBboxPatch((start, 3.2), 3.6, 0.9,
                               boxstyle="round,pad=0.1",
                               facecolor=color, edgecolor='#2D3436',
                               linewidth=1.2, alpha=0.7)
        ax.add_patch(rect)
        ax.text(start + 1.8, 3.65, "Frame %d: 160 new samples" % i, ha='center', va='center',
                fontsize=8, fontweight='bold')

        if i > 0:
            overlap_rect = FancyBboxPatch((start, 3.9), 2.16, 0.5,
                                           boxstyle="round,pad=0.05",
                                           facecolor='#2D3436', edgecolor='#2D3436',
                                           linewidth=0.5, alpha=0.3)
            ax.add_patch(overlap_rect)
            ax.text(start + 1.08, 4.15, "96-sample overlap", ha='center', va='center',
                    fontsize=6.5, color='#636E72', style='italic')

    arrow_start = 0.8 + 3.6
    ax.annotate('', xy=(arrow_start + 0.1, 3.2), xytext=(arrow_start + 0.1, 1.5),
                arrowprops=dict(arrowstyle='->', color='#2D3436', lw=1.5))

    rect = FancyBboxPatch((0.8, 1.0), 4.1, 0.8,
                           boxstyle="round,pad=0.1",
                           facecolor='#FF6B6B', edgecolor='#2D3436', linewidth=1.5, alpha=0.5)
    ax.add_patch(rect)
    rect = FancyBboxPatch((3.9, 1.0), 2.16, 0.8,
                           boxstyle="round,pad=0.1",
                           facecolor='#B2BEC3', edgecolor='#2D3436', linewidth=1.5, alpha=0.5)
    ax.add_patch(rect)
    ax.text(2.85, 1.4, "Old data (96)", ha='center', va='center', fontsize=8, color='white', fontweight='bold')
    ax.text(4.98, 1.4, "New (160)", ha='center', va='center', fontsize=8, fontweight='bold')

    ax.annotate('', xy=(5.2, 1.4), xytext=(5.8, 1.4),
                arrowprops=dict(arrowstyle='->', color='#2D3436', lw=1.5))
    rect = FancyBboxPatch((5.8, 1.0), 2.0, 0.8,
                           boxstyle="round,pad=0.1",
                           facecolor=COLORS['orange'], edgecolor='#2D3436', linewidth=1.2)
    ax.add_patch(rect)
    ax.text(6.8, 1.4, "Hanning\nWindow", ha='center', va='center', fontsize=8, fontweight='bold')

    ax.annotate('', xy=(8.0, 1.4), xytext=(8.6, 1.4),
                arrowprops=dict(arrowstyle='->', color='#2D3436', lw=1.5))
    rect = FancyBboxPatch((8.6, 1.0), 2.0, 0.8,
                           boxstyle="round,pad=0.1",
                           facecolor=COLORS['purple'], edgecolor='#2D3436', linewidth=1.2)
    ax.add_patch(rect)
    ax.text(9.6, 1.4, "FFT\n(256 pt)", ha='center', va='center', fontsize=8, fontweight='bold')

    params_text = (
        "Key Parameters:\n"
        "  kFftSize = 256\n"
        "  kNsFrameSize = 160 (10ms @ 16kHz equivalent)\n"
        "  kOverlapSize = 256 - 160 = 96\n"
        "  Window: Hybrid Hanning + flat (96 pts each side)"
    )
    ax.text(6, 5.3, params_text, ha='center', fontsize=9, family='monospace',
            bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '03_overlap_add.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 03_overlap_add.png")


# ============================================================
# Figure 04: Noise Estimation - Quantile + Parametric
# ============================================================
def fig04_noise_estimation():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 5.5))
    fig.suptitle("04 - Noise Estimation: Quantile Method + Parametric Model", fontsize=13, fontweight='bold')

    ax1.set_xlim(0, 10)
    ax1.set_ylim(0, 10)
    ax1.axis('off')
    ax1.set_title("Quantile Noise Estimator", fontsize=12, color=COLORS['red'])

    draw_box(ax1, 5, 9, 3.0, 0.7, "Log-Magnitude Spectrum", COLORS['input'], 8)
    draw_arrow(ax1, 5, 8.3, 5, 7.8)

    draw_box(ax1, 2, 7.0, 1.8, 0.7, "Estimator 1\n(~67 frames)", COLORS['blue'], 7)
    draw_box(ax1, 5, 7.0, 1.8, 0.7, "Estimator 2\n(~133 frames)", COLORS['orange'], 7)
    draw_box(ax1, 8, 7.0, 1.8, 0.7, "Estimator 3\n(~200 frames)", COLORS['green'], 7)

    ax1.annotate('', xy=(3.5, 7.0), xytext=(4.5, 7.0),
                arrowprops=dict(arrowstyle='<->', color='#636E72', lw=1))
    ax1.text(4.0, 7.6, "kSimult = 3", ha='center', fontsize=7, color='#636E72')

    detail_text = (
        "Each estimator at freq bin k:\n"
        "  if log_spec > log_quantile:\n"
        "    log_quantile += 0.25*delta/(counter+1)\n"
        "  else:\n"
        "    log_quantile -= 0.75*delta/(counter+1)\n\n"
        "Ratio 0.25:0.75 tracks lower quantile\n"
        "(noise tends to be minimum energy)"
    )
    ax1.text(5, 5.2, detail_text, ha='center', fontsize=8, family='monospace',
             bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    draw_arrow(ax1, 5, 3.8, 5, 3.0)
    draw_box(ax1, 5, 2.3, 2.5, 0.8, "Quantile Noise\nSpectrum (129 bins)", COLORS['estimate'], 8)

    # Right: Parametric Model
    ax2.set_xlim(0, 10)
    ax2.set_ylim(0, 10)
    ax2.axis('off')
    ax2.set_title("Parametric Noise Model (Startup)", fontsize=12, color=COLORS['green'])

    draw_box(ax2, 5, 9, 3.0, 0.7, "First 50 frames only", COLORS['input'], 8)
    draw_arrow(ax2, 5, 8.3, 3, 7.5)
    draw_arrow(ax2, 5, 8.3, 7, 7.5)

    draw_box(ax2, 3, 6.8, 2.0, 0.8, "White Noise\nLevel Estimation", COLORS['blue'], 7)
    draw_box(ax2, 7, 6.8, 2.0, 0.8, "Pink Noise\nSlope Estimation", COLORS['orange'], 7)

    draw_arrow(ax2, 3, 6.0, 3, 5.3)
    draw_arrow(ax2, 7, 6.0, 7, 5.3)

    eq_text = (
        "Parametric Spectrum:\n"
        "  N(f) = white_level      (if pink_exp=0)\n"
        "  N(f) = num / f^exp       (if pink_exp>0)\n\n"
        "Blend with quantile estimate:\n"
        "  noise = a(n)*quantile + (1-a(n))*parametric"
    )
    ax2.text(5, 4.2, eq_text, ha='center', fontsize=9, family='monospace',
             bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    draw_arrow(ax2, 5, 3.2, 5, 2.5)
    draw_box(ax2, 5, 1.8, 3.0, 0.8, "Conservative + Parametric\nNoise Spectra", COLORS['estimate'], 8)

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '04_noise_estimation.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 04_noise_estimation.png")


# ============================================================
# Figure 05: SNR Computation
# ============================================================
def fig05_snr_computation():
    fig, ax = plt.subplots(1, 1, figsize=(12, 5.5))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 7)
    ax.axis('off')
    fig.suptitle("05 - SNR Estimation: Post SNR & Decision-Directed Prior SNR", fontsize=13, fontweight='bold')

    draw_box(ax, 2, 6, 2.0, 0.7, "Signal Spectrum\nS(k)", COLORS['input'], 8)
    draw_box(ax, 6, 6, 2.0, 0.7, "Noise Spectrum\nN(k)", COLORS['estimate'], 8)
    draw_box(ax, 10, 6, 2.0, 0.7, "Previous Filter\nH_prev(k)", COLORS['purple'], 8)

    draw_arrow(ax, 2, 5.3, 2, 4.8)
    draw_arrow(ax, 6, 5.3, 5, 4.8)
    draw_box(ax, 3.5, 4.3, 3.0, 0.7, "Post SNR:\ngamma(k) = max(S(k)/N(k)-1, 0)", COLORS['red'], 8)

    draw_arrow(ax, 6, 5.3, 8, 4.8)
    draw_arrow(ax, 10, 5.3, 9, 4.8)
    draw_box(ax, 8.5, 4.3, 3.0, 0.7, "Previous Estimate:\nprev = S_prev(k)/N_prev(k)*H_prev(k)", COLORS['blue'], 7)

    draw_arrow(ax, 3.5, 3.6, 5, 3.0)
    draw_arrow(ax, 8.5, 3.6, 7, 3.0)
    draw_box(ax, 6, 2.5, 4.0, 0.7, "Decision-Directed Prior SNR:\nxi(k) = 0.98*prev_est + 0.02*gamma(k)", COLORS['orange'], 8)

    explain_text = (
        "The Decision-Directed approach (Ephraim & Malah, 1984):\n"
        "  - Heavily weights prior frame's estimate (a=0.98) for smoothness\n"
        "  - Slightly blends in current frame's post SNR (1-a=0.02) for tracking\n"
        "  - Solved the problem of 'musical noise' in early spectral subtraction\n\n"
        "Performed per frequency bin (k = 0..128, 129 bins total)"
    )
    ax.text(6, 1.2, explain_text, ha='center', fontsize=9, family='monospace',
            bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '05_snr_computation.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 05_snr_computation.png")


# ============================================================
# Figure 06: Speech Probability Estimation
# ============================================================
def fig06_speech_probability():
    fig, ax = plt.subplots(1, 1, figsize=(13, 7))
    ax.set_xlim(0, 13)
    ax.set_ylim(0, 10)
    ax.axis('off')
    fig.suptitle("06 - Speech Probability: Three Feature Indicators -> Likelihood", fontsize=13, fontweight='bold')

    draw_box(ax, 2.2, 8.5, 2.5, 0.8, "Feature 1: LRT\nLog Likelihood Ratio", COLORS['red'], 8)
    draw_arrow(ax, 2.2, 7.8, 2.2, 7.2)
    draw_box(ax, 2.2, 6.5, 2.8, 0.7, "Indicator 0:\nsigma(w*(LRT-theta_LRT))", COLORS['orange'], 7)

    draw_box(ax, 6.5, 8.5, 2.5, 0.8, "Feature 2: Spectral\nFlatness", COLORS['green'], 8)
    draw_arrow(ax, 6.5, 7.8, 6.5, 7.2)
    draw_box(ax, 6.5, 6.5, 2.8, 0.7, "Indicator 1:\nsigma(w*(theta_flat-Flatness))", COLORS['orange'], 7)

    draw_box(ax, 10.8, 8.5, 2.5, 0.8, "Feature 3: Spectral\nDifference", COLORS['blue'], 8)
    draw_arrow(ax, 10.8, 7.8, 10.8, 7.2)
    draw_box(ax, 10.8, 6.5, 2.8, 0.7, "Indicator 2:\nsigma(w*(Diff-theta_diff))", COLORS['orange'], 7)

    draw_arrow(ax, 2.2, 5.8, 4.5, 5.0)
    draw_arrow(ax, 6.5, 5.8, 6.5, 5.0)
    draw_arrow(ax, 10.8, 5.8, 8.5, 5.0)

    draw_box(ax, 6.5, 4.3, 5.0, 0.7, "Weighted Sum: P = w_LRT*I0 + w_flat*I1 + w_diff*I2", COLORS['purple'], 8)

    ax.text(6.5, 5.7, "sigma(x) = 0.5 * (tanh(x) + 1)", ha='center', fontsize=8,
            color='#636E72', style='italic')

    draw_arrow(ax, 6.5, 3.6, 6.5, 3.2)
    draw_box(ax, 6.5, 2.5, 4.0, 0.7, "Prior Speech Prob: q += 0.1 * (P - q)", COLORS['decision'], 8)

    draw_arrow(ax, 6.5, 1.8, 6.5, 1.4)
    detail = (
        "Per-bin speech probability:\n"
        "p(k) = 1 / (1 + (1-q)/q * exp(-avg_log_lrt(k)))\n"
        "-> High LRT bin -> high speech prob\n"
        "-> Low LRT bin -> low speech prob (noise)"
    )
    ax.text(6.5, 0.7, detail, ha='center', fontsize=8, family='monospace',
            bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    ax.text(6.5, 9.2, "Thresholds theta & weights w updated every 500 frames via histogram analysis",
            ha='center', fontsize=8, color='#636E72', style='italic',
            bbox=dict(boxstyle='round', facecolor='#F8F9FA', alpha=0.8))

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '06_speech_probability.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 06_speech_probability.png")


# ============================================================
# Figure 07: Wiener Filter
# ============================================================
def fig07_wiener_filter():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 5.5))
    fig.suptitle("07 - Wiener Filter: Filter Design & Application", fontsize=13, fontweight='bold')

    ax1.set_xlim(0, 12)
    ax1.set_ylim(0, 10)
    ax1.axis('off')
    ax1.set_title("Wiener Filter Design", fontsize=12, color=COLORS['red'])

    draw_box(ax1, 2, 9, 2.0, 0.7, "Prior SNR xi(k)", COLORS['orange'], 8)
    draw_box(ax1, 7, 9, 2.0, 0.7, "Over-subtraction\nFactor alpha", COLORS['blue'], 8)

    draw_arrow(ax1, 2, 8.3, 3.5, 7.7)
    draw_arrow(ax1, 7, 8.3, 5.5, 7.7)

    eq_box = (
        "H(k) = xi(k) / (alpha + xi(k))\n\n"
        "Clamped:\n"
        "  H_min <= H(k) <= 1.0\n\n"
        "where H_min = minimum_attenuating_gain\n"
        "  (k6dB -> 0.5, k12dB -> 0.25,\n"
        "   k18dB -> 0.125, k21dB -> 0.09)"
    )
    ax1.text(4.5, 6.8, eq_box, ha='center', fontsize=9, family='monospace',
             bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    draw_arrow(ax1, 4.5, 4.8, 4.5, 4.3)
    draw_box(ax1, 4.5, 3.5, 4.5, 0.8, "Startup Phase (first 50 frames):\nBlend with initial spectral estimate filter", COLORS['purple'], 7)

    # Right: Filter Application
    ax2.set_xlim(0, 12)
    ax2.set_ylim(0, 10)
    ax2.axis('off')
    ax2.set_title("Filter Application & Gain Adjustment", fontsize=12, color=COLORS['green'])

    draw_box(ax2, 3, 9, 2.5, 0.7, "FFT Coeffs:\nRe(k), Im(k)", COLORS['input'], 8)
    draw_box(ax2, 9, 9, 2.5, 0.7, "Filter:\nH(k) 129 bins", COLORS['red'], 8)

    draw_arrow(ax2, 3, 8.3, 4.5, 7.7)
    draw_arrow(ax2, 9, 8.3, 7.5, 7.7)

    draw_box(ax2, 6, 7.2, 3.5, 0.7, "Re'(k) = Re(k)*H(k)\nIm'(k) = Im(k)*H(k)", COLORS['process'], 8)

    draw_arrow(ax2, 6, 6.5, 6, 6.0)
    draw_box(ax2, 6, 5.3, 2.5, 0.7, "IFFT -> Time Domain", COLORS['blue'], 8)

    draw_arrow(ax2, 6, 4.6, 6, 4.1)
    draw_box(ax2, 6, 3.4, 3.5, 0.7, "Overall Scaling Factor:\nAdaptive gain adjustment", COLORS['orange'], 8)

    scale_text = (
        "ComputeOverallScalingFactor:\n"
        "  gain = sqrt(E_after / E_before)\n"
        "  scale = q*f1(gain) + (1-q)*f2(gain)\n"
        "  -> Protects speech, avoids over-attenuation"
    )
    ax2.text(6, 2.0, scale_text, ha='center', fontsize=8, family='monospace',
             bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '07_wiener_filter.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 07_wiener_filter.png")


# ============================================================
# Figure 08: Upper Band Processing
# ============================================================
def fig08_upper_band():
    fig, ax = plt.subplots(1, 1, figsize=(12, 5.5))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 8)
    ax.axis('off')
    fig.suptitle("08 - Upper Band Processing: Delay + Gain from Band 0 Statistics", fontsize=13, fontweight='bold')

    draw_box(ax, 6, 7.5, 3.0, 0.7, "Band 0 (0-8kHz): Fully processed\nwith Wiener filter + OLA", COLORS['red'], 8)

    draw_arrow(ax, 6, 6.8, 6, 6.2)
    draw_box(ax, 6, 5.5, 4.5, 0.7, "Extract statistics from Band 0 last 32 bins (high end):\navg(speech_prob) + avg(filter_gain) + spectrum ratio", COLORS['blue'], 7)

    draw_arrow(ax, 6, 4.8, 3, 4.2)
    draw_arrow(ax, 6, 4.8, 9, 4.2)

    draw_box(ax, 3, 3.5, 3.0, 0.7, "Speech Prob -> Gain:\ngain_s = 0.5*(1+tanh(2p-1))\n(sigmoid mapping)", COLORS['orange'], 7)

    draw_box(ax, 9, 3.5, 3.0, 0.7, "Combine with Filter Gain:\ngain = blend(gain_s, avg(H))\nclamp to [H_min, 1.0]", COLORS['purple'], 7)

    draw_arrow(ax, 3, 2.8, 5, 2.3)
    draw_arrow(ax, 9, 2.8, 7, 2.3)

    draw_box(ax, 6, 1.8, 3.5, 0.7, "Upper Band Single Gain ->\nBand 1: delay(24 samples)*gain\nBand 2: delay(24 samples)*gain", COLORS['green'], 8)

    explain_text = (
        "Key insight: Human ear is less sensitive to noise at high frequencies.\n"
        "A single time-domain gain (scalar) is applied to Bands 1 & 2,\n"
        "rather than full per-frequency Wiener filtering.\n"
        "This saves significant computation (no FFT for upper bands)."
    )
    ax.text(6, 0.5, explain_text, ha='center', fontsize=9, family='monospace',
            bbox=dict(boxstyle='round', facecolor='#F8F9FA', edgecolor='#B2BEC3'))

    plt.tight_layout()
    fig.savefig(os.path.join(OUT_DIR, '08_upper_band.png'), dpi=150, facecolor='white')
    plt.close(fig)
    print("Generated: 08_upper_band.png")


# ============================================================
# Main
# ============================================================
if __name__ == '__main__':
    print("Saving figures to: %s\n" % OUT_DIR)
    fig01_overall_pipeline()
    fig02_filterbank_splitting()
    fig03_overlap_add()
    fig04_noise_estimation()
    fig05_snr_computation()
    fig06_speech_probability()
    fig07_wiener_filter()
    fig08_upper_band()
    print("\nAll 8 figures generated successfully!")
