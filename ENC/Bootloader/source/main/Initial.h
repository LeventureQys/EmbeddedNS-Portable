/****************************************************************************/
/* System Name 	: AP3852													*/
/* File Name 	: Initial.h 											    */
/* Version 		: 1.0.0 													*/
/* Histroy 		: 															*/
/* Date 		: 2019/07/25												*/
/* 				Initial version												*/
/* 				by huangzheng 												*/
/****************************************************************************/

#ifndef __INITIAL_H
#define __INITIAL_H

#include "main.h"

#define SYS_POWER_SAVE_EN                           (1)
#define SYS_CLK_DOWN_EN                             (0)//(0)         //0, no change, 1, 2/3*192M, 2, 3/4*192M, 3, 4/5*192M, 4, 整体调整PLL0时钟

#define CLK_190M_SSEN                               (0)           // 190M±2M,20KFs,SSEN=1
#define CLK_120M                                    (1)
#define CLK_115M                                    (2)
#define CLK_110M                                    (3)
#define CLK_105M                                    (4)
#define CLK_100M                                    (5)
#define CLK_90M                                     (6)
#define CLK_80M                                     (7)
#define CLK_70M                                     (8)
#define CLK_60M                                     (9)
#define CLK_50M                                     (10)
#define CLK_40M                                     (11)
#define CLK_30M                                     (12)
#define CLK_20M                                     (13)
#define CLK_55M                                     (14)

#define SYS_CLK_PLL0_CFG_SEL                        (CLK_100M)         // default 96M, if re-config pll, system clk must <= 80M


//系统时钟变化，差不多每16MHz是3.5mA左右的样子
//DSP是8mA
//RAM02是8mA
//RAM1是6.2mA
//如果是在2/3，即128M时，DSP/RAM02都关掉，可以减 4*3.5+8+8+6.2=36.2mA左右

void TargetResetInit(void);

#endif
