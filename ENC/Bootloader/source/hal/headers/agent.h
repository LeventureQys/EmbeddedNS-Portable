
#ifndef __AGENT_LEVEL_H
#define __AGENT_LEVEL_H

#include "jat_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

// 代理商初始化
void SystemCoreInit(void);

// 客户efuse校验
// 返回： 0，校验成功
//        1，检验失败
int32_t EFuseCheck(uint32_t efuse);

#ifdef __cplusplus
}
#endif

#endif
