/* NS_Component —— 处理器生命周期与算法入口。
 *
 * 本文件由冻结源 ENC/MainApp/source/driver/alg/dsp/enc/enc.c 改写而来:
 *   - 删除 GPIO 打点、固件头文件依赖与固件 I/O 层全局对象
 *   - enc.c:29-35 的全局状态迁入 struct NSHandler
 *   - enc.c:48 的 __breakpoint(0) 改为 free + 返回 NULL
 *   - 新增诊断量快照采集(观测插桩, 不参与任何既有计算)
 *
 * 算法调用序列与 enc.c:72-76 逐行等价, 顺序不得变。
 */

#include <stdlib.h>
#include <string.h>

#include "ns_api.h"
#include "ns_patch.h"
#include "ns_glue.h"
#include "enc.h"

/* 由 enc.c:26 原样迁入 —— 文件级全局(非 static) */
arm_rfft_fast_instance_f32 g_InstRfftF32;

struct NSHandler {
    Enc           enc;                              /* 含三分频器与降噪器状态 */
    float         split_data[kNumBands][kSplitBandSize]; /* 480 个 float, 等价 enc.c:33 的 g_split_data */
    float*        row_pointers[kNumBands];          /* 等价 enc.c:34 的 g_row_pointers */
    float         in_frame[kFullBandSize];          /* API 的 const in 需要一份可写拷贝 */
    int32_t       level;
    NsDiagnostics diag;                             /* 诊断量快照 */
};

NSHandler* NS_Init(int level)
{
    NSHandler* h;
    int32_t i;

    /* level 不在 [0,3] 内直接失败, 不做 fallback */
    if (level < 0 || level > 3)
    {
        return NULL;
    }

    /* 必须 calloc: 所有跨帧状态的初值必须为 0 */
    h = (NSHandler*)calloc(1, sizeof(struct NSHandler));
    if (h == NULL)
    {
        return NULL;
    }

    /* 等价 enc.c:43-44 */
    ThreeBandFilterBank_Init(&h->enc.three_band_filter_bank);
    NoiseSuppressor_Init(&h->enc.noise_suppressor, level);

    /* enc.c:46-49 的 __breakpoint(0) 在此改为失败返回 */
    if (arm_rfft_fast_init_f32(&g_InstRfftF32, 256) != ARM_MATH_SUCCESS)
    {
        free(h);
        return NULL;
    }

    /* 等价 enc.c:52-55 —— 初始化 split_data 双重指针 */
    for (i = 0; i < kNumBands; i++)
    {
        h->row_pointers[i] = h->split_data[i];
    }

    h->level = level;

    /* 诊断量初值全 0(calloc 已保证, 此处显式写出以便自证) */
    memset(&h->diag, 0, sizeof(h->diag));

    return h;
}

void NS_Free(NSHandler* h)
{
    if (h == NULL)
    {
        return;
    }
    free(h);
}

int NS_Process48k(NSHandler* h, const float* in, float* out)
{
    int32_t i;
    float   prob_sum;

    if (h == NULL)
    {
        return NS_ERR_NULL_HANDLE;      /* 不写 out */
    }
    if (in == NULL || out == NULL)
    {
        return NS_ERR_NULL_BUFFER;      /* 不写 out */
    }

    /* API 的 in 是 const 且允许 in == out; 算法内部签名需要 float*, 先拷贝一份 */
    for (i = 0; i < kFullBandSize; i++)
    {
        h->in_frame[i] = in[i];
    }

    /* 等价 enc.c:72 —— ThreeBandFilterBank_Analysis 移出来的清零步骤, 必须保留。
       split_data[kNumBands][kSplitBandSize] 共 kFullBandSize 个 float, 字节数恰好一致 */
    memset(h->split_data, 0, kFullBandSize * sizeof(float));

    /* 算法调用序列与 enc.c:73-76 逐行等价 */
    ThreeBandFilterBank_Analysis(&h->enc.three_band_filter_bank, h->in_frame, h->row_pointers);
    NoiseSuppressor_Analyze(&h->enc.noise_suppressor, h->row_pointers);
    NoiseSuppressor_Process(&h->enc.noise_suppressor, h->row_pointers);
    ThreeBandFilterBank_Synthesis(&h->enc.three_band_filter_bank, h->row_pointers, out);

    /* ---- 诊断量快照采集(纯读取, 不参与任何计算) ---- */
    h->diag.num_analyzed_frames = h->enc.noise_suppressor.num_analyzed_frames;
    h->diag.signal_energy       = h->enc.noise_suppressor.last_signal_energy;
    h->diag.signal_spectral_sum = h->enc.noise_suppressor.last_signal_spectral_sum;
    h->diag.diff_normalization  = h->enc.noise_suppressor.speech_probability_estimator
                                     .signal_model_estimator.diff_normalization;
    h->diag.prior_speech_prob   = h->enc.noise_suppressor.speech_probability_estimator
                                     .prior_speech_prob;
    h->diag.gain_adjustment     = h->enc.noise_suppressor.last_gain_adjustment;
    h->diag.clamp_hits          = h->enc.noise_suppressor.clamp_hits;
    h->diag.zero_frame_skips    = h->enc.noise_suppressor.zero_frame_skips;

    /* speech_probability[0..kFftSizeBy2Plus1-1] 共 129 个元素的算术平均 */
    prob_sum = 0.0f;
    for (i = 0; i < kFftSizeBy2Plus1; i++)
    {
        prob_sum += h->enc.noise_suppressor.speech_probability_estimator.speech_probability[i];
    }
    h->diag.speech_prob_mean = prob_sum / (float)kFftSizeBy2Plus1;

    return NS_OK;
}

int NS_GetDiagnostics(const NSHandler* h, NsDiagnostics* d)
{
    if (h == NULL)
    {
        return NS_ERR_NULL_HANDLE;      /* 不写 d */
    }
    if (d == NULL)
    {
        return NS_ERR_NULL_BUFFER;
    }

    *d = h->diag;
    return NS_OK;
}

/* 变体名与开关位：五个构建变体各自的真实标识。
   契约澄清：计划书 5.1 给出的值域为 "BASELINE" 或 "OPTIMIZED"；
   D1 / D1D5 / D1D5D6 是 Q1 消融实验的中间变体，其名称计划书未定义，
   此处按开关位派生唯一名，便于 build_info.txt 自证。
   注意：D8 为执行记录 B-24 新发现的缺陷，经用户决策纳入修复范围（Q1 由三项扩为四项）。 */
const char* NS_GetVariantName(void)
{
#if (NS_PATCH_D1 == 0) && (NS_PATCH_D5 == 0) && (NS_PATCH_D6 == 0) && (NS_PATCH_D8 == 0)
    return "BASELINE";
#elif (NS_PATCH_D1 == 1) && (NS_PATCH_D5 == 1) && (NS_PATCH_D6 == 1) && (NS_PATCH_D8 == 1)
    return "OPTIMIZED";
#elif (NS_PATCH_D1 == 1) && (NS_PATCH_D5 == 0) && (NS_PATCH_D6 == 0) && (NS_PATCH_D8 == 0)
    return "OPTIMIZED_D1";
#elif (NS_PATCH_D1 == 1) && (NS_PATCH_D5 == 1) && (NS_PATCH_D6 == 0) && (NS_PATCH_D8 == 0)
    return "OPTIMIZED_D1D5";
#elif (NS_PATCH_D1 == 1) && (NS_PATCH_D5 == 1) && (NS_PATCH_D6 == 1) && (NS_PATCH_D8 == 0)
    return "OPTIMIZED_D1D5D6";
#else
    return "CUSTOM";
#endif
}

/* bit0=D1 bit1=D5 bit2=D6 bit3=D8 */
int NS_GetPatchFlags(void)
{
    return (NS_PATCH_D1 & 1)
         | ((NS_PATCH_D5 & 1) << 1)
         | ((NS_PATCH_D6 & 1) << 2)
         | ((NS_PATCH_D8 & 1) << 3);
}
