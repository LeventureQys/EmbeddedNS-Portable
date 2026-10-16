/* T2 smoke test —— 临时验证程序(不参与库构建: CMakeLists 的 GLOB 只收 src/*.c
   与 port/arm_math_compat.c)。
   验证 ns_api.h 的错误处理契约与 480 样本帧处理链路是否可用。 */

#include <stdio.h>
#include <string.h>

#include "ns_api.h"

#define FRAME_SIZE 480

static int g_fails = 0;

#define CHECK(cond, fmt, ...)                                                  \
    do {                                                                       \
        int ok_ = (cond) ? 1 : 0;                                              \
        printf("[%s] " fmt "\n", ok_ ? "PASS" : "FAIL", ##__VA_ARGS__);        \
        if (!ok_) { g_fails++; }                                               \
    } while (0)

static void dump_diag(const char* tag, const NsDiagnostics* d)
{
    printf("  diag(%s): num_analyzed_frames=%d signal_energy=%g "
           "signal_spectral_sum=%g\n",
           tag, (int)d->num_analyzed_frames, (double)d->signal_energy,
           (double)d->signal_spectral_sum);
    printf("  diag(%s): diff_normalization=%g speech_prob_mean=%g "
           "prior_speech_prob=%g\n",
           tag, (double)d->diff_normalization, (double)d->speech_prob_mean,
           (double)d->prior_speech_prob);
    printf("  diag(%s): gain_adjustment=%g clamp_hits=%d zero_frame_skips=%d\n",
           tag, (double)d->gain_adjustment, (int)d->clamp_hits,
           (int)d->zero_frame_skips);
}

int main(void)
{
    NSHandler* h;
    NsDiagnostics d;
    float in[FRAME_SIZE];
    float out[FRAME_SIZE];
    int i, rc;

    printf("=== ns_api build info ===\n");
    printf("variant=%s patch_flags=%d\n", NS_GetVariantName(), NS_GetPatchFlags());

    printf("\n=== 1. NS_Init level guard ===\n");
    CHECK(NS_Init(-1) == NULL, "NS_Init(-1) == NULL");
    CHECK(NS_Init(4) == NULL, "NS_Init(4)  == NULL");

    h = NS_Init(3);
    CHECK(h != NULL, "NS_Init(3)  != NULL%s",
          h ? "" : "  <-- FFT compat layer missing or init failed");
    if (h == NULL) {
        printf("\nABORT: cannot continue without a handle (fails=%d)\n", g_fails);
        return 1;
    }
    CHECK(NS_Init(0) != NULL, "NS_Init(0)  != NULL");

    printf("\n=== 2. null-argument contract ===\n");
    memset(out, 0, sizeof(out));
    rc = NS_Process48k(NULL, in, out);
    CHECK(rc == NS_ERR_NULL_HANDLE, "NS_Process48k(NULL, in, out) = %d (want %d)",
          rc, NS_ERR_NULL_HANDLE);

    rc = NS_Process48k(h, NULL, out);
    CHECK(rc == NS_ERR_NULL_BUFFER, "NS_Process48k(h, NULL, out) = %d (want %d)",
          rc, NS_ERR_NULL_BUFFER);

    rc = NS_Process48k(h, in, NULL);
    CHECK(rc == NS_ERR_NULL_BUFFER, "NS_Process48k(h, in, NULL) = %d (want %d)",
          rc, NS_ERR_NULL_BUFFER);

    rc = NS_GetDiagnostics(NULL, &d);
    CHECK(rc == NS_ERR_NULL_HANDLE, "NS_GetDiagnostics(NULL, &d) = %d (want %d)",
          rc, NS_ERR_NULL_HANDLE);

    rc = NS_GetDiagnostics(h, NULL);
    CHECK(rc == NS_ERR_NULL_BUFFER, "NS_GetDiagnostics(h, NULL) = %d (want %d)",
          rc, NS_ERR_NULL_BUFFER);

    NS_Free(NULL);
    printf("[PASS] NS_Free(NULL) did not crash\n");

    printf("\n=== 3. silent frame (480 x 0.0f) ===\n");
    memset(in, 0, sizeof(in));
    memset(out, 0, sizeof(out));
    rc = NS_Process48k(h, in, out);
    CHECK(rc == NS_OK, "NS_Process48k(h, zero_frame) = %d (want %d)", rc, NS_OK);
    rc = NS_GetDiagnostics(h, &d);
    CHECK(rc == NS_OK, "NS_GetDiagnostics after silent frame = %d", rc);
    dump_diag("silent", &d);
    CHECK(d.zero_frame_skips == 1, "zero_frame_skips == 1 (got %d)",
          (int)d.zero_frame_skips);

    printf("\n=== 4. constant frame (480 x 1000.0f), 3 frames back-to-back ===\n");
    for (i = 0; i < FRAME_SIZE; i++) {
        in[i] = 1000.0f;
    }
    for (i = 0; i < 3; i++) {
        memset(out, 0, sizeof(out));
        rc = NS_Process48k(h, in, out);
        CHECK(rc == NS_OK, "frame %d: NS_Process48k = %d (want %d)", i + 1, rc, NS_OK);
        rc = NS_GetDiagnostics(h, &d);
        CHECK(rc == NS_OK, "frame %d: NS_GetDiagnostics = %d", i + 1, rc);
        printf("  frame %d out[0]=%g out[1]=%g out[479]=%g\n", i + 1,
               (double)out[0], (double)out[1], (double)out[479]);
        dump_diag("const", &d);
        CHECK(d.signal_energy == d.signal_energy, "frame %d: signal_energy is not NaN", i + 1);
        CHECK(d.speech_prob_mean == d.speech_prob_mean,
              "frame %d: speech_prob_mean is not NaN", i + 1);
    }
    CHECK(d.num_analyzed_frames == 2,
          "after 3 non-silent frames num_analyzed_frames == 2 (got %d)",
          (int)d.num_analyzed_frames);
    CHECK(d.zero_frame_skips == 1,
          "zero_frame_skips still 1 after 3 non-silent frames (got %d)",
          (int)d.zero_frame_skips);

    printf("\n=== 5. ramp input, 100 frames (stability) ===\n");
    for (i = 0; i < 100; i++) {
        int k;
        for (k = 0; k < FRAME_SIZE; k++) {
            in[k] = (float)((k * 37 + i * 11) % 4096) - 2048.0f;
        }
        rc = NS_Process48k(h, in, out);
        if (rc != NS_OK) {
            CHECK(0, "ramp frame %d: NS_Process48k = %d", i + 1, rc);
            break;
        }
    }
    CHECK(rc == NS_OK, "100 ramp frames completed without error");
    rc = NS_GetDiagnostics(h, &d);
    dump_diag("ramp100", &d);

    printf("\n=== 6. in-place (in == out) ===\n");
    for (i = 0; i < FRAME_SIZE; i++) {
        in[i] = (float)i;
    }
    rc = NS_Process48k(h, in, in);
    CHECK(rc == NS_OK, "NS_Process48k(h, in, in) = %d (want %d)", rc, NS_OK);

    NS_Free(h);
    printf("[PASS] NS_Free(h) completed\n");

    printf("\n=== RESULT: %s (fails=%d) ===\n", g_fails ? "FAIL" : "ALL PASS", g_fails);
    return g_fails ? 1 : 0;
}
