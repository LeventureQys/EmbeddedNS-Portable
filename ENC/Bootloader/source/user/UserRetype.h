/*
*****************************************Copyright (c)**********************************************
**
**
**                                  深圳摩罗志远科技有限公司
**                             Shenzhen Maono Technology Co.,Ltd
**
**                                        www.maono.cn
**
**
**  文件名 : UserRetype.h
**
**  程序员 : 
**
**	日　期  : 2023年1月12日
**
**  文件描述: 工程数据类型重定义。
**
****************************************************************************************************
*/
#ifndef _USER_RETYPE_H
#define _USER_RETYPE_H



/* 常用宏定义 ----------------------------------------------------------------------------------- */
#ifndef true
#define true                (1)
#endif
#ifndef false
#define false               (0)
#endif

#ifndef null
#define null                (0)
#endif

#ifndef SIZE_OF
#define SIZE_OF(x)          (sizeof(x) / sizeof(*(x)))
#endif

#ifndef VAR_UNUSED
#define VAR_UNUSED(x)       (void)(x)
#endif

#ifndef MEMSET_ZERO
#define MEMSET_ZERO(x)      memset(&x, 0, sizeof(x))
#endif

#ifndef BIT_SET
#define BIT_SET(v, b)       (v) |= (1u<<(b))
#endif
#ifndef BIT_CLR
#define BIT_CLR(v, b)       (v) &= ~(1u<<(b))
#endif
#ifndef BIT_XOR
#define BIT_XOR(v, b)       (v) ^= (1u<<(b))
#endif
#ifndef BIT_GET
#define BIT_GET(v, b)       ((v) & (1u<<(b)))
#endif
//#ifndef BIT
//#define BIT(b)              (1u << (b))
//#endif

#ifndef MIN
#define MIN(a, b)           (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)           (((a) > (b)) ? (a) : (b))
#endif

#define HEX(x, y)           0x##x##y
#define HEX2(x, y)          0x##x##y
#define HEX3(x, y, z)       0x##x##y##z
#define HEX4(x, y, z, u)    0x##x##y##z##u

#define DEC(x, y)           x##y
#define DEC2(x, y)          x##y
#define DEC3(x, y, z)       x##y##z
#define DEC4(x, y, z, u)    x##y##z##u


/* 重定义变量声明 ------------------------------------------------------------------------------- */
typedef unsigned  char          uint8;      /*  无符号8位整型变量        */
typedef signed    char          int8;       /*  有符号8位整型变量        */
typedef unsigned  short         uint16;     /*  无符号16位整型变量       */
typedef signed    short         int16;      /*  有符号16位整型变量       */
typedef unsigned  int           uint32;     /*  无符号32位整型变量       */
typedef signed    int           int32;      /*  有符号32位整型变量       */
typedef unsigned  long long     uint64;     /*  无符号64位整型变量       */
typedef           long long     int64;      /*  有符号64位整型变量       */

typedef           float         fp32;       /*  单精度浮点数（32位长度） */
typedef           double        fp64;       /*  双精度浮点数（64位长度） */


#include "ProjectConfig.h"


#if JLINK_DEBUG
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"

#define PRINTF(...)        SEGGER_RTT_printf(0, __VA_ARGS__)
#define LOG(...)           SEGGER_RTT_printf(0, __VA_ARGS__)
#else
#define PRINTF(...)        
#define LOG(...)           
#endif


#if __DEBUG
#define DEBUG(code)         code
#else
#define DEBUG(code)         
#endif

#endif




