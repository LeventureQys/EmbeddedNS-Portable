/* WAV 读写 —— 仅 16bit PCM。
 *
 * 读取: 支持标准 RIFF/WAVE 布局, 按块遍历, fmt 与 data 之间/之前存在的
 *       其它块(如 LIST)一律按块长跳过; data 之后的内容不在本次处理范围内。
 *       实测输入文件(Test_Audio/AudioSample-48000hz/*.wav) 为标准 44 字节头,
 *       data 之后附有 Adobe BWF 元数据块(LIST/bext/iXML/_PMX), 按块长读取
 *       即天然忽略。
 * 写出: 单声道 16bit PCM, 标准 44 字节头。
 * 端序: 小端(目标平台 Windows/x86-64 为小端, 直接按内存布局读写)。
 */

/* MSVC 把 fopen/strcpy 等标准 C 函数标为"不安全"; 本模块为工具程序,
   不使用 _s 变体, 在包含任何 CRT 头之前关闭该弃用警告(/W3 零警告要求)。 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "wav_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- 小端读写辅助 ---------------- */

static uint16_t rd_u16le(const unsigned char* p)
{
    return (uint16_t)((uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8));
}

static uint32_t rd_u32le(const unsigned char* p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static void wr_u16le(unsigned char* p, uint16_t v)
{
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
}

static void wr_u32le(unsigned char* p, uint32_t v)
{
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}

/* ---------------- 读 ---------------- */

int wav_read_pcm16(const char* path, int16_t** samples, int32_t* n_frames, int32_t* n_ch, int32_t* rate)
{
    FILE*          f;
    unsigned char  riff[12];
    unsigned char  chunk[8];
    int            have_fmt  = 0;
    int            have_data = 0;
    int            fmt_tag   = 0;
    int            bits      = 0;
    int            channels  = 0;
    uint32_t       sample_rate = 0u;
    uint32_t       data_size   = 0u;
    long           data_offset = 0;
    int32_t        frames;
    long           total;
    int16_t*       buf;

    if (path == NULL || samples == NULL || n_frames == NULL || n_ch == NULL || rate == NULL) {
        return -1;
    }
    *samples  = NULL;
    *n_frames = 0;
    *n_ch     = 0;
    *rate     = 0;

    f = fopen(path, "rb");
    if (f == NULL) {
        return -2;
    }

    if (fread(riff, 1u, 12u, f) != 12u) { fclose(f); return -3; }
    if (memcmp(riff, "RIFF", 4u) != 0 || memcmp(riff + 8, "WAVE", 4u) != 0) {
        fclose(f);
        return -4;
    }

    for (;;) {
        uint32_t csz;

        if (fread(chunk, 1u, 8u, f) != 8u) {
            break;                      /* 到达 EOF: 未见到 data */
        }
        csz = rd_u32le(chunk + 4);
        if (csz > 0x7FFFFFFEu) {        /* 块长越界, 拒绝 */
            fclose(f);
            return -5;
        }

        if (memcmp(chunk, "fmt ", 4u) == 0) {
            unsigned char fb[16];
            long          skip;

            if (csz < 16u) { fclose(f); return -6; }
            if (fread(fb, 1u, 16u, f) != 16u) { fclose(f); return -7; }

            fmt_tag     = (int)rd_u16le(fb);
            channels    = (int)rd_u16le(fb + 2);
            sample_rate = rd_u32le(fb + 4);
            bits        = (int)rd_u16le(fb + 14);
            have_fmt    = 1;

            skip = (long)(csz - 16u) + (long)(csz & 1u);   /* 扩展区 + 块对齐字节 */
            if (skip > 0 && fseek(f, skip, SEEK_CUR) != 0) { fclose(f); return -8; }
        } else if (memcmp(chunk, "data", 4u) == 0) {
            data_offset = ftell(f);
            if (data_offset < 0) { fclose(f); return -9; }
            data_size = csz;
            have_data = 1;
            break;
        } else {
            long skip = (long)csz + (long)(csz & 1u);      /* 其它块: 按块长跳过 */
            if (fseek(f, skip, SEEK_CUR) != 0) { fclose(f); return -10; }
        }
    }

    if (!have_fmt)  { fclose(f); return -11; }
    if (!have_data) { fclose(f); return -12; }
    if (fmt_tag != 1)   { fclose(f); return -13; }   /* 非 PCM(WAVE_FORMAT_EXTENSIBLE 等) */
    if (bits != 16)     { fclose(f); return -14; }
    if (channels <= 0 || channels > 32) { fclose(f); return -15; }
    if (sample_rate == 0u) { fclose(f); return -16; }

    frames = (int32_t)(data_size / ((uint32_t)channels * 2u));
    if (frames <= 0) { fclose(f); return -17; }
    if ((uint32_t)frames * (uint32_t)channels * 2u != data_size) {
        fprintf(stderr,
                "[wav_io] warning: %s: data size %u bytes is not a whole number of "
                "frames, truncated to %d frames\n",
                path, data_size, (int)frames);
    }

    total = (long)frames * (long)channels;
    buf   = (int16_t*)malloc((size_t)total * sizeof(int16_t));
    if (buf == NULL) { fclose(f); return -18; }

    if (fseek(f, data_offset, SEEK_SET) != 0) { free(buf); fclose(f); return -19; }
    if (fread(buf, sizeof(int16_t), (size_t)total, f) != (size_t)total) {
        free(buf);
        fclose(f);
        return -20;
    }
    fclose(f);

    *samples  = buf;
    *n_frames = frames;
    *n_ch     = channels;
    *rate     = (int32_t)sample_rate;
    return 0;
}

/* ---------------- 写 ---------------- */

int wav_write_pcm16_mono(const char* path, const int16_t* samples, int32_t n_frames, int32_t rate)
{
    FILE*         f;
    unsigned char hdr[44];
    uint32_t      data_size;
    size_t        n;

    if (path == NULL || samples == NULL || n_frames < 0 || rate <= 0) {
        return -1;
    }
    data_size = (uint32_t)n_frames * 2u;

    memcpy(hdr, "RIFF", 4u);
    wr_u32le(hdr + 4, 36u + data_size);          /* RIFF 块长度 = 文件长度 - 8 */
    memcpy(hdr + 8, "WAVE", 4u);
    memcpy(hdr + 12, "fmt ", 4u);
    wr_u32le(hdr + 16, 16u);
    wr_u16le(hdr + 20, 1u);                      /* audio format = PCM */
    wr_u16le(hdr + 22, 1u);                      /* 单声道 */
    wr_u32le(hdr + 24, (uint32_t)rate);
    wr_u32le(hdr + 28, (uint32_t)rate * 2u);     /* byte rate = rate * ch(1) * 2 */
    wr_u16le(hdr + 32, 2u);                      /* block align = ch(1) * 2 */
    wr_u16le(hdr + 34, 16u);                     /* bits per sample */
    memcpy(hdr + 36, "data", 4u);
    wr_u32le(hdr + 40, data_size);

    f = fopen(path, "wb");
    if (f == NULL) { return -2; }

    if (fwrite(hdr, 1u, sizeof(hdr), f) != sizeof(hdr)) { fclose(f); return -3; }

    n = (size_t)n_frames;
    if (n > 0u && fwrite(samples, sizeof(int16_t), n, f) != n) { fclose(f); return -4; }

    if (fclose(f) != 0) { return -5; }
    return 0;
}

void wav_free(void* p)
{
    free(p);
}
