#ifndef MODULES_AUDIO_PROCESSING_AUDIO_BUFFER_H_
#define MODULES_AUDIO_PROCESSING_AUDIO_BUFFER_H_
///AudioBuffer.h
// 
// Designer : Leventure
// Date: 2025.2.7
// Info:基础信息流数据结构设计，用于存放单个分频的数据Band信息
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define kBandSize 160

//每个单Band的长度必定为160
/// <summary>
/// 分频的音频数据结构体，默认只处理单声道，支持16khz-higher的音频，但是默认我这里假设只支持48khz音频
/// 处理方式是一致的，但是尚未测试，沿用webrtc的设计
/// 需要注意的是，FrameBuffer只是单帧的分频数据
/// 输入的数据一般为10ms，比如采样率为48khz那么一帧10ms的数据就是 480长度，
/// 然后每个band中存放 480 / 3 = 160长度的数据
/// </summary>
typedef struct {
	
	int band_count;
	float** split_data_;
	//float[3][16]
} FrameBuffer;

int InitFrameBuffer(FrameBuffer* buffer, int band_count);
void FreeFrameBuffer(FrameBuffer* buffer);
//void I_InputPCMData(FrameBuffer* buffer, int band, int* data_, size_t size);
//void F_InputPCMData(FrameBuffer* buffer, int band, float* data_, size_t size);




#endif