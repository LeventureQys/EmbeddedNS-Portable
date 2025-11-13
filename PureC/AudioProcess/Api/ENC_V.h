//info : ENC_V
//desc: the api of noise suppressor that process the audio with 48khz
//author : Venture Wendy
//date: 2025.2.18

#ifndef ENCV_H

#ifdef ENCV
#define ENCV_API __declspec(dllexport)
#else
#define ENCV_API __declspec(dllimport)
#endif

#include <stdbool.h>

#ifdef __cplusplus // 如果是C++环境
extern "C" {      // 使用extern "C"
#endif

//#include <stdbool>
//初始化48k指针对象
ENCV_API void* InitNSHandler(int level);

/// <summary>
/// author:Venture
/// info:降噪处理音频，一次只支持480个sample，多给不处理哈
/// </summary>
/// <param name="handler"></param>
/// <param name="input"></param>
/// <param name="output"></param>
/// <returns></returns>
ENCV_API bool NS_Process_48kAudio(void* handler, float* input, float* output);
#ifdef __cplusplus // 如果是C++环境
}                // 结束extern "C"
#endif
#endif