# ENC_Webrtc — WebRTC 降噪算法 C 语言移植

从 WebRTC AudioProcessing 模块中提取 NoiseSuppressor，由 C++ 移植为纯 C，适用于嵌入式 MCU 及各类平台。

## 目录结构

| 目录 | 作者 | 说明 |
| :--- | :--- | :--- |
| `Project/` | Venture, Wendy | **主工程**：C 核心库 (`ENC_Core.dll`) + C++ 演示程序 (`ENC_Demo.exe`) |
| `PureC/` | Wendy | 独立 C 库构建（与 `Project/` 同步，无演示程序） |
| `Original/` | Venture | C++ 原始参考实现（不直接交付） |
| `SpilitFilter/` | Wendy | 新三分频滤波器 C 实现（WIP） |
| `Webrtc_Source/` | Venture | webrtc-audio-processing-1.0 完整源码（Meson 构建，仅作参考） |
| `Document/` | — | 降噪算法教学文档与流程图 |

## 效果展示

| 未降噪 | 降噪后 |
| :---: | :---: |
| ![](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/企业微信截图_17400148087576.png) | ![](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/20250220163058.png) |

## 编译

依赖：CMake 3.10+，Visual Studio 2022 (MSVC 17)。

```pwsh
# 主工程（演示程序 + 动态库）
cd Project
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
# 输出: build/bin/Release/ENC_Demo.exe, ENC_Core.dll

# 纯 C 库（独立构建）
cd PureC
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

零外部依赖，Demo 需要 C++17 编译器。

## 快速使用

```c
#include "ENC_V.h"

void* handler = InitNSHandler(3);                  // 降噪级别 0–3（0=关闭，3=最强）
float input[480], output[480];                     // 480 采样 = 10ms @ 48kHz
NS_Process_48kAudio(handler, input, output);       // 逐帧处理
FreeNSHandler(handler);
```

**关键约定**：音频采样为 `floatS16` 格式，即 `float` 值域 `[-32768.0, 32767.0]`，**非** `[-1, 1]` 归一化。

## 技术参数

- 采样率：48 kHz
- 帧长：480 samples（10 ms）
- 声道：单声道
- FFT：256 点 → 129 个频点
- 降噪流程：三分频 → 分析（噪声估计 + 语音概率）→ Wiener 滤波 → 合成
