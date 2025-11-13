#include "ENC_V.h"
#include "../NoiseSuppressor.h"
void* InitNSHandler(int level) {
    NoiseSuppressor* item = (NoiseSuppressor*)malloc(sizeof(NoiseSuppressor));
    if (item == NULL) {
        return NULL; // 内存分配失败
    }
    NoiseSuppressor_init(item, level); // 默认三级降噪
    return (void*)item;
}

ENCV_API void FreeNSHandler(void* handler)
{
    NoiseSuppressor* item = (NoiseSuppressor*)handler;
    if (item == NULL) {
        return;
    }
    free(item);
}

bool NS_Process_48kAudio(void* handler, float* input, float* output)
{
    //流程上总的来说遵循如下几步:
    //1. 转换NoiseSuppressor为当前可以用的，如果这个地方转换都失败了，则可能会导致崩溃，需要调用者保证handler是正常处理过的
    //2. 将input数据分频，转换成FrameBuffer的数据
    //3. 将分频后的数据输入Analyse
    //4. 将Analyse后的数据Process
    //5. 合并三分频，并给到output

    NoiseSuppressor* item = (NoiseSuppressor*)handler;
    if (item == NULL) {
        return false;
    }

    size_t size_ns = sizeof(NoiseSuppressor);
    //先将input数据进行分频
    FrameBuffer* buffer = (FrameBuffer*)malloc(sizeof(FrameBuffer));
    InitFrameBuffer(buffer,3);  //执行三分频， 并将数据输入到FrameBuffer中
    ThreeBandFilterBank_Analysis(&item->splitting_filter, input, buffer->split_data_);
    float _input[480];
    float _output[3][160];

    for (int i = 0;i < 480;i++) {
        _input[i] = input[i];
    }

    for (int i = 0;i < 3;++i) {
        for (int j = 0;j < 160;j++) {
            _output[i][j] = buffer->split_data_[i][j];
        }
    }
    ////执行NoiseSuppressor_Analyze
    NoiseSuppressor_Analyze(item, buffer);
    //float aout[3][160];
    //for (int i = 0;i < 3;++i) {
    //    for (int j = 0;j < 160;j++) {
    //        aout[i][j] = buffer->split_data_[i][j];
    //    }
    //}
    ////执行NoiseSuppressor_Process
    NoiseSuppressor_Process(item, buffer);

    ////check process 
    //
    //float bout[3][160];
    //for (int i = 0;i < 3;++i) {
    //    for (int j = 0;j < 160;j++) {
    //        bout[i][j] = buffer->split_data_[i][j];
    //    }
    //}
    //对三分频执行合并
    ThreeBandFilterBank_Synthesis(&item->splitting_filter, buffer->split_data_, output);

    FreeFrameBuffer(buffer);
    free(buffer);
}
