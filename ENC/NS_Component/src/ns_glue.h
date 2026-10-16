#ifndef NS_GLUE_H
#define NS_GLUE_H

#include "arm_math_compat.h"

/* FFT 实例。由冻结源 enc.c:26 原样迁入, 保持文件级全局(非 static):
   noise_suppressor.c:12 的 extern 声明依赖该符号名与链接属性。
   该实例只记载长度, 无跨帧状态, 不构成共享可变状态。 */
extern arm_rfft_fast_instance_f32 g_InstRfftF32;

#endif /* NS_GLUE_H */
