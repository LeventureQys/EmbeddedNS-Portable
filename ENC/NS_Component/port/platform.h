#ifndef NS_PLATFORM_H
#define NS_PLATFORM_H

/* 嵌入式段属性宏在 PC 上退化为空 */
#ifndef __RAM_CODE
#define __RAM_CODE
#endif
#ifndef __RAM3
#define __RAM3
#endif

/* 对齐宏。源码中的用法是"后缀形式"(float buf[960] __ALIGNED(4);),
   MSVC 的 __declspec(align(n)) 只支持前缀形式, 故 MSVC 下定义为空
   —— PC 侧无 DMA 对齐要求。 */
#ifndef __ALIGNED
#if defined(_MSC_VER)
#define __ALIGNED(n)
#else
#define __ALIGNED(n) __attribute__((aligned(n)))
#endif
#endif

#endif /* NS_PLATFORM_H */
