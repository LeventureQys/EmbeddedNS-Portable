/* ============================================================================
 * arm_math_compat_test.c —— T1 兼容层自测程序(独立可执行, 含 main)
 *
 * 四项自测(计划书 T1 任务书):
 *   1. 往返回环 1000 组伪随机输入(种子 12345, 自实现 xorshift64),
 *      正变换 -> 逆变换, 逐点比对, 最大绝对误差 < 1e-6
 *   2. 边界 A: x[n] == 1  -> pOut[0]==256.0, pOut[1]==0.0, 其余为 0;
 *              逆变换还原为全 1
 *   3. 边界 B: x[n] == delta[n] -> pOut[0]==1.0, pOut[1]==1.0,
 *              其余实部为 1、虚部为 0; 逆变换还原为 delta
 *   4. numpy 交叉验证: 同一批伪随机输入与 numpy.fft.rfft 的 129 个复数值
 *      比对, 最大相对误差 < 1e-5(由 _compat_check.py 完成后半程)
 *
 * 本文件与实现文件 port/arm_math_compat.c 同属 arm_math_compat_test target 的
 * 两个源文件(见 CMakeLists.txt), 各自独立编译后链接。本文件只依赖公共头
 * port/arm_math_compat.h 暴露的接口, 不引用实现文件的任何内部宏或静态符号。
 *
 * 用法:
 *   arm_math_compat_test                      仅跑内置自测
 *   arm_math_compat_test --emit-vectors       写出 _compat_vectors.txt 与
 *                                             _compat_vectors_c.txt
 *                                             (供 _compat_check.py 交叉验证)
 *   arm_math_compat_test --vectors <in.txt>   读输入向量, 输出向量写
 *                                             _compat_vectors_resp.txt
 * 所有文件均写入自身源码所在目录(port/), 不落到别处。
 * ==========================================================================*/

#define _CRT_SECURE_NO_WARNINGS 1

#include "arm_math_compat.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

/* 变换长度 256, 与计划书 2.4 节规定一致。
   本文件不复用实现文件的内部宏, 以避免跨翻译单元的隐式依赖。 */
#define TEST_LEN      256
#define RAND_SEED     12345ULL
#define RAND_CASES    1000
#define TOL_ROUNDTRIP 1e-6
#define TOL_BOUNDARY  1e-6

/* --------------------------------------------------------------------------
 * 自实现伪随机数: xorshift64, 不依赖任何第三方库
 * ------------------------------------------------------------------------*/
static unsigned long long s_rng_state;

static unsigned long long rng_next(void)
{
    unsigned long long x = s_rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s_rng_state = x;
    return x;
}

static void rng_seed(unsigned long long seed)
{
    s_rng_state = (seed != 0ULL) ? seed : 88172645463325252ULL;
}

/* 归一化到 [-1, 1) */
static double rng_unit(void)
{
    unsigned long long v = rng_next() >> 11; /* 53 bit */
    return (double)v / 9007199254740992.0 * 2.0 - 1.0;
}

/* --------------------------------------------------------------------------
 * 输出路径: 由本文件的编译路径推出 port/ 目录(__FILE__ 为绝对或相对路径,
 * 均以本文件所在目录作为落盘位置)
 * ------------------------------------------------------------------------*/
static const char* port_dir(void)
{
    static char buf[2048];
    const char* f = __FILE__;
    const char* p1 = strrchr(f, '\\');
    const char* p2 = strrchr(f, '/');
    const char* slash = (p1 > p2) ? p1 : p2;
    if (slash == NULL) {
        return ".";
    }
    {
        size_t n = (size_t)(slash - f);
        if (n >= sizeof(buf)) {
            n = sizeof(buf) - 1;
        }
        memcpy(buf, f, n);
        buf[n] = '\0';
    }
    return buf;
}

static FILE* open_in_port(const char* name, const char* mode)
{
    char path[4096];
    sprintf_s(path, sizeof(path), "%s\\%s", port_dir(), name);
    return fopen(path, mode);
}

/* 边界测试实测值, 供日志留档使用 */
typedef struct {
    double dc;            /* pOut[0] */
    double nyq;           /* pOut[1] */
    double tail_max;      /* 其余分量最大绝对值 */
    double inv_err;       /* 逆变换还原偏差 */
} bound_result_t;

static bound_result_t s_ba;   /* 边界 A: x[n]==1 */
static bound_result_t s_bb;   /* 边界 B: x[n]==delta[n] */
static double s_extra_dim_err = 0.0;  /* 24bit 量纲往返误差(信息性) */
static int s_extra_untouched = 1;     /* 输入缓冲是否未被改写 */
static int s_extra_len_ok = 1;        /* init 长度校验是否正确 */

/* --------------------------------------------------------------------------
 * 自测 1: 往返回环
 * ------------------------------------------------------------------------*/
static double test_roundtrip(double* out_in_max, double* out_out_max)
{
    static float x[TEST_LEN];
    static float spec[TEST_LEN];
    static float back[TEST_LEN];
    static double keep[TEST_LEN];
    arm_rfft_fast_instance_f32 S;
    int c, i;
    double max_abs = 0.0, max_abs_in = 0.0, max_abs_out = 0.0;

    if (arm_rfft_fast_init_f32(&S, TEST_LEN) != ARM_MATH_SUCCESS) {
        printf("  [FAIL] arm_rfft_fast_init_f32(256) 未返回 ARM_MATH_SUCCESS\n");
        return -1.0;
    }

    rng_seed(RAND_SEED);
    for (c = 0; c < RAND_CASES; ++c) {
        for (i = 0; i < TEST_LEN; ++i) {
            double v = rng_unit();       /* 参考实数序列 */
            keep[i] = v;
            x[i] = (float)v;             /* 兼容层接口为 float */
        }
        arm_rfft_fast_f32(&S, x, spec, 0);
        arm_rfft_fast_f32(&S, spec, back, 1);
        for (i = 0; i < TEST_LEN; ++i) {
            double e = (double)back[i] - (double)x[i];  /* 对 float 输入比对 */
            double e2 = (double)back[i] - keep[i];      /* 对 double 真值比对 */
            if (e < 0.0) e = -e;
            if (e2 < 0.0) e2 = -e2;
            if (e > max_abs) max_abs = e;
            if (e2 > max_abs) max_abs = e2;
            if (e > max_abs_in) max_abs_in = e;
            if (e2 > max_abs_out) max_abs_out = e2;
        }
    }
    *out_in_max = max_abs_in;
    *out_out_max = max_abs_out;
    return max_abs;
}

/* --------------------------------------------------------------------------
 * 自测 2: 边界 A  x[n] == 1
 * ------------------------------------------------------------------------*/
static int test_boundary_a(void)
{
    static float x[TEST_LEN];
    static float spec[TEST_LEN];
    static float back[TEST_LEN];
    arm_rfft_fast_instance_f32 S;
    int i, ok = 1;
    double max_tail = 0.0, max_inv_err = 0.0;

    arm_rfft_fast_init_f32(&S, TEST_LEN);
    for (i = 0; i < TEST_LEN; ++i) {
        x[i] = 1.0f;
    }
    arm_rfft_fast_f32(&S, x, spec, 0);
    s_ba.dc = (double)spec[0];
    s_ba.nyq = (double)spec[1];

    printf("    边界A: pOut[0]=%.9f (期望 256)   pOut[1]=%.9f (期望 0)\n",
           s_ba.dc, s_ba.nyq);
    if (fabs(s_ba.dc - 256.0) > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] pOut[0] 偏离 256.0\n");
    }
    if (fabs(s_ba.nyq - 0.0) > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] pOut[1](Nyquist) 偏离 0.0\n");
    }
    for (i = 2; i < TEST_LEN; ++i) {
        double v = fabs((double)spec[i]);
        if (v > max_tail) max_tail = v;
    }
    s_ba.tail_max = max_tail;
    printf("    边界A: 其余 %d 个分量最大绝对值 = %.3e (期望 0)\n",
           TEST_LEN - 2, max_tail);
    if (max_tail > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] 非 DC/非 Nyquist 分量非零\n");
    }

    arm_rfft_fast_f32(&S, spec, back, 1);
    for (i = 0; i < TEST_LEN; ++i) {
        double v = fabs((double)back[i] - 1.0);
        if (v > max_inv_err) max_inv_err = v;
    }
    s_ba.inv_err = max_inv_err;
    printf("    边界A: 逆变换还原全 1 的最大偏差 = %.3e (期望 0, 阈值 %.0e)\n",
           max_inv_err, TOL_BOUNDARY);
    if (max_inv_err > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] 逆变换未还原为全 1\n");
    }
    return ok;
}

/* --------------------------------------------------------------------------
 * 自测 3: 边界 B  x[n] == delta[n]
 * ------------------------------------------------------------------------*/
static int test_boundary_b(void)
{
    static float x[TEST_LEN];
    static float spec[TEST_LEN];
    static float back[TEST_LEN];
    arm_rfft_fast_instance_f32 S;
    int i, ok = 1;
    double max_re_err = 0.0, max_im_err = 0.0, max_inv_err = 0.0;

    arm_rfft_fast_init_f32(&S, TEST_LEN);
    for (i = 0; i < TEST_LEN; ++i) {
        x[i] = 0.0f;
    }
    x[0] = 1.0f;
    arm_rfft_fast_f32(&S, x, spec, 0);
    s_bb.dc = (double)spec[0];
    s_bb.nyq = (double)spec[1];

    printf("    边界B: pOut[0]=%.9f (期望 1)   pOut[1]=%.9f (期望 1)\n",
           s_bb.dc, s_bb.nyq);
    if (fabs(s_bb.dc - 1.0) > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] pOut[0] 偏离 1.0\n");
    }
    if (fabs(s_bb.nyq - 1.0) > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] pOut[1](Nyquist) 偏离 1.0\n");
    }
    for (i = 1; i < TEST_LEN / 2; ++i) {   /* k = 1..127, 共 128 个频点 k=0..128 去掉两端 */
        double er = fabs((double)spec[2 * i] - 1.0);
        double ei = fabs((double)spec[2 * i + 1] - 0.0);
        if (er > max_re_err) max_re_err = er;
        if (ei > max_im_err) max_im_err = ei;
    }
    /* 边界 B 的 tail 字段复用为 "实部偏离 1 与虚部偏离 0 的较大者" */
    s_bb.tail_max = (max_re_err > max_im_err) ? max_re_err : max_im_err;
    printf("    边界B: k=1..127 实部偏离 1 的最大值 = %.3e\n", max_re_err);
    printf("    边界B: k=1..127 虚部偏离 0 的最大值 = %.3e\n", max_im_err);
    if (max_re_err > TOL_BOUNDARY || max_im_err > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] 非 DC/Nyquist 分量不是 1+j0\n");
    }

    arm_rfft_fast_f32(&S, spec, back, 1);
    for (i = 0; i < TEST_LEN; ++i) {
        double want = (i == 0) ? 1.0 : 0.0;
        double v = fabs((double)back[i] - want);
        if (v > max_inv_err) max_inv_err = v;
    }
    s_bb.inv_err = max_inv_err;
    printf("    边界B: 逆变换还原 delta 的最大偏差 = %.3e (期望 0)\n",
           max_inv_err);
    if (max_inv_err > TOL_BOUNDARY) {
        ok = 0;
        printf("    [FAIL] 逆变换未还原为 delta\n");
    }
    return ok;
}

/* --------------------------------------------------------------------------
 * 额外诊断(不作为验收项): init 长度校验 + 输入缓冲不被改写 + 量纲尺度往返
 * ------------------------------------------------------------------------*/
static int test_extra(void)
{
    static float x[TEST_LEN];
    static float spec[TEST_LEN];
    static float back[TEST_LEN];
    arm_rfft_fast_instance_f32 S;
    int i, ok = 1;
    int untouched = 1;
    double max_abs = 0.0;

    /* init 长度校验 */
    if (arm_rfft_fast_init_f32(&S, 128) == ARM_MATH_SUCCESS) {
        ok = 0;
        s_extra_len_ok = 0;
        printf("    [FAIL] init(128) 应返回非零\n");
    } else {
        printf("    init(128) 返回非零: 正确\n");
    }
    if (arm_rfft_fast_init_f32(&S, 512) == ARM_MATH_SUCCESS) {
        ok = 0;
        s_extra_len_ok = 0;
        printf("    [FAIL] init(512) 应返回非零\n");
    } else {
        printf("    init(512) 返回非零: 正确\n");
    }
    arm_rfft_fast_init_f32(&S, TEST_LEN);

    /* 输入缓冲 p 不被改写 */
    rng_seed(777ULL);
    for (i = 0; i < TEST_LEN; ++i) {
        x[i] = (float)rng_unit();
    }
    {
        static float copy[TEST_LEN];
        memcpy(copy, x, sizeof(x));
        arm_rfft_fast_f32(&S, x, spec, 0);
        for (i = 0; i < TEST_LEN; ++i) {
            if (x[i] != copy[i]) {
                untouched = 0;
            }
        }
    }
    printf("    输入缓冲 p 在正变换后未被改写: %s\n",
           untouched ? "是" : "否");
    s_extra_untouched = untouched;
    if (!untouched) {
        ok = 0;
    }

    /* 量纲尺度(24bit, +-8388608)往返, 仅报告, 阈值放宽到 float 可以达到的水平 */
    rng_seed(2024ULL);
    for (i = 0; i < TEST_LEN; ++i) {
        x[i] = (float)(rng_unit() * 8388608.0);
    }
    arm_rfft_fast_f32(&S, x, spec, 0);
    arm_rfft_fast_f32(&S, spec, back, 1);
    for (i = 0; i < TEST_LEN; ++i) {
        double e = fabs((double)back[i] - (double)x[i]);
        if (e > max_abs) max_abs = e;
    }
    printf("    量纲尺度(+-8388608)往返最大绝对误差 = %.3e "
           "(float 精度下的信息性数值, 不参与验收)\n", max_abs);
    s_extra_dim_err = max_abs;
    return ok;
}

/* --------------------------------------------------------------------------
 * numpy 交叉验证数据交换
 * ------------------------------------------------------------------------*/
static int emit_vectors(const char* self_exe_path)
{
    static float x[TEST_LEN];
    static float spec[TEST_LEN];
    arm_rfft_fast_instance_f32 S;
    FILE* fp;
    int c, i;

    fp = open_in_port("_compat_vectors.txt", "w");
    if (fp == NULL) {
        printf("  [FAIL] 无法创建 _compat_vectors.txt\n");
        return 1;
    }
    fprintf(fp, "%d %d\n", RAND_CASES, TEST_LEN);
    rng_seed(RAND_SEED);
    for (c = 0; c < RAND_CASES; ++c) {
        for (i = 0; i < TEST_LEN; ++i) {
            x[i] = (float)rng_unit();
            fprintf(fp, "%.9e%c", (double)x[i], (i + 1 == TEST_LEN) ? '\n' : ' ');
        }
    }
    fclose(fp);

    /* 同时把第一组输入交给兼容层, 输出向量留在 _compat_vectors_c.txt,
       Python 侧据此与 numpy.fft.rfft 逐点比对(见 _compat_check.py) */
    fp = open_in_port("_compat_vectors_c.txt", "w");
    if (fp == NULL) {
        printf("  [FAIL] 无法创建 _compat_vectors_c.txt\n");
        return 1;
    }
    fprintf(fp, "%d %d\n", RAND_CASES, TEST_LEN);
    arm_rfft_fast_init_f32(&S, TEST_LEN);
    rng_seed(RAND_SEED);
    for (c = 0; c < RAND_CASES; ++c) {
        for (i = 0; i < TEST_LEN; ++i) {
            x[i] = (float)rng_unit();
        }
        arm_rfft_fast_f32(&S, x, spec, 0);
        for (i = 0; i < TEST_LEN; ++i) {
            fprintf(fp, "%.9e%c", (double)spec[i], (i + 1 == TEST_LEN) ? '\n' : ' ');
        }
    }
    fclose(fp);

    if (self_exe_path != NULL) {
        printf("  已写出向量文件到 %s\\\n", port_dir());
    }
    return 0;
}

/* --------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
    double rt_max = 0.0, rt_in = 0.0, rt_out = 0.0;
    int ok_a, ok_b, ok_x, ok = 1;
    FILE* log;

    printf("=== T1 CMSIS RFFT 兼容层自测 ===\n");
    printf("实现: 256 点 double radix-2 Cooley-Tukey, 正变换无缩放 / 逆变换含 1/256\n");
    printf("随机源: xorshift64, 种子 %llu, %d 组\n\n",
           (unsigned long long)RAND_SEED, RAND_CASES);

    if (argc > 1 && strcmp(argv[1], "--emit-vectors") == 0) {
        return emit_vectors((argc > 2) ? argv[2] : NULL);
    }
    if (argc > 2 && strcmp(argv[1], "--vectors") == 0) {
        static float x[TEST_LEN];
        static float spec[TEST_LEN];
        arm_rfft_fast_instance_f32 S;
        FILE* fin = fopen(argv[2], "r");
        FILE* fout;
        int ncases = 0, nlen = 0;
        int c, k;
        if (fin == NULL) {
            printf("  [FAIL] 无法打开 %s\n", argv[2]);
            return 1;
        }
        if (fscanf(fin, "%d %d", &ncases, &nlen) != 2 || nlen != TEST_LEN) {
            printf("  [FAIL] 向量文件头非法\n");
            fclose(fin);
            return 1;
        }
        fout = open_in_port("_compat_vectors_resp.txt", "w");
        if (fout == NULL) {
            printf("  [FAIL] 无法创建 _compat_vectors_resp.txt\n");
            fclose(fin);
            return 1;
        }
        fprintf(fout, "%d %d\n", ncases, nlen);
        arm_rfft_fast_init_f32(&S, TEST_LEN);
        for (c = 0; c < ncases; ++c) {
            for (k = 0; k < TEST_LEN; ++k) {
                double v = 0.0;
                if (fscanf(fin, "%lf", &v) != 1) {
                    printf("  [FAIL] 向量数据不足\n");
                    fclose(fin);
                    fclose(fout);
                    return 1;
                }
                x[k] = (float)v;
            }
            arm_rfft_fast_f32(&S, x, spec, 0);
            for (k = 0; k < TEST_LEN; ++k) {
                fprintf(fout, "%.9e%c", (double)spec[k],
                        (k + 1 == TEST_LEN) ? '\n' : ' ');
            }
        }
        fclose(fin);
        fclose(fout);
        printf("  已写出 %d 组输出向量到 _compat_vectors_resp.txt\n", ncases);
        return 0;
    }

    printf("[1] 往返回环 %d 组\n", RAND_CASES);
    rt_max = test_roundtrip(&rt_in, &rt_out);
    if (rt_max < 0.0) {
        ok = 0;
        printf("  跳过\n\n");
    } else {
        printf("    最大绝对误差(对 float 输入) = %.6e\n", rt_in);
        printf("    最大绝对误差(对 double 真值) = %.6e\n", rt_out);
        printf("    判定阈值 %.0e -> %s\n\n", TOL_ROUNDTRIP,
               (rt_max < TOL_ROUNDTRIP) ? "PASS" : "FAIL");
        if (!(rt_max < TOL_ROUNDTRIP)) {
            ok = 0;
        }
    }

    printf("[2] 边界 A: x[n] == 1\n");
    ok_a = test_boundary_a();
    printf("    判定 -> %s\n\n", ok_a ? "PASS" : "FAIL");
    ok = ok && ok_a;

    printf("[3] 边界 B: x[n] == delta[n]\n");
    ok_b = test_boundary_b();
    printf("    判定 -> %s\n\n", ok_b ? "PASS" : "FAIL");
    ok = ok && ok_b;

    printf("[4] 附加诊断\n");
    ok_x = test_extra();
    printf("    判定 -> %s\n\n", ok_x ? "PASS" : "FAIL");
    ok = ok && ok_x;

    printf("[5] numpy 交叉验证: 由 _compat_check.py 完成, 见同目录比对输出\n");
    printf("    向量文件: _compat_vectors.txt(输入) / _compat_vectors_c.txt(C 输出)\n\n");

    printf("=== 内置自测结论: %s ===\n", ok ? "全部通过" : "存在失败项");

    /* 原始输出留档 */
    log = open_in_port("arm_math_compat_test_output.txt", "w");
    if (log != NULL) {
        fprintf(log, "=== T1 CMSIS RFFT 兼容层自测 原始输出留档 ===\n");
        fprintf(log, "实现: 256 点 double radix-2 Cooley-Tukey FFT(含位反转重排)\n");
        fprintf(log, "正向: 无缩放标准 DFT; 逆向: 标准 IDFT(含 1/256)\n");
        fprintf(log, "随机源: 自实现 xorshift64, 种子 %llu, %d 组, 输入幅度 [-1,1)\n\n",
                (unsigned long long)RAND_SEED, RAND_CASES);
        fprintf(log, "[1] 往返测试\n");
        fprintf(log, "    最大绝对误差(对 float 输入) = %.6e\n", rt_in);
        fprintf(log, "    最大绝对误差(对 double 真值) = %.6e\n", rt_out);
        fprintf(log, "    阈值 %.0e -> %s\n\n", TOL_ROUNDTRIP,
                (rt_in < TOL_ROUNDTRIP) ? "PASS" : "FAIL");
        fprintf(log, "[2] 边界A x[n]==1 -> %s\n", ok_a ? "PASS" : "FAIL");
        fprintf(log, "    实测 pOut[0] = %.9f (期望 256.0)\n", s_ba.dc);
        fprintf(log, "    实测 pOut[1] = %.9f (期望 0.0)\n", s_ba.nyq);
        fprintf(log, "    其余 254 个分量最大绝对值 = %.6e (期望 0)\n", s_ba.tail_max);
        fprintf(log, "    逆变换还原全 1 的最大偏差 = %.6e\n\n", s_ba.inv_err);
        fprintf(log, "[3] 边界B x[n]==delta[n] -> %s\n", ok_b ? "PASS" : "FAIL");
        fprintf(log, "    实测 pOut[0] = %.9f (期望 1.0)\n", s_bb.dc);
        fprintf(log, "    实测 pOut[1] = %.9f (期望 1.0)\n", s_bb.nyq);
        fprintf(log, "    k=1..127 实部偏离 1 / 虚部偏离 0 的最大值 = %.6e\n",
                s_bb.tail_max);
        fprintf(log, "    逆变换还原 delta 的最大偏差 = %.6e\n\n", s_bb.inv_err);
        fprintf(log, "[4] 附加诊断 -> %s\n", ok_x ? "PASS" : "FAIL");
        fprintf(log, "    init(128)/init(512) 返回非零: %s\n",
                s_extra_len_ok ? "是" : "否");
        fprintf(log, "    正变换不改写输入缓冲 p: %s\n",
                s_extra_untouched ? "是" : "否");
        fprintf(log, "    24bit 量纲(+-8388608)往返最大绝对误差 = %.3e "
                     "(float 精度信息值, 不参与验收)\n\n", s_extra_dim_err);
        fprintf(log, "[5] numpy 交叉验证\n");
        fprintf(log, "    由 _compat_check.py 读 _compat_vectors.txt(输入) 与\n");
        fprintf(log, "    _compat_vectors_c.txt(C 输出), 与 numpy.fft.rfft 逐点比对,\n");
        fprintf(log, "    最大相对误差写入本文件末尾(见下方 numpy 段)\n\n");
        fprintf(log, "内置自测结论: %s\n", ok ? "全部通过" : "存在失败项");
        fclose(log);
        printf("原始输出已写入 %s\\arm_math_compat_test_output.txt\n", port_dir());
    } else {
        printf("[WARN] 未能写出 arm_math_compat_test_output.txt\n");
    }

    /* 让 main 里未使用的变量不产生警告(仅形式用途) */
    (void)0;
    return ok ? 0 : 1;
}
