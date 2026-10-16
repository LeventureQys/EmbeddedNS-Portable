# NS 组件化与对比测试 — 执行记录

> **本文件定位**：计划书（`NS组件化与测试开发计划_零上下文版.md`）执行过程的审计轨迹。
> 记录三类内容：**(A)** 计划书事实的独立复核结果；**(B)** 计划书未覆盖处的自决项与理由；
> **(C)** 阶段进展与验收证据索引。
>
> 计划书的六项冻结决策（Q1–Q6）**未作任何更改**。本文件记录的全部是计划书未定义或
> 与实测环境不符之处，逐条给出决策与理由。

- 执行日期：2026-10-16
- 执行环境：Windows + MSVC 19.44.35226.0 + CMake 4.3.2 + Visual Studio 2022 Community
- 仓库：`D:\workshop\Github\EmbeddedNS-Portable`

---

## A. 计划书事实的独立复核

开工前对计划书的关键事实逐项实测，结论：**全部命中，无需修正**。

| 计划书断言 | 复核方法 | 结果 |
|-----------|---------|------|
| 冻结源为 29 个 `.c/.h` + 1 个冗余副本 | 列目录 | ✓ 30 个条目 = 29 + `three_band_filter_bank - 副本.c` |
| `enc.c:13-14` GPIO 头、`:19` `ENC_GPIO_PROFILE_EN`、`:48` `__breakpoint`、`:65` `__RAM_CODE` | 逐行读 | ✓ 行号精确 |
| `enc.c:26` `g_InstRfftF32` 全局定义；`noise_suppressor.c:12` extern 声明 | grep | ✓（计划书 2.3 节未列该 extern，见 B-4） |
| `__RAM_CODE` 仅 `fast_math.c` 10 处 + `enc.c:65` | grep | ✓ 行号 14/15/23/29/40/47/52/57/62/68 |
| `__ALIGNED` 仅 `enc.c:30,31,33` 与 `enc.h:43,44` | grep | ✓ 且均为**后缀**用法 |
| D1 插桩点 `noise_suppressor.c:330-332`、`:340-344`、`:387-389` | 读源码 | ✓ 与计划书 2.5 节代码片段逐字一致 |
| D5 钳位在 `noise_suppressor.c:439` | 读源码 | ✓ 基线字面量 `-8388607.0f` / `8388607.0f` |
| D6 调用点 `noise_suppressor.c:134`、`wiener_filter.c:88` | 读源码 | ✓ |
| 诊断字段路径 `speech_probability_estimator.signal_model_estimator.diff_normalization` | 读头文件 | ✓ 字段存在 |
| 测试音频 9 个、16bit/2ch/48kHz、总时长 190.42 s | Python 读 WAV 头 | ✓ **帧数与时长逐项一致** |
| `Test_Audio/AudioSample-48000hz/output_ns/` 含 9 个参考 WAV | 列目录 | ✓ |
| `Project/ENC_Core` 含 CMakeLists、API 为 `InitNSHandler`/`NS_Process_48kAudio` | 读文件 + GBK 解码 | ✓ 与 T6 描述一致 |
| LibreOffice Kit `checks.json` / `manifest.json` 结构 | 读既有冒烟产物 | ✓ `verdict`、`missingFonts` 字段确认存在 |

**结论**：计划书的事实基础可靠，无需重跑任何前置分析。

---

## B. 自决项（计划书未覆盖或与实测不符）

### B-1 `NS_GetVariantName()` 值域扩展为四个变体名
- **问题**：计划书 5.1 注释给出 `"BASELINE" 或 "OPTIMIZED"`，但 Q1 消融实验需要 4 个构建变体
  （`BASELINE`/`+D1`/`+D1D5`/`+D1D5D6`），中间两个变体没有名字，`build_info.txt` 无法自证链接了哪个库。
- **决策**：按开关位派生唯一名 —— `BASELINE` / `OPTIMIZED_D1` / `OPTIMIZED_D1D5` / `OPTIMIZED`。
  `NS_GetPatchFlags()` 严格按 `bit0=D1 bit1=D5 bit2=D6` 返回 0/1/3/7（与计划书 T5 验收项一致）。
- **理由**：不弱化任何验收项；两个端点的返回值与计划书字面完全一致，新增的只是计划书未定义的两个中间态。

### B-2 MSVC 需要 `/wd4068`（未知 pragma）
- **问题**：源码含 ARMCC 专有 pragma —— `noise_suppressor.c:339,355,397,417` 的 `#pragma unroll`；
  `speech_probability_estimator.c:36,37,80` 的 `#pragma push` / `#pragma O0` / `#pragma pop`。
  MSVC 报 C4068，与 T2 验收项「无警告（/W3）」冲突。
- **决策**：CMake 加 `/wd4068` 屏蔽该警告，**源码一字不改**。
- **副作用（已知偏差）**：这些 pragma 在 MSVC 下被忽略 → `speech_probability_estimator.c` 的
  `#pragma O0` 不生效，该函数在 PC 侧按 `/O2` 编译。MSVC 不做浮点重排，浮点结果不受影响，
  但**与固件的 ARMCC 编译产物不构成逐位等价**。此项已写入报告偏差说明。

### B-3 MSVC 需要 `/utf-8`
- **问题**：29 个源文件转 UTF-8 后含大量中文注释。MSVC 默认按本地代码页（GBK）解析无 BOM 的 UTF-8 文件，
  会产生 C4819 或更危险的「注释吞行」。
- **决策**：CMake 加 `/utf-8`（`/source-charset:utf-8 /execution-charset:utf-8`）。
- **理由**：纯编译适配，不触碰源码字符。

### B-4 `g_InstRfftF32` 保持文件级全局
- **问题**：`noise_suppressor.c:12` 有 `extern arm_rfft_fast_instance_f32 g_InstRfftF32;` 声明
  （计划书 2.3/5.6 未列此点）。搬运时若把 FFT 实例收进 handler，就必须改动 `noise_suppressor.c` 的
  3 处调用点与第 12 行，违反「除变更点外不得有任何改动」。
- **决策**：定义随 `enc.c:26` 迁入 `ns_glue.c`，保持**文件级全局（非 static）**；
  `noise_suppressor.c:12` 的 extern 声明保留原样。
- **理由**：兼容层中该实例无状态（只记 `fftLenRFFT`），不构成跨帧共享状态，无并发风险；
  这是符合计划书 5.6 改动表的最小改动路径。

### B-5 `ns_glue.c` 必须显式 `#include <string.h>`
- **问题**：`enc.c:72` 调用 `memset`，但 `enc.c` 及其包含链**没有任何 `<string.h>`**
  （固件环境靠隐式包含）。MSVC 下会产生 C4013（未声明函数）并可能链接失败。
- **决策**：`ns_glue.c` 显式包含 `<string.h>`。

### B-6 `enc.h` 必须删除 6 处固件侧 extern 声明
- **问题**：计划书 14.1 标记 `enc.h` "是否需改 include：否"，但该头文件还声明了 4 个固件全局对象与 1 个固件接口
  （`g_EncParam`、`enc_level`、`EncOnOff`、`enc_in`、`enc_out`、`Enc_SetOnOff`），组件里没有定义者，
  保留会导致链接期未解析符号。
- **决策**：删除这 6 行；保留 `Enc` 结构体与 `Enc_Init`/`Enc_Process` 声明。逐条记入 README 对照表。

### B-7 测试驱动采用「一变体一可执行文件」
- **问题**：计划书 T3 规定 `ns_runner <variant> <level> <in> <out>`，`<variant>` 决定「链接哪个库」。
  但计划书 A2 验收项要求四个库**导出符号集一致** —— 同名符号在一个进程内无法共存，
  单个 exe 不可能同时链接四个库。
- **决策**：构建 4 个可执行文件 `ns_runner_{baseline,d1,d1d5,optimized}.exe`，
  仍接受计划书规定的 4 个命令行参数；**并在启动时用 `NS_GetVariantName()` 与命令行 `<variant>` 交叉校验**，
  不一致立即以非零码退出。
- **理由**：满足 A2 的符号一致性约束，同时用运行时自检杜绝「链接错库仍出数据」这一最致命的静默失败。

### B-8 `clamp_hits` 用独立前置判断计数
- **问题**：`NsDiagnostics.clamp_hits` 要求统计「本帧触达钳位边界的样本数」，
  而计划书 5.5 要求钳位表达式逐字保持 `MIN(MAX(x, NS_CLAMP_MIN), NS_CLAMP_MAX)` 形式。
  若把计数揉进表达式，就改变了算式结构。
- **决策**：在钳位前用独立 `if (x < NS_CLAMP_MIN || x > NS_CLAMP_MAX) clamp_hits++;` 计数，
  钳位表达式本身保持原样。
- **理由**：计数是纯观测插桩，钳位结果与基线逐位一致；D5 关闭时 `NS_CLAMP_*` 展开为原字面量。

### B-9 `ns_glue.c` 用 `in_frame[480]` 拷贝输入
- **问题**：公共 API 是 `NS_Process48k(NSHandler*, const float*, float*)`，
  而 `ThreeBandFilterBank_Analysis` 的形参是非 const `float*`。
- **决策**：handler 内加 `float in_frame[kFullBandSize]`，调用前拷贝一次（480 float = 1.92 KB）。
- **理由**：避免 const 强转与 `in == out` 混叠风险；拷贝开销相对单帧算法（256 点 FFT + 三分频滤波）可忽略。

### B-10 T9 改为全量 34 页渲染 + 视觉模型逐页检查
- **问题**：计划书假定「执行模型不具备图像输入能力」，因此 T9 只渲染 5 张关键页 PNG，
  并要求第 34 页标注「版式待用户确认」、禁止用 `read_image` 读取渲染结果。
- **现况**：本次执行具备视觉模型能力（DeepSeek Flash-Vision），该假定不成立。
- **决策**：
  1. T9 渲染**全部 34 页** PNG（不传 `--pages`，默认即 all）；
  2. 由视觉模型**逐页**检查版式（溢出、遮挡、字号、图例可读性、中文字体渲染）；
  3. 第 34 页仍写明视觉检查的实际执行方式，并注明**最终以用户审阅为准**。
- **理由**：这只是**加强**了检查力度，不改变任何冻结决策；计划书禁止「以结构校验替代视觉检查」，
  本决策的方向恰好相反。

### B-11 `--pages` 不接受范围语法
- **实测**：`libreoffice-kit` 的 `render --pages` 校验正则为 `/^[1-9]\d*(?:,[1-9]\d*)*$/`，
  只接受 `all` 或逗号分隔页码，**不接受 `1-34`**；`--max-pages` 默认 100。
- **影响**：计划书 T9 给出的 `--pages 1,2,20,24,34` 语法合法，但全量渲染的正确写法是省略 `--pages`。
  已把该实测结论转达 T4-II。

### B-12 编排决策：T4 拆分为 T4-I / T4-II 两路并行
- **决策**：`T4-I` 负责 `common.py`/`fig_style.py`/全部 `fig_*.py`/`run_all.py`；
  `T4-II` 负责 `deck_data.py`/`build_deck.py`/`check_deck.py`。
  两者共享的契约（目录、22 个图件文件名、CSV 列定义）**由编排者固定**，
  T4-II 独立实现自己的 CSV 读取，不 import `common.py`。
- **理由**：T4 单体工作量约 2500 行 Python；拆分后两路并行且无接口耦合，
  契约由编排者统一持有，避免 SubAgent 间直接沟通。

### B-13 编排决策：双构建树
- **决策**：`ENC/NS_Component/build`（四库 + 兼容层自测）与 `ENC/Test/build`（四个 ns_runner）
  为两棵独立 CMake 树；`ENC/Test/harness/CMakeLists.txt` 通过 `add_subdirectory` 引入
  `ENC/NS_Component` 复用同一份源码与 CMakeLists，**不复制源码**。
- **理由**：计划书 D2 明确要求两套构建产物路径；Test 树内不重复源码可保证「测的就是交付的库」。

### B-14 T6 交叉参照构建：预研已完成（结论：可执行，非降级）
- **构建树**：`ENC/Test/build/enc_core_probe`（置于 `ENC/Test/` 之下，符合计划书 13.1 的产出物边界；
  未污染 `Project/`）。
- **配置**：`cmake -S Project/ENC_Core -B ENC/Test/build/enc_core_probe -G "Visual Studio 17 2022"` **成功**
  —— `cmake_minimum_required(VERSION 3.10)` 在 CMake 4.3.2 下无兼容问题，**不需要** `-DCMAKE_POLICY_VERSION_MINIMUM`。
- **构建**：`--config Release` **成功**，产物 `ENC/Test/build/enc_core_probe/Release/ENC_Core.dll`（+ `.lib`/`.exp`）。
  编译期有 7 条警告（C4244 ×6 double→float/int 窄化、C4715 ×1），**属既有代码问题，不得修改**（T6 禁止改参照物）。
- **接入要点（T6 必须知道）**：
  1. `NS_Process_48kAudio` 的实现**没有 return 语句**（C4715「不是所有的控件路径都返回值」），
     其返回值未定义。**T6 的参照 runner 不得依赖该返回值**，只按「调用后取 output 缓冲」处理。
  2. API 为 C 链接导出：`void* InitNSHandler(int level)` / `void FreeNSHandler(void*)` /
     `bool NS_Process_48kAudio(void*, float*, float*)`；x64 下无名字修饰，
     直接 `LoadLibraryA("ENC_Core.dll")` + `GetProcAddress` 即可（DLL 为 `add_library(ENC_Core SHARED)`）。
  3. 量纲为 floatS16（±32768），接入时**不做 ×256 换算**（计划书 T6 已规定）。

### B-16 `output_ns/` 参考输出的真实规格（计划书 2.6 的表述需修正）
- **计划书 2.6 原文**：「同目录下存在 `output_ns/` 子目录，内含 9 个同规格 WAV」。
- **实测**：声道数/位深/采样率相同（2ch / 16bit / 48kHz），但**帧数只有输入的一半**：
  | 文件 | 输入帧数 | `output_ns` 帧数 | 比值 |
  |------|---------|-----------------|------|
  | 1.wav | 893952 | 446976 | 0.5000 |
  | 2.wav | 1010688 | 505344 | 0.5000 |
  | 9.wav | 819200 | 409600 | 0.5000 |
- **根因**（读 `Project/test_main.cpp` 确认）：该测试驱动的分帧循环按 **int16 个数**推进
  （`samples_to_read = min(480, ...)`，直接 `read(... samples_to_read * sizeof(int16_t))`），
  **没有做声道分离** —— 即它把左右交错的 480 个 int16 当作一个「480 样本的单声道帧」处理，
  却仍按 2 声道写回 WAV 头。等价于：每处理一次消费 240 个立体声帧，输出文件时长因此减半。
- **影响与决策**：
  1. `output_ns/` **不能**直接用于 T6 的形态对比（长度不对、声道处理方式与本次 Q4 决策不同）。
  2. T6 的形态对比改用**用同一套 harness 现场生成**的 `ENC/Test/data/reference/` 数据。
  3. 该现象本身是报告第 28 页（交叉验证）与第 30 页（覆盖不到的部分）的**有价值的观察**：
    另一条移植路径的**测试驱动**存在声道处理缺陷（这是驱动层问题，不是算法结论，措辞须严格区分）。

### B-17 float 在 24 bit 量纲下的精度边界（T1 附加诊断）
- 兼容层往返测试用 [-1,1) 量纲输入，最大绝对误差 **6.33e-08**（阈值 1e-6）✓。
- 但若用真实工况量纲 ±8388608 做往返，误差达 **0.5 量级** —— 这是 `float` 在 2²³ 量级上的
  固有舍入（24 位有效位），**不是兼容层缺陷**，也与缩放约定无关。
- **含义**：报告在讨论 24 bit 量纲的正确性时，须区分「算法级误差」与「float 表示精度」，
  不得把后者算作移植引入的误差。此项与 D6（数学近似精度 0.096%/2.9%）不是同一层面的量。

### B-18 编排侧修正：兼容层自测程序不得 `#include` 实现文件
- T1 首轮交付中 `arm_math_compat_test.c` 以 `#include "arm_math_compat.c"` 并入实现（因 target 只列了测试文件）。
  这属反模式（隐式依赖实现文件私有宏、与库 target 并存时易符号重复）。
- **修正**：由编排者修改 `CMakeLists.txt`，把 `arm_math_compat.c` 作为独立源文件加入
  `arm_math_compat_test` target；测试文件改为只 `#include "arm_math_compat.h"`，
  并消除对 `NS_RFFT_LEN`/`NS_CFFT_LEN` 两个实现私有宏的依赖。

### B-19 T6 接入链路预研：已实测打通（结论：可执行）
用 Python `ctypes` 直接加载已构建的 `ENC_Core.dll` 并跑通完整调用链，实测证据：

```
DLL exists: True
handler = 0x2911f41e180
frames=200 rms_in=2828.427 rms_out=256.122 atten_db=20.862
non-finite in out: False
CALL_CHAIN_OK
```

- 输入：1000 Hz 正弦，幅度 4000，**floatS16 量纲**（不经 ×256 换算），连跑 200 帧达稳态
- 衰减 **20.86 dB**，与 level 3 的标称最大衰减 **-21 dB** 高度吻合
- 输出无 NaN/Inf，`FreeNSHandler` 正常

**意义**：
1. T6 的核心风险（DLL 能否加载 / API 能否调用 / 量纲是否需要换算）**已全部排除**，从「可降级探索」变为「确定性可执行」。
2. 该数值为报告第 28 页（交叉验证）与第 31 页（性能与资源）提供了独立参照锚点：
   若主路径组件在 level 3 下也给出 ~21 dB 量级的衰减，则两条**完全独立**的移植路径（CMSIS FFT 路径 vs Ooura `fft4g` 路径）互证成立。

### B-20 契约补齐：`data/reference/` 的目录结构
- **缺口**：计划书 9.4 只写 FigT20 的数据源为 `data/reference/`，未定义目录内部结构；
  而 T4-I 的绘图脚本与 T6 的产出分属不同执行者，若各自定义会导致 FigT20 无法生成。
- **补齐**（已下发 T4-I）：`reference` 作为与 `baseline`/`optimized` 同构的 variant 值使用，
  路径为 `<DATA_DIR>/reference/L3/{<name>.wav, metrics_<name>.csv, summary_reference_L3.csv}`
  与 `<DATA_DIR>/reference/build_info.txt`；列定义与计划书 6.2/6.3 完全一致。
- **理由**：最大化复用已有的读取逻辑，任何一处为零改动即可读通。

### B-21 计划书 5.6 变更表的缺口：`fast_math.c` 需要 `#include "platform.h"`
- **问题**：计划书 2.3 节 C 表指出 `__RAM_CODE` 出现在 `fast_math.c` 的 10 个函数定义上，
  但 5.6 节的「变更点汇总」**没有**要求为 `fast_math.c` 增加 `platform.h` 的 include；
  而在 PC 上 `__RAM_CODE` 必须由 `platform.h` 提供（否则编译失败）。
- **处理**：T2 在 `fast_math.c` 第 10 行新增 `#include "platform.h"`。经独立比对，
  该文件**仅此 1 行差异**，其余逐字节等价。
- **同类清点**：`__RAM3`（原 `enc.c:29-31`）随全局对象迁入 `ns_glue.c` 后不再使用；
  `__ALIGNED`（原 `enc.c:30,31,33`、`enc.h:43,44`）所在声明随着固件 I/O 全局对象的删除而消失，
  故 `enc.h` / `ns_glue.c` 均无需 `platform.h`。全仓库 `platform.h` 的唯一使用点是 `fast_math.c`。

### B-22 冻结源为混合编码（GBK 主体 + 个别 UTF-8 注释行）
- **发现**：独立比对时发现 `noise_suppressor.c` 等文件的**多数注释为 GBK，但个别注释行是 UTF-8**
  （例如三处 `//	status=arm_rfft_fast_init_256_f32(...)//Niko 替换为...` 注释）。
  整体做「先 UTF-8、失败回退 GBK」的探测会把 UTF-8 行解成乱码。
- **处理**：验收工具 `verify_transcode.py` 改为**逐行探测编码**，以得到准确的源侧文本；
  T2 的实际转码结果经核验为**正确的纯 UTF-8**（目标侧这些注释显示为正常中文）。
- **结论**：转码保真度合格，见 B-23。

### B-23 搬运保真度独立验证结果（`ENC/Test/analysis/verify_transcode.py`）
对 28 个逐行可比对的文件（`enc.c` 整体改写为 `ns_glue.c`，不参与逐行比对）：

| 类别 | 数量 | 说明 |
|------|------|------|
| **零差异** | **17** | 除行尾归一化外逐字节等价，证明转码未损坏任何内容 |
| 预期差异 | 11 | 全部命中计划书 5.6 与 T2 任务书列明的变更点 |
| 未预期差异 | **0** | —— |

零差异的 17 个文件包括 `three_band_filter_bank.c`（244 行）、`noise_estimator.c`（185 行）、
`signal_model_estimator.c`（170 行）、`prior_signal_model_estimator.c`（163 行）等核心算法文件。
`noise_suppressor.c` 的全部 9 处变更（include 替换、`ns_patch.h`、D1×2、D5、D6、诊断插桩 4 处）
经完整 unified diff 逐行核对，无一处越界。

### B-24 【阻塞项】基线测量在第 176 帧产生 NaN —— 新发现缺陷（暂记 D8）
**现象**（实测，`ns_runner_baseline.exe baseline 3 ...`）：
```
1.wav frame_idx=176  speech_prob_mean = -nan
  同帧其余字段均正常: in_rms=648334 out_rms=608017.125 atten_db=0.5577
  signal_energy=6.77837737e+13 signal_spectral_sum=367676608
  diff_normalization=1.24064912e+13 prior_speech_prob=0.9732 gain_adjustment=1
  clamp_hits=0 zero_frame_skips=0 num_analyzed_frames=176
```
harness 按 T3 契约以退出码 7 中止（计划书 6.2「禁止写 NaN 或 Inf，若出现视为缺陷并上报」）。

**判别矩阵**（T3 实测，4 变体 × 3 level = 12 组，全部重跑）：

| variant | patch_flags | L0 | L1 | L3 |
|---|---|---|---|---|
| baseline | 0 | NaN@176 | NaN@176 | NaN@176 |
| d1 | 1 | NaN@176 | NaN@176 | NaN@176 |
| d1d5 | 3 | NaN@176 | NaN@176 | NaN@176 |
| optimized | 7 | NaN@176 | NaN@176 | NaN@176 |

⇒ **与 D1/D5/D6 三个修复开关及 suppression level 全部无关**，是独立缺陷。

**根因（已由位运算精确复现确认）**：`src/fast_math.c` 的 `fastpow2` 是**纯位近似**：
```c
float fastpow2(float x){ union {int32_t i; float f;} v = {.i=(int32_t)(8388608.0f*(x+126.9420151f))}; return v.f; }
```
`v.i` 是截断整数，随后**按位重解释为 float**。当 `x < -126.9420151f` 时该整数为负、符号位为 1，
`v.f` 即成为**负数**；在 x86 上 `(int32_t)` 转换溢出返回 `INT32_MIN`(0x80000000) 时 `v.f = -0.0`，
而落在 NaN 位模式区间时直接返回 NaN。实测（Python 精确复现 float32 位运算）：

| x | `fastpow2(x)` | 精确 `exp2(x)` | 判定 |
|---|---|---|---|
| −88 | 3.1375e-27 | 3.2312e-27 | 正常 |
| −126.9 | 4.9390e-40 | 6.2993e-39 | 正常 |
| **−127** | **nan** | 5.8775e-39 | **NaN** |
| **−130** | **−8.2604e+37** | 7.3468e-40 | **巨大负数** |

`fastexp(y) = fastpow2(1.4427·y)` 故：

| y | `fastexp(y)` | 精确 `exp(y)` |
|---|---|---|
| −88 | 6.0866e-39 | 6.0546e-39 |
| **−89** | **nan** | 2.2274e-39 |
| **−90** | **−1.39899e+38** | 8.1940e-40 |

**传导链**：`avg_log_lrt[i]`（`signal_model_estimator.c` 的累积更新）一旦增大到 **> 88.7**，
`inv_lrt = fastexp(-avg_log_lrt[i])` 即变为 NaN 或 −1.4e38 →
`speech_probability[i] = 1.0f/(1.0f+gain_prior*inv_lrt)` 变为 NaN。
输出音频未被污染，是因为 `wiener_filter.c` 的 `MIN(NaN,1.0f)` 在 C 宏语义下返回 `1.0f`（NaN 比较恒为 false）。

**对计划书 D6 结论的修正**：计划书 2.5 节 D6 把 `fastpow2` 的问题定性为「误差 2.9%」并断言
「不属于本次移植引入的新缺陷」。该评估**只考察了相对误差，未考察定义域溢出行为**。
实测表明这是**定性缺陷（符号翻转 / NaN）**，而非量化误差。

**该缺陷是固件的真实行为吗？** —— 是。`fast_math.c` 经逐字节保真校验（见 B-23，零差异），
且量纲与固件一致（2.7 节 24bit），故固件在同等输入下会产生同一 NaN。

**阻塞影响**：P2 基线无法产出任何数据（9 个文件中第 1 个即中止），P4/T5/T6/T7 全部顺延。

**待决策（已上报用户）**：
1. 是否新增 **D8** 修复开关（对 `fastpow2` 的定义域外行为做下溢钳位返回 `0.0f`）——
   这将**扩展冻结决策 Q1 的修复范围**（原为 D1+D5+D6 三项），属计划书第 3 节明令「须停止并上报」的情形。
2. 基线是否保留该 NaN 行为（真实固件行为），以及验收项 B1「无 NaN/Inf」如何解读。
3. harness 对非有限值的处理策略：是否允许「记录并继续」（显式上报）而非「中止」。

### B-25 T2 的事实修正：29 个源文件实测全部已是 UTF-8
- **计划书 2.2 原文**：「源文件注释为中文，部分为 GBK 编码。搬运时统一转 UTF-8」。
- **T2 字节级检测结论**：29 个源文件**全部已是 UTF-8（无 BOM）**，不存在 GBK/GB2312 文件。
  其中 `enc.c` 为 CRLF 行尾，其余为 LF。因此「转码」实为 no-op ——
  这也正是 B-23 中 17 个文件能逐字符一致的原因。
- **影响**：AGENTS.md 中「注释为中文（mostly GBK/GB2312 编码）」的说明同样需要更正。
  本仓库其它目录（如 `Project/ENC_Core/AudioProcess/Api/ENC_V.h`）**确实**是 GBK 编码，
  容易被以偏概全。

### B-26 T2 的编译警告问题与处置（已由编排者决策）
- **现象**：四个库每个变体 6 条 /W3 警告，全部为冻结源固有、非搬运引入：
  - `histograms.c:41/48/55`、`quantile_noise_estimator.c:36` — C4244（float/double → int32_t 隐式转换）
  - `noise_suppressor.c:304/381` — C4101（`arm_rfft_fast_instance_f32 InstRfftF32;` 未使用）
- **冲突**：计划书 T2 验收要求「无警告」，而计划书 2.3 节 E 注**明确要求**把这两处未使用局部变量
  「保留原样」，13.2 又禁止改算法源码 ⇒ 三者不可同时满足。
- **决策（方案 A′）**：在 `CMakeLists.txt` 中用 `set_source_files_properties` **定向**对
  `histograms.c`、`quantile_noise_estimator.c` 屏蔽 `/wd4244`、对 `noise_suppressor.c` 屏蔽 `/wd4101`，
  **不改动任何源码字符、不做全局屏蔽**。与 P0 已确立的 `/wd4068`（ARMCC pragma）完全同性质。
- **残留风险**：C4244 指向的隐式截断在 `histograms.c` 中经审查**有边界检查**
  （`lrt<100 → idx<1000`）、`quantile_noise_estimator.c:36` 亦在安全范围；已确认无越界。

### B-27 【已决策并实施】D8 的处置：新增修复开关，消融由四组扩为五组
- **触发**：B-24 的 NaN 阻塞项。计划书第 3 节规定「若发现决策确有错误，停止执行并上报，不得自行改选」，
  故本项上报用户决策。
- **用户决策（2026-10-16）**：
  1. **方案 A** —— 新增 **D8** 修复开关，优化版扩展为 **D1+D5+D6+D8**；消融实验由 4 组扩为 **5 组**；
     基线保留 NaN（如实代表固件当前行为）。
  2. **NaN 表达** —— CSV 中该字段写字面量 `nan`，同时在 stderr 逐文件统计；**summary 列定义保持不变**。
- **实施（编排者完成）**：
  | 文件 | 改动 |
  |------|------|
  | `src/ns_patch.h` | 新增 `NS_PATCH_D8` 开关与 `NS_FASTPOW2_GUARD(x)` 宏（下溢返回 `0.0f`、上溢返回 `FLT_MAX`） |
  | `src/fast_math.c` | `fastpow2` 中把「声明+初始化」拆为「声明 → 守卫 → 赋值」，使保护**求值于截断之前**（截断溢出本身即 UB，写在初始化列表里则保护无效） |
  | `CMakeLists.txt` | `ns_add_variant` 扩为 4 开关参数；5 个 target：BASELINE(0,0,0,0) / D1(1,0,0,0) / D1D5(1,1,0,0) / **D1D5D6(1,1,1,0)** / OPTIMIZED(1,1,1,1) |
  | `src/ns_glue.c` | `NS_GetVariantName()` 新增 `OPTIMIZED_D1D5D6`；`NS_GetPatchFlags()` 增加 `bit3=D8` |
  | `ENC/Test/harness/ns_runner.c` | 新增 `d1d5d6` 变体映射；非有限值容错（写 `nan` 字面量 + stderr 汇总 + `NS_RUNNER_STRICT=1` 可恢复中止语义）；**`optimized` 变体若仍出现 NaN 则硬失败退出码 7** |
- **`patch_flags` 新映射**：`baseline=0` / `d1=1` / `d1d5=3` / `d1d5d6=7` / `optimized=15`（bit0=D1 bit1=D5 bit2=D6 **bit3=D8**）
- **对计划书验收标准的影响（已同步更新）**：
  | 项 | 原标准 | 新标准 |
  |----|--------|--------|
  | 11.2 B4 | `patch_flags` 匹配 0/1/3/7 | 匹配 **0/1/3/7/15** |
  | 11.2 B5 | 消融四组齐备 | 消融 **五组**齐备 |
  | 11.2 B1 | 全域无 NaN/Inf | **基线/中间变体允许出现 `nan`**（固件固有 D8 缺陷，须显式上报）；**`optimized` 必须无 NaN** |
- **验证**：`ns_core_OPTIMIZED` 重建通过，**零警告零错误**（与其余四个 target 一致，A′ 屏蔽持续生效）。
- **D8 修复的边界常数推导**（记录以便复核）：
  `2^x` 的位近似在 `8388608·(x+126.9420151)` 落入 `0x7F800001..0x7FFFFFFF` 时返回 NaN，
  对应 **128.058 ≤ x < 129.058**；`x ≥ 129.058` 时 int32 转换溢出。
  故守卫取 `x < -126.9420151f → 0.0f`（下溢）与 `x > 128.0f → FLT_MAX`（上溢）。

### B-28 容错机制验证 + NaN 的实际影响范围（修正性发现）
**容错机制已验证工作**（隔离目录 `ENC/Test/build/_probe_data`，未污染正式数据）：
```
[5] WARNING: non-finite metrics in 4 frame(s), 4 field(s)
[6] WARNING: non-finite metrics in 12 frame(s), 12 field(s)
[7] WARNING: non-finite metrics in 4 frame(s), 4 field(s)
[8] WARNING: non-finite metrics in 1 frame(s), 1 field(s)
SUMMARY: non-finite metrics present in 8 file(s), 121 frame(s), 121 field(s)
         first occurrence: 1 frame 176
EXIT=0
```
`metrics_1.csv` 第 177 行（frame_idx=176）原文：
```
176,648334,623590.188,0.337990493,6.77837737e+13,367676608,1.24064576e+13,nan,0.975479424,1,0,20
```
表头仍为 12 列，`nan` 精确落在 `speech_prob_mean` 列 ✓

**修正性发现：NaN 是间歇性的，不是持续污染**
- 全量 19046 帧中仅 **121 帧**出现非有限值（**0.64%**），分布于 **8/9 个文件**
- 每帧只污染 **1 个字段**（`speech_prob_mean`），与 NaN 来自少数 bin 的判断一致
- 说明 `avg_log_lrt[i]` 在阈值 88.7 附近**振荡**（时而越界、时而回落），而非一旦越界即永久锁死
  —— 这修正了 T3 报告中「此后持续到末尾」的推测性描述
- **影响面评估**：其余 11 列（含全部可用于 A/B 对比的时域指标）**完全有效**，
  故基线与优化版的对比不受实质影响；`speech_prob_mean` 一列需在报告中单独标注

**两条独立移植路径的形态互证已可成立**（计划书第 12 节风险表要求的互证手段）：

| 文件 | baseline L0（24bit 量纲，主路径） | reference L3（floatS16，Ooura 路径） |
|------|-----------------------------------|--------------------------------------|
| 1 | 1.459 dB | 0.624 dB |
| 2 | 1.301 dB | 0.576 dB |
| 3 | 0.634 dB | 0.520 dB |
| 4 | 0.529 dB | 1.501 dB |
| 5 | 0.511 dB | 0.727 dB |
| 6 | 0.422 dB | 0.501 dB |
| 7 | 0.422 dB | 0.473 dB |
| 8 | 0.409 dB | 0.424 dB |
| 9 | 0.417 dB | 0.432 dB |

两条路径的整段衰减落在同一量级（0.4–1.5 dB），**趋势一致**，构成独立互证。
（注意：两者量纲不同、level 不同，按计划书 T6 只做形态对比，**不做逐点相减**。）

### B-29 D8 有效性验证（决定性证据）
在隔离目录（`ENC/Test/build/_probe_data`）用同一输入、同一 level 3 各跑一轮：

| 变体 | patch_flags | 非有限值帧数 | 退出码 |
|------|-------------|--------------|--------|
| `baseline` | 0 | **120 帧 / 7 个文件** | 0（容错） |
| `optimized` | **15** | **0 帧**（无任何 WARNING） | 0 |

⇒ **D8 修复完全消除了 NaN**，且未引入任何新警告。这是「D8 生效」的直接判据。

**整段 `atten_db` 同 level 对比**（两者均为 level 3，可比）：

| 文件 | baseline | optimized | 差值 |
|------|----------|-----------|------|
| 1 | 0.944 | 0.659 | −0.285 |
| 2 | 0.577 | 0.459 | −0.118 |
| 3 | 0.463 | 0.521 | +0.058 |
| 4 | 0.439 | 1.037 | +0.598 |
| 5 | 0.431 | 0.706 | +0.275 |
| 6 | 0.409 | 0.471 | +0.062 |
| 7 | 0.419 | 0.434 | +0.015 |
| 8 | 0.406 | 0.411 | +0.005 |
| 9 | 0.414 | 0.421 | +0.007 |
| **均值** | **≈0.500** | **≈0.524** | 差异极小、方向不一致 |

**解读**：四项修复（D1/D5/D6/D8）对**整段 RMS 衰减**的影响很小且非单调 ——
这与计划书第 12 节风险表的预判完全一致：
> 「D1 修复后音频差异极小，测不出 → 用诊断量曲线（signal_energy 等）呈现差异，
>   把不可见差异变成可见曲线；消融实验逐项归因」

⇒ 报告的论证重心必须放在**诊断量曲线**（`signal_energy` / `diff_normalization` /
`gain_adjustment` / `speech_prob_mean`）与 **NaN 计数的归零**上，而不是整段衰减的差异。
D8 的价值面很清晰：**它把 0.64% 帧的诊断量与语音概率估计从 NaN 中救回来**。

**单帧耗时**：两轮均为 20–25 µs/帧（19046 帧约 400–450 ms 总计），
RTF ≈ 0.0021–0.0024，远低于实时要求。

### B-30 完整数据矩阵产出并通过验收（P2 / T5 / P4 全部完成）
| 变体 | patch_flags | 覆盖 level | 帧数/文件 | 结论 |
|------|-------------|-----------|-----------|------|
| `baseline` | 0 | L0 / L1 / L3 | 19046 | ✅ |
| `optimized` | 15 | L0 / L1 / L3 | 19046 | ✅ |
| `d1` | 1 | L3 | 19046 | ✅ |
| `d1d5` | 3 | L3 | 19046 | ✅ |
| `d1d5d6` | 7 | L3 | 19046 | ✅ |
| `reference` | N/A | L3 | 19046 | ✅（Ooura 路径） |

`verify_data.py --strict` 判定：**PASS，错误 0 项 / 警告 0 项**。
覆盖验收项 B1（无崩溃、逐帧落盘）、B2（输出 WAV 与源等长）、B3（帧数与列定义一致、可逐帧对齐）、
B4（`patch_flags` = 0/1/3/7/15 与变体匹配）、B5（消融五组齐备）、B6（L0/L1/L3 在基线与优化下均齐备）。

**关键发现：D6 会微移 NaN 的位置（NaN 帧数并非常量）**

| 变体 | flags | NaN 帧数 | 首次出现 | 分布 |
|------|-------|---------|---------|------|
| `baseline` | 0 | 120 | 1.wav frame 176 | 7 个文件 |
| `d1` | 1 | 120 | 1.wav frame 176 | 7 个文件 |
| `d1d5` | 3 | 120 | 1.wav frame 176 | 7 个文件 |
| `d1d5d6` | **7** | **122** | 1.wav frame 176 | **8 个文件**（8.wav 新增 1 帧） |
| `optimized` | **15** | **0** | — | — |

D6（`fastsqrt` → 精确 `sqrtf`）改变 `signal_spectrum` 的数值 → 微移 `avg_log_lrt` 的轨迹 →
NaN 的帧数与分布随之变化。含义：
1. **加强**了「NaN 是系统性数值现象、而非偶发噪声」的判断（位置可变但必然出现，且只有 D8 能根除）。
2. 计划书原判断「D6 的影响可忽略」需**限定范围**：对**整段衰减指标**可忽略，
   但它确实改变了诊断量轨迹（这也是 D6 唯一可观测的效应）。

### B-31 harness 的非有限值机制（已实测）
- 逐文件 stderr WARNING + 全局 SUMMARY；`metrics`/`summary` 表头与列数**逐字未变**（12 列 / 10 列）
- `nan` 字面量可被 `float()` 与 numpy 解析；harness 自报计数与 Python 独立扫描**完全一致**
- 严格模式 `NS_RUNNER_STRICT=1` 可恢复「遇非有限值即中止（退出码 7）」
- **硬校验**：`optimized`（已启用 D8）若出现 NaN → 退出码 7，防止容错机制掩盖修复失效
- 变体交叉校验负例已验证（三组错配均以退出码 1 拒绝运行）

### B-32 T6 交叉参照的实施方式（自决项）
- **决策**：用 Python `ctypes` 直接加载 `ENC_Core.dll` 驱动
  （`ENC/Test/harness/run_reference.py`），而非在 C 侧写 `LoadLibrary` + 函数指针桩。
- **理由**：
  1. 计划书 T6 的实质要求是「接入同一套 harness」—— 同输入、同取左声道、同 480 分帧、
     同量纲约定、同指标口径；`run_reference.py` 逐条满足（与 `ns_runner.c` 同构）。
  2. `Project/ENC_Core` 以 SHARED library 构建，C 侧接入需 `LoadLibrary` + 函数指针转换；
     而 ctypes 链路已由预研实测打通（见 B-19）。
  3. 不新增 C target，避免触碰 `Project/` 的构建配置；脚本可读可复现。
- **产出**：`ENC/Test/data/reference/L3/` 下 9 个 WAV + 9 个 metrics + `summary_reference_L3.csv`
  + `reference/build_info.txt`，格式与计划书 6.1/6.2/6.3 同构（便于 T4-I 的脚本复用同一套读取逻辑）。
- **已知语义差异**：该路径不暴露 `NsDiagnostics`，故 metrics 的 **6 个诊断列写 `nan` 作为
  「不可得」占位** —— 与主路径的 D8 缺陷 NaN **语义不同**。
  `verify_data.py` 已对 `reference` 的这 6 列豁免非有限值检查（见其 `check_combo`）。
- **遵守的禁止项**：未修改 `Project/` 下任何文件（参照物保持原样）；
  未对两条不同量纲的路径做逐点相减或逐点误差统计（仅形态对比）。

### B-33 【已修正】harness 跨文件复用同一个 NSHandler（影响基线有效性）
**发现**（诊断线程实测，编排者已复核其证据链）：`ns_runner.c` 的 `main()` 只调用一次 `NS_Init(level)`，
9 个输入文件共用同一个 handler，`process_one()` 内部不重置。

**实测证据**（同一次运行，仅此一处差异）：

| 观测 | 整目录单进程（原行为） | 每文件独立初始化 |
|------|----------------------|-----------------|
| `metrics_3.csv` 第 0 帧 | `speech_prob_mean=1`、`prior_speech_prob=1`、`diff_normalization=9.15e12` | `speech_prob_mean=0.0188`、`prior_speech_prob=0.45`、`diff_normalization=1.084e10`（=该帧 `signal_energy`，正确初值） |
| `metrics_8.csv` 第 0 帧 `atten_db` | 4.82 dB | 16.92 dB |
| 文件 1 两种模式 | 完全一致 | 完全一致（排除其它解释） |

**机理解释**：文件 1 一旦出现 NaN，`avg_log_lrt` 变 NaN → `model->lrt` NaN → `indicator0` NaN →
`ind_prior` NaN → `prior_speech_prob = MAX(MIN(NaN,1.0),0.01)` **被钳成恰好 1.0** →
`gain_prior ≡ 0` → 后续所有文件的语音概率估计整体退化。

**后果（三条）**：
1. baseline 的 2–9 号文件指标不是「干净输入下的算法响应」，`summary` 的 `atten_db` 列**跨文件不可比**；
   而计划书 6.3 与 FigT01 正是要逐文件对比衰减量。
2. NaN 计数随口径变化：整目录 **120 帧 / 7 文件（0.63%）** vs 每文件新 handler
   **481 帧 / 9 文件（2.525%）**。后者的逐文件分布 **26/50/43/29/28/67/80/79/79** 已由诊断程序
   与 runner 的 9 条 WARNING 计数**逐字交叉验证一致**。
3. 与参照路径（`run_reference.py` 每文件独立初始化）**口径不一致**，
   会使第 28 页的两路径形态对比失去可比性。

**处置与验证结果（已修正）**：`ns_runner.c` 改为「每个输入文件在 `main()` 循环内独立
`NS_Init` / `NS_Free`」（`h` 释放后立即置 NULL；`NS_Free(NULL)` 已核实安全），
5 个 target 重建零警告，9 次运行全部退出码 0。验收判据**精确命中**：

| 判据 | 预期 | 实测 |
|------|------|------|
| 各文件首帧 `speech_prob_mean` | 干净初值（0.01–0.05，非 1.0） | **0.0134–0.0329** ✓ |
| 各文件首帧 `prior_speech_prob` | ≈0.45 | **0.449999988（9 文件全部）** ✓ |
| 首帧 `diff_normalization` | == 该帧 `signal_energy` | **成立** ✓ |
| baseline/L3 NaN | 481 帧 / 9 文件 | **481 帧 / 9 文件** ✓ |
| 逐文件分布 | 26/50/43/29/28/67/80/79/79 | **逐字一致** ✓ |
| `optimized` 全 level | 0 NaN | **0（stderr 零字节）** ✓ |
| `metrics_8.csv` 首帧 `atten_db` | 16.92 | **16.9182529** ✓（修正前为 4.82） |
| 文件 1 是否受影响 | 不受影响 | `frame_idx=176` 行与修正前**逐字一致** ✓ |

**修正后的 NaN 全貌**（每文件独立 handler 口径）：

| 变体 | level | NaN 帧数 |
|------|-------|---------|
| `baseline` | L0 / L1 / L3 | 482 / 486 / **481** |
| `optimized` | L0 / L1 / L3 | **0 / 0 / 0** |
| `d1` | L3 | 481 |
| `d1d5` | L3 | 481 |
| `d1d5d6` | L3 | **500** |

⇒ `d1d5d6` 的 500 帧显著高于其余三组（481）：**D6 会改变 NaN 的发生位置与数量**，
旧口径下看到的 122 帧是被文件 1 污染掩盖后的假象。这使消融实验中「D6 并非无影响」这一结论更明确。
只有 **D8** 能把它归零。

### B-35 图件产出进展与两处质量问题的发现
图件管线已验证可在**正式数据**下产出（输出至 `ENC/Document/figures/`）。22 张中 **21 张已落盘**，
期间发现并纠正两处问题：

1. **FigT16 卡死**（`fig_trace.py`）：右面板把 `speech_prob_mean`（0~1）与
   `diff_normalization`（1e10~1e13）画在同一**线性 Y 轴**上，跨度约 14 个数量级。
   已逐项排除两个嫌疑：`autoscale()`（纯算术无循环）、`ticks_linear()`（有 `n>64` 收敛防御）
   与 `ticks_log()`（有边界与数量上限）。定位为 `line()` 的 nan/分段路径或双量级共轴组合。
   **修复中**。
2. **FigT08 的反推公式无物理依据**（编排者审查发现）：原用
   `Y[257] = signal_energy − signal_spectral_sum`，但两者量纲不同 ——
   `signal_energy` 是平方和量纲（1e9~1e14），`signal_spectral_sum` 是幅度和量纲（1e6~1e8），
   相减不构成有物理意义的量。已要求改为用 **baseline 与 `ns_core_D1` 的 `signal_energy` 差值**反推：
   ```
   |Y[257]| = sqrt(129 × (signal_energy_baseline − signal_energy_d1))
   ```
   并在图注写明：量纲为 24bit 原始量纲、**符号不可恢复**（原始实现读的是栈残留）、
   `Δ≤0` 的帧须单独计数并标注。**修复中**。

**图件已揭示的关键数据**（正式数据，每文件独立 handler 口径）：

| 图 | 关键数值 |
|----|---------|
| FigT01 九文件整段衰减 | 基线 0.94/0.45/0.44/1.31/0.66/0.42/0.42/0.41/0.42 dB；优化 0.66/0.55/0.51/1.51/0.73/0.50/0.46/0.42/0.43 dB |
| FigT13 分频带衰减 | 0–4k：基线 −0.91 / 优化 −0.59；16–24k：−2.86 / −2.39 dB |
| FigT14 LTAS | 100–8k 平均：原始 88.59 / 基线 88.09 / 优化 88.11 dB |
| FigT15 钳位命中 | **五组全部为 0** —— D5 在 24bit 量纲 + 本批数据下完全不触发 |
| FigT19 单帧耗时 | 中位 21 µs、P95 35 µs、最大 802 µs；RTF(中位) = 0.0021 |
| FigT21 分档衰减 | 无语音帧：基线 n=99 中位 18.23 dB → 优化 n=261 中位 20.09 dB；语音帧：1698→1430，1.21→0.62 dB |
| FigT22 异常统计 | baseline NaN 26/50/43/29/28/67/80/79/79 = 481；optimized **全 0** |
| FigT03 全段波形 | RMS：原始 1062.0 / 基线 952.6 / 优化 984.4 |

**`FigT15` 全零的含义**（报告第 26 页须准确表述）：D5 的修复对象是"钳位量程写死为 24bit"，
而当前数据在 24bit 量纲下的输出峰值远未触及 ±8388607（`clamp_hits` 全为 0），
因此 **D5 在本批输入下没有可观测效果** —— 这是"条件触发型修复"的正常表现，
不代表 D5 无意义（一旦 `I2S_INOUT_RESOLUTION_LEVEL` 改回 16bit 量纲，缺少 D5 就会立即失效）。

### B-36 T8 / T9 链路已端到端验证（用真实数据，非自测数据）
- **T8 组装**：`build_deck.py` 读正式数据生成 34 页 PPTX —— **152 个 shape、越界 0、无法判定 0**，
  要点文本框最多 5 条，自检报告落 `ENC/Test/analysis/_reports/deck_bounds.json`。
- **T9 校验**：`check_deck.py` 输出
  - `checks.json verdict = pass`、`slides = 34`（`package` / `contains 结论` / `contains 附录` / `count` 四项全过）
  - `manifest.json pageCount = 34`、**`missingFonts = []`**（字体全部可用，无缺失）
  - 34 张 PNG（`preview/page-0001..0034.png`，1920×1081 @144dpi）
  - `preview/NS报告.pdf`（3107.8 KB）
  - 渲染边距自检 **34/34 页通过**
  - 汇总落 `_reports/deck_check_summary.json`
- **结论**：T8/T9 脚本链路无需再调试。剩余工作仅为：等 T4-I / T4-II 的最终修复 → 重跑 →
  执行视觉检查 → 重新汇总。

**最后一步（视觉检查）的落盘契约**（由 T4-II 定义、`check_deck.py` 消费）：
```
路径：ENC/Test/analysis/_reports/visual_check.json
结构：{"pages_checked": 34, "problems": ["...", ...], "notes": "..."}
```
未提供时 `check_deck.py` 打印「待执行」提示并**不计入失败判定**。
本次执行具备视觉模型能力（环境能力已更新，见 B-10），将对 `page-0001..0034.png` **逐页**检查
版式溢出、遮挡、字号可读性与中文字体渲染，结果写入上述文件。

**修复后状态（22 张全部就绪且均为最新版，11:02:40–11:05:18）**：
- **FigT16 改为三面板**：`atten_db`（线性）/ `speech_prob_mean`（显式 `ylim −0.05~1.05`）/
  `log10(diff_normalization)`（对数量纲，实测 9.895~13.155），彻底分离量纲；
- **FigT09 同步改为对数纵轴**（原线性轴下 `diff_normalization` 动态范围仅约 2 倍，曲线近乎水平线）；
- **FigT08 改用 Δ 公式**（与编排者独立验算逐位对齐）；
- **FigT15 增加「未触发」显式表达**（避免把 D5 无触发误读为绘图失败）；
- **通用缺陷修复**：`_include()` 原在 x/y 尺寸不整除时直接 `return`，而 `boxplot()` 恰以
  「2 个 x 端点 + 5 个 y 统计量」调用它 → 数据范围未登记 → `ylim` 退化成先验值 (0,1) →
  **箱线被画到画布外**（`FigT18`/`FigT21` 一度整幅竖线）。已改为广播语义；
- `fig_trace.py` 全流程实测 **0.79 s**（早前观察到的 5.4 分钟未复现，判定为并行编辑竞争所致，
  原「卡死」定性已撤回）。

**FigT16 揭示的关键对比**（`1.wav`，1863 帧）：

| 量 | baseline | optimized |
|----|----------|-----------|
| `speech_prob_mean` 中位 | **1.0** | **0.932** |
| `speech_prob_mean` 范围 | 0.0176 ~ 1.0 | 0.0103 ~ 0.9999 |
| NaN 帧 | 26 | **0** |
| 末值 | 1.0 | 0.0104 |
| 前 200 帧 `atten_db` 均值 / 最大 | 10.087 / 23.639 dB | 9.910 / 24.317 dB |

⇒ baseline 的 `speech_prob_mean` **中位数恰为 1.0**，这正是 B-34 所述机制的直接可视化证据：
NaN 污染 `avg_log_lrt` → `prior_speech_prob` 被 `MAX(MIN(NaN,1.0),0.01)` 钳成 1.0 →
`gain_prior ≡ 0` → 概率恒为 1。`optimized` 则恢复正常的 0.932。

### B-37 NaN 统计的权威口径（报告全部页面须以此为准）
**关键事实**：`atten_db` 列在**所有变体、所有 level** 上**都不含非有限值**；
NaN **只出现在 `speech_prob_mean` 一列**。

| variant | level | `atten_db` 的 nan | `speech_prob_mean` 的 nan |
|---------|-------|------------------|--------------------------|
| `baseline` | L3 / L0 / L1 | 0 / 0 / 0 | **481 / 482 / 486** |
| `d1` | L3 | 0 | **481** |
| `d1d5` | L3 | 0 | **481** |
| `d1d5d6` | L3 | 0 | **500** |
| `optimized` | L3 / L0 / L1 | 0 / 0 / 0 | **0 / 0 / 0** |
| `reference` | L3 | 0 | 19046（诊断列占位，**语义不同**） |

**为什么只有这一列被污染**：`wiener_filter.c` 的 `MIN(NaN, 1.0f)` 在 C 宏语义下返回 `1.0f`
（`NaN < b` 恒为 false），把 NaN 挡在滤波器之外 → 输出音频、`atten_db`、`in_rms`/`out_rms` **全部正常**。
这也正是 harness 报「N frame(s), **N field(s)**」（每帧恰好一个字段）的原因。

**含义（对报告结论有利）**：基线的**全部时域与衰减指标完全有效**，A/B 对比不受实质影响；
只有诊断量 `speech_prob_mean` 需单独标注。即：缺陷是真实的，但**没有污染主指标** ——
这反而**提升**了本次测试的可信度（差异确实来自算法行为，而非测量失效）。

**两个必须留意的口径边界**：
1. `d1/L0`、`d1/L1`、`d1d5/L0`、`d1d5/L1` 是 T3 期的**遗留不完整目录**（各 1 文件 / 176 帧，无 nan），
   不在交付矩阵内。任何 NaN 统计**须限定到 L3**，否则「文件数」口径会从 9 漂到 10。
2. `reference/L3` 的 `speech_prob_mean` 全为 nan 属**诊断列占位**（该路径不暴露 `NsDiagnostics`），
   与主路径的 **D8 缺陷**语义完全不同 —— 报告第 28 页引用时**不得混用**。

### B-38 视觉检查通道：经 workflow 的模型覆盖调用 vision 模型（关键突破）
**问题**：本会话（`mainagent` preset）与全部 `subagent` 系工具都固定运行 `deepseek-v4-flash`，
该模型**不声明图像输入能力**。`read_image` 的调用全部失败，原文：
```
model "deepseek-v4-flash" does not declare image input; switch to an image-capable model to read images
```
（`eng-writer` preset 中配置了 `subagent_vision` 工具、绑定
`deepseek-official / deepseek-v4-flash-vision-exp`，但 **`mainagent` preset 没有这一行**，
故本会话的工具列表中没有该入口。未改动 preset —— 改配置需重启 DSH 才生效，且超出本任务范围。）

**解法（已实测可用）**：`workflow` 工具的 `agent()` 支持 `provider` / `model` 覆盖：
```js
await agent(prompt, { provider: "deepseek-official", model: "deepseek-v4-flash-vision-exp" })
```
实测返回成功，视觉模型**确实读取了图片像素**，正确报告了页面标题、5 根柱子的数据标签
（0.581/0.581/0.581/0.580/0.621）、横轴分类（基线 / +D1 / +D1D5 / +D1D5D6 / +D1D5D6+D8）、
图例与脚注，并判定中文显示正常、无排版缺陷。

**该探测同时暴露一处内容错误（副产物）**：第 25 页写「基线 nan 文件数 0（共 9 个）」，
与实测矛盾（基线 9 个文件**均**含 NaN）。根因即 B-37 的口径错误（统计了 `atten_db` 列而非
`speech_prob_mean` 列）。已同步 T4-II 修正。

**结论**：视觉检查可以完成。T9 将以 4 个并行 `workflow` agent（各 8–9 页、`model` 覆盖为
`deepseek-v4-flash-vision-exp`）对全部 34 页逐页检查，结论写入
`ENC/Test/analysis/_reports/visual_check.json`。

### B-39 客观通道的版式检查结果（试点，由无视觉能力的 SubAgent 用像素/几何测量完成）
在视觉通道打通前，一个 SubAgent 用**像素级测量 + PPTX 几何/文本提取**完成了客观检查
（非视觉判定），结果如下 —— 这些**仍有效**，可与视觉结论互相印证：

| 检查项 | 结果 |
|--------|------|
| 几何越界 | `_reports/deck_bounds.json`：violations=[]、unknown=[]，34 页 / 152 shape |
| 形状重叠 | 逐对检测 4 页顶层 shape 交叠面积 → **无命中** |
| 渲染留白/裁切 | 四页 ink 边界均未触及画布四边 → **无裁切证据** |
| 图片变形 | page-0001 位图 1800×845（2.1302）→ shape 5.900×2.770 in（2.1300），偏差 **0.01%** |
| 表格结构 | page-0014：10×8，列宽合计 11.700 = 表宽，行高 0.420×10 = 4.200 = 表高，72 格**全部单行无换行** |
| 字体 | 全篇 run 的 latin 与 ea 均显式指定 `微软雅黑`；`manifest.json` 的 `missingFonts = []` |

**客观通道发现的两处问题**：
1. **page-0034 要点框文本溢出框底**：框底 6.950 in，渲染实测文本底 **7.007 in**（溢出 0.057 in ≈ 8 px），
   与页脚间隙仅 0.118 in。**未溢出画布**，但属真实版式缺陷。
2. **四页均存在低于 14 pt 的字号**：page-0014 表头 13 / 数据 12 pt、page-0034 要点 13 / 表格 11–12 pt、
   四页统一页脚 11 pt。page-0001 要点 15 pt、page-0025 要点 14 pt 达标。

另有一处**事实纠偏**（对该 SubAgent 的任务描述有误，非报告缺陷）：
page-0014 的表格 72 格中**不含任何 `nan` 字面量**（`nan` 只出现在 page-0025 与 page-0034），
因为该页取的是 `summary` 的时域指标，而 NaN 只污染 `speech_prob_mean`（见 B-37）。

### B-40 全量视觉检查（视觉模型，34 页）：19 页 OK / 15 页有问题
**通道**：`workflow` 的 `agent(prompt, { provider: "deepseek-official", model: "deepseek-v4-flash-vision-exp" })`
（见 B-38），4 批并行、每批 8–9 页。

**结果**：**19 页 OK**（2/3/6/7/8/9/10/11/12/14/18/19/25/27/29/30/32），
**15 页存在实质版式缺陷**，且**全部集中在图件内部** —— 不是 PPT 摆放问题
（PPT 层的 152 shape / 越界 0 几何自检是干净的）。page 34 是唯一属组装层的缺陷，已由 T4-II 修复。

**缺陷的四种模式**：

| 模式 | 表现 | 涉及页 |
|------|------|--------|
| **A 元素叠印** | 竖排轴标题与刻度逐行重叠；说明/注释行压坐标轴与刻度；图内标题压柱体与数据标签 | 13/15/16/17/20/21/22/23/24/26/28/31 |
| **B 图例问题** | 系列名被截断成省略号（「优化 optimiz…」）；图例框遮数据或超出图片边界 | 4/13/15/17/22/23/24/28/31 |
| **C 图内字号过小** | 多页 18–20 px（等效 9–10 pt），最差 10–13 px（等效 **5–6.5 pt**），远低于投影 14 pt 下限 | 1/5/23/28/31/33 |
| **D 子图压盖** | `FigT20`：中间内嵌小图被右侧大图左缘覆盖，又压住左侧大图的右缘 | 28 |

**暴露的工具盲区（重要）**：`check_layout.py` 只检查**画布级**属性（画布尺寸、非白边距、图号、
行高 ≥24 px），**不检查元素之间的重叠**。因此「图例压在刻度上」「轴标题与刻度叠印」这类缺陷
**全部逃逸** —— 22/22 通过却仍有大面积重叠。**「行高 ≥24 px」是必要条件而非充分条件。**
已要求补上元素两两相交检测。

**已处置**：
1. **15 页的图件缺陷 → 全部交给 T4-I 修复**：含布局重排、图例宽度自适应（禁止截断）、
   注释移出轴区、子图间距按内容高度计算、`FigT20` 改为不重叠布局、`FigT02` 卡片正文字号提高；
   并要求轴标题/刻度/图例/注释统一提到 **≥28 px**（计划书 9.3 的 24 px 是**下限**，提高是允许的），
   画布尺寸**不得改动**。
2. **page 34 的文本压页脚 → 已由 T4-II 修复**：要点 5→4 条、框高 2.40→2.45 in、
   要点字号 13→15 pt，像素复核留白 0.049 in、框底之下无墨迹。
3. **`page-0034` 表格正文 12 pt 的偏差 → 决定接受**（计划书 10.2 规定 14 pt）：
   该表含完整命令行原文，提字号会使表高超出该页可用高度；而计划书 10.3 强制 34 页结构，
   不可拆页。**记录为已知偏差**。

**验收方式变更**：修复后必须**再用视觉模型复检**（脚本自检不足以判定版式合格），
以视觉结论为准。最终 `visual_check.json` 将在最后一轮 `check_deck.py` 之前写入。

### B-41 DNS-MOS 语音整体质量对比（用户新增需求，已完成）
**方法选择（关键判断）**：PESQ / STOI 等**有参考**指标在本项目**不可用** ——
被测输入本身就是含噪语音，仓库内不存在干净参考；若以含噪输入为参考，
得到的只是「降噪输出偏离输入多少」，而降噪的目标恰恰就是偏离输入，该分数没有解释力。
故改用 **DNS-MOS**（**无参考**感知 MOS，Microsoft DNS-Challenge 官方发布模型），
它直接回答「修复前后语音整体质量如何」，是当前降噪领域事实上的标准评价工具。

**实现**：`ENC/Test/analysis/dns_mos_eval.py`，严格复现官方 `dnsmos_local.py` 流程
（16 kHz 重采样、9.01 s 窗 / 1 s 跳步、逐段算术平均、官方固定多项式映射；
`sig_bak_ovr.onnx` 输入为**原始波形**，`model_v8.onnx` 输入为 mel 频谱）。
mel 与重采样**直接调用 `librosa`** 而非自行重写 —— 避免预处理偏差导致分数不可信。
环境新增依赖：`onnxruntime` 1.31.0、`librosa` 1.0.0、`soundfile`、`scipy`（经用户要求，
**偏离计划书 1.4「不安装任何 pip 包」**，已记录）。

**结果（9 文件均值）**：

| 变体 | SIG | BAK | OVRL | P808_MOS | OVRL_std |
|------|-----|-----|------|----------|----------|
| `input`（完全不处理） | 3.6772 | 3.9775 | **3.3207** | 3.8919 | 0.1149 |
| `baseline` | 3.6516 | 3.9870 | **3.3011** | 3.9267 | 0.1347 |
| `d1` | 3.6516 | 3.9870 | 3.3011 | 3.9267 | 0.1347 |
| `d1d5` | 3.6516 | 3.9870 | 3.3011 | 3.9267 | 0.1347 |
| `d1d5d6` | 3.6564 | 3.9936 | 3.3078 | 3.9264 | 0.1403 |
| **`optimized`** | 3.6466 | **4.1205** | **3.3743** | **3.9701** | **0.0643** |
| `reference` | 3.6505 | 4.1334 | 3.3838 | 3.9708 | 0.0582 |

**决定性证据：输出 WAV 逐字节对比（全 9 文件 SHA256）**

| 对比 | 结果 |
|------|------|
| `baseline` vs `d1` | **逐字节完全相同** ✓ |
| `baseline` vs `d1d5` | **逐字节完全相同** ✓ |
| `baseline` vs `d1d5d6` | 有差异 |
| `baseline` vs `optimized` | 有差异 |

**结论（四项修复的分层效果）**：

| 修复项 | 对输出音频 | 对感知质量 |
|--------|-----------|-----------|
| **D1** | **严格零**（逐字节相同） | **严格零** |
| **D5** | **严格零**（`clamp_hits` 全 0，未触发） | **严格零** |
| **D6** | 微小（改变 16bit 输出） | 微小（OVRL +0.0067） |
| **D8** | **显著** | **显著**：OVRL **+0.0732**、BAK **+0.1335**、P808 **+0.0434**；**OVRL 标准差 0.1347 → 0.0643（一致性翻倍）** |

**两项超出计划书预期的重要发现**：

1. **基线（固件当前行为）的感知质量低于"完全不处理"**：OVRL 3.3011 < 3.3207（Δ = −0.0196）。
   而**修复后反超不处理**（3.3743，Δ = +0.0536）——
   即 **D8 把该算法从「负收益」变成「正收益」**，这是比"消除 NaN"更有说服力的修复价值论证。
2. **两条独立移植路径互证成立**：`optimized`（3.3743）与 `reference`（Ooura 路径，3.3838）
   在四项 MOS 上几乎持平（OVRL 差 0.0095、BAK 差 0.0129、P808 差 0.0007）——
   说明修复后的主路径已达到另一条成熟路径的质量水平。

### B-34 诊断线程的附加取证（报告素材）
- **反向对照**：`Project/ENC_Core`（Ooura `fft4g` 路径）在**同样输入、同样 24bit 量纲**下
  输出缓冲**全程无 NaN/Inf**（300 帧与全 1863 帧、×256 与 ×1 两种量纲各测）。
  根因在代码层：该路径的 `fast_math.c` 用 `sqrtf`/`powf(2.f,p)`，**没有**整数按位重解释的 `fastpow2`。
  ⇒ 可作为报告第 28/30 页的反向对照证据：「24bit 量纲本身不产生 NaN，缺陷专属位近似实现」。
  （局限：该 DLL 不暴露内部状态，「无 NaN」仅对输出缓冲成立。）
- **NaN 的 bin 分布不固定**：531 个 bin-事件 / 481 帧，每次平均 1.10 个 bin；
  9 文件合计 122 个不同 bin 曾首发。频段偏好逐文件不同（1.wav 偏高频 16/27 在 bin 96–128；
  7/8/9.wav 低频占比高）。帧号间隔呈「零星孤立 + 短突发」（min=1、中位 6–39、max=670）。
- **间歇性机制已实测**：首帧 NaN 后 `prior_speech_prob` 被钳成 1.0 → `gain_prior≡0` →
  平时 `prob ≡ 1.0`；只有当某 bin 再次落入约 0.70 宽的窗口使 `inv_lrt=NaN` 时，
  `0*NaN` 才让该 bin 变 NaN。铁证：7.wav frame 146 —— 129 个 bin 中 128 个恰为 1.0，仅 bin 54 为 NaN。
- **D8 的修复层次确认**（1.wav frame 176）：`max avg_log_lrt = 182.46`（仍远超 88.7），
  而该 bin 的 `inv_lrt` 在 D8 下为 `+0.0`（基线为 `−0.0153617859`），`NaN(inv)/NaN(prob) = 0/0`，
  `prior_speech_prob` 正常演进（0.973→0.976→0.978）而非被钳成 1 ——
  证明修复发生在**数学函数的定义域**（`fastpow2` 截断前的守卫），不是掩盖调用方。
- **D8 还修正了符号逻辑**（建议进报告）：基线 frame 174 出现
  `inv_lrt min = −1.108815e+15`、`speech_probability min = −2.632274e-14`
  —— 概率跑到 `[0,1]` 之外，且「强语音证据」被算成「确定噪声」；
  D8 下 `inv_lrt min = 0`、`prob ∈ [0.95, 1]`（数学正确：`exp(−187)≈0 → prob→1`）。
  ⇒ 消融中 `optimized` 与 `baseline` 的差异是**真实行为差异**，不是数值噪声。

### B-15 环境实测差异（非决策，仅记录）
| 项目 | 计划书写法 | 实测 | 处理 |
|------|-----------|------|------|
| python-pptx | 1.2.0 | **1.0.2** | 以实测为准；若遇 1.2.0 独有 API 改用等价写法 |
| CMake | （未给版本） | 4.3.2 | `cmake_minimum_required(VERSION 3.20)` |
| MSVC | （未给版本） | 19.44.35226.0（VS 2022 Community） | 写入 `build_info.txt` |
| `libreoffice-kit --help` | —— | 不支持该选项 | 改读源码探明参数 |
| `Project/ENC_Core/CMakeLists.txt` | 可构建 | `cmake_minimum_required(VERSION 3.10)`，CMake 4.3.2 下**配置与构建均成功** | 见 B-14；构建树 `ENC/Test/build/enc_core_probe` 复用即可 |
| `NS_Process_48kAudio` 返回值 | 计划书按正常 API 使用 | 实现缺 `return`，返回值未定义（C4715） | T6 不得依赖返回值，见 B-14 |

---

## C. 阶段进展与证据索引

| 阶段 | 任务 | 状态 | 证据 |
|------|------|------|------|
| Phase 0 | P0 骨架与契约 | ✅ 完成 | 配置成功；4 target Release 构建零警告通过；产物 `build/lib/Release/*.lib` |
| Phase 1 | T1 CMSIS 兼容层 | 🔄 进行中 | — |
| Phase 1 | T2 算法源码摘取 | 🔄 进行中 | — |
| Phase 1 | T3 测试 harness | 🔄 进行中 | — |
| Phase 1 | T4-I 绘图管线 | 🔄 进行中 | — |
| Phase 1 | T4-II PPT 组装 | 🔄 进行中 | — |
| Phase 2 | P2 基线测量 | ⏳ 待启动 | — |
| Phase 3 | T5 优化版测量 | ⏳ 待启动 | — |
| Phase 3 | T6 交叉参照 | ⏳ 待启动（预研已完成） | — |
| Phase 4 | P4 优化后测量 | ⏳ 待启动 | — |
| Phase 5 | T7 图件 / T8 演示文稿 | ⏳ 待启动 | — |
| Phase 6 | T9 校验与预览 | ⏳ 待启动 | — |
| Phase 7 | P6 验收与交付 | ⏳ 待启动 | — |

### 关键命令（已实测可用）

```pwsh
# 组件库配置与构建（增量，指定 target）
& "C:\Program Files\CMake\bin\cmake.exe" -S ENC/NS_Component -B ENC/NS_Component/build -G "Visual Studio 17 2022"
& "C:\Program Files\CMake\bin\cmake.exe" --build ENC/NS_Component/build --config Release --target ns_core_BASELINE

# 符号集对比
cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && dumpbin /symbols "ENC\NS_Component\build\lib\Release\ns_core_BASELINE.lib"'

# 测试驱动
& "C:\Program Files\CMake\bin\cmake.exe" -S ENC/Test/harness -B ENC/Test/build -G "Visual Studio 17 2022"
& "C:\Program Files\CMake\bin\cmake.exe" --build ENC/Test/build --config Release --target ns_runner_baseline

# PPT 结构校验
& "<python>" D:\Devtools\DSH\resources\runtime\office-skills\scripts\check_office.py `
  "ENC\Document\NS组件化与对比测试报告.pptx" --out "ENC\Document\preview\checks.json" --count 34 --contains "结论" --contains "附录"

# PPT 渲染（全量 34 页：不传 --pages）
& "<node>" "<libreoffice-kit>\lib\cli.js" render --input "ENC\Document\NS组件化与对比测试报告.pptx" `
  --output-dir "ENC\Document\preview\pages" --dpi 144

# PPT 转 PDF
& "<node>" "<libreoffice-kit>\lib\cli.js" convert --input "ENC\Document\NS组件化与对比测试报告.pptx" `
  --output "ENC\Document\preview\NS报告.pdf"
```
