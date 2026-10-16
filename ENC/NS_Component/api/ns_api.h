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
