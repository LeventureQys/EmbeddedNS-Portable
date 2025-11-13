#include "AudioBuffer.h"

int InitFrameBuffer(FrameBuffer* buffer, int band_count)
{
    if (buffer == NULL || band_count <= 0) {
        return -1; // 检查输入参数是否有效
    }

    // 设置 band_count
    buffer->band_count = band_count;

    // 分配内存给 split_data_（指针数组）
    buffer->split_data_ = (float**)malloc(band_count * sizeof(float*));
    if (buffer->split_data_ == NULL) {
        perror("Failed to allocate memory for split_data_");
        return -1;
    }

    // 为每个 band 分配内存
    for (int i = 0; i < band_count; i++) {
        buffer->split_data_[i] = (float*)malloc(kBandSize * sizeof(float));
        if (buffer->split_data_[i] == NULL) {
            perror("Failed to allocate memory for a band");

            // 如果分配失败，释放之前分配的内存
            for (int j = 0; j < i; j++) {
                free(buffer->split_data_[j]);
            }
            free(buffer->split_data_);
            return -1;
        }

        // 初始化每个 band 的数据为 0.0f
        for (int j = 0; j < kBandSize; j++) {
            buffer->split_data_[i][j] = 0.0f;
        }
    }

    return 1; // 成功初始化
}
void FreeFrameBuffer(FrameBuffer* buffer) {
    if (buffer == NULL) {
        return; // 如果 buffer 指针为空，则直接返回，无需释放任何内存
    }

    if (buffer->split_data_ != NULL) {
        // 假设 split_data_ 是一个指向 float* 数组的指针，每个 float* 指针指向一个 float 数组（即二维数组的行）
        for (int i = 0; i < buffer->band_count; ++i) {
            if (buffer->split_data_[i] != NULL) {
                free(buffer->split_data_[i]); // 释放每一行 float 数组的内存
                buffer->split_data_[i] = NULL; // 释放后将指针置空，防止悬 dangling pointer
            }
        }
        free(buffer->split_data_); // 释放指向 float* 数组的内存
        buffer->split_data_ = NULL; // 释放后将指针置空
    }

    // 注意：band_count 是 int 类型，通常不动态分配内存，
    // 而是直接存储在 FrameBuffer 结构体本身的空间里。
    // 因此，这里不需要 free(buffer->band_count);

    buffer->band_count = 0; // 将 band_count 重置为 0，表示 FrameBuffer 已被释放，这是一个好的习惯
}
//void I_InputPCMData(FrameBuffer* buffer, int band, int* data_, size_t size) {
//    if (buffer == NULL || buffer->split_data_ == NULL) {
//        fprintf(stderr, "Error: FrameBuffer is not initialized.\n");
//        return;
//    }
//
//    if (band < 0 || band >= buffer->band_count) {
//        fprintf(stderr, "Error: Band index is out of range.\n");
//        return;
//    }
//
//    if (size > 160) {
//        fprintf(stderr, "Error: Size exceeds maximum allowed (160).\n");
//        return;
//    }
//
//    if (data_ == NULL) {
//        fprintf(stderr, "Error: Input data is NULL.\n");
//        return;
//    }
//
//    float* float_data = (float*)malloc(size * sizeof(float));
//    if (float_data == NULL) {
//        perror("Memory allocation failed");
//        return;
//    }
//
//    for (size_t i = 0; i < size; i++) {
//        float_data[i] = (float)data_[i];
//    }
//
//    F_InputPCMData(buffer, band, float_data, size);  // Call F_InputPCMData
//    free(float_data);
//}
//
//void F_InputPCMData(FrameBuffer* buffer, int band, float* data_, size_t size) {
//    if (buffer == NULL || buffer->split_data_ == NULL) {
//        fprintf(stderr, "Error: FrameBuffer is not initialized.\n");
//        return;
//    }
//
//    if (band < 0 || band >= buffer->band_count) {
//        fprintf(stderr, "Error: Band index is out of range.\n");
//        return;
//    }
//
//    if (size > 160) {
//        fprintf(stderr, "Error: Size exceeds maximum allowed (160).\n");
//        return;
//    }
//
//    if (data_ == NULL) {
//        fprintf(stderr, "Error: Input data is NULL.\n");
//        return;
//    }
//
//    memcpy(buffer->split_data_[band], data_, size * sizeof(float));
//}
