// mem_profile.c - 测量 ENC 降噪算法活跃内存占用
// 用于评估嵌入式移植时的 RAM 需求
#include <stdio.h>
#include "AudioProcess/NoiseSuppressor.h"
#include "AudioProcess/AudioBuffer.h"

int main(void) {
    printf("============================================================\n");
    printf("  ENC Noise Suppressor - Active Memory Profile\n");
    printf("  (算法移植嵌入式所需 RAM 评估)\n");
    printf("============================================================\n\n");

    // === 1. 持久状态内存 (Handler, 全程存在) ===
    printf("[1] 持久状态内存 (NoiseSuppressor Handler)\n");
    printf("    sizeof(NoiseSuppressor)            = %zu bytes (%.2f KB)\n",
           sizeof(NoiseSuppressor), sizeof(NoiseSuppressor) / 1024.0);
    printf("      ├── sizeof(SuppressionParams)    = %zu bytes\n", sizeof(SuppressionParams));
    printf("      ├── sizeof(FilterBankState)      = %zu bytes\n", sizeof(FilterBankState));
    printf("      ├── sizeof(ThreeBandFilterBank)  = %zu bytes\n", sizeof(ThreeBandFilterBank));
    printf("      └── sizeof(ChannelState)         = %zu bytes\n", sizeof(ChannelState));
    printf("            ├── sizeof(SpeechProbabilityEstimator) = %zu bytes\n", sizeof(SpeechProbabilityEstimator));
    printf("            │     ├── sizeof(SignalModelEstimator) = %zu bytes\n", sizeof(SignalModelEstimator));
    printf("            │     │     ├── sizeof(Histograms)     = %zu bytes\n", sizeof(Histograms));
    printf("            │     │     ├── sizeof(PriorSignalModel) = %zu bytes\n", sizeof(PriorSignalModel));
    printf("            │     │     └── sizeof(SignalModel)    = %zu bytes\n", sizeof(SignalModel));
    printf("            │     └── speech_probability_[129]     = %zu bytes\n", sizeof(float) * 129);
    printf("            ├── sizeof(WienerFilter)   = %zu bytes\n", sizeof(WienerFilter));
    printf("            ├── sizeof(NoiseEstimator) = %zu bytes\n", sizeof(NoiseEstimator));
    printf("            │     └── sizeof(QuantileNoiseEstimator) = %zu bytes\n", sizeof(QuantileNoiseEstimator));
    printf("            ├── prev_analysis_signal_spectrum[129] = %zu bytes\n", sizeof(float) * kFftSizeBy2Plus1);
    printf("            ├── analyze_analysis_memory[96]        = %zu bytes\n", sizeof(float) * (kFftSize - kNsFrameSize));
    printf("            ├── process_analysis_memory[96]        = %zu bytes\n", sizeof(float) * kOverlapSize);
    printf("            ├── process_synthesis_memory[96]       = %zu bytes\n", sizeof(float) * kOverlapSize);
    printf("            └── process_delay_memory[2][96]        = %zu bytes\n", sizeof(float) * (num_bands_ - 1) * kOverlapSize);

    // === 2. 每帧动态分配 (FrameBuffer, 每帧 malloc/free) ===
    printf("\n[2] 每帧动态分配 (FrameBuffer)\n");
    printf("    sizeof(FrameBuffer struct)         = %zu bytes\n", sizeof(FrameBuffer));
    printf("    split_data_ 指针数组 (3 bands)     = %zu bytes\n", sizeof(float*) * num_bands_);
    printf("    split_data_ 数据区 (3x160 float)   = %zu bytes\n", sizeof(float) * num_bands_ * kBandSize);
    printf("    每帧 malloc 总计                   = %zu bytes\n",
           sizeof(FrameBuffer) + sizeof(float*) * num_bands_ + sizeof(float) * num_bands_ * kBandSize);

    // === 3. 运行时栈峰值 (Analyze + Process 调用链) ===
    printf("\n[3] 运行时栈峰值估算 (局部变量)\n");

    // NoiseSuppressor_Analyze 栈
    size_t analyze_stack = 0;
    analyze_stack += sizeof(float) * kNsFrameSize;          // y_band0[160]
    analyze_stack += sizeof(float) * kFftSize;              // extended_frame[256]
    analyze_stack += sizeof(float) * (kFftSize + 2);        // fft_out[258]
    analyze_stack += sizeof(float) * kFftSizeBy2Plus1;      // signal_spectrum[129]
    analyze_stack += sizeof(float) * kFftSizeBy2Plus1;      // post_snr[129]
    analyze_stack += sizeof(float) * kFftSizeBy2Plus1;      // prior_snr[129]
    printf("    NoiseSuppressor_Analyze 栈:         = %zu bytes\n", analyze_stack);

    // NoiseSuppressor_Process 栈
    size_t process_stack = 0;
    process_stack += sizeof(FilterBankState);                // filter_bank_states (copy)
    process_stack += sizeof(float) * (kFftSize + 2);        // fft_out[258]
    process_stack += sizeof(float) * kFftSizeBy2Plus1;      // signal_spectrum[129]
    process_stack += sizeof(float) * kFftSizeBy2Plus1;      // filter_data[129]
    process_stack += sizeof(float) * num_channels_;         // upper_band_gains[1]
    process_stack += sizeof(float) * num_channels_;         // gain_adjustments[1]
    process_stack += sizeof(float) * kNsFrameSize;          // y_band[160]
    process_stack += sizeof(float) * kNsFrameSize;          // delayed_frame[160]
    printf("    NoiseSuppressor_Process 栈:         = %zu bytes\n", process_stack);

    // ENC_V NS_Process_48kAudio 栈
    size_t encv_stack = 0;
    encv_stack += sizeof(float) * 480;                      // _input[480]
    encv_stack += sizeof(float) * 3 * 160;                  // _output[3][160]
    printf("    NS_Process_48kAudio 栈:             = %zu bytes\n", encv_stack);

    // 子函数栈 (WienerFilter_Update, ComputeUpperBandsGain 等较小, 估算)
    size_t sub_stack = sizeof(float) * kFftSizeBy2Plus1 * 2 + 256; // 保守估计
    printf("    子函数调用栈 (保守估计):            = %zu bytes\n", sub_stack);

    size_t peak_stack = analyze_stack + process_stack + encv_stack + sub_stack;
    printf("    栈峰值合计 (保守):                  = %zu bytes (%.2f KB)\n", peak_stack, peak_stack / 1024.0);

    // === 4. 汇总 ===
    size_t handler_size = sizeof(NoiseSuppressor);
    size_t framebuffer_size = sizeof(FrameBuffer) + sizeof(float*) * num_bands_ + sizeof(float) * num_bands_ * kBandSize;
    size_t total_active = handler_size + framebuffer_size + peak_stack;

    printf("\n============================================================\n");
    printf("  [汇总] 算法活跃内存总占用\n");
    printf("============================================================\n");
    printf("    持久状态 (Handler):     %6zu bytes  (%.2f KB)\n", handler_size, handler_size / 1024.0);
    printf("    每帧动态 (FrameBuffer): %6zu bytes  (%.2f KB)\n", framebuffer_size, framebuffer_size / 1024.0);
    printf("    运行时栈峰值:           %6zu bytes  (%.2f KB)\n", peak_stack, peak_stack / 1024.0);
    printf("    ----------------------------------------------------\n");
    printf("    总计 (RAM 需求):        %6zu bytes  (%.2f KB)\n", total_active, total_active / 1024.0);
    printf("============================================================\n");

    printf("\n[注] 嵌入式移植建议:\n");
    printf("    - Handler 可分配为全局静态变量 (.bss)\n");
    printf("    - FrameBuffer 可改为静态数组避免 malloc\n");
    printf("    - 栈峰值需确保 MCU 栈空间 >= %.1f KB\n", peak_stack / 1024.0);
    printf("    - 若将栈上局部变量也改为静态, 总 RAM ≈ %.1f KB\n",
           (handler_size + framebuffer_size + peak_stack) / 1024.0);

    return 0;
}
