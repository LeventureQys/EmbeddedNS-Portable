#ifndef WAV_IO_H
#define WAV_IO_H

#include <stdint.h>

/* 读 16bit PCM WAV。成功返回 0, 失败返回非 0。
   成功时 *samples 为 malloc 的交错样本缓冲(调用方用 wav_free 释放),
   *n_frames 为每声道采样帧数, *n_ch 为声道数, *rate 为采样率。
   仅支持标准 44 字节头 + fmt / data 块; 其它格式返回错误。 */
int  wav_read_pcm16(const char* path, int16_t** samples, int32_t* n_frames, int32_t* n_ch, int32_t* rate);

/* 写单声道 16bit PCM WAV。成功返回 0。 */
int  wav_write_pcm16_mono(const char* path, const int16_t* samples, int32_t n_frames, int32_t rate);

/* 释放 wav_read_pcm16 分配的缓冲。传 NULL 安全。 */
void wav_free(void* p);

#endif /* WAV_IO_H */
