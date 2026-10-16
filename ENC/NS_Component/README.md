# NS_Component — 独立算法组件

从 `ENC/MainApp/source/driver/alg/dsp/enc/` 摘出的 WebRTC 噪声抑制算法组件的
PC 可编译版本。执行依据：`ENC/Document/NS组件化与测试开发计划_零上下文版.md`。

## 目录

| 路径 | 内容 |
|------|------|
| `api/ns_api.h` | 公共 API（P0 契约，逐字照抄计划书 5.1） |
| `src/` | 29 个算法源文件的搬运结果 + `ns_patch.h` + `ns_glue.c/h` |
| `port/` | 平台适配层：`arm_math_compat.*`、`platform.h`、`vendor_types.h` |
| `build/` | CMake 构建树（中间产物，保留以便复核） |

## 构建

```pwsh
cmake -S ENC/NS_Component -B ENC/NS_Component/build -G "Visual Studio 17 2022"
cmake --build ENC/NS_Component/build --config Release
```

四个静态库 target（唯一差异是编译期开关）：

| target | NS_PATCH_D1 | NS_PATCH_D5 | NS_PATCH_D6 | patch_flags |
|--------|-------------|-------------|-------------|-------------|
| `ns_core_BASELINE` | 0 | 0 | 0 | 0 |
| `ns_core_D1` | 1 | 0 | 0 | 1 |
| `ns_core_D1D5` | 1 | 1 | 0 | 3 |
| `ns_core_OPTIMIZED` | 1 | 1 | 1 | 7 |

产物：`build/lib/Release/*.lib`、`build/bin/Release/arm_math_compat_test.exe`。

## 契约澄清（P0 阶段的自决项，已记录理由）

计划书在以下几处未给出可直接照抄的规定，P0 作出如下决定，**不改变任何冻结决策**：

1. **变体名**：`NS_GetVariantName()` 对四个变体分别返回
   `BASELINE` / `OPTIMIZED_D1` / `OPTIMIZED_D1D5` / `OPTIMIZED`。
   计划书仅定义了 `BASELINE`/`OPTIMIZED` 两个值，Q1 消融实验新增的两个中间变体
   其名称未定义；按开关位派生唯一名，使 `build_info.txt` 可自证实际链接的变体。
   `NS_GetPatchFlags()` 严格按 `bit0=D1 bit1=D5 bit2=D6` 返回 0/1/3/7。

2. **MSVC 编译选项**（计划书 5.6 变更表未列，属纯编译适配，不改动任何源码字符）：
   - `/wd4068` — 源码含 `#pragma unroll`（`noise_suppressor.c` 4 处）、
     `#pragma O0` / `#pragma push` / `#pragma pop`（`speech_probability_estimator.c` 3 处），
     均为 ARMCC 语义，MSVC 报 C4068。屏蔽该警告以满足 T2「无警告」验收。
     副作用：这些 pragma 在 MSVC 下被忽略，`speech_probability_estimator.c` 的
     `#pragma O0` 不生效（PC 侧按 /O2 编译）。已记录为已知偏差。
   - `/utf-8` — 29 个文件转 UTF-8 后含大量中文注释，MSVC 需显式指定源字符集，
     否则按本地代码页(GBK)解析，可能触发 C4819 或注释吞行。

3. **`g_InstRfftF32` 保持文件级全局**：`noise_suppressor.c:12` 原有
   `extern arm_rfft_fast_instance_f32 g_InstRfftF32;` 声明，搬运时保留原样（属
   「不得有任何改动」范围）；其定义随 `enc.c:26` 迁入 `ns_glue.c`，仍为文件级全局。
   该实例无状态，不构成跨帧共享状态。

4. **`ns_glue.c` 显式 `#include <string.h>`**：原 `enc.c:72` 调用 `memset` 但未包含该头
   （固件环境靠隐式包含）。属去平台化的必要适配。

## 摘取对照表

> T2 填写。覆盖全部 29 个源文件：原文件 ↔ 新文件 ↔ 改动说明。
> 所有行号均指**冻结源文件中的原始行号**。

| # | 原文件（冻结源） | 新文件 | 改动说明 |
|---|-----------------|--------|---------|
| 1 | `enc.c` | `src/ns_glue.c` | **重写**（非逐行拷贝）：删除 GPIO 头与打点（原 13-14、16-19、67-69、78-80 行）、删除 `Enc_SetOnOff`（58-61）、删除固件全局 `enc_level`/`enc_in`/`enc_out`（27、30-31）、`__breakpoint(0)`（48）改为 `free`+返回 NULL、`g_EncParam`/`g_split_data`/`g_row_pointers`/`split_data`（29、33-35）迁入 `struct NSHandler`、新增 `#include <stdlib.h>` 与 `<string.h>`。`g_InstRfftF32`（26）原样迁入且保持文件级全局。算法调用序列（73-76）与 `memset` 清零（72）逐行保留 |
| 2 | `enc.h` | `src/enc.h` | 删除 6 处固件侧外部声明：原 39 行 `extern Enc g_EncParam ;`、40 行 `extern uint8 enc_level ;`、41 行 `extern bool EncOnOff ;`、43 行 `extern float enc_in[480*2]`、44 行 `extern float enc_out[480*2]`、48 行 `void Enc_SetOnOff(bool on);`。保留 `Enc` 结构体定义与 `Enc_Init`/`Enc_Process` 声明 |
| 3 | `fast_math.c` | `src/fast_math.c` | 新增 `#include "platform.h"`（第 10 行，原 9 行 `#include "fast_math.h"` 之后）：为 10 处 `__RAM_CODE` 提供 PC 侧退化定义。**见下方注 1** |
| 4 | `fast_math.h` | `src/fast_math.h` | 原 12-18 行的厂商 include 块 → 单行 `#include "vendor_types.h"` |
| 5 | `histograms.c` | `src/histograms.c` | 仅转码，无改动 |
| 6 | `histograms.h` | `src/histograms.h` | 仅转码，无改动 |
| 7 | `noise_estimator.c` | `src/noise_estimator.c` | 仅转码，无改动 |
| 8 | `noise_estimator.h` | `src/noise_estimator.h` | 仅转码，无改动 |
| 9 | `noise_suppressor.c` | `src/noise_suppressor.c` | ① 11 行 `arm_math.h` → `arm_math_compat.h`，并新增 `#include "ns_patch.h"`；② D6：原 134 行 `fastsqrt(` → `NS_SQRT(`；③ 原 312-315 行 `if(zero_frame)` 的 `return;` 前插入 `zero_frame_skips++`；④ 原 332 行后、389 行后各插入 `NS_FIX_D1_AFTER_FFT(Y);`；⑤ 原 345 行后插入 `last_signal_energy`/`last_signal_spectral_sum` 赋值；⑥ 原 415 行后插入 `last_gain_adjustment` 赋值；⑦ 原 434 行前插入 `clamp_hits=0` 重置；⑧ 原 439 行钳位字面量 `-8388607.0f`/`8388607.0f` → `NS_CLAMP_MIN`/`NS_CLAMP_MAX`，并在其前插入 `clamp_hits` 独立前置判断。**算式、常量值、循环边界、下标、注释文字一律未动** |
| 10 | `noise_suppressor.h` | `src/noise_suppressor.h` | 在 `NoiseSuppressor` 结构体末尾新增 5 个诊断成员（见插桩清单） |
| 11 | `ns_common.h` | `src/ns_common.h` | 仅转码，无改动 |
| 12 | `prior_signal_model.c` | `src/prior_signal_model.c` | 仅转码，无改动 |
| 13 | `prior_signal_model.h` | `src/prior_signal_model.h` | 原 12-18 行厂商 include 块 → 单行 `#include "vendor_types.h"` |
| 14 | `prior_signal_model_estimator.c` | `src/prior_signal_model_estimator.c` | 仅转码，无改动 |
| 15 | `prior_signal_model_estimator.h` | `src/prior_signal_model_estimator.h` | 仅转码，无改动 |
| 16 | `quantile_noise_estimator.c` | `src/quantile_noise_estimator.c` | 仅转码，无改动 |
| 17 | `quantile_noise_estimator.h` | `src/quantile_noise_estimator.h` | 原 12-18 行厂商 include 块 → 单行 `#include "vendor_types.h"` |
| 18 | `signal_model.c` | `src/signal_model.c` | 仅转码，无改动 |
| 19 | `signal_model.h` | `src/signal_model.h` | 原 12-18 行厂商 include 块 → 单行 `#include "vendor_types.h"` |
| 20 | `signal_model_estimator.c` | `src/signal_model_estimator.c` | 仅转码，无改动 |
| 21 | `signal_model_estimator.h` | `src/signal_model_estimator.h` | 仅转码，无改动 |
| 22 | `speech_probability_estimator.c` | `src/speech_probability_estimator.c` | 仅转码，无改动（保留 36、37、80 行的 `#pragma push`/`O0`/`pop` 原文） |
| 23 | `speech_probability_estimator.h` | `src/speech_probability_estimator.h` | 仅转码，无改动 |
| 24 | `suppression_params.c` | `src/suppression_params.c` | 仅转码，无改动 |
| 25 | `suppression_params.h` | `src/suppression_params.h` | 原 12-18 行厂商 include 块 → 单行 `#include "vendor_types.h"` |
| 26 | `three_band_filter_bank.c` | `src/three_band_filter_bank.c` | 仅转码，无改动 |
| 27 | `three_band_filter_bank.h` | `src/three_band_filter_bank.h` | 原 12-18 行厂商 include 块 → 单行 `#include "vendor_types.h"` |
| 28 | `wiener_filter.c` | `src/wiener_filter.c` | 新增 `#include "ns_patch.h"`（原 10 行之后）；D6：原 88 行 `fastsqrt(` → `NS_SQRT(` |
| 29 | `wiener_filter.h` | `src/wiener_filter.h` | 仅转码，无改动 |

**不搬运**：`three_band_filter_bank - 副本.c`（496 行调试分支）。

> **注 1（T2 阶段的自决项）**：`fast_math.c` 的 10 处 `__RAM_CODE` 必须有宏定义才能编译，
> 而计划书 5.6 变更表与 T2 任务书均未规定 `platform.h` 在何处被包含
> （`vendor_types.h`、`platform.h`、`CMakeLists.txt` 在 T2 中均为禁改文件，
> 命令行注入 `__RAM_CODE` 同样需要改 `CMakeLists.txt`）。T2 采取的方案是：
> 在 `fast_math.c` 的 `#include "fast_math.h"` 之后新增一行 `#include "platform.h"`。
> 该改动等价于 ARMCC 环境下由工具链提供 `__RAM_CODE`，不触及任何算式与数据。
> 另：`enc.c:65` 的 `__RAM_CODE` 随重写自然消失（`NS_Process48k` 无该属性）。
>
> **注 2（编码实测）**：任务书与 `AGENTS.md` 均称冻结源"部分文件为 GBK/GB2312"。
> 对全部 30 个条目做字节级检测的结果是：**29 个待搬运文件全部已是 UTF-8（无 BOM）**，
> `enc.c` 为 CRLF 换行、其余为 LF。因此"GBK → UTF-8 转码"实为 no-op，
> 17 个仅转码文件在去除注释与空白后与源文件**逐字符一致**（见插桩/校验说明）。

## 观测插桩清单

> T2 填写：为导出 `NsDiagnostics` 而新增的结构体成员与赋值点（只允许新增，
> 不得改变任何既有计算）。行号为**搬运后** `src/` 文件中的实际行号。

### A. 新增结构体成员

| # | 成员 | 位置 | 类型/说明 |
|---|------|------|----------|
| 1 | `last_signal_energy` | `src/noise_suppressor.h`，`NoiseSuppressor` 结构体内 `WienerFilter wiener_filter;` 之后 | `float`，`NoiseSuppressor_Analyze` 导出的最近一帧 `signal_energy` |
| 2 | `last_signal_spectral_sum` | 同上 | `float`，同上位置导出的 `signal_spectral_sum` |
| 3 | `last_gain_adjustment` | 同上 | `float`，`NoiseSuppressor_Process` 导出的 `gain_adjustment` |
| 4 | `clamp_hits` | 同上 | `int32_t`，本帧触达钳位边界的样本数 |
| 5 | `zero_frame_skips` | 同上 | `int32_t`，累计因零帧跳过 `Analyze` 的次数 |
| 6 | `split_data[kNumBands][kSplitBandSize]` | `src/ns_glue.c`，`struct NSHandler` | `float[3][160]`，即 480 个 `float`，等价 `enc.c:33` 的 `g_split_data` |
| 7 | `row_pointers[kNumBands]` | 同上 | `float*[3]`，等价 `enc.c:34` 的 `g_row_pointers`（`split_data` 双重指针同步收进 handler） |
| 8 | `in_frame[kFullBandSize]` | 同上 | `float[480]`，API 的 `const float* in` 需要一份可写拷贝（并避免 `in == out` 混叠） |
| 9 | `level` | 同上 | `int32_t`，构造时传入的降噪等级 |
| 10 | `diag` | 同上 | `NsDiagnostics`，逐帧诊断量快照 |

### B. 新增赋值语句（全部为纯读取后写入，不参与任何计算）

| # | 赋值 | 位置 | 说明 |
|---|------|------|------|
| 11 | `noise_suppressor->zero_frame_skips++;` | `src/noise_suppressor.c:315`（`if(zero_frame)` 的 `return;` 之前） | 零帧提前返回处递增（原 314 行） |
| 12 | `noise_suppressor->last_signal_energy=signal_energy;` | `src/noise_suppressor.c:349`（`:348` 的 `signal_energy*=kOneByFftSizeBy2Plus1;` 之后） | 原 345 行之后 |
| 13 | `noise_suppressor->last_signal_spectral_sum=signal_spectral_sum;` | `src/noise_suppressor.c:350` | 同上 |
| 14 | `noise_suppressor->last_gain_adjustment=gain_adjustment;` | `src/noise_suppressor.c:422`（`gain_adjustment=WienerFilter_ComputeOverallScalingFactor(...)` 之后） | 原 415 行之后 |
| 15 | `noise_suppressor->clamp_hits=0;` | `src/noise_suppressor.c:441`（钳位双重循环 `for(band=0; ...)` 即 `:442` 之前） | 每帧重置一次（原 434 行之前） |
| 16 | `noise_suppressor->clamp_hits++;` | `src/noise_suppressor.c:449` | 钳位循环体内的**独立前置判断** `:447`：`if(split_data[band][i]<NS_CLAMP_MIN \|\| split_data[band][i]>NS_CLAMP_MAX)`。计数不揉进 `MIN(MAX(...))`，钳位表达式 `:451` 保持计划书 5.5 规定的逐字形式 |
| 17 | `h->diag.* = ...`（9 个字段） | `src/ns_glue.c:115-132`，`NS_Process48k` 末尾 | 从 `Enc` 内部状态与新增成员读取；`speech_prob_mean`（`:126-132`）为 `speech_probability[0..128]`（129 个元素）的算术平均 |
| 18 | `arm_rfft_fast_instance_f32 g_InstRfftF32;` | `src/ns_glue.c:21`（由 `enc.c:26` 迁入） | 文件级全局（非 `static`），`noise_suppressor.c:12` 的 `extern` 声明依赖该符号 |

### C. `enc.h` 的删除项（固件 I/O 层全局对象，本组件不搬）

| # | 删除内容 | 原行号 | 理由 |
|---|---------|--------|------|
| 19 | `extern Enc g_EncParam ;` | 39 | 全局参数对象改由 handler 持有 |
| 20 | `extern uint8 enc_level ;` | 40 | 固件侧全局降噪等级，改由 `NS_Init(level)` 传入 |
| 21 | `extern bool EncOnOff ;` | 41 | 定义在 `drv_alg.c`，属固件 I/O 层 |
| 22 | `extern float enc_in[480*2] __ALIGNED(4);` | 43 | 固件 I/O 缓冲 |
| 23 | `extern float enc_out[480*2] __ALIGNED(4);` | 44 | 固件 I/O 缓冲 |
| 24 | `void Enc_SetOnOff(bool on);` | 48 | 固件接口，供按键模块调用 |

## 转码与搬运正确性校验

校验脚本：`build/_t2/verify_transcode.py`（结果：`build/_t2/verify_transcode_result.txt`）。

方法：对每个文件，把**冻结源**与**`src/` 下的新文件**分别去掉 C 注释（正确处理字符串与
字符字面量、转义）并去掉全部空白字符，再用 `difflib.SequenceMatcher` 对齐比对。

结果：**17 个仅转码文件逐字符一致**（含任务书点名的 `noise_suppressor.c` 的算式主体、
`three_band_filter_bank.c`、`signal_model_estimator.c`）；11 个有改动文件的差异块
共 33 处，逐条对应上表的 include 改动、D1/D5/D6 插桩与 `enc.h` 删除项，**无意外差异**。
`enc.c` 因改写为 `ns_glue.c` 不参与该 1:1 比对。
