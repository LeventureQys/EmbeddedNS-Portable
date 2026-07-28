Project Security Level:TopSecurity

项目涉密等级:绝密

# ENC_Webrtc — 基于 WebRTC 的嵌入式语音降噪引擎

> 从 WebRTC AudioProcessing 模块中剥离 Noise Suppressor，完成 C++ → 纯 C (C99) 移植，面向单片机及资源受限嵌入式设备部署。

---

## 项目简介

本项目将 WebRTC 中成熟的单通道频域降噪算法（基于统计信号模型 + Wiener 滤波）进行完整移植，使其能在无操作系统、无第三方库依赖的嵌入式平台上运行。算法核心流程：

```
原始音频 (48kHz)
    │
    ▼
三分频器 (ThreeBandFilterBank) ──→ 低频 / 中频 / 高频
    │
    ▼  (取低频 0~8kHz)
OLA 帧拼接 + 256-pt FFT
    │
    ├──→ 噪声估计 (Quantile Noise Estimator)
    ├──→ SNR 计算 (Prior / Post)
    ├──→ 语音概率估计 (多特征融合)
    │
    ▼
Wiener 滤波器 ──→ 频域增益
    │
    ▼
IFFT + OLA 重建 + 高频增益补偿
    │
    ▼
三分频合成 ──→ 降噪后音频 (48kHz)
```

### 降噪效果

未降噪：

![未降噪波形](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/企业微信截图_17400148087576.png)

降噪后：

![降噪后波形](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/20250220163058.png)

---

## 核心特性

| 特性 | 说明 |
|------|------|
| 纯 C 实现 | C99 兼容，无 C++ 依赖，适合 MCU 编译 |
| 零外部依赖 | 不依赖任何第三方库，仅需标准 C 库 (`math.h`, `string.h`, `stdlib.h`) |
| 48kHz 采样率 | 原生支持 48kHz 输入/输出，内部三分频处理 |
| 4 档降噪强度 | 6dB / 12dB / 18dB / 21dB 可选 |
| 低延迟 | 每帧处理 480 samples (10ms @ 48kHz) |
| 新分频器 | 自研稀疏 FIR 三分频器，性能优于原始实现 |

### 降噪强度参数

| 级别 | 最小衰减增益 | 过减因子 | 衰减量 |
|------|:-----------:|:-------:|:-----:|
| Level 0 | 0.5   | 1.0  | -6 dB  |
| Level 1 | 0.25  | 1.0  | -12 dB |
| Level 2 | 0.125 | 1.1  | -18 dB |
| Level 3 | 0.09  | 1.25 | -21 dB |

---

## 目录结构

```
EmbeddedNS-Portable/
├── Project/                  # 可运行的 Demo 工程 (CMake)
│   ├── ENC_Core/             #   核心降噪库源码 (纯C, 编译为动态库)
│   │   └── AudioProcess/     #     算法实现 + API 接口
│   ├── main.cpp              #   Demo 入口 (批量处理 WAV 文件)
│   └── CMakeLists.txt
├── PureC/                    # 纯 C 移植版源码 (独立编译)
│   └── AudioProcess/         #   完整算法模块
│       ├── Api/              #     对外 API (ENC_V.h / ENC_V.c)
│       ├── SplitFilter/      #     三分频器实现
│       └── base/             #     基础类型定义
├── Original/                 # C++ 原始满血版 (参考对照)
│   └── AudioProcessing/      #   WebRTC 原始 C++ 代码
├── SpilitFilter/             # 新分频器独立实现
├── Webrtc_Source/            # WebRTC 完整源码 (仅供参考)
└── Document/                 # 算法教学文档 + 流程图
    └── NoiseSuppressor_Teaching_Document.md
```

---

## API 接口

核心 API 定义于 `ENC_V.h`，仅两个函数：

```c
#include "Api/ENC_V.h"

// 初始化降噪处理器
// level: 降噪强度 (0~3)，对应 6dB/12dB/18dB/21dB
void* InitNSHandler(int level);

// 处理一帧 48kHz 音频
// handler: InitNSHandler 返回的句柄
// input:   480 个 float 样本 (10ms @ 48kHz, 单声道)
// output:  480 个 float 样本 (降噪后)
bool NS_Process_48kAudio(void* handler, float* input, float* output);
```

### 使用示例

```c
#include "Api/ENC_V.h"

// 1. 初始化 (选择 Level 3 = -21dB 最强降噪)
void* ns = InitNSHandler(3);

// 2. 逐帧处理 (每帧 480 samples = 10ms @ 48kHz)
float input_frame[480];
float output_frame[480];

while (has_audio_data()) {
    read_audio_frame(input_frame, 480);   // 读取 10ms 音频
    NS_Process_48kAudio(ns, input_frame, output_frame);  // 降噪
    write_audio_frame(output_frame, 480); // 输出
}

// 3. 释放
free(ns);
```

---

## 编译构建

### 环境要求

- CMake >= 3.10
- C99 兼容编译器 (GCC / MSVC / ARM Compiler 等)
- 无其他依赖

### 编译 Demo 工程 (Project/)

```bash
cd Project
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

编译产物：
- `ENC_Core.dll` / `ENC_Core.lib` — 降噪核心库
- `ENC_Demo.exe` — 演示程序（批量处理 WAV 文件）

### 编译纯 C 库 (PureC/)

```bash
cd PureC
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 嵌入式移植

直接将 `PureC/AudioProcess/` 目录下的所有 `.c` / `.h` 文件加入工程即可，无需 CMake：

1. 将 `AudioProcess/` 整个目录拷贝到目标工程
2. 在编译器中添加 include 路径指向 `AudioProcess/`
3. 编译所有 `.c` 文件
4. 调用 `InitNSHandler()` + `NS_Process_48kAudio()` 即可

---

## 算法模块说明

| 模块 | 文件 | 功能 |
|------|------|------|
| 三分频器 | `SplitFilter/` | 将 48kHz 信号分为 3 个 16kHz 子带 |
| FFT | `fft4g.c` | 256 点实数 FFT / IFFT |
| 噪声估计 | `noise_estimator.c` | 基于分位数的自适应噪声追踪 |
| 量化噪声估计 | `quantile_noise_estimator.c` | 分位数噪声底估计 |
| 语音概率 | `speech_probability_estimator.c` | 多特征融合 (LRT + 谱平坦 + 谱差异) |
| Wiener 滤波 | `wiener_filter.c` | 计算频域最优增益 |
| 信号模型 | `signal_model.c` | 先验/后验 SNR 建模 |
| 抑制参数 | `suppression_params.c` | 降噪强度配置 |
| 直方图 | `histograms.c` | 特征统计分布 |
| 快速数学 | `fast_math.c` | 嵌入式优化的数学函数 |

---

## 技术规格

| 参数 | 值 |
|------|----|
| 采样率 | 48000 Hz |
| 帧长 | 480 samples (10 ms) |
| FFT 长度 | 256 points |
| 子带数 | 3 (三分频) |
| 处理带宽 | 0 ~ 24000 Hz |
| 降噪带宽 | 0 ~ 8000 Hz (Wiener 滤波) |
| 声道 | 单声道 (Mono) |
| 数据格式 | float (内部) / int16 (I/O 可选) |
| 内存占用 | ~15 KB (估算，含所有状态) |

---

## 文档

详细的算法原理教学文档位于 `Document/` 目录：

- [NoiseSuppressor_Teaching_Document.md](Document/NoiseSuppressor_Teaching_Document.md) — 完整的算法解析，包含数据流图、公式推导、学习路线图

---

## 版本历史

| 日期 | 作者 | 内容 |
|------|------|------|
| 2025.2.2 | Venture | 完成 C++ 原始版本整理 (Original/) |
| 2025.2.2 | Wendy | 完成纯 C 语言移植 (PureC/) |
| 2025.2.18 | Venture | 添加 WebRTC 完整源码参考 |
| 2025.2.20 | Wendy | 实现新稀疏 FIR 分频器 (SpilitFilter/) |

---

## 许可证

本项目基于 WebRTC 源码，遵循其原始 BSD 许可证。详见 `Webrtc_Source/webrtc-audio-processing-1.0/COPYING`。
