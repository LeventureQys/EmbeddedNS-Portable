# NS 算法组件化与对比测试 — 开发计划（零上下文执行版）

> **本文件的定位**：这是**唯一执行依据**。任何执行者（人或 Agent）仅凭本文件即可完整开工，不需要阅读任何其他文档、不需要访问任何对话历史、不需要向他人询问。
>
> **读者**：负责 P0/T1–T9/P2/P4/P6 各任务书的执行者。
>
> **配套文件**（可选阅读，非必需）：
> - `ENC/Document/NS组件化与测试开发计划.md` — 决策过程与备选方案的存档（含 6 项决策的原始分析）
> - `ENC/Document/NS移植缺陷审查报告.md` — 算法级缺陷的审查报告（本文件第 2.5 节已内联其全部结论）
>
> **冻结状态**：所有决策已关闭，第 3 节结论不得在执行中更改。若发现决策确有错误，停止执行并上报，不得自行改选。

---

## 0. 如何使用本文件

1. **开工前**：读第 1～6 节，建立完整认知。这些是执行所需的事实、契约与格式定义。
2. **接收任务时**：直接读第 8 节对应的任务书。每份任务书自足，含依赖前置、输入、输出、步骤、验收标准、禁止事项、错误处理。
3. **交付前**：对照第 11 节的验收标准自检。
4. **遇到未定义情况**：查第 12 节"风险与降级"；仍无覆盖则停止并上报，不得自行发明方案。

**术语约定**：

| 术语 | 含义 |
|------|------|
| 基线（BASELINE） | 原样保留既有缺陷的构建变体，代表固件当前行为 |
| 优化（OPTIMIZED） | 打开 D1/D5/D6 修复开关的构建变体 |
| 变体（variant） | 上述二者之一，在文件中以 `baseline` / `optimized` 标识 |
| 冻结源 | `ENC/MainApp/source/driver/alg/dsp/enc/` 下的 29 个文件，只读 |
| 缺陷编号 | D1/D2/D3/D5/D6，定义见第 2.5 节 |

---

## 1. 背景与目标

### 1.1 背景

`ENC/` 目录是一个基于 JAT1821（Cortex-M4）的音频设备固件工程。其中 `ENC/MainApp/source/driver/alg/dsp/enc/` 是一份从 WebRTC 噪声抑制算法（`webrtc-audio-processing-1.0`，模块 `audio_processing/ns`）移植而来的降噪实现，改写为纯 C、使用 CMSIS-DSP 的实数 FFT 替代原版的 Ooura `rdft`，并按 24 bit 音频量纲调整了输出钳位。

这份实现目前**无法在 PC 上编译或调用**：它依赖 Keil/ARMCC 特有的段属性宏、厂商头文件、以及 CMSIS 静态库（`arm_cortexM4lf_math.lib`，12 MB，仅 ARM 目标可用）。同时，一份前置审查已在该实现中定位出若干缺陷（第 2.5 节内联了全部结论），但**这些结论尚未经过任何实测验证**——它们建立在代码比对与时序分析之上。

### 1.2 目标

1. 把上述算法**摘成独立、自足、可在 PC（Windows + MSVC）上编译与调用的纯 C 组件**，源码对嵌入式保持可编译性。
2. 用该组件跑通 `Test_Audio/AudioSample-48000hz/` 的全量测试音频，产出**改动前（基线）**与**优化后**两套可比对的测量数据。
3. 交付一份**图文并茂的 PowerPoint 测试报告**（34 页、22 张图），回答"前置审查的缺陷是否成立、影响多大、修复能改善多少"。

### 1.3 交付物

| 编号 | 交付物 | 路径 |
|------|--------|------|
| D1 | 独立算法组件（源码 + 平台适配层 + CMake） | `ENC/NS_Component/` |
| D2 | 构建产物：基线库、优化库、测试驱动 | `ENC/NS_Component/build/`、`ENC/Test/bin/` |
| D3 | 基线输出音频与指标 CSV | `ENC/Test/data/baseline/` |
| D4 | 优化后输出音频与指标 CSV | `ENC/Test/data/optimized/` |
| D5 | 分析、绘图与 PPT 生成脚本 | `ENC/Test/analysis/` |
| D6 | 图件素材（PNG，供 PPT 嵌入） | `ENC/Document/figures/` |
| D7 | **最终测试报告（PowerPoint）** | `ENC/Document/NS组件化与对比测试报告.pptx` |
| D8 | 渲染预览（PDF + 关键页 PNG） | `ENC/Document/preview/` |
| D9 | 本文件 | `ENC/Document/NS组件化与测试开发计划_零上下文版.md` |

### 1.4 明确不做的事

- 不修改 `ENC/MainApp/` 下的任何文件（固件源码、BSP、驱动、Keil 工程一律只读）。
- 不修改 `Webrtc_Source/`、`Project/`、`Original/`、`SpilitFilter/`、`PureC/`、`Test_Audio/`。
- 不删除或修改 `ENC/Document/` 下已有的审查报告、计划文档、图件与脚本。
- 不把组件的编译接入任何固件工程。
- 不在 PPT 中使用网络图片或在线素材。
- 不安装任何 pip / npm 包——沿用环境已具备的库（见第 2.8 节）。
- 不搜索或调用系统安装的 LibreOffice；渲染统一走 DSH 内置的 LibreOffice Kit。

---

## 2. 技术事实基础

本节是执行的事实依据，全部经实测核实，可直接采信，无需重新求证（复核亦无妨）。

### 2.1 算法是什么

48 kHz 单声道输入的频域维纳滤波噪声抑制器，处理链路：

```
输入 480 样本（10 ms）
  → 三分频滤波组（DCT 调制的多相 FIR）拆成 3 个子带，各 160 样本
  → 对子带 0（0–8 kHz）做：256 点加窗 FFT → 幅度谱 → 噪声谱估计
     → 语音概率估计 → 维纳增益 → IFFT → 重叠相加
  → 子带 1、2（8–16 kHz、16–24 kHz）按同一时域增益整体衰减
  → 三分频合成回 480 样本
```

关键常量（`ns_common.h` 与 `three_band_filter_bank.h`）：

| 常量 | 值 | 含义 |
|------|-----|------|
| `kNumBands` | 3 | 子带数 |
| `kFftSize` | 256 | FFT 点数 |
| `kFftSizeBy2Plus1` | 129 | 频点+1 |
| `kNsFrameSize` | 160 | 子带帧长 |
| `kOverlapSize` | 96 | 重叠长度 |
| `kFullBandSize` | 480 | 全带帧长 |
| `kSplitBandSize` | 160 | 子带帧长 |
| `kShortStartupPhaseBlocks` | 50 | 短启动期帧数 |
| `kLongStartupPhaseBlocks` | 200 | 长启动期帧数 |
| `kFeatureUpdateWindowSize` | 500 | 特征更新窗口 |
| `kHistogramSize` | 1000 | 直方图档数 |

### 2.2 源码位置与文件清单

**冻结源目录**：`ENC/MainApp/source/driver/alg/dsp/enc/`

**需搬运的文件：29 个**

```
enc.c  enc.h
fast_math.c  fast_math.h
histograms.c  histograms.h
noise_estimator.c  noise_estimator.h
noise_suppressor.c  noise_suppressor.h
ns_common.h
prior_signal_model.c  prior_signal_model.h
prior_signal_model_estimator.c  prior_signal_model_estimator.h
quantile_noise_estimator.c  quantile_noise_estimator.h
signal_model.c  signal_model.h
signal_model_estimator.c  signal_model_estimator.h
speech_probability_estimator.c  speech_probability_estimator.h
suppression_params.c  suppression_params.h
three_band_filter_bank.c  three_band_filter_bank.h
wiener_filter.c  wiener_filter.h
```

**不搬运**：`three_band_filter_bank - 副本.c`（496 行的调试分支，含 `#if 0` 死代码，属冗余项）。

> 源文件注释为中文，部分为 GBK 编码。搬运时统一转 UTF-8，**不得改动任何注释之外的字符**。

### 2.3 平台耦合面（完整清单）

对 29 个文件做全符号扫描后的精确结果。除此之外**没有任何隐藏依赖**。

**A. 需要提供的宏（4 个）**

| 宏 | 来源 | 语义 |
|----|------|------|
| `MIN(a,b)` | `ENC/MainApp/source/user/UserRetype.h:67` | `(((a) < (b)) ? (a) : (b))` |
| `MAX(a,b)` | 同上第 70 行 | `(((a) > (b)) ? (a) : (b))` |
| `ABS(x)` | `ENC/MainApp/source/driver/alg/dsp/Headers/marco.h:150` | `(((x) >= 0) ? (x) : (-(x)))` |
| `ARM_MATH_SUCCESS` | CMSIS | 枚举值，取 `0` |

**B. 需要提供的类型（4 个）**

| 类型 | 来源 | 处理 |
|------|------|------|
| `int32_t` | `<stdint.h>` | 直接包含标准头 |
| `bool` | `<stdbool.h>` | 直接包含标准头 |
| `uint8` | 厂商定义（`UserRetype.h`） | `typedef uint8_t uint8;` |
| `arm_status` | CMSIS | `typedef int arm_status;`（枚举语义，取 `0` 为成功） |

**C. 需要处理的函数属性宏（3 个）**

| 宏 | 出现位置 | 语义 |
|----|---------|------|
| `__RAM_CODE` | `fast_math.c` 全部 10 个函数定义、`enc.c:65` | 函数放到 RAM 执行（ARMCC 特性） |
| `__RAM3` | `enc.c:29-31` | 变量放到 RAM3 段 |
| `__ALIGNED(n)` | `enc.c:30,31,33`、`enc.h:43,44` | 按 n 字节对齐 |

**D. 必须移除的硬件相关代码（仅 `enc.c`）**

| 符号 | 出现位置 | 处理 |
|------|---------|------|
| `#include "a0_hal_gpio.h"`、`#include "Gpio.h"` | `enc.c:13-14` | 删除 |
| `HAL_GPIO_BIT_SET(GPIO0, GPIO_PIN_28)` | `enc.c:68` | 连同 `#if (ENC_GPIO_PROFILE_EN)` 块一起删除 |
| `HAL_GPIO_BIT_CLR(GPIO0, GPIO_PIN_28)` | `enc.c:79` | 同上 |
| `__breakpoint(0)` | `enc.c:48` | 改为返回错误码（见第 5.1 节） |
| `ENC_GPIO_PROFILE_EN` 宏定义 | `enc.c:19` | 删除 |

**E. 需要被替代的 CMSIS 符号（3 个 + 1 个类型）**

```c
arm_rfft_fast_instance_f32  g_InstRfftF32;        /* enc.c:26 全局实例 */
arm_rfft_fast_init_f32(&g_InstRfftF32, 256);      /* enc.c:46 初始化 */
arm_rfft_fast_f32(&g_InstRfftF32, in, out, flag); /* noise_suppressor.c:330, :387, :408 */
arm_status                                        /* noise_suppressor.c:302, :375 声明未用局部变量 */
```

注：`noise_suppressor.c:303` 与 `:376` 各声明了一个**从未使用**的局部 `arm_rfft_fast_instance_f32 InstRfftF32;`，以及 `:302`、`:375` 的 `arm_status status`。搬运时**保留原样**（属代码卫生项，不在本次修复范围；但兼容层必须能编译它们）。

**F. 内部函数**：其余全部符号（`NoiseSuppressor_*`、`WienerFilter_*`、`FilterCore` 等）都由算法自身定义，无需外部提供。

### 2.4 CMSIS FFT 的格式与缩放约定（最关键的技术约束）

这是替代实现必须严格复刻的行为。**弄错这一项，整个基线就失去意义。**

**正向变换** `arm_rfft_fast_f32(S, p, pOut, 0)`：

- 输入 `p`：256 个 float，表示 256 点时域实数序列，**函数会原地修改 p**
- 输出 `pOut`：256 个 float，压缩格式：

```
pOut[0]     = X[0].re      (直流)
pOut[1]     = X[128].re    (Nyquist，实部)
pOut[2k]    = X[k].re      (k = 1..127)
pOut[2k+1]  = X[k].im      (k = 1..127)
```

- **正向无缩放**，为标准 DFT：`X[k] = Σ_{n=0}^{255} x[n]·e^{-j2πkn/256}`

**逆向变换** `arm_rfft_fast_f32(S, p, pOut, 1)`：

- 输入 `p`：与正向输出**完全相同的压缩格式**
- 输出 `pOut`：256 个 float，**标准 IDFT 时域序列**

```
pOut[n] = (1/256)·Σ_{k=0}^{255} X[k]·e^{+j2πkn/256}
```

- **内部路径**（CMSIS-DSP 官方源码）：`merge_rfft_f32` 完成"压缩格式→128 点复数格式"的结构转换（含 0.5 因子），随后 `arm_cfft_f32(Sint, pOut, 1, 1)` 做 128 点复数逆变换，其末尾执行 `invL = 1.0f / L`（L = 128）缩放。

> **禁止用标量乘积来推导缩放**：`merge_rfft` 的 `0.5` 作用于格式转换而非整体增益，`0.5 × 1/128 = 1/256` 与 `2/256` 不是同一层面的量。**必须走完整条数学路径**。
>
> 正确判据：`riff(fft(x)) == x`（往返恒等）。这是唯一需要保证的性质。

**初始化** `arm_rfft_fast_init_f32(S, 256)`：仅需记载长度 256，返回 `ARM_MATH_SUCCESS`。

### 2.5 已知缺陷清单（本文件内联完整定义）

前置审查定位出 3 个缺陷与 3 项偏差。**本节的代码片段与行号即为判据，执行者无需查阅其他文档。**

#### D1（严重）未初始化栈变量污染 signal_energy

**位置**：`noise_suppressor.c:297`（声明）、`:330-332`（FFT 与回填）、`:340-344`（累加）；`NoiseSuppressor_Process` 内 `:370`、`:387-389` 同源。

**代码事实**：

```c
297:  float Y[2*kFftSizeBy2Plus1];        /* 258 个 float, 局部数组, 未初始化 */
...
330:  arm_rfft_fast_f32(&g_InstRfftF32, extended_frame, Y, 0);  /* 仅写入 Y[0..255] */
331:  Y[kFftSize]=Y[1];                    /* 把 Nyquist 搬到 Y[256] */
332:  Y[1]=0.0f;
...
340:  for(i=0; i<kFftSizeBy2Plus1; i++)   /* i = 0..128 */
341:  {
342:      signal_energy+=Y[2*i]*Y[2*i]+Y[2*i+1]*Y[2*i+1];
343:      signal_spectral_sum+=signal_spectrum[i];
344:  }
```

`i=128` 时读取 `Y[256]`（已赋值）与 **`Y[257]`（从未赋值）**。

**参考实现的正确做法**：WebRTC 原版在 FFT 后写 `imag[kFftSizeBy2Plus1 - 1] = 0;`（即 `imag[128] = 0`），使该累加项为 `real[128]² + 0`。

**性质**：读取未初始化自动变量是未定义行为。Cortex-M 上该位置通常保留上一次同栈深度调用的残留值，表现为**持续的确定性偏差**而非随机噪声。

**影响传导链**（这是选择测量指标的根据）：

```
signal_energy
  → SignalModelEstimator_AdjustNormalization  (signal_model_estimator.c:127-132)
       diff_normalization = (diff_normalization*n + signal_energy)/(n+1)   [n < 200 每帧]
  → signal_energy_sum += signal_energy        (signal_model_estimator.c:144)
       每 500 帧参与 diff_normalization 更新   (:158-159)
  → ComputeSpectralDiff 的归一化分母          (signal_model_estimator.c:86)
       return spectral_diff / (diff_normalization + 0.0001f)
  → features_.spectral_diff                   (signal_model_estimator.c:142)
  → indicator2                                (speech_probability_estimator.c:64-65)
  → prior_speech_prob                         (:68)
  → speech_probability                        (:76)
  → WienerFilter_Update 的 filter[]           (wiener_filter.c:34-56)
  → 降噪增益
```

**为什么在 24 bit 量纲下影响明显**：当前输入量纲为 ±8388608，256 点 FFT 的 DC 分量可达 2.1e9，平方和可达 4.6e18。该量级仍在 float 范围内（上限 3.4e38），因此 `Y[257]` 取到大值时**不会溢出为 inf**，而是被同量级地放大——症状是特征持续偏移，不是崩溃。

**修复方式**：在 `noise_suppressor.c:331` 与 `:388` 之后补一句 `Y[kFftSize + 1] = 0.0f;`。
**禁止**改用 `float Y[258] = {0};`——那会在每帧引入 1032 字节的 memset 开销。

#### D2（严重）中断触发丢失导致双缓冲错位

**位置**：`ENC/MainApp/source/app/app_i2s.c:465-469`（翻转与触发）、`:319-344`（USER2 处理）；优先级配置 `ENC/MainApp/source/app/app_dma.c:247` 与 `ENC/MainApp/source/driver/alg/dsp/drv_alg.c:554,558`。

**代码事实**：

```c
/* app_i2s.c */
465:  enc_write = !enc_write;              /* 无条件翻转 */
466:  if (flagUser2IrqHandler == 0)
468:      NVIC_SetPendingIRQ(USER2_IRQn);  /* 有条件触发 */
```

**机理**：`enc_write` 决定后续 480 个样本写哪一半缓冲、读哪一半结果；但"处理哪一半"由 USER2 中断按当时的 `enc_write` 值决定。翻转无条件、触发有条件，二者一旦脱钩，`enc_out` 的对应半边就不会更新。`USER0_IRQn` 优先级为 1（`app_dma.c:247`），高于 `USER2_IRQn` 的 2（`drv_alg.c:558`），故 USER0 可抢占 USER2；I2S 帧周期为 48 样本 = 1 ms，只要 `Enc_Process` 单次耗时超过 1 ms 即会触发。

**本次是否覆盖**：**不覆盖**。这是固件中断调度问题，PC 侧组件无中断上下文，无法复现。报告中必须显式声明。

#### D3（中等）ENC 关闭期间采集指针不递增

**位置**：`ENC/MainApp/source/driver/alg/dsp/drv_alg.c:669-677`。

```c
669:  fInAdcL = (float *)(pMem[2]);
670:  for(uint8_t i = 0; i<48; i++)
671:  {
672:      enc_in[adcl_in_i++] = *fInAdcL;
673:      if (EncOnOff)
674:      {
675:          *fInAdcL++ =  enc_out[adcl_out_i++];   /* 递增在条件分支内 */
676:      }
677:  }
```

**本次是否覆盖**：**不覆盖**。同 D2，属固件侧问题，PC 组件不重现。

#### D5（偏差）输出钳位量程

**位置**：`noise_suppressor.c:438-439`。

```c
438: //  split_data[band][i]=MIN(MAX(split_data[band][i], -32768.0f), 32767.0f);//16bit
439:     split_data[band][i]=MIN(MAX(split_data[band][i], -8388607.0f), 8388607.0f);//24bit
```

**判断**：当前固件输入量纲为 24 bit（见 2.7 节），因此 24 bit 钳位是**与数据约定匹配的正确选择**，不是错误。但它把量程写死在代码里，一旦 `I2S_INOUT_RESOLUTION_LEVEL` 改回 1（16 bit 量纲），钳位就失效。

**修复方式**：改为随分辨率宏派生的常量（见第 5.5 节 `ns_patch.h`）。

#### D6（偏差）数学近似精度

**位置**：`fast_math.c:40-45`（`fastsqrt`）、`:29-33`（`fastpow2`）、`:23-27`（`fastlog2` 偏置常数）。调用点：`noise_suppressor.c:134`、`wiener_filter.c:88`（`fastsqrt`）。

**实测误差**（以 Python 复现位运算后对比精确函数，30 万随机样本）：

| 函数 | 参考实现 | 本实现 | 最大相对误差 |
|------|---------|--------|-------------|
| 开方 | `sqrtf` 精确 | `fastsqrt`（位近似 + 一次牛顿迭代） | **0.096%** |
| 指数 | `powf(2,p)` 精确 | `fastpow2`（纯位近似） | **2.9%** |
| log2 偏置 | `126.942695` | `126.9420151` | 引入 +6.8e-4 的系统性平移 |

`fastexp` 链路两者误差同量级（本实现 1.4%–3.0%，参考实现 0.4%–4.9%），**不属于本次移植引入的新缺陷**。

**修复方式**：把 `fastsqrt` 的两处调用点改为 `sqrtf`（`fastsqrt` 本体保留，供消融对比）。

#### D7（提示）代码卫生

- `three_band_filter_bank.c:134-141` 内部的输出清零被注释掉，改由 `enc.c:72` 的 `memset` 承担——当前调用方唯一，逻辑正确，但形成隐式契约。
- `noise_suppressor.c:303`、`:376` 的未使用局部变量。
- `three_band_filter_bank - 副本.c` 冗余文件（不搬运）。

**是否设开关**：D7 **不单列开关**，作为搬运时的顺带处理（见 T2 任务书）。

### 2.6 测试数据规格

**路径**：`D:\workshop\Github\EmbeddedNS-Portable\Test_Audio\AudioSample-48000hz\`

| 文件 | 声道 | 位深 | 采样率 | 帧数 | 时长 |
|------|------|------|--------|------|------|
| `1.wav` | 2 | 16 bit | 48000 Hz | 893952 | 18.62 s |
| `2.wav` | 2 | 16 bit | 48000 Hz | 1010688 | 21.06 s |
| `3.wav` | 2 | 16 bit | 48000 Hz | 814080 | 16.96 s |
| `4.wav` | 2 | 16 bit | 48000 Hz | 858112 | 17.88 s |
| `5.wav` | 2 | 16 bit | 48000 Hz | 857088 | 17.86 s |
| `6.wav` | 2 | 16 bit | 48000 Hz | 2030592 | 42.30 s |
| `7.wav` | 2 | 16 bit | 48000 Hz | 942080 | 19.63 s |
| `8.wav` | 2 | 16 bit | 48000 Hz | 914432 | 19.05 s |
| `9.wav` | 2 | 16 bit | 48000 Hz | 819200 | 17.07 s |

合计 190.42 s（约 914 万采样点 × 2 声道），全部为未压缩 PCM。

**注意**：同目录下存在 `output_ns/` 子目录，内含 9 个同规格 WAV，是仓库内既有的参考输出（由另一条移植路径 `Project/ENC_Core` 生成）。**处理主目录时须跳过该子目录**；`output_ns/` 仅可用于 T6 的形态对比，**不作为本次基线**。

### 2.7 量纲约定

固件侧送进算法的浮点值量纲为 **24 bit**：

```
ENC/MainApp/source/driver/alg/dsp/drv_alg.c:46
    #define I2S_INOUT_RESOLUTION_LEVEL          (2)//(1)

ENC/MainApp/source/driver/alg/dsp/drv_alg.c:646
    *fInAdcL++ = (float)(*i32InAdcL++ >> 8);
```

即 `float` 值的有效范围为 ±8388608，与 `noise_suppressor.c:439` 的钳位 ±8388607 一致。

测试 WAV 是 16 bit（±32768）。**本次测试的换算规则**：

```
输入端： sample24 = (float)(int16_sample) * 256.0f      /* 等价于左移 8 位 */
输出端： out16 = clamp(round(out24 / 256.0f), -32768, 32767)
```

> 此规则为已冻结决策（第 3 节 Q3）。若固件实际出厂的 `I2S_INOUT_RESOLUTION_LEVEL` 与此不同，基线需重跑；该假设必须写入报告第 7 页与第 34 页。

### 2.8 环境与工具链

| 项目 | 状态 | 位置/版本 |
|------|------|----------|
| CMake | 可用 | `C:\Program Files\CMake\bin\cmake.exe` |
| Visual Studio 2022 | 可用 | Community 版 |
| Python | 可用 | `C:\Users\lenovo\.dsh\dsh-runtimes\dsh-primary-runtime\dependencies\python\python.exe` |
| numpy | 可用 | 2.3.5 |
| Pillow | 可用 | 12.3.0 |
| python-pptx | 可用 | 1.2.0 |
| **matplotlib** | **不可用** | 全部绘图必须用 Pillow |
| 中文字体 | 可用 | `C:/Windows/Fonts/msyh.ttc`、`msyhbd.ttc`、`simhei.ttf` |
| PPT 结构校验 | 可用 | `<python> D:\Devtools\DSH\resources\runtime\office-skills\scripts\check_office.py` |
| PPT 渲染 | 可用 | node: `D:\Devtools\DSH\resources\runtime\primary-runtime\dependencies\node\bin\node.exe`<br>cli: `D:\Devtools\DSH\resources\app.asar.unpacked\dsh\node_modules\@deepseek-ai\libreoffice-kit\lib\cli.js` |
| 图像输入能力 | **不可用** | 执行模型不能读图，PPT 视觉版式无法自动检查（见第 12 节） |

---

## 3. 已冻结的决策

**执行中不得更改。若发现决策有误，停止并上报。**

| 编号 | 议题 | 冻结结论 | 对执行的约束 |
|------|------|---------|-------------|
| **Q1** | 优化范围 | 打开 **D1 + D5 + D6** 三项；D7 顺带处理不设开关 | 消融实验为 3 维，共 4 个构建：`BASELINE`、`+D1`、`+D1D5`、`+D1D5D6` |
| **Q2** | 组件形态 | `ENC/NS_Component/` 下纯 C99 静态库 + CMake | 不生成 Keil 工程，不并入 `Project/`；源码须对 ARMCC 保持可编译 |
| **Q3** | 量纲策略 | 输入左移 8 位对齐 24 bit，输出右移 8 位还原 | 见 2.7 节换算规则；量纲假设写入报告 |
| **Q4** | 声道处理 | 取左声道，输出单声道 | 不做通道聚合；不做 L/R 独立对比 |
| **Q5** | 原版基准 | 不投入关键路径；用 `Project/ENC_Core` 交叉参照 + numpy 公式级复算 | 原版 C++ 编译列为可降级探索（T6）；失败不阻塞 P4 |
| **Q6** | 降噪等级 | 主测 level 3，补测 level 0 与 level 1 | 每个等级在基线与优化各跑一轮，等级维度共 6 轮 |

**遗留假设**（写入报告第 7 页与第 34 页）：

1. Q3 的量纲结论基于 `drv_alg.c:46` 的当前值 `(2)`。若出厂固件使用其他值，基线需按 16 bit 重跑。
2. 所有 PC 侧结论受限于"无中断上下文"，D2/D3 不在覆盖范围。

---

## 4. 目标架构

### 4.1 完整目录树

```text
ENC/
├── NS_Component/                       # 新建：独立算法组件
│   ├── api/
│   │   └── ns_api.h                    # 公共 API（与固件 ENC_V.h 同形 + 诊断扩展）
│   ├── src/                            # 从冻结源摘出的算法源码（去平台化）
│   │   ├── <29 个文件的搬运结果，见 2.2>
│   │   ├── ns_patch.h                  # 本组件特有：D1/D5/D6 条件化修复
│   │   └── ns_glue.c / ns_glue.h       # 由 enc.c 改写：去 GPIO、去全局变量外露
│   ├── port/
│   │   ├── arm_math_compat.h           # CMSIS 三符号的替代声明
│   │   ├── arm_math_compat.c           # 基于标准 radix-2 FFT 的实现
│   │   ├── arm_math_compat_test.c      # 兼容层自测（独立可执行）
│   │   ├── platform.h                  # __RAM_CODE / __RAM3 / __ALIGNED
│   │   └── vendor_types.h              # MIN/MAX/ABS/uint8/arm_status 的最小桩
│   ├── CMakeLists.txt
│   └── README.md                       # 组件说明 + 摘取对照表
├── Test/                               # 新建：测试工程
│   ├── harness/
│   │   ├── wav_io.c / wav_io.h         # WAV 读写
│   │   ├── ns_runner.c                 # 主程序：遍历、分帧、调用、导出
│   │   └── CMakeLists.txt
│   ├── analysis/                       # Python 分析与出料
│   │   ├── common.py                   # WAV/CSV 读取与指标计算
│   │   ├── fig_style.py                # PPT 版式规范：配色、字号、画布
│   │   ├── fig_waveform.py             # FigT03/T04
│   │   ├── fig_spectrogram.py          # FigT05/T06/T14
│   │   ├── fig_trace.py                # FigT07/T09/T10/T11/T12/T16
│   │   ├── fig_summary.py              # FigT01/T08/T13/T15/T17/T18/T19/T21/T22
│   │   ├── fig_reference.py            # FigT20（依赖 T6）
│   │   ├── build_deck.py               # 组装 .pptx
│   │   ├── check_deck.py               # 调用 check_office + 渲染预览
│   │   └── run_all.py                  # 一键：出图 → 出 PPT → 校验
│   ├── data/                           # 生成物：输出音频与 CSV
│   └── bin/                            # 生成物：可执行文件
└── Document/                           # 交付文档（figures/、scripts/ 已存在）
    ├── NS组件化与测试开发计划_零上下文版.md    # 本文件
    ├── NS组件化与测试开发计划.md               # 决策存档（既有）
    ├── NS移植缺陷审查报告.md                   # 前置审查（既有）
    ├── NS组件化与对比测试报告.pptx             # 最终交付
    ├── preview/                               # 渲染预览
    └── figures/                               # 图件素材
```

### 4.2 数据流

```text
原始 WAV (16bit/2ch/48kHz)
  → 取左声道
  → ×256 对齐 24bit 量纲
  → 480 样本/帧切分
  → ns_core_<variant>(level)
  → ÷256 还原 + 钳位
  → 输出 WAV (16bit/1ch/48kHz) + 逐帧 metrics CSV
  → Python 分析 → figures/*.png
  → python-pptx → .pptx
  → check_office.py + LibreOffice → checks.json + preview/
```

**基线与优化版共用同一 harness、同一输入**，唯一变量是编译期开关。

---

## 5. 接口契约

以下头文件内容为**规定实现**，执行者可直接照抄创建。

### 5.1 `api/ns_api.h`

```c
#ifndef NS_API_H
#define NS_API_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && defined(NS_BUILD_DLL)
#define NS_API __declspec(dllexport)
#elif defined(_WIN32) && defined(NS_USE_DLL)
#define NS_API __declspec(dllimport)
#else
#define NS_API
#endif

/* 错误码 */
typedef enum {
    NS_OK               =  0,
    NS_ERR_NULL_HANDLE  = -1,
    NS_ERR_NULL_BUFFER  = -2,
    NS_ERR_BAD_LEVEL    = -3,
    NS_ERR_FFT_INIT     = -4,
    NS_ERR_BAD_HANDLE   = -5
} NsStatus;

/* 逐帧诊断量。全部为"最近一次 NS_Process48k 调用后"的快照。 */
typedef struct {
    int32_t num_analyzed_frames;   /* 已分析帧计数, 初值 -1, 首帧后为 0 */
    float   signal_energy;         /* 最近一帧的 signal_energy (含 D1 效应) */
    float   signal_spectral_sum;   /* 最近一帧的幅度谱和 */
    float   diff_normalization;    /* 当前 diff_normalization */
    float   speech_prob_mean;      /* 最近一帧 speech_probability[0..128] 的均值 */
    float   prior_speech_prob;     /* 最近一帧的 prior_speech_prob */
    float   gain_adjustment;       /* 最近一帧的 gain_adjustment */
    int32_t clamp_hits;            /* 本帧触达钳位边界的样本数 */
    int32_t zero_frame_skips;      /* 累计因零帧跳过的 Analyze 次数 */
} NsDiagnostics;

typedef struct NSHandler NSHandler;

/* 创建处理器。level ∈ [0,3]，对应 suppression_params.c 的四档参数。
   失败返回 NULL。 */
NS_API NSHandler* NS_Init(int level);

/* 释放处理器。传 NULL 安全（无操作）。 */
NS_API void NS_Free(NSHandler* h);

/* 处理一帧 480 样本。
   in/out: 长度 480 的 float 数组, 量纲为 24bit (±8388608), 可以原位(in == out)。
   返回 NsStatus。 */
NS_API int NS_Process48k(NSHandler* h, const float* in, float* out);

/* 读取诊断量快照。d 由调用方提供。 */
NS_API int NS_GetDiagnostics(const NSHandler* h, NsDiagnostics* d);

/* 编译期信息查询（用于 build_info.txt 与报告） */
NS_API const char* NS_GetVariantName(void);   /* "BASELINE" 或 "OPTIMIZED" */
NS_API int         NS_GetPatchFlags(void);    /* bit0=D1 bit1=D5 bit2=D6 */

#ifdef __cplusplus
}
#endif
#endif /* NS_API_H */
```

**生命周期**：`NS_Init` → N 次 `NS_Process48k` → `NS_Free`。处理器内部保存跨帧状态（噪声谱、滤波器、三分频器状态），**不得在处理中途重新初始化**。

**错误处理契约**：

| 情况 | 行为 |
|------|------|
| `NS_Process48k(NULL, ...)` | 返回 `NS_ERR_NULL_HANDLE`，不写 `out` |
| `in == NULL \|\| out == NULL` | 返回 `NS_ERR_NULL_BUFFER`，不写 `out` |
| `NS_Init` 传入 `level ∉ [0,3]` | 返回 NULL（不 fallback） |
| FFT 初始化失败 | `NS_Init` 返回 NULL（原 `__breakpoint(0)` 的替代） |
| `NS_GetDiagnostics` 传 NULL 句柄 | 返回 `NS_ERR_NULL_HANDLE`，不写 `d` |

### 5.2 `port/arm_math_compat.h`

```c
#ifndef ARM_MATH_COMPAT_H
#define ARM_MATH_COMPAT_H

typedef int arm_status;
#define ARM_MATH_SUCCESS 0

/* 与原版 CMSIS 结构体的字段名保持一致的占位定义。
   兼容层内部不使用这些字段, 但源码会声明同类型的局部变量。 */
typedef struct {
    void*   Sint;            /* 占位 */
    uint16_t fftLenRFFT;
    float*  pTwiddleRFFT;
} arm_rfft_fast_instance_f32;

arm_status arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32* S, uint16_t fftLen);

/* ifftFlag: 0 = 正向, 1 = 逆向。
   输入输出格式与缩放约定见本文件第 2.4 节 (计划文档) —— 必须严格遵守。 */
void arm_rfft_fast_f32(arm_rfft_fast_instance_f32* S, float* p, float* pOut,
                       uint8_t ifftFlag);

#endif
```

### 5.3 `port/platform.h`

```c
#ifndef NS_PLATFORM_H
#define NS_PLATFORM_H

/* 嵌入式段属性宏在 PC 上退化为空 */
#ifndef __RAM_CODE
#define __RAM_CODE
#endif
#ifndef __RAM3
#define __RAM3
#endif

/* 对齐宏: 同时兼容 MSVC 与 GCC/ARMCC */
#ifndef __ALIGNED
#if defined(_MSC_VER)
#define __ALIGNED(n) __declspec(align(n))
#else
#define __ALIGNED(n) __attribute__((aligned(n)))
#endif
#endif

#endif
```

> **注意**：`__ALIGNED` 在源码中的用法是**后缀**形式，如 `float buf[960] __ALIGNED(4);`。MSVC 的 `__declspec(align(n))` 只支持**前缀**形式，因此 MSVC 下需把 `platform.h` 的宏定义为空，或用 `#define __ALIGNED(n)` 并在需要时改用 `#pragma pack`。**推荐做法：MSVC 下定义为空**（PC 上无 DMA 对齐要求）。若某处确实需要，单独处理。

### 5.4 `port/vendor_types.h`

```c
#ifndef NS_VENDOR_TYPES_H
#define NS_VENDOR_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

typedef uint8_t uint8;

#ifndef MIN
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(x)      (((x) >= 0) ? (x) : (-(x)))
#endif

#endif
```

> 三个宏与厂商头文件中的定义**逐字一致**（来源见 2.3 节表）。`ABS` 注意其参数为浮点时也成立。

### 5.5 `src/ns_patch.h`

所有修复的集中开关。基线构建下三个开关全为 0，行为与固件逐位一致。

```c
#ifndef NS_PATCH_H
#define NS_PATCH_H

/* 由 CMake 注入, 默认全关 = 基线 */
#ifndef NS_PATCH_D1
#define NS_PATCH_D1 0
#endif
#ifndef NS_PATCH_D5
#define NS_PATCH_D5 0
#endif
#ifndef NS_PATCH_D6
#define NS_PATCH_D6 0
#endif

/* D1: 未初始化 Y[kFftSize+1] 参与 signal_energy 累加
   打开后, 在 FFT 回填 Nyquist 的同时把虚部显式置零。 */
#if NS_PATCH_D1
    #define NS_FIX_D1_AFTER_FFT(Y)   do { (Y)[kFftSize + 1] = 0.0f; } while (0)
#else
    #define NS_FIX_D1_AFTER_FFT(Y)   do { } while (0)
#endif

/* D5: 输出钳位量程写死为 24bit
   打开后, 改用随 ENC_SAMPLE_DEPTH 派生的常量。 */
#if NS_PATCH_D5
    #if (ENC_SAMPLE_DEPTH == 16)
        #define NS_CLAMP_MAX   32767.0f
        #define NS_CLAMP_MIN  -32768.0f
    #else
        #define NS_CLAMP_MAX   8388607.0f
        #define NS_CLAMP_MIN  -8388608.0f
    #endif
#else
    #define NS_CLAMP_MAX   8388607.0f
    #define NS_CLAMP_MIN  -8388607.0f
#endif

/* D6: 幅度谱开方使用位近似
   打开后, 改用精确 sqrtf。fastsqrt 本体保留, 供消融对比。 */
#if NS_PATCH_D6
    #define NS_SQRT(x)   sqrtf(x)
#else
    #define NS_SQRT(x)   fastsqrt(x)
#endif

#endif /* NS_PATCH_H */
```

**使用约定**：

- `noise_suppressor.c:331` 与 `:388` 之后插入 `NS_FIX_D1_AFTER_FFT(Y);`
- `noise_suppressor.c:439` 的钳位改为 `MIN(MAX(split_data[band][i], NS_CLAMP_MIN), NS_CLAMP_MAX)`
- `noise_suppressor.c:134` 与 `wiener_filter.c:88` 的 `fastsqrt(...)` 改为 `NS_SQRT(...)`
- `ENC_SAMPLE_DEPTH` 由 CMake 定义（本次测试固定为 `24`）

### 5.6 变更点汇总（T2 搬运时必须精确改动的位置）

除下列位置外，29 个文件的内容**不得有任何改动**。

| 文件 | 位置 | 改动 |
|------|------|------|
| 全部 `.h` | `#include "UserFunction.h"`、`"UserRetype.h"`、`"marco.h"` | 删除，改 `#include "vendor_types.h"` |
| `noise_suppressor.c` | `#include "arm_math.h"` (第 11 行) | 改 `#include "arm_math_compat.h"` |
| `enc.c` | 第 10 行 `#include "arm_math.h"` | 改 `#include "arm_math_compat.h"` |
| `enc.c` | 第 13-19 行（GPIO 头与宏） | 删除 |
| `enc.c` | 第 48 行 `__breakpoint(0)` | 改为 `return NULL;` 语义（搬入 `ns_glue.c` 时处理） |
| `enc.c` | 第 67-69、78-80 行（GPIO 打点） | 删除 |
| `enc.c` | 第 29-35 行（全局变量） | 迁入 handler 结构体（见 T2 任务书） |
| 全部文件 | `#include "ns_patch.h"` | 在需要用到 `NS_FIX_*`/`NS_SQRT` 的文件中新增（仅 `noise_suppressor.c`、`wiener_filter.c`） |

---

## 6. 数据格式契约

### 6.1 输入输出 WAV

| 项目 | 输入 | 输出 |
|------|------|------|
| 路径 | `Test_Audio/AudioSample-48000hz/*.wav` | `ENC/Test/data/<variant>/<level>/<name>.wav` |
| 声道 | 2 | 1 |
| 位深 | 16 bit PCM | 16 bit PCM |
| 采样率 | 48000 | 48000 |
| 长度 | N 帧 | 与输入同长（末帧补零后裁剪回原长） |

### 6.2 逐帧指标 CSV

**路径**：`ENC/Test/data/<variant>/<level>/metrics_<name>.csv`

**列定义**（顺序固定，表头必须逐字一致）：

```csv
frame_idx,in_rms,out_rms,atten_db,signal_energy,signal_spectral_sum,diff_normalization,speech_prob_mean,prior_speech_prob,gain_adjustment,clamp_hits,elapsed_us
```

| 列 | 类型 | 说明 |
|----|------|------|
| `frame_idx` | int | 帧序号，从 0 开始 |
| `in_rms` | float | 本帧输入 RMS（24bit 量纲） |
| `out_rms` | float | 本帧输出 RMS（24bit 量纲） |
| `atten_db` | float | `20*log10(in_rms / out_rms)`，`out_rms==0` 时记为 `999.0` |
| `signal_energy` | float | 诊断量，见 `NsDiagnostics` |
| `signal_spectral_sum` | float | 同上 |
| `diff_normalization` | float | 同上 |
| `speech_prob_mean` | float | 同上 |
| `prior_speech_prob` | float | 同上 |
| `gain_adjustment` | float | 同上 |
| `clamp_hits` | int | 本帧触达钳位边界的样本数 |
| `elapsed_us` | int | 本帧 `NS_Process48k` 耗时（微秒） |

数值格式：浮点用 `%.9g`，整数用 `%d`。**禁止写 NaN 或 Inf**——若出现，视为缺陷并上报。

### 6.3 汇总 CSV

**路径**：`ENC/Test/data/<variant>/<level>/summary_<variant>_<level>.csv`

```csv
file,duration_s,frames,total_elapsed_ms,in_rms,out_rms,atten_db,in_peak,out_peak,clamp_hits_total
```

`atten_db` 为整段（全帧）的 `20*log10(in_rms_total / out_rms_total)`。

### 6.4 构建指纹

**路径**：`ENC/Test/data/<variant>/build_info.txt`

```text
variant         = BASELINE            # 或 OPTIMIZED
patch_flags     = 0                   # bit0=D1 bit1=D5 bit2=D6
sample_depth    = 24
cmake_generator = Visual Studio 17 2022
build_type      = Release
compiler        = MSVC 19.xx.xxxxx
c_flags         = /O2 /W3
git_commit      = <若可用, 否则填 N/A>
build_time      = 2026-xx-xx xx:xx:xx
```

### 6.5 文件命名

| 模式 | 用途 |
|------|------|
| `<variant>` | `baseline` 或 `optimized` |
| `<level>` | `L0` / `L1` / `L3` |
| `data/<variant>/<level>/` | 一级为变体，二级为等级 |
| `metrics_<name>.csv` | `<name>` 为源文件名去扩展名（如 `1`） |
| `figures/FigT<NN>_<中文简述>.png` | 图件，两位数字，从 01 起 |

---

## 7. 任务分解

### 7.1 任务总表

| 任务 | 名称 | 依赖前置 | 可并行 | 关键路径 |
|------|------|---------|--------|---------|
| **P0** | 骨架与契约 | 无 | — | ✅ |
| **T1** | CMSIS 兼容层 | P0 | T2/T3/T4 | ✅ |
| **T2** | 算法源码摘取 | P0 | T1/T3/T4 | ✅ |
| **T3** | 测试 harness | P0 | T1/T2/T4 | ✅ |
| **T4** | 分析绘图管线 | P0 | T1/T2/T3 | ❌ |
| **P2** | 基线测量 | T1+T2+T3 | — | ✅ |
| **T5** | 修复 / 优化版 | P2 | T6 | ✅ |
| **T6** | 交叉参照构建 | P2 | T5 | ❌ |
| **P4** | 优化后测量 | T5 | — | ✅ |
| **T7** | PPT 素材图件 | P4+T4 | T8 | ❌ |
| **T8** | 演示文稿组装 | P4(+T7) | T7 | ✅ |
| **T9** | 结构校验与预览 | T8 | — | ✅ |
| **P6** | 验收与交付 | T9 | — | ✅ |

**关键路径**：P0 → T1/T2/T3 → P2 → T5 → P4 → T8 → T9 → P6（8 跳）。

**最大并行度**：Phase 1 四路（T1–T4）；Phase 3 两路（T5、T6）；Phase 5 两路（T7 出素材图时 T8 可先行搭版式）。

![FigP01 并行任务依赖图](figures/FigP01_并行任务依赖图.png)

*图：13 个任务节点与依赖关系。绿色为可并行/校验节点，蓝色为串行节点，橙色为测量节点，红色为修复工作包，灰色为可降级节点。*

---

## 8. 任务书

每份任务书自足。执行者只读自己这一份即可开工。

---

### P0 — 骨架与契约

- **所属阶段**：Phase 0（必须最先完成）
- **依赖前置**：**无，可立即开工**
- **并行状态**：串行节点，独占
- **输入**：本文件第 4、5、6 节
- **产出**：
  1. `ENC/NS_Component/` 下按 4.1 节创建全部目录与**占位文件**（头文件写完整内容，`.c` 可先空实现）
  2. `ENC/Test/harness/`、`ENC/Test/analysis/`、`ENC/Test/data/`、`ENC/Test/bin/` 目录
  3. `NS_Component/CMakeLists.txt`：定义 4 个 target
     - `ns_core_BASELINE`（静态库，`NS_PATCH_D1=0, D5=0, D6=0`）
     - `ns_core_OPTIMIZED`（静态库，`NS_PATCH_D1=1, D5=1, D6=1`）
     - `ns_core_D1`（静态库，仅 `D1=1`）
     - `ns_core_D1D5`（静态库，`D1=1, D5=1`）
  4. `Test/harness/CMakeLists.txt`
  5. `NS_Component/README.md`：摘取对照表骨架
- **步骤**：
  1. 创建目录树（`New-Item -ItemType Directory` 或等效）
  2. 写 5 个接口头文件（内容见第 5 节，逐字照抄）
  3. 写 CMakeLists：`project(ns_core C)`，`set(CMAKE_C_STANDARD 99)`，为每个 target 用 `target_compile_definitions` 注入开关与 `ENC_SAMPLE_DEPTH=24`
  4. 每个 target 的 `target_include_directories` 需含 `api/`、`src/`、`port/`
  5. 配置并空编译验证
- **验收标准**：
  - `cmake -S ENC/NS_Component -B ENC/NS_Component/build -G "Visual Studio 17 2022"` 配置成功
  - `cmake --build ENC/NS_Component/build --config Release` 四个 target 全部通过（此时 `src/*.c` 尚为空或占位）
- **禁止事项**：此时不得搬运算法代码；不得修改冻结源目录。

---

### T1 — CMSIS 兼容层

- **所属阶段**：Phase 1
- **依赖前置**：P0
- **并行状态**：可与 T2/T3/T4 完全并行，无相互依赖
- **输入**：本文件第 2.4 节（格式与缩放约定）、第 5.2 节（头文件）
- **产出**：
  - `port/arm_math_compat.c`
  - `port/arm_math_compat_test.c`（独立自测程序，含 `main`）
- **实现要求**：
  1. `arm_rfft_fast_init_f32(S, 256)`：记载长度，返回 `ARM_MATH_SUCCESS`；长度非 256 时返回非零
  2. `arm_rfft_fast_f32(..., 0)`：256 点时域 → 压缩频域格式（见 2.4 节），**无缩放**
  3. `arm_rfft_fast_f32(..., 1)`：压缩频域格式 → 256 点时域，**标准 IDFT（含 1/256）**
  4. 实现建议：MSVC 下用 `double` 精度的 radix-2 FFT（先实现 256 点复数 FFT），再在其上构造实数变换。256 点性能压力极小，**优先保证正确性**
  5. **不得引入第三方 FFT 库**；不得复制 CMSIS 的任何源文件
- **自测（必须实际运行并留存输出）**：
  1. **往返测试**：1000 组伪随机输入（固定种子 12345）→ 正变换 → 逆变换，与输入逐点比对，**最大绝对误差 < 1e-6**
  2. **边界测试 A**：`x[n] ≡ 1` → 验证 `pOut[0] == 256.0`、`pOut[1] == 0.0`（Nyquist 为 0）、其余为 0；逆变换还原为全 1
  3. **边界测试 B**：`x[n] = δ[n]`（仅 x[0]=1）→ 验证 `pOut[0] == 1.0`、`pOut[1] == 1.0`（Nyquist 为 1）、其余实部为 1、虚部为 0；逆变换还原为 δ
  4. **numpy 交叉验证**：同一组随机输入，C 实现输出与 `numpy.fft.rfft` 的 129 个复数值比对，**最大相对误差 < 1e-5**
- **验收标准**：四项自测全部通过；自测输出保存至 `port/arm_math_compat_test_output.txt`
- **禁止事项**：
  - **不得改变缩放约定**。唯一判据是 `riff(fft(x)) == x`，不得用标量因子乘积推导
  - 不得为通过测试而放宽误差阈值
  - 不得修改 `arm_math_compat.h` 的签名
- **错误处理**：若某项自测无法通过，**停止并上报**，不得继续 P2。

> **本任务是整个基线可信度的支点。** 若兼容层行为偏离真实 CMSIS，基线就不再代表固件行为，后续所有对比失效。

---

### T2 — 算法源码摘取

- **所属阶段**：Phase 1
- **依赖前置**：P0
- **并行状态**：可与 T1/T3/T4 完全并行
- **输入**：本文件第 2.2 节（文件清单）、第 2.3 节（耦合面）、第 5.6 节（变更点汇总）
- **产出**：
  - `src/` 下 29 个文件的搬运结果
  - `src/ns_glue.c` / `src/ns_glue.h`
  - `api/ns_api.h` 的实现（可放在 `ns_glue.c` 或单独 `src/ns_api.c`）
- **搬运规则（逐条执行，不得即兴发挥）**：
  1. 从冻结源目录拷贝 29 个文件到 `src/`，**GBK → UTF-8 转码**
  2. 按 5.6 节表格逐处修改 include 与平台宏
  3. **不改动任何算式、常量、循环边界、数组下标、函数签名**
  4. 不搬运 `three_band_filter_bank - 副本.c`
  5. 在 `noise_suppressor.c` 与 `wiener_filter.c` 中 `#include "ns_patch.h"`，并按 5.5 节的"使用约定"插入三个宏调用点（**注意：宏在基线构建下展开为空，行为不变**）
- **`ns_glue.c` 的改写要求**（由 `enc.c` 改写）：
  1. 删除全部 GPIO 相关代码（见 2.3 节表 D）
  2. 把 `enc.c:29-35` 的全局变量收进 `struct NSHandler`：
     ```c
     struct NSHandler {
         Enc                   enc;            /* 含三分频器与降噪器状态 */
         float                 split_data[kNumBands][kSplitBandSize];
         float*                row_pointers[kNumBands];
         int32_t               level;
         NsDiagnostics         diag;           /* 诊断量快照 */
         /* 以下为诊断量累加所需的临时量, 每帧覆盖 */
     };
     ```
  3. 实现 `NS_Init`：等价于原 `Enc_Init`，FFT 初始化失败时 `free` 并返回 `NULL`
  4. 实现 `NS_Process48k`：等价于原 `Enc_Process`，并在其中采集诊断量（见 T3 任务书的采集点清单）
  5. 实现 `NS_GetDiagnostics` / `NS_GetVariantName` / `NS_GetPatchFlags`
  6. 保留 `memset(g_split_data, 0, ...)` 的清零（原 `enc.c:72` 的必要步骤）
- **诊断量采集点**（需在 `ns_glue.c` 中读取 `Enc` 内部状态，或加只读访问器到 `noise_suppressor.h`）：

  | 字段 | 来源 |
  |------|------|
  | `num_analyzed_frames` | `noise_suppressor->num_analyzed_frames` |
  | `signal_energy` | 需在 `NoiseSuppressor_Analyze` 内导出（新增一个 `float last_signal_energy` 成员并在 `:345` 后赋值） |
  | `signal_spectral_sum` | 同上 |
  | `diff_normalization` | `speech_probability_estimator.signal_model_estimator.diff_normalization` |
  | `speech_prob_mean` | 对 `speech_probability_estimator.speech_probability[0..128]` 求均值 |
  | `prior_speech_prob` | `speech_probability_estimator.prior_speech_prob` |
  | `gain_adjustment` | `NoiseSuppressor_Process` 内的局部变量，需导出为成员 |
  | `clamp_hits` | 在 `noise_suppressor.c:434-442` 的钳位循环中计数 |
  | `zero_frame_skips` | 在 `:312` 的 `if (zero_frame) return;` 处递增 |

  > **约束**：为导出这些量而新增的结构体成员与赋值语句属于"观测插桩"，是允许的；但**不得改变任何既有计算**。所有插桩点的改动要记录在 `README.md` 的对照表中。
- **验收标准**：
  - 四个库 target 全部编译通过，**无警告**（`/W3`）
  - `NS_Process48k` 对 480 个全零样本能正常返回 `NS_OK`，不崩溃
  - `README.md` 的摘取对照表覆盖全部 29 个源文件（原文件 ↔ 新文件 ↔ 改动说明）
  - 用 `dumpbin /symbols`（或等效）确认 `ns_core_BASELINE` 与 `ns_core_OPTIMIZED` 导出符号集一致
- **禁止事项**：
  - **不得在此任务中修 D1/D5/D6 的算法逻辑**——修复一律走 `ns_patch.h` 开关
  - 不得改动 `#include "ns_patch.h"` 之外的任何头文件内容
  - 不得"顺手优化"任何循环或算式

---

### T3 — 测试 harness

- **所属阶段**：Phase 1
- **依赖前置**：P0
- **并行状态**：可与 T1/T2/T4 完全并行
- **输入**：本文件第 5.1 节（API）、第 6 节（数据格式）
- **产出**：
  - `Test/harness/wav_io.c` / `.h`
  - `Test/harness/ns_runner.c`
  - `Test/harness/CMakeLists.txt`
- **`wav_io` 要求**：
  - `wav_read_pcm16(path, &samples, &n_frames, &n_ch, &rate)`：读 16 bit PCM WAV，返回交错样本
  - `wav_write_pcm16_mono(path, samples, n_frames, rate)`：写单声道 16 bit PCM WAV
  - 仅需支持标准 44 字节头 + `fmt ` / `data` 块；遇到其他格式返回错误
- **`ns_runner` 要求**：
  1. 命令行：`ns_runner <variant> <level> <input_dir> <output_base_dir>`
     - `<variant>`：`baseline` / `optimized` / `d1` / `d1d5`（决定链接哪个库）
     - `<level>`：`0` / `1` / `3`
     - `<input_dir>`：WAV 所在目录
     - `<output_base_dir>`：通常是 `ENC/Test/data`
  2. 遍历 `<input_dir>/*.wav`，**跳过子目录**（特别是 `output_ns/`）
  3. 每个文件：
     - 取**左声道**样本
     - 按 2.7 节规则换算：`f = (float)sample * 256.0f`
     - 按 480 样本切帧；末帧不足补零，记录有效长度
     - 逐帧调用 `NS_Process48k`
     - 用 `QueryPerformanceCounter` 计时（微秒）
     - 每帧调用 `NS_GetDiagnostics` 并写 metrics 行
     - 输出还原：`out16 = clamp((int)lrintf(out24 / 256.0f), -32768, 32767)`
     - 裁剪回原始帧数后写 WAV
  4. 写 `summary_<variant>_<level>.csv` 与 `build_info.txt`（从库的 `NS_GetVariantName` / `NS_GetPatchFlags` 读取真实值，不写死）
  5. 输出目录：`<output_base_dir>/<variant>/L<level>/`
- **验收标准**：
  - 对 9 个文件全部跑通，`NS_Process48k` 全程返回 `NS_OK`
  - 输出 WAV 帧数与源文件一致，可正常播放（用任意播放器验证一次并记录）
  - metrics CSV 列数与表头与 6.2 节逐字一致；**无 NaN / Inf**
- **禁止事项**：
  - 不得在 harness 内做任何算法补偿（额外归一化、增益修正、自动电平）
  - 不得修改输入音频文件

---

### T4 — 分析绘图管线

- **所属阶段**：Phase 1
- **依赖前置**：P0（只需 6.2 节的 CSV 列名约定）
- **并行状态**：可与 T1/T2/T3 完全并行。**不在关键路径上**
- **输入**：本文件第 9 节（图件规范）
- **产出**：`Test/analysis/` 下的 Python 模块与 `fig_style.py`
- **要求**：
  1. 全部绘图使用 **Pillow**（环境无 matplotlib），numpy 仅用于数值计算
  2. `fig_style.py` 提供统一的：配色常量、字体加载、坐标轴绘制、图例绘制、曲线绘制原语
  3. 中文字体统一 `C:/Windows/Fonts/msyh.ttc`
  4. 每个 `fig_*.py` 可独立运行，读 CSV 出 PNG 到 `ENC/Document/figures/`
  5. 每个脚本运行时**打印关键数值到控制台**（用于交叉核对）
- **验收标准**：`python run_all.py` 在**已有 CSV 数据**条件下能生成全部图件且无异常；每张图有轴标题、单位、图例、图号
- **禁止事项**：不得在分析脚本中修改数据；不得生成装饰性图件

---

### P2 — 基线测量

- **所属阶段**：Phase 2
- **依赖前置**：**T1 + T2 + T3 全部完成**
- **并行状态**：串行
- **产出**：`ENC/Test/data/baseline/L3/` 下 9 个输出 WAV + 9 个 metrics CSV + summary + `build_info.txt`
- **步骤**：构建 `ns_core_BASELINE` → 运行 harness（level 3）→ 核验 → 补跑 level 0 与 level 1
- **关键要求**：
  - **必须确认 `build_info.txt` 中 `patch_flags = 0`**——若不满足，说明链接了错误的库，基线无效
- **验收标准**：
  - 9 个文件全部产出；`summary` 的帧数与 2.6 节表一致（允许 ±1 帧的补零差异）
  - 无 NaN / Inf
- **禁止事项**：不得在基线构建中打开任何 patch 开关

---

### T5 — 修复 / 优化版

- **所属阶段**：Phase 3
- **依赖前置**：P2（需要基线产物做即时对照）
- **并行状态**：可与 T6 并行
- **产出**：
  - `ns_core_OPTIMIZED`、`ns_core_D1`、`ns_core_D1D5` 三个库构建通过
  - `ENC/Test/data/optimized/L3/`、`data/d1/L3/`、`data/d1d5/L3/` 各一套产物
- **步骤**：
  1. 按 5.5 节确认三个修复点的实现（D1 补零、D5 钳位派生、D6 换 `sqrtf`）
  2. 构建四个变体
  3. 各跑一轮 level 3
  4. 与基线做即时对照，确认差异符合预期方向
- **验收标准**：
  - 四个变体的 `build_info.txt` 中 `patch_flags` 分别为 `0` / `1` / `3` / `7`
  - 优化版的 `signal_energy` 轨迹与基线**有明显可测差异**（这是 D1 生效的直接证据）
- **禁止事项**：不得改动与三个修复点无关的任何算式

---

### T6 — 交叉参照构建（可降级）

- **所属阶段**：Phase 3
- **依赖前置**：P2
- **并行状态**：可与 T5 并行
- **产出**：`ENC/Test/data/reference/` 下的输出与 `figures/FigT20_*.png`
- **步骤**：
  1. 尝试构建 `Project/ENC_Core`（该目录含 `CMakeLists.txt`）
  2. 若成功，用其 API（`InitNSHandler` / `NS_Process_48kAudio`）接入同一套 harness
  3. **量纲对齐**：`Project/ENC_Core` 工作在 floatS16 量纲，接入时**不做 ×256 换算**，直接送 16 bit 值
  4. 对比仅做**形态对比**（曲线走势、衰减量分布），**不做逐点相减**
- **降级条件**：若构建失败、API 不匹配、或对齐成本超预算，**直接标记为"未执行"**并记录原因，不阻塞 P4
- **验收标准**：成功则产出对比图并在报告中标注量纲差异；失败则产出说明文档
- **禁止事项**：
  - 不得为了"跑通"而修改 `Project/ENC_Core/` 下的任何源码——它是参照物，改了就不再是参照
  - 不得对量纲不同的两条路径做逐点相减或逐点误差统计

---

### P4 — 优化后测量

- **所属阶段**：Phase 4
- **依赖前置**：T5
- **产出**：`data/optimized/L0|L1/` 补测数据
- **验收标准**：
  - 与基线产物**同目录结构、同 CSV 列、同帧数**，可逐帧对齐
  - level 0/1/3 三个等级齐备
- **禁止事项**：
  - 不得更换输入音频、采样率或分帧长度——任何与基线不同的输入条件都会使 A/B 对比失效
  - 不得用优化版的输出覆盖基线目录

---

### T7 — PPT 素材图件生成

- **所属阶段**：Phase 5
- **依赖前置**：P4 与 T4
- **并行状态**：可与 T8 并行（T8 先搭版式骨架）
- **输入**：本文件第 9 节
- **产出**：`ENC/Document/figures/` 下 22 张 `FigT01`–`FigT22`
- **验收标准**：每张图与 PPT 对应页一一对应；图内最小字号 ≥ 24 px；画布尺寸与版位比例匹配
- **禁止事项**：不得套用为文档设计的横长图版式；不得生成装饰性图件

---

### T8 — 演示文稿组装

- **所属阶段**：Phase 5
- **依赖前置**：P4（T7 的图件陆续并入）
- **并行状态**：可与 T7 并行
- **输入**：本文件第 10 节
- **产出**：`ENC/Document/NS组件化与对比测试报告.pptx`（34 页）
- **技术约束**：
  - 使用 `python-pptx` 1.2.0
  - 幻灯片尺寸固定 `13.333 × 7.5 in`（16:9）
  - 统计类图表用**原生图表**（`CategoryChartData` + `add_chart`），数据必须内嵌且与源 CSV 一致
  - 信号可视化类用本地 PNG 素材
  - 明细表格用**原生表格**
  - 字体统一 `微软雅黑`
- **验收标准**：见第 11.3 节
- **禁止事项**：不得手工在 PowerPoint 中编辑后交付——`.pptx` 必须能由 `build_deck.py` 一键重建

---

### T9 — 结构校验与渲染预览

- **所属阶段**：Phase 6
- **依赖前置**：T8
- **产出**：
  - `ENC/Document/preview/checks.json`
  - `ENC/Document/preview/NS报告.pdf`
  - `ENC/Document/preview/key-*.png`（关键页）
- **步骤**：
  1. 结构校验：
     ```
     <python> D:\Devtools\DSH\resources\runtime\office-skills\scripts\check_office.py \
       <pptx> --out <preview>\checks.json --count 34 \
       --contains "结论" --contains "附录"
     ```
  2. 渲染 PDF：
     ```
     <node> <cli> convert --input <pptx> --output <preview>\NS报告.pdf
     ```
  3. 渲染关键页 PNG（封面、结论页、声谱图页、消融页、附录页）：
     ```
     <node> <cli> render --input <pptx> --output-dir <preview> --pages 1,2,20,24,34 --dpi 144
     ```
  4. 检查返回的 `missingFonts` 是否为空
- **验收标准**：
  - `checks.json` 的 `verdict` 为 `pass`
  - PDF 与关键页 PNG 生成成功
  - `missingFonts` 为空
- **限制说明**：执行模型**不具备图像输入能力**，无法自动检查视觉版式。必须在报告中标注"版式待用户确认"。
- **禁止事项**：
  - 不得以"结构校验通过"替代视觉检查，也不得在报告中暗示视觉验收已完成
  - 不得用 `read_image` 或类似方式尝试读取渲染结果（当前模型不支持图像输入）
  - 不得搜索或调用系统安装的 LibreOffice，渲染只能走上述内置路径

---

### P6 — 验收与交付

- **所属阶段**：Phase 7
- **依赖前置**：T9
- **产出**：交付物索引 + 自检清单
- **检查项**：见第 11 节全部条目
- **交付方式**：用 `present` 工具呈现 `.pptx` 与预览文件
- **禁止事项**：
  - 不得在任一验收项未通过时标记任务完成
  - 不得在自检清单中把"未执行"或"未验证"的条目标为通过
  - 第 11.3 节 C9 项（视觉检查责任归属）在用户确认前**不得标为通过**

---

## 9. 图件规范

### 9.1 技术约束

- 绘图库：**Pillow**（`PIL`），numpy 仅用于数值计算。**环境无 matplotlib**
- 中文字体：`C:/Windows/Fonts/msyh.ttc`（常规）、`msyhbd.ttc`（粗体）
- 输出格式：PNG，白底不透明
- 输出路径：`ENC/Document/figures/`
- 命名：`FigT<NN>_<中文简述>.png`，`<NN>` 为两位数字

### 9.2 版位与画布

幻灯片固定 16:9（13.333 × 7.5 in）。图件按三种版位生成：

| 版位 | 幻灯片占用 | 画布像素 | 宽高比 |
|------|-----------|---------|--------|
| 整页图 | 12.13 × 5.70 in | **1800 × 845** | 2.130 |
| 半页图 | 5.90 × 4.10 in | **900 × 625** | 1.440 |
| 条带图 | 12.13 × 3.00 in | **1800 × 445** | 4.045 |

幻灯片的其余区域放标题（顶部约 1.1 in）与要点文本。

### 9.3 视觉规范

**配色**（与既有图件保持一致，避免 PPT 内风格断裂）：

| 用途 | 色值 |
|------|------|
| 主色 / 主链路 | `#2c6fa8` |
| 基线（baseline） | `#7f8c8d` |
| 优化（optimized） | `#c0392b` |
| 正向强调 / 一致 | `#3d7f33` |
| 警示 / 需注意 | `#c98a12` |
| 网格线 | `#e6ebf0` |
| 坐标轴 / 文字 | `#33414f` |
| 底色 | `#ffffff` |

**字号**（按投影折算：幻灯片 13.333 in 对应投影 1920 px，即 144 px/in，1 pt = 2 px；画布 1800 px 映射到版位 12.13 in，近似 1:1）：

| 元素 | 字号 |
|------|------|
| 图标题 | 32 px |
| 坐标轴标题 | 28 px |
| 图例 | 28 px |
| 数据标签 | 24 px |
| 刻度标签 | 24 px |
| **图内最小字号** | **24 px** |

> **不要沿用文档图的 14–16 px 字号**——在投影下只有 7–8 pt，会糊成一片。

**其他**：右上角固定图号 `FigTxx`（36 px）；单位写在轴标题内；不使用透明背景。

### 9.4 图件清单

| 编号 | 内容 | 数据源 | 版位 |
|------|------|--------|------|
| FigT01 | 9 个文件的时长与能量衰减汇总柱状图（基线 vs 优化双柱） | 各 `summary_*.csv` | 半页 |
| FigT02 | 组件架构与数据流（标注真实文件路径） | 静态绘制 | 整页 |
| FigT03 | 波形对比（1.wav 全段）：原始 / 基线 / 优化，三行 | 输入 WAV + 两份输出 WAV | 条带 |
| FigT04 | 波形对比（放大 0.5 s 片段，取 5–5.5 s） | 同上 | 半页 |
| FigT05 | 声谱图三联（原始 / 基线 / 优化）：512 点 STFT，汉宁窗，50% 重叠 | 同上 | 整页 |
| FigT06 | 声谱差异图（基线谱 − 优化谱，dB） | 同上 | 整页 |
| FigT07 | `signal_energy` 轨迹（基线 vs 优化，对数纵轴） | `metrics_1.csv` 两份 | 条带 |
| FigT08 | `Y[257]` 取值由 `signal_energy` 反推 | 同上 | 半页 |
| FigT09 | `diff_normalization` 轨迹对比 | 同上 | 条带 |
| FigT10 | `speech_prob_mean` 轨迹对比 | 同上 | 条带 |
| FigT11 | `gain_adjustment` 轨迹对比 | 同上 | 条带 |
| FigT12 | 逐帧 `atten_db` 轨迹（基线 vs 优化） | 同上 | 条带 |
| FigT13 | 分频带衰减柱状对比（0–4 / 4–8 / 8–12 / 12–16 / 16–24 kHz 五段） | 输入与两份输出 WAV | 半页 |
| FigT14 | LTAS 对比（原始 / 基线 / 优化三条曲线） | 同上 | 半页 |
| FigT15 | 钳位命中统计（逐文件柱状 + 总命中） | `clamp_hits` 列 | 半页 |
| FigT16 | 启动期（前 200 帧）放大视图 | `metrics_1.csv` | 条带 |
| FigT17 | 消融四组对比（基线 / +D1 / +D1D5 / +D1D5D6） | 四份 `summary_*.csv` | 整页 |
| FigT18 | 9 个文件的 `atten_db` 箱线图 | 各 `summary_*.csv` | 半页 |
| FigT19 | 单帧耗时分布直方图 + RTF 标注 | `elapsed_us` 列 | 半页 |
| FigT20 | 与 `Project/ENC_Core` 参照的形态对比（T6 未执行则跳过并说明） | `data/reference/` | 整页 |
| FigT21 | 无语音段 vs 语音段的衰减对比（按 `speech_prob_mean` 分档） | `metrics_1.csv` | 半页 |
| FigT22 | 失败与边界案例（如出现 NaN、异常衰减、钳位超限） | 全部 CSV | 半页 |

---

## 10. 演示文稿规范

### 10.1 技术约束

- `python-pptx` 1.2.0
- 尺寸：`13.333 × 7.5 in`
- 字体：微软雅黑（`微软雅黑` / `Microsoft YaHei`）
- 图表：原生 `XL_CHART_TYPE.COLUMN_CLUSTERED` 等，数据内嵌
- 图片：本地 PNG（第 9 节产物），**禁止网络图片**
- 生成脚本：`Test/analysis/build_deck.py`，必须可一键重建

### 10.2 版式参数

| 元素 | 位置与尺寸 |
|------|-----------|
| 页标题 | 左上 (0.6, 0.35)，宽 12.1 in，字号 28–30 pt，加粗 |
| 副标题/结论行 | 标题下方 (0.6, 1.05)，宽 12.1 in，字号 16 pt，颜色 `#5a6b7c` |
| 要点文本框 | 左侧 (0.6, 1.5)，宽 5.0 in，字号 17–18 pt，**不超过 5 条** |
| 整页图 | (0.6, 1.4)，宽 12.13 in |
| 半页图（两块并排） | 左 (0.6, 1.4) / 右 (6.85, 1.4)，各宽 5.9 in |
| 条带图 | (0.6, 3.9)，宽 12.13 in；要点放上方 |
| 原生表格 | (0.8, 1.4)，宽 11.7 in，表头 15 pt 加粗，正文 14 pt |
| 页脚 | (0.6, 7.05)，字号 11 pt，内容：报告名 + 页码 |

### 10.3 页面清单（34 页）

**第 1 章 结论（1–4）**

| 页 | 标题（即结论） | 版式 | 内容 | 图 |
|----|--------------|------|------|-----|
| 1 | 封面 | 标题页 | 报告名、被测对象、日期、环境 | FigT02 缩略 |
| 2 | 三项核心结论 | 三栏卡片 | D1 是否成立 / 影响幅度 / 优化改善量 | — |
| 3 | 本次测试回答了什么 | 路线图 | 8 章导览 + 关键数字前置 | — |
| 4 | 一句话答案 | 大图页 | 基线 vs 优化核心指标 | FigT01 |

**第 2 章 被测对象（5–9）**

| 页 | 标题 | 版式 | 内容 | 图 |
|----|------|------|------|-----|
| 5 | 从固件工程中摘出的独立组件 | 架构图 | 分层、摘取范围、去平台化手段 | FigT02 |
| 6 | 基线与优化版只差三个开关 | 对照表 | D1/D5/D6 的代码级差异 | — |
| 7 | 测试量纲必须与固件一致 | 流程图 | 16bit → 24bit 位移对齐与还原 | — |
| 8 | 9 个音频、190 秒、全覆盖 | 数据表 | 文件规格、总时长、帧数 | — |
| 9 | 构建可复现 | 指纹页 | 编译器、优化选项、target、依赖版本 | — |

**第 3 章 测试方法（10–12）**

| 页 | 标题 | 版式 | 内容 | 图 |
|----|------|------|------|-----|
| 10 | 逐帧处理，逐帧记录 | 数据流图 | 分帧、调用、指标导出全链路 | — |
| 11 | 12 类指标覆盖时域/频域/诊断/性能 | 表 | 指标定义与用途 | — |
| 12 | 一键复现 | 命令行页 | 构建、运行、出图、出 PPT 的完整命令 | — |

**第 4 章 基线结果（13–17）**

| 页 | 标题 | 版式 | 内容 | 图 |
|----|------|------|------|-----|
| 13 | 改动前的整体表现 | 指标卡 | 平均衰减、RMS、RTF | FigT01 |
| 14 | 逐文件明细 | 原生表格 | 9 个文件完整指标 | — |
| 15 | 启动期的异常特征 | 条带图 | `signal_energy` 轨迹与理论值偏离 | FigT07 |
| 16 | 声谱上看不出明显异常 | 整页图 | 原始 vs 基线声谱对比 | FigT05 |
| 17 | 但诊断量暴露了问题 | 条带图 | 传导链四级量的基线形态 | FigT09 |

**第 5 章 优化结果（18–23）**

| 页 | 标题 | 版式 | 内容 | 图 |
|----|------|------|------|-----|
| 18 | 优化后整体改善 | 指标卡 + 柱状 | 关键指标对比 | FigT01 |
| 19 | 逐文件对比 | 原生表格 | 基线与优化并排数值 | — |
| 20 | 声谱三联对比 | 整页图 | 原始 / 基线 / 优化 | FigT05 |
| 21 | 差异集中在启动期与噪声段 | 整页图 | 声谱差异图 | FigT06 |
| 22 | 波形层面的改动 | 两栏图 | 全段 + 0.5 s 放大 | FigT03 + FigT04 |
| 23 | D1 传导链被切断 | 四联条带图 | `signal_energy` → `gain_adjustment` | FigT07/09/10/11 |

**第 6 章 消融分析（24–27）**

| 页 | 标题 | 版式 | 内容 | 图 |
|----|------|------|------|-----|
| 24 | 三项修复各自的贡献 | 整页图 | 四组对比 | FigT17 |
| 25 | D1 是主因 | 分析页 | 单独打开 D1 的收益占比 | FigT17 |
| 26 | D5 只在特定条件下触发 | 分析页 | 钳位命中统计 | FigT15 |
| 27 | D6 的影响可忽略 | 分析页 | 数学近似的量化误差 | — |

**第 7 章 交叉验证与讨论（28–31）**

| 页 | 标题 | 版式 | 内容 | 图 |
|----|------|------|------|-----|
| 28 | 与另一条移植路径互证 | 对比页 | `Project/ENC_Core` 形态对比 | FigT20 |
| 29 | 审查结论逐条验证 | 原生表格 | D1/D2/D3/D5/D6 验证状态 | — |
| 30 | 两项缺陷本次覆盖不到 | 说明页 | D2/D3 及原因 | — |
| 31 | 性能与资源 | 图表页 | 单帧耗时分布、RTF | FigT19 |

**第 8 章 结论与附录（32–34）**

| 页 | 标题 | 版式 | 内容 | 图 |
|----|------|------|------|-----|
| 32 | 结论与建议 | 要点页 | 结论、固件回接建议、后续验证项 | — |
| 33 | 附录 A：图件与脚本索引 | 表 | 全部图件、生成脚本、数据路径 | — |
| 34 | 附录 B：检查方法与限制 | 说明页 | **强制页**，见下 | — |

> **第 34 页为强制页**，必须写明：
> 1. 本报告完成了哪些校验（结构校验 / 内容校验 / 渲染预览）
> 2. 视觉版式是否经人工确认
> 3. 若执行模型无图像输入能力，必须标注"**版式待用户审阅 `preview/` 后确认**"
> 4. 2.7 节的量纲假设
>
> **不得让读者误以为视觉验收已完成。**

### 10.4 生成与校验流程

```text
python build_deck.py            # 生成 .pptx
python check_deck.py            # 结构校验 + 渲染预览
```

`check_deck.py` 内部依次调用第 8 节 T9 任务书中的三条命令。

---

## 11. 验收标准

### 11.1 组件验收

| 项 | 通过标准 |
|----|---------|
| A1 | `cmake -S ENC/NS_Component -B build -G "Visual Studio 17 2022"` 配置成功；`cmake --build build --config Release` 四个 target 零错误 |
| A2 | `ns_core_BASELINE` 与 `ns_core_OPTIMIZED` 导出符号集一致 |
| A3 | CMSIS 兼容层四项自测通过（往返 / 边界 A / 边界 B / numpy 交叉） |
| A4 | 组件不依赖 `ENC/MainApp/` 下任何文件；可整体拷贝到空目录独立编译 |
| A5 | `README.md` 摘取对照表覆盖全部 29 个源文件 |

### 11.2 测试验收

| 项 | 通过标准 |
|----|---------|
| B1 | 9 个 WAV 在基线与优化版下全部跑通，无崩溃、无 NaN/Inf |
| B2 | 输出 WAV 与源文件等长（补零裁剪后），可正常播放 |
| B3 | 基线与优化的帧数、CSV 列完全一致，可逐帧对齐 |
| B4 | `build_info.txt` 的 `patch_flags` 与变体匹配（0/1/3/7） |
| B5 | 消融四组数据齐备（BASELINE / +D1 / +D1D5 / +D1D5D6） |
| B6 | level 0/1/3 三档在基线与优化下均齐备 |

### 11.3 报告（PPTX）验收

| 项 | 通过标准 |
|----|---------|
| C1 | `check_office.py` 校验通过：ZIP/XML 完整、内部关系正确、页数为 34 |
| C2 | 每张素材图在对应页被引用并解释其支持的结论；无"只贴图不解释"的页面 |
| C3 | D1/D2/D3/D5/D6 逐条给出"已验证 / 未覆盖 / 结论修正"（第 29 页） |
| C4 | 未覆盖项（D2/D3）明确写出原因，不默认通过（第 30 页） |
| C5 | 全部素材图由 `Test/analysis/` 脚本生成；`.pptx` 由 `build_deck.py` 一键重建 |
| C6 | 文本与图片未溢出幻灯片边界（13.333 × 7.5 in） |
| C7 | 原生图表的数值与源 CSV 一致；原生表格明细可逐项核对 |
| C8 | 渲染预览（PDF + 关键页 PNG）已生成并留档 |
| C9 | 第 34 页已写明检查方法与视觉检查责任归属 |
| C10 | 失败案例与限制单独成页 |

---

## 12. 风险与降级

| 风险 | 影响 | 对策与降级 |
|------|------|-----------|
| **CMSIS 兼容层行为偏离真实 CMSIS** | **高**：整个基线失效 | T1 四项自测必须全过；额外用 `Project/ENC_Core` 的 Ooura 路径跑同一输入做形态对比，两者输出高度相似则互证。若自测不过则**停止 P2 并上报** |
| 固件实际 `I2S_INOUT_RESOLUTION_LEVEL` 与仓库不符 | 高：量纲策略错 | 已冻结为按当前值执行；报告中标注为假设；建议硬件复核 |
| 诊断量插桩改动了既有计算 | 高：基线不再等价 | T2 的插桩只允许**新增成员与赋值**，不得修改既有算式；代码走查确认 |
| `Project/ENC_Core` 无法构建 | 低：仅影响 T6 | T6 设计为可降级，标记"未执行"并说明原因 |
| 原版 C++ 基准打通失败 | 低 | 已列为可降级探索，不在关键路径 |
| D1 修复后音频差异极小，测不出 | 中：报告缺乏说服力 | 用诊断量曲线（`signal_energy` 等）呈现差异，把不可见差异变成可见曲线；消融实验逐项归因 |
| 执行模型无图像输入能力 | 中：PPT 版式无法自查 | 渲染预览全量输出；第 34 页标注视觉检查责任；请用户审阅后确认 |
| PPT 字体缺失导致排版位移 | 中 | 只用微软雅黑；渲染时检查 `missingFonts`；为空才继续 |
| 页面膨胀导致信息密度失控 | 低 | 严格按 34 页规划；每页要点不超过 5 条 |

---

## 13. 执行约定

1. 所有新建文件位于 `ENC/NS_Component/`、`ENC/Test/`、`ENC/Document/figures/`、`ENC/Document/preview/` 之内，不越界。
2. **算法源码的唯一合法改动途径是 `src/ns_patch.h` 的开关**。任何直接改 `src/*.c` 算式行为的做法都需要单独上报并说明理由。
3. 每个任务完成后产出：可运行结果 + 自测证据 + 与任务书验收标准的对照说明。
4. 图件与 PPT 不手工修改；需要变更时改脚本并重新生成。
5. 中间产物（`build/`、`bin/`、`data/`）保留在仓库内便于复核。
6. **遇到任务书未覆盖的情况，停止并上报**，不得自行发明方案。

---

## 14. 附录

### 14.1 冻结源文件清单（29 个，逐文件搬运）

| # | 文件 | 是否需改 include | 是否含平台宏 | 备注 |
|---|------|----------------|-------------|------|
| 1 | `enc.c` | 是 | 是 | 改写为 `ns_glue.c` |
| 2 | `enc.h` | 否 | 是 | `__ALIGNED` 在全局声明上 |
| 3 | `fast_math.c` | 否 | 是 | `__RAM_CODE` × 10 |
| 4 | `fast_math.h` | 是 | 否 | 含 `USE_FAST_FUNC` 等未用宏 |
| 5 | `histograms.c` | 否 | 否 | — |
| 6 | `histograms.h` | 是 | 否 | — |
| 7 | `noise_estimator.c` | 否 | 否 | — |
| 8 | `noise_estimator.h` | 是 | 否 | — |
| 9 | `noise_suppressor.c` | 是 | 否 | **D1/D5/D6 位置** |
| 10 | `noise_suppressor.h` | 是 | 否 | — |
| 11 | `ns_common.h` | 是 | 否 | 常量定义 |
| 12 | `prior_signal_model.c` | 否 | 否 | — |
| 13 | `prior_signal_model.h` | 是 | 否 | — |
| 14 | `prior_signal_model_estimator.c` | 否 | 否 | — |
| 15 | `prior_signal_model_estimator.h` | 是 | 否 | — |
| 16 | `quantile_noise_estimator.c` | 否 | 否 | — |
| 17 | `quantile_noise_estimator.h` | 是 | 否 | — |
| 18 | `signal_model.c` | 否 | 否 | — |
| 19 | `signal_model.h` | 是 | 否 | — |
| 20 | `signal_model_estimator.c` | 否 | 否 | — |
| 21 | `signal_model_estimator.h` | 是 | 否 | — |
| 22 | `speech_probability_estimator.c` | 否 | 否 | 含 `#pragma O0` |
| 23 | `speech_probability_estimator.h` | 是 | 否 | — |
| 24 | `suppression_params.c` | 否 | 否 | 四档参数 |
| 25 | `suppression_params.h` | 是 | 否 | — |
| 26 | `three_band_filter_bank.c` | 否 | 否 | — |
| 27 | `three_band_filter_bank.h` | 是 | 否 | — |
| 28 | `wiener_filter.c` | 否 | 否 | **D6 调用点** |
| 29 | `wiener_filter.h` | 是 | 否 | — |

### 14.2 关键命令速查

```powershell
# 构建
cmake -S ENC/NS_Component -B ENC/NS_Component/build -G "Visual Studio 17 2022"
cmake --build ENC/NS_Component/build --config Release

# 兼容层自测
ENC/NS_Component/build/bin/Release/arm_math_compat_test.exe

# 基线测量（level 3）
ENC/Test/bin/Release/ns_runner.exe baseline 3 `
  "Test_Audio/AudioSample-48000hz" "ENC/Test/data"

# 优化测量
ENC/Test/bin/Release/ns_runner.exe optimized 3 `
  "Test_Audio/AudioSample-48000hz" "ENC/Test/data"

# 出图 + 出 PPT + 校验
cd ENC/Test/analysis
python run_all.py

# PPT 结构校验（单独调用）
python D:\Devtools\DSH\resources\runtime\office-skills\scripts\check_office.py `
  ENC/Document/NS组件化与对比测试报告.pptx --out checks.json --count 34

# PPT 渲染
& "D:\Devtools\DSH\resources\runtime\primary-runtime\dependencies\node\bin\node.exe" `
  "D:\Devtools\DSH\resources\app.asar.unpacked\dsh\node_modules\@deepseek-ai\libreoffice-kit\lib\cli.js" `
  convert --input ENC/Document/NS组件化与对比测试报告.pptx `
  --output ENC/Document/preview/NS报告.pdf
```

### 14.3 执行前自检清单

开工前逐项确认：

- [ ] 已读第 1～6 节
- [ ] 清楚自己被分配的是哪个任务书
- [ ] 已确认该任务的依赖前置已产出
- [ ] 已确认冻结源目录路径可读
- [ ] 已确认测试音频目录存在且为 9 个 WAV
- [ ] 若涉及 PPT，已确认 `python-pptx`、`check_office.py`、LibreOffice Kit 三条路径可用
- [ ] 已理解"不得自行发明方案，遇未定义情况停止上报"

### 14.4 交付前自检清单

- [ ] 第 11 节对应验收项全部通过
- [ ] 自测证据已留存（日志、截图说明、CSV 片段）
- [ ] 未越界修改任何冻结源或其他目录文件
- [ ] 若修改了契约文件，已在任务完成说明中列出并说明理由
