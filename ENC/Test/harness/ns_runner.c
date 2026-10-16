/* ns_runner —— NS 算法组件的 PC 侧测试驱动。
 *
 * 命令行: ns_runner_<variant>.exe <variant> <level> <input_dir> <output_base_dir>
 *
 * 处理链(见计划书 2.7 / 4.2 / 6.1):
 *   16bit PCM WAV(交错) -> 取左声道 -> x256 对齐 24bit 量纲
 *   -> 480 样本/帧(末帧补零) -> NS_Process48k -> 逐帧诊断量 + QPC 耗时
 *   -> /256 还原 + 钳位 -> 裁剪回原始帧数 -> 单声道 16bit WAV + metrics CSV
 *
 * 本文件不做任何算法补偿: 无额外归一化、无增益修正、无自动电平、无静音跳过。
 *
 * 状态隔离: 每个输入文件各自新建 handler(NS_Init)并在处理完后立即销毁(NS_Free),
 * 文件之间不共享任何跨帧状态; 单个文件处理过程中绝不重新初始化。
 * 理由: 9 个输入文件是彼此独立的用例, 复用同一实例会让前一文件残留的状态
 * (例如已被 NaN 污染的先验)改变后一文件的算法响应, 使逐文件对比失去物理意义。
 *
 * 退出码:
 *   0 成功
 *   1 变体交叉校验失败(链接的库与命令行 <variant> 不一致)
 *   2 参数错误(个数不足 / variant 未知 / level 非法)
 *   3 输入目录不存在
 *   4 NS_Init 返回 NULL
 *   5 输出目录创建失败
 *   6 NS_Process48k 或 NS_GetDiagnostics 调用失败
 *   7 严格模式(NS_RUNNER_STRICT=1)下出现 NaN / Inf;
 *     或 optimized 变体(含 D8)出现 NaN / Inf —— 硬失败, 见执行记录 B-24
 *   8 文件 IO 或输入格式错误
 */

#define NOMINMAX

/* MSVC 把 fopen/strcpy 等标准 C 函数标为"不安全"; 本模块为工具程序,
   不使用 _s 变体, 在包含任何 CRT 头之前关闭该弃用警告(/W3 零警告要求)。 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <direct.h>
#include <windows.h>

#include "ns_api.h"
#include "wav_io.h"

/* ---------------- 常量(计划书 2.1 / 2.7) ---------------- */

#define NS_FRAME_LEN       480          /* 10 ms @ 48 kHz */
#define NS_SAMPLE_RATE     48000
#define NS_SCALE_16_TO_24  256.0f       /* 输入: sample24 = sample16 * 256 */
#define NS_SCALE_24_TO_16  (1.0f / 256.0f)
#define NS_CLAMP_LO        (-32768)
#define NS_CLAMP_HI        (32767)

#define PATH_BUF_LEN       1024

/* ---------------- 退出码 ---------------- */

enum {
    EXIT_OK               = 0,
    EXIT_VARIANT_MISMATCH = 1,
    EXIT_BAD_ARGS         = 2,
    EXIT_NO_INPUT_DIR     = 3,
    EXIT_INIT_FAILED      = 4,
    EXIT_MKDIR_FAILED     = 5,
    EXIT_PROCESS_FAILED   = 6,
    EXIT_NONFINITE        = 7,
    EXIT_IO_FAILED        = 8
};

/* ---------------- 变体交叉校验表 ----------------
 * 命令行 <variant> -> NS_GetVariantName() 的期望返回值。
 * 命令行 <variant> 同时用作输出路径中的目录名。 */

typedef struct {
    const char* cli;            /* 命令行参数 */
    const char* lib_variant;    /* NS_GetVariantName() 必须返回的值 */
} VariantEntry;

static const VariantEntry kVariants[] = {
    { "baseline",  "BASELINE"        },
    { "d1",        "OPTIMIZED_D1"    },
    { "d1d5",      "OPTIMIZED_D1D5"  },
    { "d1d5d6",    "OPTIMIZED_D1D5D6" },
    { "optimized", "OPTIMIZED"       }
};

/* 命令行变体清单(usage 与未知变体错误提示共用, 避免两处文案失同步) */
static const char kUsageVariants[] = "baseline | d1 | d1d5 | d1d5d6 | optimized";

/* ---------------- CSV 表头(逐字, 计划书 6.2 / 6.3) ---------------- */

static const char kMetricsHeader[] =
    "frame_idx,in_rms,out_rms,atten_db,signal_energy,signal_spectral_sum,"
    "diff_normalization,speech_prob_mean,prior_speech_prob,gain_adjustment,"
    "clamp_hits,elapsed_us\n";

static const char kSummaryHeader[] =
    "file,duration_s,frames,total_elapsed_ms,in_rms,out_rms,atten_db,"
    "in_peak,out_peak,clamp_hits_total\n";

/* ---------------- 小工具 ---------------- */

static int dir_exists(const char* path)
{
    DWORD a = GetFileAttributesA(path);
    if (a == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    return ((a & FILE_ATTRIBUTE_DIRECTORY) != 0u) ? 1 : 0;
}

/* 逐级创建目录(相对或绝对路径均可)。成功返回 0。 */
static int ensure_dir_tree(const char* path)
{
    char   tmp[PATH_BUF_LEN];
    size_t len = strlen(path);
    size_t i;

    if (len == 0u || len >= sizeof(tmp)) {
        return -1;
    }
    memcpy(tmp, path, len + 1u);

    for (i = 1u; i < len; ++i) {
        if (tmp[i] == '\\' || tmp[i] == '/') {
            char save = tmp[i];
            if (i == 2u && tmp[1] == ':') {
                continue;                       /* 跳过 "D:" 这类驱动器前缀 */
            }
            tmp[i] = '\0';
            if (!dir_exists(tmp) && _mkdir(tmp) != 0 && !dir_exists(tmp)) {
                return -1;
            }
            tmp[i] = save;
        }
    }
    if (!dir_exists(tmp) && _mkdir(tmp) != 0 && !dir_exists(tmp)) {
        return -1;
    }
    return 0;
}

/* 浮点有效性检查: NaN 或 Inf 均视为非法。 */
static int f_is_bad(float v)
{
    return (isnan((double)v) != 0) || (isinf((double)v) != 0);
}

/* ---------------- 非有限值策略（用户决策, 见执行记录 B-24） ----------------
 * 基线存在固件固有的 NaN 缺陷(D8): fastexp 在 y < -88.7 时返回 NaN。
 * 该缺陷本身是本次测试的重要发现，不能因为它的存在让整个测量无法进行。
 * 策略:
 *   - 非有限值按字面量 nan / inf / -inf 写入 CSV(Python float() 与 numpy 均可解析)
 *   - 逐文件在 stderr 汇总; summary 的列定义保持不变(计划书 6.3)
 *   - 计划书 6.2「禁止写 NaN 或 Inf」解读为「禁止静默写 NaN」—— 必须显式上报
 *   - 置 NS_RUNNER_STRICT=1 可恢复严格的「遇非有限值即中止(退出码 7)」语义
 */
static int g_strict_mode     = 0;   /* 由环境变量 NS_RUNNER_STRICT 控制 */
static int g_nan_frames      = 0;   /* 本次运行中含非有限值的帧总数 */
static int g_nan_fields      = 0;   /* 本次运行中非有限值字段总数 */
static int g_nan_files       = 0;   /* 出现非有限值的文件数 */
static int g_first_nan_frame = -1;  /* 首个非有限值帧号(全局, -1 表示无) */
static const char* g_first_nan_file = NULL;

/* 首个非有限值所在文件名。process_one 的 name 指向 main 内的循环栈缓冲区,
 * 不能被长期持有(下一轮文件会被覆写), 故复制到本静态缓冲区。 */
static char g_first_nan_file_buf[PATH_BUF_LEN];

/* 文件级去重标记: 进入每个文件时清零, 该文件首次命中时置 1。 */
static int g_nan_file_counted = 0;

/* 统计一组字段中的非有限值个数。 */
static int f_count_bad(const float* v, int n)
{
    int i;
    int c = 0;

    for (i = 0; i < n; ++i) {
        if (f_is_bad(v[i])) {
            ++c;
        }
    }
    return c;
}

/* 把 float 写成 CSV 字段: 非有限值写为可解析的字面量。 */
static const char* f_csv(float v, char* buf, size_t buflen)
{
    if (isnan((double)v)) {
        snprintf(buf, buflen, "nan");
    } else if (isinf((double)v)) {
        snprintf(buf, buflen, (v > 0.0f) ? "inf" : "-inf");
    } else {
        snprintf(buf, buflen, "%.9g", (double)v);
    }
    return buf;
}

/* ---------------- 输入文件列表 ---------------- */

typedef struct {
    char** items;
    int    count;
    int    cap;
} NameList;

static int namelist_push(NameList* l, const char* s)
{
    if (l->count == l->cap) {
        int    newcap = (l->cap == 0) ? 16 : (l->cap * 2);
        char** p = (char**)realloc(l->items, (size_t)newcap * sizeof(char*));
        if (p == NULL) {
            return -1;
        }
        l->items = p;
        l->cap   = newcap;
    }
    l->items[l->count] = (char*)malloc(strlen(s) + 1u);
    if (l->items[l->count] == NULL) {
        return -1;
    }
    strcpy(l->items[l->count], s);
    l->count += 1;
    return 0;
}

static void namelist_free(NameList* l)
{
    int i;
    for (i = 0; i < l->count; ++i) {
        free(l->items[i]);
    }
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->cap   = 0;
}

static int name_cmp(const void* a, const void* b)
{
    const char* sa = *(const char* const*)a;
    const char* sb = *(const char* const*)b;
    return strcmp(sa, sb);
}

/* 遍历 <dir>\*.wav, 跳过目录(尤其是 output_ns\), 只收 ".wav" 结尾的普通文件。 */
static int collect_wavs(const char* dir, NameList* out)
{
    char             pattern[PATH_BUF_LEN];
    WIN32_FIND_DATAA fd;
    HANDLE           h;

    if (snprintf(pattern, sizeof(pattern), "%s\\*.wav", dir) < 0) {
        return -1;
    }
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return 0;                               /* 无匹配文件 */
    }

    do {
        size_t len;

        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0u) {
            continue;
        }
        len = strlen(fd.cFileName);
        if (len <= 4u) {
            continue;
        }
        if (_stricmp(fd.cFileName + (len - 4u), ".wav") != 0) {
            continue;                           /* 排除 *.wavx 之类的 8.3 通配残留 */
        }
        if (namelist_push(out, fd.cFileName) != 0) {
            FindClose(h);
            return -1;
        }
    } while (FindNextFileA(h, &fd) != 0);

    FindClose(h);
    return 0;
}

/* ---------------- 构建指纹(计划书 6.4) ---------------- */

static void compiler_string(char* buf, size_t n)
{
#if defined(_MSC_FULL_VER)
    snprintf(buf, n, "MSVC %d.%d.%d",
             (int)(_MSC_FULL_VER / 10000000),
             (int)((_MSC_FULL_VER / 100000) % 100),
             (int)(_MSC_FULL_VER % 100000));
#elif defined(__GNUC__)
    snprintf(buf, n, "GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    snprintf(buf, n, "unknown");
#endif
}

/* 尝试取当前工作目录所属 git 仓库的短 commit; 失败(无 git / 非仓库)填 N/A。 */
static void git_commit_string(char* buf, size_t n)
{
    FILE* p;

    buf[0] = '\0';
    p = _popen("git rev-parse --short HEAD 2>NUL", "r");
    if (p != NULL) {
        if (fgets(buf, (int)n, p) != NULL) {
            size_t len = strlen(buf);
            while (len > 0u && (buf[len - 1u] == '\n' || buf[len - 1u] == '\r' ||
                                buf[len - 1u] == ' '  || buf[len - 1u] == '\t')) {
                buf[len - 1u] = '\0';
                len -= 1u;
            }
        }
        _pclose(p);
    }
    if (buf[0] == '\0') {
        snprintf(buf, n, "N/A");
    }
}

static int write_build_info(const char* path)
{
    FILE* f;
    char  comp[128];
    char  commit[128];

    compiler_string(comp, sizeof(comp));
    git_commit_string(commit, sizeof(commit));

    f = fopen(path, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: cannot write %s\n", path);
        return -1;
    }

    /* variant / patch_flags 一律来自库的真实返回值, 不得写死 */
    fprintf(f, "variant         = %s\n", NS_GetVariantName());
    fprintf(f, "patch_flags     = %d\n", NS_GetPatchFlags());
    fprintf(f, "sample_depth    = 24\n");
    fprintf(f, "cmake_generator = Visual Studio 17 2022\n");
    fprintf(f, "build_type      = Release\n");
    fprintf(f, "compiler        = %s\n", comp);
    fprintf(f, "c_flags         = /O2 /W3\n");
    fprintf(f, "git_commit      = %s\n", commit);
    fprintf(f, "build_time      = %s %s\n", __DATE__, __TIME__);

    if (fclose(f) != 0) {
        fprintf(stderr, "ERROR: cannot flush %s\n", path);
        return -1;
    }
    return 0;
}

/* ---------------- 运行上下文 ---------------- */

typedef struct {
    NSHandler*  h;
    const char* input_dir;
    const char* out_dir;
    LONGLONG    qpc_freq;           /* QueryPerformanceFrequency */
    FILE*       summary_fp;
    long long   total_elapsed_us;   /* 全部文件累计 */
    int32_t     total_frames;       /* 全部文件累计 */
    int         file_count;
} RunCtx;

/* ---------------- 单文件处理 ---------------- */

/* file_name: 输入目录中的原始文件名(含 .wav 扩展名)。
   name     : 去扩展名的基名, 用于输出文件命名。 */
static int process_one(RunCtx* ctx, const char* file_name, const char* name)
{
    int       rc = EXIT_OK;
    char      in_path[PATH_BUF_LEN];
    char      out_wav_path[PATH_BUF_LEN];
    char      metrics_path[PATH_BUF_LEN];
    int16_t*  samples = NULL;
    int16_t*  out16   = NULL;
    float*    in24    = NULL;
    FILE*     mf      = NULL;

    int32_t   n_frames = 0;
    int32_t   n_ch     = 0;
    int32_t   rate     = 0;
    int32_t   nframes_total;
    int32_t   fi, k, i;

    /* 段级累计量(统计口径 = 全部处理帧的 480*nframes_total 个样本, 末帧含补零) */
    double    sum_in_sq  = 0.0;
    double    sum_out_sq = 0.0;
    double    in_peak    = 0.0;
    double    out_peak   = 0.0;
    long long total_samples = 0;
    long long file_elapsed_us = 0;
    int       clamp_total = 0;

    /* 本文件的非有限值计数(非严格模式下用于逐文件 WARNING 汇总) */
    int       file_nan_frames = 0;
    int       file_nan_fields = 0;

    g_nan_file_counted = 0;                  /* 文件级去重标记复位 */

    if (snprintf(in_path, sizeof(in_path), "%s/%s", ctx->input_dir, file_name) < 0 ||
        snprintf(out_wav_path, sizeof(out_wav_path), "%s/%s.wav", ctx->out_dir, name) < 0 ||
        snprintf(metrics_path, sizeof(metrics_path), "%s/metrics_%s.csv", ctx->out_dir, name) < 0) {
        fprintf(stderr, "ERROR: path too long for '%s'\n", file_name);
        return EXIT_IO_FAILED;
    }

    rc = wav_read_pcm16(in_path, &samples, &n_frames, &n_ch, &rate);
    if (rc != 0) {
        fprintf(stderr, "ERROR: cannot read WAV '%s' (wav_io rc=%d)\n", in_path, rc);
        return EXIT_IO_FAILED;
    }
    rc = EXIT_OK;

    if (rate != NS_SAMPLE_RATE) {
        fprintf(stderr, "ERROR: '%s' sample rate is %d Hz, expected %d Hz\n",
                file_name, (int)rate, NS_SAMPLE_RATE);
        wav_free(samples);
        return EXIT_IO_FAILED;
    }
    if (n_ch < 1) {
        fprintf(stderr, "ERROR: '%s' has no audio channel\n", file_name);
        wav_free(samples);
        return EXIT_IO_FAILED;
    }

    /* 左声道 + 16bit -> 24bit 量纲 */
    in24 = (float*)malloc((size_t)n_frames * sizeof(float));
    out16 = (int16_t*)malloc((size_t)n_frames * sizeof(int16_t));
    if (in24 == NULL || out16 == NULL) {
        fprintf(stderr, "ERROR: out of memory for '%s' (%d frames)\n", file_name, (int)n_frames);
        free(in24);
        free(out16);
        wav_free(samples);
        return EXIT_IO_FAILED;
    }
    memset(out16, 0, (size_t)n_frames * sizeof(int16_t));

    for (i = 0; i < n_frames; ++i) {
        in24[i] = (float)samples[(size_t)i * (size_t)n_ch] * NS_SCALE_16_TO_24;
    }
    wav_free(samples);
    samples = NULL;

    mf = fopen(metrics_path, "wb");
    if (mf == NULL) {
        fprintf(stderr, "ERROR: cannot create %s\n", metrics_path);
        free(in24);
        free(out16);
        return EXIT_IO_FAILED;
    }
    fputs(kMetricsHeader, mf);

    nframes_total = (n_frames + NS_FRAME_LEN - 1) / NS_FRAME_LEN;

    for (fi = 0; fi < nframes_total; ++fi) {
        float   in_frame[NS_FRAME_LEN];
        float   out_frame[NS_FRAME_LEN];
        int32_t base  = fi * NS_FRAME_LEN;
        int32_t valid = n_frames - base;
        double  in_sq  = 0.0;
        double  out_sq = 0.0;
        float   in_rms;
        float   out_rms;
        float   atten_db;
        int32_t elapsed_us;
        NsDiagnostics diag;
        LARGE_INTEGER t0;
        LARGE_INTEGER t1;
        LONGLONG      ticks;
        int           st;

        if (valid > NS_FRAME_LEN) {
            valid = NS_FRAME_LEN;                /* 末帧不足 480 时补零(记录有效长度) */
        }
        for (k = 0; k < NS_FRAME_LEN; ++k) {
            in_frame[k] = (k < valid) ? in24[base + k] : 0.0f;
        }
        memset(out_frame, 0, sizeof(out_frame));

        /* 只计 NS_Process48k 本身的耗时 */
        QueryPerformanceCounter(&t0);
        st = NS_Process48k(ctx->h, in_frame, out_frame);
        QueryPerformanceCounter(&t1);
        ticks      = t1.QuadPart - t0.QuadPart;
        elapsed_us = (int32_t)((ticks * 1000000LL) / ctx->qpc_freq);

        if (st != NS_OK) {
            fprintf(stderr, "ERROR: NS_Process48k failed on '%s' frame %d (NsStatus=%d)\n",
                    name, (int)fi, st);
            rc = EXIT_PROCESS_FAILED;
            goto cleanup;
        }

        memset(&diag, 0, sizeof(diag));
        st = NS_GetDiagnostics(ctx->h, &diag);
        if (st != NS_OK) {
            fprintf(stderr, "ERROR: NS_GetDiagnostics failed on '%s' frame %d (NsStatus=%d)\n",
                    name, (int)fi, st);
            rc = EXIT_PROCESS_FAILED;
            goto cleanup;
        }

        /* 24bit 量纲 RMS(含补零样本) 与段级峰值 */
        for (k = 0; k < NS_FRAME_LEN; ++k) {
            double a = (double)in_frame[k];
            double b = (double)out_frame[k];
            in_sq  += a * a;
            out_sq += b * b;
            if (a < 0.0) { a = -a; }
            if (b < 0.0) { b = -b; }
            if (a > in_peak)  { in_peak  = a; }
            if (b > out_peak) { out_peak = b; }
        }
        in_rms  = (float)sqrt(in_sq / (double)NS_FRAME_LEN);
        out_rms = (float)sqrt(out_sq / (double)NS_FRAME_LEN);

        if (out_rms == 0.0f) {
            atten_db = 999.0f;                   /* 计划书 6.2: out_rms==0 记 999.0 */
        } else {
            atten_db = (float)(20.0 * log10((double)in_rms / (double)out_rms));
        }

        /* 9 个逐帧诊断字段的非有限值检查。
         * 严格模式: 保持原有语义(全套诊断 + 退出码 7 + 中止)。
         * 非严格模式: 计数并继续 —— 基线 NaN 是被测对象的真实行为(D8),
         *             不是 harness 故障, 但必须显式上报(stderr 逐文件 + 全局 SUMMARY)。 */
        {
            const float probe[9] = {
                in_rms, out_rms, atten_db,
                diag.signal_energy, diag.signal_spectral_sum,
                diag.diff_normalization, diag.speech_prob_mean,
                diag.prior_speech_prob, diag.gain_adjustment
            };
            int nbad = f_count_bad(probe, 9);

            if (nbad > 0) {
                if (g_strict_mode) {
                    fprintf(stderr, "ERROR: non-finite metric on '%s' frame %d\n", name, (int)fi);
                    fprintf(stderr, "  in_rms=%.9g out_rms=%.9g atten_db=%.9g\n",
                            (double)in_rms, (double)out_rms, (double)atten_db);
                    fprintf(stderr, "  signal_energy=%.9g signal_spectral_sum=%.9g\n",
                            (double)diag.signal_energy, (double)diag.signal_spectral_sum);
                    fprintf(stderr, "  diff_normalization=%.9g speech_prob_mean=%.9g\n",
                            (double)diag.diff_normalization, (double)diag.speech_prob_mean);
                    fprintf(stderr, "  prior_speech_prob=%.9g gain_adjustment=%.9g\n",
                            (double)diag.prior_speech_prob, (double)diag.gain_adjustment);
                    fprintf(stderr, "  clamp_hits=%d zero_frame_skips=%d num_analyzed_frames=%d\n",
                            (int)diag.clamp_hits, (int)diag.zero_frame_skips,
                            (int)diag.num_analyzed_frames);
                    rc = EXIT_NONFINITE;
                    goto cleanup;
                }
                ++g_nan_frames;
                g_nan_fields      += nbad;
                file_nan_frames   += 1;
                file_nan_fields   += nbad;
                if (!g_nan_file_counted) {
                    ++g_nan_files;
                    g_nan_file_counted = 1;
                }
                if (g_first_nan_frame < 0) {
                    g_first_nan_frame = (int)fi;
                    snprintf(g_first_nan_file_buf, sizeof(g_first_nan_file_buf), "%s", name);
                    g_first_nan_file = g_first_nan_file_buf;
                }
            }
        }

        {
            char b[9][24];
            fprintf(mf, "%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,%d\n",
                    (int)fi,
                    f_csv(in_rms, b[0], sizeof b[0]),
                    f_csv(out_rms, b[1], sizeof b[1]),
                    f_csv(atten_db, b[2], sizeof b[2]),
                    f_csv(diag.signal_energy, b[3], sizeof b[3]),
                    f_csv(diag.signal_spectral_sum, b[4], sizeof b[4]),
                    f_csv(diag.diff_normalization, b[5], sizeof b[5]),
                    f_csv(diag.speech_prob_mean, b[6], sizeof b[6]),
                    f_csv(diag.prior_speech_prob, b[7], sizeof b[7]),
                    f_csv(diag.gain_adjustment, b[8], sizeof b[8]),
                    (int)diag.clamp_hits,
                    (int)elapsed_us);
        }

        /* 24bit -> 16bit 还原并裁剪回原始帧数 */
        for (k = 0; k < NS_FRAME_LEN; ++k) {
            int32_t idx = base + k;
            long    v;
            if (idx >= n_frames) {
                break;
            }
            v = lrintf(out_frame[k] / 256.0f);
            if (v < (long)NS_CLAMP_LO) { v = (long)NS_CLAMP_LO; }
            if (v > (long)NS_CLAMP_HI) { v = (long)NS_CLAMP_HI; }
            out16[idx] = (int16_t)v;
        }

        sum_in_sq     += in_sq;
        sum_out_sq    += out_sq;
        total_samples += (long long)NS_FRAME_LEN;
        file_elapsed_us += (long long)elapsed_us;
        clamp_total   += (int)diag.clamp_hits;
    }

    if (fclose(mf) != 0) {
        mf = NULL;
        fprintf(stderr, "ERROR: cannot flush %s\n", metrics_path);
        rc = EXIT_IO_FAILED;
        goto cleanup;
    }
    mf = NULL;

    rc = wav_write_pcm16_mono(out_wav_path, out16, n_frames, NS_SAMPLE_RATE);
    if (rc != 0) {
        fprintf(stderr, "ERROR: cannot write %s (wav_io rc=%d)\n", out_wav_path, rc);
        rc = EXIT_IO_FAILED;
        goto cleanup;
    }
    rc = EXIT_OK;

    /* 段级汇总 */
    {
        double    duration_s    = (double)n_frames / (double)NS_SAMPLE_RATE;
        double    total_elapsed_ms = (double)file_elapsed_us / 1000.0;
        float     total_in_rms  = (float)sqrt(sum_in_sq  / (double)total_samples);
        float     total_out_rms = (float)sqrt(sum_out_sq / (double)total_samples);
        float     total_atten;
        float     in_peak_f     = (float)in_peak;
        float     out_peak_f    = (float)out_peak;

        if (total_out_rms == 0.0f) {
            total_atten = 999.0f;
        } else {
            total_atten = (float)(20.0 * log10((double)total_in_rms / (double)total_out_rms));
        }

        /* summary 的非有限值检查: 严格模式中止; 非严格模式计数并继续(仍写该行)。 */
        {
            const float probe[5] = {
                total_in_rms, total_out_rms, total_atten, in_peak_f, out_peak_f
            };
            int nbad = f_count_bad(probe, 5);

            if (nbad > 0) {
                if (g_strict_mode) {
                    fprintf(stderr, "ERROR: non-finite summary metric for '%s'\n", name);
                    rc = EXIT_NONFINITE;
                    goto cleanup;
                }
                g_nan_fields    += nbad;
                file_nan_fields += nbad;
                if (!g_nan_file_counted) {
                    ++g_nan_files;
                    g_nan_file_counted = 1;
                }
            }
        }

        {
            char b[5][24];
            fprintf(ctx->summary_fp, "%s,%.6g,%d,%.6g,%s,%s,%s,%s,%s,%d\n",
                    name,
                    duration_s,
                    (int)nframes_total,
                    total_elapsed_ms,
                    f_csv(total_in_rms, b[0], sizeof b[0]),
                    f_csv(total_out_rms, b[1], sizeof b[1]),
                    f_csv(total_atten, b[2], sizeof b[2]),
                    f_csv(in_peak_f, b[3], sizeof b[3]),
                    f_csv(out_peak_f, b[4], sizeof b[4]),
                    clamp_total);
        }

        printf("[%s] frames=%d duration=%.3fs atten_db=%.3f total_ms=%.3f\n",
               name, (int)nframes_total, duration_s, (double)total_atten, total_elapsed_ms);

        /* 逐文件上报(非严格模式): 该文件出现过非有限值就必须显式可见。 */
        if (!g_strict_mode && file_nan_fields > 0) {
            fprintf(stderr, "[%s] WARNING: non-finite metrics in %d frame(s), %d field(s)\n",
                    name, file_nan_frames, file_nan_fields);
        }

        ctx->total_elapsed_us += file_elapsed_us;
        ctx->total_frames     += nframes_total;
        ctx->file_count       += 1;
    }
    rc = EXIT_OK;

cleanup:
    if (mf != NULL) {
        fclose(mf);
    }
    free(in24);
    free(out16);
    return rc;
}

/* ---------------- main ---------------- */

int main(int argc, char** argv)
{
    const char*         cli_variant;
    const char*         level_str;
    const char*         input_dir;
    const char*         output_base;
    const VariantEntry* ve = NULL;
    const char*         lib_variant;
    int                 level = 0;
    int                 i;
    int                 rc;
    int                 patch_flags = 0;
    size_t              vi;

    char      variant_dir[PATH_BUF_LEN];
    char      out_dir[PATH_BUF_LEN];
    char      build_info_path[PATH_BUF_LEN];
    char      summary_path[PATH_BUF_LEN];
    NSHandler* h = NULL;
    LARGE_INTEGER qpf;
    LONGLONG  qpc_freq;
    NameList  files;
    RunCtx    ctx;
    FILE*     summary_fp = NULL;

    /* 非有限值策略: 默认容错 + 显式上报; NS_RUNNER_STRICT 存在且非空则恢复严格中止。 */
    {
        const char* strict_env = getenv("NS_RUNNER_STRICT");
        g_strict_mode = (strict_env != NULL && strict_env[0] != '\0') ? 1 : 0;
    }

    if (argc < 5) {
        fprintf(stderr, "usage: %s <variant> <level> <input_dir> <output_base_dir>\n", argv[0]);
        fprintf(stderr, "  <variant>         : %s\n", kUsageVariants);
        fprintf(stderr, "  <level>           : 0 | 1 | 3\n");
        fprintf(stderr, "  <input_dir>       : directory with 16bit/2ch/48kHz PCM *.wav\n");
        fprintf(stderr, "  <output_base_dir> : base output directory, e.g. ENC/Test/data\n");
        return EXIT_BAD_ARGS;
    }

    cli_variant = argv[1];
    level_str   = argv[2];
    input_dir   = argv[3];
    output_base = argv[4];

    for (vi = 0u; vi < (sizeof(kVariants) / sizeof(kVariants[0])); ++vi) {
        if (strcmp(cli_variant, kVariants[vi].cli) == 0) {
            ve = &kVariants[vi];
            break;
        }
    }
    if (ve == NULL) {
        fprintf(stderr, "ERROR: unknown variant '%s' (expected: %s)\n",
                cli_variant, kUsageVariants);
        return EXIT_BAD_ARGS;
    }

    if (strcmp(level_str, "0") == 0) {
        level = 0;
    } else if (strcmp(level_str, "1") == 0) {
        level = 1;
    } else if (strcmp(level_str, "3") == 0) {
        level = 3;
    } else {
        fprintf(stderr, "ERROR: invalid level '%s' (expected: 0 | 1 | 3)\n", level_str);
        return EXIT_BAD_ARGS;
    }

    /* 变体交叉校验: 链接错库绝不允许继续跑 */
    lib_variant = NS_GetVariantName();
    if (lib_variant == NULL) {
        lib_variant = "(null)";
    }
    if (strcmp(lib_variant, ve->lib_variant) != 0) {
        fprintf(stderr,
                "ERROR: variant mismatch: linked library reports '%s', "
                "command line asked for '%s' (expected library variant '%s')\n",
                lib_variant, cli_variant, ve->lib_variant);
        return EXIT_VARIANT_MISMATCH;
    }

    if (!dir_exists(input_dir)) {
        fprintf(stderr, "ERROR: input directory does not exist: %s\n", input_dir);
        return EXIT_NO_INPUT_DIR;
    }

    /* 预检: 确认该 level 可初始化, 保持"Init 失败先于创建输出目录"的原语义。
       真正用于处理数据的 handler 由下方文件循环为每个输入文件单独新建 ——
       绝不复用同一实例, 否则前一文件的跨帧状态会带入后一文件。 */
    h = NS_Init(level);
    if (h == NULL) {
        fprintf(stderr, "ERROR: NS_Init(%d) returned NULL\n", level);
        return EXIT_INIT_FAILED;
    }
    NS_Free(h);
    h = NULL;                       /* NS_Free(NULL) 安全, 下方早退路径沿用原调用 */

    if (snprintf(variant_dir, sizeof(variant_dir), "%s/%s", output_base, cli_variant) < 0 ||
        snprintf(out_dir, sizeof(out_dir), "%s/L%d", variant_dir, level) < 0 ||
        snprintf(build_info_path, sizeof(build_info_path), "%s/build_info.txt", variant_dir) < 0 ||
        snprintf(summary_path, sizeof(summary_path), "%s/summary_%s_L%d.csv",
                 out_dir, cli_variant, level) < 0) {
        fprintf(stderr, "ERROR: output path too long (base='%s')\n", output_base);
        NS_Free(h);
        return EXIT_MKDIR_FAILED;
    }

    if (ensure_dir_tree(out_dir) != 0) {
        fprintf(stderr, "ERROR: cannot create output directory: %s\n", out_dir);
        NS_Free(h);
        return EXIT_MKDIR_FAILED;
    }

    if (write_build_info(build_info_path) != 0) {
        NS_Free(h);
        return EXIT_IO_FAILED;
    }

    patch_flags = NS_GetPatchFlags();
    printf("variant        = %s (library reports '%s', patch_flags=%d)\n",
           cli_variant, lib_variant, patch_flags);
    printf("level          = %d\n", level);
    printf("input_dir      = %s\n", input_dir);
    printf("output_dir     = %s\n", out_dir);

    if (!QueryPerformanceFrequency(&qpf) || qpf.QuadPart <= 0) {
        fprintf(stderr, "ERROR: QueryPerformanceFrequency failed\n");
        NS_Free(h);
        return EXIT_IO_FAILED;
    }
    qpc_freq = qpf.QuadPart;

    memset(&files, 0, sizeof(files));
    if (collect_wavs(input_dir, &files) != 0) {
        fprintf(stderr, "ERROR: failed to enumerate *.wav in %s\n", input_dir);
        namelist_free(&files);
        NS_Free(h);
        return EXIT_IO_FAILED;
    }
    if (files.count == 0) {
        fprintf(stderr, "ERROR: no *.wav found in %s\n", input_dir);
        namelist_free(&files);
        NS_Free(h);
        return EXIT_IO_FAILED;
    }
    qsort(files.items, (size_t)files.count, sizeof(char*), name_cmp);

    printf("input files    = %d:", files.count);
    for (i = 0; i < files.count; ++i) {
        printf(" %s", files.items[i]);
    }
    printf("\n");

    summary_fp = fopen(summary_path, "wb");
    if (summary_fp == NULL) {
        fprintf(stderr, "ERROR: cannot create %s\n", summary_path);
        namelist_free(&files);
        NS_Free(h);
        return EXIT_IO_FAILED;
    }
    fputs(kSummaryHeader, summary_fp);

    memset(&ctx, 0, sizeof(ctx));
    ctx.input_dir  = input_dir;
    ctx.out_dir    = out_dir;
    ctx.qpc_freq   = qpc_freq;
    ctx.summary_fp = summary_fp;
    /* ctx.h 由文件循环逐文件设置(每个文件一份全新 handler), 此处保持 NULL */

    rc = EXIT_OK;
    for (i = 0; i < files.count; ++i) {
        const char* fn   = files.items[i];
        size_t      flen = strlen(fn);
        char        name[PATH_BUF_LEN];

        if (flen - 4u >= sizeof(name)) {
            fprintf(stderr, "ERROR: file name too long: %s\n", fn);
            rc = EXIT_IO_FAILED;
            break;
        }
        memcpy(name, fn, flen - 4u);
        name[flen - 4u] = '\0';

        /* 每个输入文件 = 一个独立用例: 全新初始化, 处理完立即销毁。
           初始化失败按既有语义以退出码 4 终止, 不静默跳过。 */
        h = NS_Init(level);
        if (h == NULL) {
            fprintf(stderr, "ERROR: NS_Init(%d) returned NULL for file '%s'\n", level, fn);
            rc = EXIT_INIT_FAILED;
            break;
        }
        ctx.h = h;

        rc = process_one(&ctx, fn, name);

        NS_Free(h);                 /* 状态不得跨文件残留 */
        h     = NULL;
        ctx.h = NULL;

        if (rc != EXIT_OK) {
            break;
        }
    }

    if (summary_fp != NULL) {
        if (fclose(summary_fp) != 0 && rc == EXIT_OK) {
            fprintf(stderr, "ERROR: cannot flush %s\n", summary_path);
            rc = EXIT_IO_FAILED;
        }
        summary_fp = NULL;
    }

    if (rc == EXIT_OK) {
        printf("TOTAL: files=%d frames=%d elapsed_ms=%.3f\n",
               ctx.file_count, (int)ctx.total_frames,
               (double)ctx.total_elapsed_us / 1000.0);
    }

    namelist_free(&files);
    NS_Free(h);                      /* h 此时恒为 NULL: handler 已在循环内逐个释放 */

    /* 硬校验(见执行记录 B-24): D8 的修复目标就是消除该 NaN。
     * 优化版若仍出现非有限值, 说明 D8 未生效或存在第二条独立路径 ——
     * 这必须作为硬失败暴露, 不能被容错机制掩盖。bit3 = D8。 */
    if ((patch_flags & 8) != 0 && g_nan_frames > 0) {
        fprintf(stderr,
                "ERROR: variant '%s' enables D8 but still produced %d non-finite frame(s)\n",
                cli_variant, g_nan_frames);
        if (g_first_nan_file != NULL) {
            fprintf(stderr, "       first occurrence: %s frame %d\n",
                    g_first_nan_file, g_first_nan_frame);
        }
        return EXIT_NONFINITE;
    }

    /* 全局非有限值汇总(仅非严格模式的成功路径)。
     * 退出码仍为 0: 非有限值是被测算法的真实行为(D8), 不是 harness 故障。 */
    if (rc == EXIT_OK && !g_strict_mode && g_nan_frames > 0) {
        fprintf(stderr,
                "SUMMARY: non-finite metrics present in %d file(s), %d frame(s), %d field(s)\n",
                g_nan_files, g_nan_frames, g_nan_fields);
        if (g_first_nan_file != NULL) {
            fprintf(stderr, "         first occurrence: %s frame %d\n",
                    g_first_nan_file, g_first_nan_frame);
        }
        fprintf(stderr,
                "         note: baseline NaN is the firmware-inherent D8 defect "
                "(see execution record B-24)\n");
    }

    return rc;
}
