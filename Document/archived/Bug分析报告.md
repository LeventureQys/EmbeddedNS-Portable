# Project C 移植 Bug 分析报告

## 摘要

对比 `Project/ENC_Core/AudioProcess/NoiseSuppressor.c` 与 WebRTC 原始代码 (`Webrtc_Source/.../ns/noise_suppressor.cc`)，发现 C 移植中存在**3 处逻辑 bug**，其中 2 处影响上频段降噪，1 处影响增益调整。

---

## Bug 1：上频段增益读错变量（致命）

### 位置
`NoiseSuppressor.c:358` vs `NoiseSuppressor.c:423`

### 现象
`ComputeUpperBandsGain()` 的结果存入了局部数组 `upper_band_gains[ch]`（line 358），但后续读取时却从 `var->upper_band_gains_heap_[0]` 取（line 423）。heap 副本在 `init` 时被 memset 为 0 且从未更新，因此 `upper_band_gain` 始终为 0。

### C 移植代码 (NoiseSuppressor.c)
```c
// Line 353-359: 正确计算增益并存入局部数组
if (num_bands_ > 1) {
    upper_band_gains[ch] = ComputeUpperBandsGain(..., signal_spectrum);
}

// ...

// Line 421-423: 却从 heap 读取（永远为 0）
if (num_bands_ > 1) {
    float upper_band_gain = var->upper_band_gains_heap_[0];  // ← BUG: 应为 upper_band_gains[0]
    for (int ch = 1; ch < num_channels_; ++ch) {
        upper_band_gain = min_local(upper_band_gain, var->upper_band_gains_heap_[ch]);
    }
```

### WebRTC 原始代码 (noise_suppressor.cc)
```cpp
// Line 442-448: 存入局部数组
if (num_bands_ > 1) {
    upper_band_gains[ch] = ComputeUpperBandsGain(...);
}

// Line 511-516: 从局部数组读取
if (num_bands_ > 1) {
    float upper_band_gain = upper_band_gains[0];  // ← 正确：读取局部变量
    for (size_t ch = 1; ch < num_channels_; ++ch) {
        upper_band_gain = std::min(upper_band_gain, upper_band_gains[ch]);
    }
```

### 影响
`upper_band_gain` 始终为 0 → 即使下一条 bug 修复了，上频段也会被完全静音。

---

## Bug 2：上频段处理结果未写回 audio（致命）

### 位置
`NoiseSuppressor.c:420-441`

### 现象
处理上频段时，代码将 `audio->split_data_[b]` 拷贝到局部数组 `y_band`，对 `y_band` 做加权，但**从未将结果写回** `audio->split_data_[b]`。

### C 移植代码 (NoiseSuppressor.c)
```c
float y_band[kNsFrameSize];                          // line 420: 局部栈变量
if (num_bands_ > 1) {
    for (int ch = 0; ch < num_channels_; ++ch) {
        for (int b = 1; b < num_bands_; ++b) {
            float delayed_frame[kNsFrameSize];
            memcpy(y_band, audio->split_data_[b], sizeof(float) * (kNsFrameSize)); // 拷入
            DelaySignal(y_band, var->channels_0.process_delay_memory[b - 1], delayed_frame);
            for (int j = 0; j < kNsFrameSize; j++) {
                y_band[j] = upper_band_gain * delayed_frame[j]; // 写入局部 y_band
            }
            // ← BUG: 未写回 audio->split_data_[b]
        }
    }
}
```

### WebRTC 原始代码 (noise_suppressor.cc)
```cpp
for (size_t b = 1; b < num_bands_; ++b) {
    rtc::ArrayView<float, kNsFrameSize> y_band(
        &audio->split_bands(ch)[b][0], kNsFrameSize);  // ← y_band 直接指向 audio 内部
    std::array<float, kNsFrameSize> delayed_frame;
    DelaySignal(y_band, channels_[ch]->process_delay_memory[b - 1], delayed_frame);
    for (size_t j = 0; j < kNsFrameSize; j++) {
        y_band[j] = upper_band_gain * delayed_frame[j]; // ← 直接写入 audio
    }
}
```

### 影响
即使 Bug 1 先被修复（upper_band_gain 正确），上频段数据也从未被修改，band 1 和 band 2 的噪声完全穿透，不受任何抑制。

### Bug 1 + Bug 2 的级联效果
- **Band 0 (0-8 kHz)**：正常 Wiener 滤波降噪
- **Band 1 (8-16 kHz)** + **Band 2 (16-24 kHz)**：**噪声完全穿透，无任何抑制**

---

## Bug 3：ComputeOverallScalingFactor 返回 -1

### 位置
`wiener_filter.c:70-84`

### 现象
当 `gain >= 0.5` 时（语音存在的常见情况），函数走到末尾 `return -1`，而不是返回组合缩放因子。这导致 `gain_adjustment` 为 -1，输入端低频信号相位反转。

### C 移植代码 (wiener_filter.c)
```c
float ComputeOverallScalingFactor(...) {
    // ...
    float kBLim = 0.5f;
    float scale_factor1 = 1.f;
    if (gain > kBLim) {
        scale_factor1 = 1.f + 1.3f * (gain - kBLim);
        if (gain * scale_factor1 > 1.f) {
            scale_factor1 = 1.f / gain;
        }
    }
    float scale_factor2 = 1.f;
    if (gain < kBLim) {                                          // 只有 gain < 0.5 进入
        gain = max_local(gain, suppression_params_->minimum_attenuating_gain);
        scale_factor2 = 1.f - 0.3f * (kBLim - gain);
        return prior_speech_probability * scale_factor1 + ... * scale_factor2; // 正确的 return
    }
    return -1;                                                   // ← BUG: gain >= 0.5 时走这里
}
```

### WebRTC 原始代码 (wiener_filter.cc)
```cpp
if (gain < kBLim) {
    gain = std::max(gain, suppression_params_.minimum_attenuating_gain);
    scale_factor2 = 1.f - 0.3f * (kBLim - gain);
}
// 始终返回组合缩放因子（不论 gain 与 kBLim 的关系）
return prior_speech_probability * scale_factor1 + (1.f - prior_speech_probability) * scale_factor2;
```

### 影响
`gain >= 0.5` 时（语音存在、Wiener 滤波衰减较小），`gain_adjustment = -1`。在 `NoiseSuppressor.c:410-411` 中被直接乘到 `extended_frame` 上，导致低频段输出相位反转，产生断续感或音质劣化。

---

## 附注

- **`NoiseSuppressor_Process` 声明为 `void` 但有 3 处 return 了值**（line 365, 442, 455）。MSVC 不报错，但属未定义行为。
- `NoiseSuppressor_init` 中对 `upper_band_gains_heap_` 执行了 3 次 `memset`（line 218-220），属复制粘贴残留。
