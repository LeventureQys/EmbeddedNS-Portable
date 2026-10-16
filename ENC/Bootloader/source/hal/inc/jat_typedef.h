/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    jat_typedef.h
 * @brief   Header file of typedef.
    | Version | Data       | Author     | Info           |
    | ------- | ---------- | ---------- | -------------- |
    | 1.0.0   | 2022/04/07 | huangzheng | First version  |
    | 1.0.1   | 2023/12/01 | huangzheng | add function pointer type |
 * @version V1.0.1
 * @date    2023/12/01
 * @author  huangzheng
 * @details
 ******************************************************************************
 */

#ifndef __JAT_TYPEDEF_H
#define __JAT_TYPEDEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* type definitions */
typedef void                        VOID;
typedef char                        CHAR8;
typedef unsigned char               UCHAR8;
typedef unsigned short              USHORT;                     /* 16-bit unsigned */
typedef unsigned char               BYTE;                       /* 8-bit unsigned */
typedef unsigned short int          WORD;                       /* 16-bit unsigned */
typedef unsigned long               DWORD;                      /* 32-bit unsigned */
typedef unsigned long long          DWORD64;                    /* 64-bit unsigned */
typedef signed char                 CHAR;                       /* 8-bit signed */
typedef signed short int            SHORT;                      /* 16-bit signed */
typedef signed long                 LONG;                       /* 32-bit signed */
typedef signed long long            LONGLONG;                   /* 64-bit signed */
typedef float                       FLOAT;                      /* 32-bit float*/
typedef wchar_t                     WCHAR;                      /* wc, 16-bit UNICODE character */
typedef unsigned char               BOOL;
typedef unsigned char               BOOLEAN;

typedef void*                       LPVOID;
typedef unsigned char*              PBYTE;                      /* 8-bit unsigned pointer */
typedef unsigned int*               PBOOL;                      /* 32-bit unsigned pointer */
typedef WORD*                       PWORD;                      /* 16-bit unsigned pointer */
typedef DWORD*                      PDWORD;                     /* 32-bit unsigned pointer */
typedef CHAR*                       PCHAR;
typedef WCHAR*                      PWCHAR;                     /* 16-bit unicode character pointer */
typedef LONG*                       PLONG;                      /* 32-bit signed pointer */

typedef void                        (*FUNCP)(void);             /* no argument function pointer */
typedef void                        (*FUNCPP)(void *);
typedef unsigned int                (*U32FUNCP)(void);
typedef unsigned int                (*U32FUNCPP)(void *);
typedef signed int                  (*I32FUNCP)(void);
typedef signed int                  (*I32FUNCPP)(void *);

typedef signed char                 INT8;
typedef signed short                INT16;
typedef signed long                 INT32;
typedef signed long long            INT64;
typedef unsigned char               UINT8;
typedef unsigned short              UINT16;
typedef unsigned long               UINT32;
typedef unsigned long long          UINT64;
typedef volatile signed char        VINT8;
typedef volatile signed short       VINT16;
typedef volatile signed long        VINT32;
typedef volatile signed long long   VINT64;
typedef volatile unsigned char      VUINT8;
typedef volatile unsigned short     VUINT16;
typedef volatile unsigned long      VUINT32;
typedef volatile unsigned long long VUINT64;
typedef const signed char           CINT8;
typedef const signed short          CINT16;
typedef const signed long           CINT32;
typedef const signed long long      CINT64;
typedef const unsigned char         CUINT8;
typedef const unsigned short        CUINT16;
typedef const unsigned long         CUINT32;
typedef const unsigned long long    CUINT64;

typedef signed char                 S8;
typedef unsigned char               U8;
typedef short                       S16;
typedef unsigned short              U16;
typedef int                         S32;
typedef unsigned int                U32;
typedef long long                   S64;
typedef unsigned long long          U64;
typedef unsigned char               BIT;

/* definations */
#define BIT0                        0x00000001U
#define BIT1                        0x00000002U
#define BIT2                        0x00000004U
#define BIT3                        0x00000008U
#define BIT4                        0x00000010U
#define BIT5                        0x00000020U
#define BIT6                        0x00000040U
#define BIT7                        0x00000080U
#define BIT8                        0x00000100U
#define BIT9                        0x00000200U
#define BIT10                       0x00000400U
#define BIT11                       0x00000800U
#define BIT12                       0x00001000U
#define BIT13                       0x00002000U
#define BIT14                       0x00004000U
#define BIT15                       0x00008000U
#define BIT16                       0x00010000U
#define BIT17                       0x00020000U
#define BIT18                       0x00040000U
#define BIT19                       0x00080000U
#define BIT20                       0x00100000U
#define BIT21                       0x00200000U
#define BIT22                       0x00400000U
#define BIT23                       0x00800000U
#define BIT24                       0x01000000U
#define BIT25                       0x02000000U
#define BIT26                       0x04000000U
#define BIT27                       0x08000000U
#define BIT28                       0x10000000U
#define BIT29                       0x20000000U
#define BIT30                       0x40000000U
#define BIT31                       0x80000000U
#define OPTION_ENABLE               1
#define OPTION_DISABLE              0

#ifndef TRUE
#define TRUE                        1
#endif

#ifndef FALSE
#define FALSE                       0
#endif

#ifdef YES
#undef YES
#endif
#define YES                         1

#ifdef NO
#undef NO
#endif
#define NO                          0

#ifdef __cplusplus
}
#endif

#endif
