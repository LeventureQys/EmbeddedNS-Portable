# ENC — 下位机降噪工程

WebRTC Noise Suppressor 移植到 MCU 的完整工程：固件、算法组件、测试脚手架、分析图件与审查报告。

> 上游基准：`../Webrtc_Source/webrtc-audio-processing-1.0/webrtc/modules/audio_processing/ns/`
> 结论报告：[Document/ENC改动版与WebRTC完整版差距分析报告.md](Document/ENC改动版与WebRTC完整版差距分析报告.md)

---

## 目录导览

| 目录 | 内容 | 形态 |
|------|------|------|
| `MainApp/` | 主固件工程，含集成后的 NS 算法 | Keil MDK，`project/JAT1821.uvprojx` |
| `Bootloader/` | 升级引导工程 | Keil MDK，`project/A0.uvprojx` |
| `NS_Component/` | 算法组件化（从固件源码摘出，PC 可编译），含 4 个补丁开关与 5 个变体 | CMake |
| `Test/` | 测试脚手架（`harness/`）+ 分析脚本（`analysis/`）+ 产物（`data/`、`bin/`、`build/`） | CMake + Python |
| `Document/` | 审查报告、执行记录、开发计划、图件（`figures/`）、PDF 预览 | Markdown / PNG |
| `Workspace/` | `WorkspaceClean.py`：清除两个子工程的 Keil `Output/` 编译产物 | Python 脚本 |

---

## 快速入口

**我想看算法源码**

| 目的 | 路径 |
|------|------|
| 下位机实际编译的版本（Keil 工程引用） | `MainApp/source/driver/alg/dsp/enc/` |
| PC 可编译的同源版本（用于在本机复现与消融） | `NS_Component/src/` |
| 上游 WebRTC 原版（C++） | `../Webrtc_Source/.../audio_processing/ns/` |
| 三分频器（上下游共用） | `MainApp/source/driver/alg/dsp/enc/three_band_filter_bank.c`、`../Webrtc_Source/.../audio_processing/three_band_filter_bank.cc` |

**我想看集成点**

| 位置 | 职责 |
|------|------|
| `MainApp/source/driver/alg/dsp/enc/enc.c` | `Enc_Init` / `Enc_Process`，降噪等级全局 `enc_level`（默认 3） |
| `MainApp/source/driver/alg/dsp/drv_alg.c` | I2S 48 样本回调、`enc_in`/`enc_out` 双缓冲、`I2S_INOUT_RESOLUTION_LEVEL` |
| `MainApp/source/app/app_i2s.c` | USER2 中断触发（每 480 样本处理一帧） |
| `MainApp/source/user/Dsp/Dsp.Program.ModuleTable.c` | 参数表下发（降噪开关 / 4 档等级） |

**我想看结论**

| 文档 | 内容 |
|------|------|
| `Document/ENC改动版与WebRTC完整版差距分析报告.md` | **先看这个**：根本原因、实测证据、改动清单、修复代码（精简版） |
| `Document/ENC改动版与WebRTC完整版差距分析报告_详版.md` | 同一结论的完整取证：逐值对照表、NaN 溯源推导、逐模块等价性判定 |
| `Document/NS移植缺陷审查报告.md` | 前序审查（D1/D2/D3/D5/D6/D7） |
| `Document/NS组件化与测试_执行记录.md` | 组件化过程、D8 的发现与决策记录 |
| `Document/NS组件化与测试开发计划.md` | 计划书（含零上下文版） |

**我想重新跑一遍测试**

见下方「构建与运行」。

---

## 构建与运行

### 1. 算法组件 + 测试脚手架（CMake / MSVC）

```powershell
cmake -S ENC/Test/harness -B ENC/Test/build -G "Visual Studio 17 2022" -A x64
cmake --build ENC/Test/build --config Release
```

产物：

- `ENC/Test/build/lib/Release/ns_core_*.lib` —— 5 个变体静态库
- `ENC/Test/bin/Release/ns_runner_*.exe` —— 5 个对应的运行器

`NS_Component` 也可单独构建（只出库，不出运行器）：

```powershell
cmake -S ENC/NS_Component -B ENC/NS_Component/build -G "Visual Studio 17 2022"
cmake --build ENC/NS_Component/build --config Release
```

### 2. 五个变体

唯一差异是编译期开关（`NS_Component/src/ns_patch.h`），源码逐字符相同：

| 变体 | `patch_flags` | 含义 |
|------|:------------:|------|
| `BASELINE` | 0 | 与固件逐位一致（含全部已知缺陷） |
| `D1` | 1 | 补 `Y[kFftSize+1]` 未初始化 |
| `D1D5` | 3 | 再补钳位量程 |
| `D1D5D6` | 7 | 再补精确开方 |
| `OPTIMIZED` | 15 | 再补 `fastpow2` 定义域守卫 |

### 3. 处理音频

```powershell
# 单个变体：<variant> <level:0|1|3> <输入目录> <输出根目录>
ENC/Test/bin/Release/ns_runner_optimized.exe optimized 3 ENC/Test/data/input_long ENC/Test/data/long_run
```

输入要求：16 bit / 48 kHz PCM WAV（多声道取左声道）。输出 `16 bit 单声道 WAV` + 逐帧 metrics CSV。

### 4. 合成十分钟测试语音并出图

```powershell
python ENC/Test/analysis/make_long_speech.py          # → data/input_long/merged_speech_10min.wav (641.44 s)
python ENC/Test/analysis/fig_longrun_spectrogram.py   # → FigL03 时频图 / FigL04 局部放大
python ENC/Test/analysis/fig_longrun_spectrum.py      # → FigL01 幅频谱 / FigL02 消融证据
python ENC/Test/analysis/probe_longrun.py             # → long_run_probe.txt（段级与逐带指标）
```

图件统一写入 `Document/figures/`。

### 5. 固件（Keil MDK）

用 MDK 打开 `MainApp/project/JAT1821.uvprojx`（Bootloader 用 `Bootloader/project/A0.uvprojx`）。
`Workspace/WorkspaceClean.py` 可一次性清除两个子工程的 `Output/` 编译产物。

---

## 关键约定

| 参数 | 值 | 出处 |
|------|-----|------|
| 采样率 / 帧长 | 48 kHz / 480 样本（10 ms） | `three_band_filter_bank.h` `kFullBandSize 480` |
| 子带数 / 子带帧长 | 3 / 160 样本 | `ns_common.h` `kNumBands 3` |
| FFT | 256 点，129 bin | `ns_common.h` `kFftSize` |
| 输入量纲 | **24 bit**，`(float)(int32 >> 8)`，范围 ±8388608 | `drv_alg.c` `I2S_INOUT_RESOLUTION_LEVEL (2)` |
| 采集/回写缓冲 | `enc_in[960]` / `enc_out[960]`，双缓冲 | `enc.c` |
| 中断分工 | USER0（I2S，每 48 样本）/ USER1（ANS）/ USER2（ENC） | `app_dma.c`、`drv_alg.c` |
| 降噪档位 | 0–3 → 6 / 12 / 18 / 21 dB | `suppression_params.c`；固件默认 `enc_level = 3` |

**注意量纲**：算法内部按 24 bit 量纲运算，输出钳位为 ±8388607。若把 `I2S_INOUT_RESOLUTION_LEVEL` 改回 1（16 bit），钳位必须同步改，否则失去保护。

---

## 当前状态

算法主体与 WebRTC 原版逐值等价（系数表、更新公式、循环边界全部核对过）。**当前有一处致命缺陷导致降噪在约 1.8 秒后全面失效**：

`fast_math.c` 的 `fastpow2` 把原版的 `powf` 换成了位技巧，在 `avg_log_lrt > 88.36` 时返回 NaN；该 NaN 经 `MAX(MIN(x,1.0f),0.01f)` 被钳成 **1.0**，使 `prior_speech_prob` 永久锁死，129 个频点的语音概率全部变成 1.0，算法从此认为"永远是语音"。

实测：10.69 分钟合成语音上，基线自第 177 帧起 `speech_prob_mean` 恒为 1.000000，此后 63945 帧无一例外；噪声段衰减仅 0.4–1.5 dB（优化版为 9.9–14.2 dB）。

修复优先级与验证口径见报告第 8 节。**四行改动即可恢复**：

```c
/* fast_math.c:29-33，守卫必须写在 (int32_t) 转换之前 */
if (x < -126.9420151f) { return 0.0f; }
if (x >  128.0f)       { return 3.4028235e38f; }
```

另有 4 项待办：双缓冲丢帧（`app_i2s.c`）、关闭期采集指针（`drv_alg.c`）、缺失的 100 Hz 高通（原版"开 NS 必开 HPF"的硬契约）、`three_band_filter_bank - 副本.c` 冗余文件。

---

## 相关脚本索引

| 脚本 | 用途 |
|------|------|
| `Test/harness/ns_runner.c` | 变体运行器：逐帧处理、导出 metrics CSV、段级汇总 |
| `Test/harness/run_reference.py` | 调用参照实现做交叉校验 |
| `Test/analysis/common.py` | 路径约定、CSV/WAV 读取、STFT 与 LTAS 工具 |
| `Test/analysis/fig_style.py` | 自研 PIL 绘图库（Canvas / Axes / heatmap / colorbar） |
| `Test/analysis/fig_longrun_spectrogram.py` | 十分钟时频图（FigL03 / FigL04） |
| `Test/analysis/fig_longrun_spectrum.py` | 十分钟幅频谱与消融证据（FigL01 / FigL02） |
| `Test/analysis/probe_longrun.py` | 五变体差异定位探针 |
| `Test/analysis/make_long_speech.py` | 从 `Test_Audio` 合成十分钟连续语音 |
| `Test/analysis/verify_transcode.py` | 校验组件源码与固件源码逐字符一致 |
| `NS_Component/port/_diag_nan.c` | NaN 溯源诊断程序（复刻调用序列、逐帧打印内部状态位模式） |

---

## 生成物说明

以下目录**不入库**（`.gitignore` 已排除），可由上述命令重建：

- `Test/build/`、`Test/bin/`、`NS_Component/build/` —— CMake 构建树
- `Test/data/` —— 测试音频、各变体输出、metrics CSV（约 440 MB）
- `Document/figures/_*/` —— 草图与自测产物
- `*/Output/`、`*/project/Listings/`、`*/project/Objects/` —— Keil 编译产物
