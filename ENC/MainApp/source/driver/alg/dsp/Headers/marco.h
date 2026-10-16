/***********************************************************************************************************************
*                                 Copyright (c) 2018,深圳创成微电子<-All rights reserved->
*               
**----------------------------------------------------------------------------------------------------------------------
*   文件名称：   marco.h
*   简要描述:    常用宏定义
*
**----------------------------------------------------------------------------------------------------------------------
*   当前版本：   V1.0
*   作者/修改者：PK
*   完成日期：   2019年09月20日
**----------------------------------------------------------------------------------------------------------------------
*   取代版本：
*   原作者  ：   
*   完成日期：
***********************************************************************************************************************/
#ifndef __MARCO_H__
#define __MARCO_H__

/*--------------------------------------------------------------------------------------------------------------------*/
#define BIT(n)                              (1ul << (n))
/*--------------------------------------------------------------------------------------------------------------------*/
#ifndef BIT_0
#define BIT_0                               (0x00000001)
#define BIT_1                               (0x00000002)
#define BIT_2                               (0x00000004)
#define BIT_3                               (0x00000008)
#define BIT_4                               (0x00000010)
#define BIT_5                               (0x00000020)
#define BIT_6                               (0x00000040)
#define BIT_7                               (0x00000080)
#define BIT_8                               (0x00000100)
#define BIT_9                               (0x00000200)
#define BIT_10                              (0x00000400)
#define BIT_11                              (0x00000800)
#define BIT_12                              (0x00001000)
#define BIT_13                              (0x00002000)
#define BIT_14                              (0x00004000)
#define BIT_15                              (0x00008000)
#define BIT_16                              (0x00010000)
#define BIT_17                              (0x00020000)
#define BIT_18                              (0x00040000)
#define BIT_19                              (0x00080000)
#define BIT_20                              (0x00100000)
#define BIT_21                              (0x00200000)
#define BIT_22                              (0x00400000)
#define BIT_23                              (0x00800000)
#define BIT_24                              (0x01000000)
#define BIT_25                              (0x02000000)
#define BIT_26                              (0x04000000)
#define BIT_27                              (0x08000000)
#define BIT_28                              (0x10000000)
#define BIT_29                              (0x20000000)
#define BIT_30                              (0x40000000)
#define BIT_31                              (0x80000000)
#endif /* BIT_7 */
/*--------------------------------------------------------------------------------------------------------------------*/
#ifndef BIT0
#define BIT0                                (0x00000001)
#define BIT1                                (0x00000002)
#define BIT2                                (0x00000004)
#define BIT3                                (0x00000008)
#define BIT4                                (0x00000010)
#define BIT5                                (0x00000020)
#define BIT6                                (0x00000040)
#define BIT7                                (0x00000080)
#define BIT8                                (0x00000100)
#define BIT9                                (0x00000200)
#define BIT10                               (0x00000400)
#define BIT11                               (0x00000800)
#define BIT12                               (0x00001000)
#define BIT13                               (0x00002000)
#define BIT14                               (0x00004000)
#define BIT15                               (0x00008000)
#define BIT16                               (0x00010000)
#define BIT17                               (0x00020000)
#define BIT18                               (0x00040000)
#define BIT19                               (0x00080000)
#define BIT20                               (0x00100000)
#define BIT21                               (0x00200000)
#define BIT22                               (0x00400000)
#define BIT23                               (0x00800000)
#define BIT24                               (0x01000000)
#define BIT25                               (0x02000000)
#define BIT26                               (0x04000000)
#define BIT27                               (0x08000000)
#define BIT28                               (0x10000000)
#define BIT29                               (0x20000000)
#define BIT30                               (0x40000000)
#define BIT31                               (0x80000000)
#endif  /* BIT0 */
/*--------------------------------------------------------------------------------------------------------------------*/
#define BitSet(Reg, Pos)                    ( (Reg)  |= (1ul << (Pos)) )
#define BitClr(Reg, Pos)                    ( (Reg)  &= (~(1ul << (Pos))) )
#define BitXor(Reg, Pos)                    ( (Reg)  ^= (1ul << (Pos)) )
#define BitCHK_1(Reg, Pos)                  ( ((Reg) & (1ul << (Pos))) == (1ul << (Pos)) )
#define BitCHK_0(Reg, Pos)                  ( ((Reg) & (1ul << (Pos))) == 0x00 )
#define BitTest(Reg, Pos)                   ( (Reg)  & (1ul << (Pos)) )
/*--------------------------------------------------------------------------------------------------------------------*/
#define ValSet(Reg, Val)                    ( (Reg)  |= (Val) )
#define ValClr(Reg, Val)                    ( (Reg)  &= ~(Val) )
#define ValXor(Reg, Val)                    ( (Reg)  ^= (Val) )
#define ValTest(Reg, Val)                   ( (Reg)  & (Val) )
#define ValChk_V(Reg, Val)                  ( ((Reg) & 0xff) == (Val) )
#define ValChk_N(Reg, Val)                  ( ((Reg) & (Val)) == (Val) )
#define ValChk_0(Reg, Val)                  ( ((Reg) & (Val)) == 0x00)
#define ValChk_1(Reg, Val)                  ( ((Reg) & (Val)) != 0x00)
/*--------------------------------------------------------------------------------------------------------------------*/
#define SETB(Reg, Pos)                      ( (Reg)  |= (1ul << (Pos)) )
#define CLRB(Reg, Pos)                      ( (Reg)  &= (~(1ul << (Pos))) )
#define XORB(Reg, Pos)                      ( (Reg)  ^= (1ul << (Pos)) )
#define CKB1(Reg, Pos)                      ( (Reg)  & (1ul << Pos) )
#define CKB0(Reg, Pos)                      (!((Reg) & (1ul << Pos)) )
#define SETV(Reg, Val)                      ( (Reg)  |= (Val) )
#define CLRV(Reg, Val)                      ( (Reg)  &= (~Val) )
#define XORV(Reg, Val)                      ( (Reg)  ^= (Val) )
#define CKV1(Reg, Val)                      ( (Reg)  & (Val) )
#define CKV0(Reg, Val)                      (!((Reg) & (Val)) )
#define CK1V(Val, V1)                       ( (Val)  == (V1) )
#define CK2V(Val, V1, V2)                   ( ((Val) == (V1)) || ((Val) == (V2)) )
#define CK3V(Val, V1, V2, V3)               ( ((Val) == (V1)) || ((Val) == (V2)) || ((Val) == (V3)) )
/*--------------------------------------------------------------------------------------------------------------------*/
#define BYTE0(n)                            ( (uint8_t )(  (uint16_t )(n)) )
#define BYTE1(n)                            ( (uint8_t )( ((uint16_t )(n)) >> 8) )
#define BYTE2(n)                            ( (uint8_t )( ((uint16_t )( ((uint32_t )(n)) >> 8) ) >> 8) )
#define BYTE3(n)                            ( (uint8_t )( ((uint16_t )( ((uint32_t )(n)) >> 16) ) >> 8) )

#define BYTE_0(n)                           ( (uint8_t )(  (uint16_t )(n)) )
#define BYTE_1(n)                           ( (uint8_t )( ((uint16_t )(n)) >> 8) )
#define BYTE_2(n)                           ( (uint8_t )( ((uint16_t )( ((uint32_t )(n)) >> 8) ) >> 8) )
#define BYTE_3(n)                           ( (uint8_t )( ((uint16_t )( ((uint32_t )(n)) >> 16) ) >> 8) )

#define WORD3(byte0, byte1, byte2, byte3)   ( (uint32_t)(((((((byte3)<<8) | (byte2)) << 8) | (byte1)) << 8) | (byte0)) )
#define WORD2(byte0, byte1, byte2)          WORD3(byte0, byte1, byte2, 0)
#define WORD1(byte0, byte1)                 WORD3(byte0, byte1, 0, 0)
#define WORD0(byte0)                        WORD3(byte0, 0, 0, 0)

#define WORD_3(byte0, byte1, byte2, byte3)  ( (uint32_t)(((((((byte3)<<8) | (byte2)) << 8) | (byte1)) << 8) | (byte0)) )
#define WORD_2(byte0, byte1, byte2)         WORD_3(byte0, byte1, byte2, 0)
#define WORD_1(byte0, byte1)                WORD_3(byte0, byte1, 0, 0)
#define WORD_0(byte0)                       WORD_3(byte0, 0, 0, 0)

#define REVERSAL_WORD(word)                 WORD_3(BYTE_3(word), BYTE_2(word), BYTE_1(word), BYTE_0(word))
/*--------------------------------------------------------------------------------------------------------------------*/
//#define GET_MAX(DataA, DataB)               ( ((DataA) > (DataB))? (DataA):(DataB) )//Niko alg_common.h重复
//#define GET_MIN(DataA, DataB)               ( ((DataA) < (DataB))? (DataA):(DataB) )//Niko alg_common.h重复
#define GET_ABS(Data)                       ( ((int32_t)(Data) >= 0)? (int32_t)(Data):(-(int32_t)(Data)) )
#define MAX_XY(x, y)                        ( ((x) > (y))? (x):(y) )
#define MIN_XY(x, y)                        ( ((x) < (y))? (x):(y) )
#define ABS(x)                              ( ((x) >= 0)? (x):(-(x)) )
#define XlessY(x, y)                        ( (x)  <  (y) )
#define XmoreY(x, y)                        ( (x)  >  (y) )
#define X_Less_Y(x, y)                      ( (x)  <  (y) )
#define X_More_Y(x, y)                      ( (x)  >  (y) )
#define Data_Is_Area(val, a, b)             ( ((val) > (a)) && ((val) < (b)) )
#define Open_interval(val, vmin, vmax)      ( ((val) > (vmin)) && ((val) < (vmax)) )            /* 开区间             */
#define Closed_interval(val, vmin, vmax)    ( ((val) >= (vmin)) && ((val) <= (vmax)) )          /* 闭区间             */
/*--------------------------------------------------------------------------------------------------------------------*/
#define Data_X_Mirror(A)                    ( (((A) & BIT(7)) >> 7) | \
                                              (((A) & BIT(6)) >> 5) | \
                                              (((A) & BIT(5)) >> 3) | \
                                              (((A) & BIT(4)) >> 1) | \
                                              (((A) & BIT(3)) << 1) | \
                                              (((A) & BIT(2)) << 3) | \
                                              (((A) & BIT(1)) << 5) | \
                                              (((A) & BIT(0)) << 7) )               /* 原点对称[76543210---01234567] */

#define Data_High2Low(A)                    ( (((A) & BIT(7)) >> 4) | \
                                              (((A) & BIT(6)) >> 4) | \
                                              (((A) & BIT(5)) >> 4) | \
                                              (((A) & BIT(4)) >> 4) | \
                                              (((A) & BIT(3)) << 4) | \
                                              (((A) & BIT(2)) << 4) | \
                                              (((A) & BIT(1)) << 4) | \
                                              (((A) & BIT(0)) << 4) )               /* 高低互换[76543210--32107654] */
/*--------------------------------------------------------------------------------------------------------------------*/


#endif
/***********************************************************************************************************************
* END OF FILE
***********************************************************************************************************************/
