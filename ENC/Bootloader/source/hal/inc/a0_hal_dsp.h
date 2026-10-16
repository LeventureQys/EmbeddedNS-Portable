/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_dsp.h
 * @brief   Header file of DSP HAL module.
    | Version | Data       | Author     | Info                                                     |
    | ------- | ---------- | ---------- | -------------------------------------------------------- |
    | 1.0.0   | 2022/04/07 | huangzheng | First version                                            |
    | 1.0.1   | 2022/09/01 | huangzheng | 1. Bug of REG_DSP_RESET() fixed                          |
    |         |            |            | 2. REG_DSP_CON_RUN_CHECK() changed to check BIT1         |
    |         |            |            | 3. REG_DSP_CON_HOLD_CHECK() changed to check BIT2        |
    |         |            |            | 4. REG_DSP_CON_START_CHECK() added to check BIT0         | 
    |         |            |            | 5. Added bit defination of the register of DSP_CON       | 
 * @version V1.0.1
 * @date    2022/09/01
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_DSP_H
#define __A0_HAL_DSP_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */

/**
 * @addtogroup DSP
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup DSP_Exported_Types DSP Exported Types
 * @{
 */
typedef union
{
    struct {
        uint32_t    start:1;                /*!< bit0, r/w, write 1 to run, pc add automatic, write 0 to stop running at once */
        uint32_t    runState:1;             /*!< bit1, r, 1, dsp is running, 0, dsp is stoped */
        uint32_t    hold:1;                 /*!< bit2, r/w, write 1 to hold dsp, read after write, when 1, dsp holded */
    }b;
    uint32_t    w;
}REG_DSP_CON;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup DSP_Exported_Constants DSP Exported Constants
 * @{
 */

#define REG_ADDR_DSP_IRAM_BASE              (A0_DSP_BASE + 0x00000000UL)
#define REG_ADDR_DSP_PRAM_BASE              (A0_DSP_BASE + 0x00004000UL)
#define REG_ADDR_DSP_RST                    (A0_DSP_BASE + 0x00008000UL)
#define REG_ADDR_DSP_INT                    (A0_DSP_BASE + 0x00008004UL)
#define REG_ADDR_DSP_CON                    (A0_DSP_BASE + 0x00008040UL)
#define REG_ADDR_DSP_PC                     (A0_DSP_BASE + 0x00008044UL)
#define REG_ADDR_DSP_CYCLE_CNT              (A0_DSP_BASE + 0x00008048UL)
// CBIT:ctrl bit, R/W
// SBIT:state bit, RO
#define REG_DSP_CON_CBIT_START			    (0x01UL)
#define REG_DSP_CON_SBIT_RUN			    (0x02UL)
#define REG_DSP_CON_CBIT_HOLD			    (0x04UL)

/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup DSP_Exported_Macros DSP Exported Macros
 * @{
 */

#define DSP_IRAM_ADDR(addr)                 (REG_ADDR_DSP_IRAM_BASE + ((addr) << 2))
#define DSP_PRAM_ADDR(addr)                 (REG_ADDR_DSP_PRAM_BASE + ((addr) << 2))
#define DSP_IRAM(addr)                      (U32REG(DSP_IRAM_ADDR(addr)))
#define DSP_PRAM(addr)                      (U32REG(DSP_PRAM_ADDR(addr)))
#define DSP_REG(regN)                       (DSP_IRAM(regN))

#define REG_DSP_PC_RD()                     (U32REG(REG_ADDR_DSP_PC) >> 16)
#define REG_DSP_PC_WR(pc)                   (U32REG(REG_ADDR_DSP_PC) = (pc & 0xFFF))
#define REG_DSP_CON_START()                 (U32REG(REG_ADDR_DSP_CON) |= REG_DSP_CON_CBIT_START)
#define REG_DSP_CON_HOLD()                  (U32REG(REG_ADDR_DSP_CON) |= REG_DSP_CON_CBIT_HOLD)
#define REG_DSP_CON_STOP()                  (U32REG(REG_ADDR_DSP_CON) &= (~REG_DSP_CON_CBIT_START))
#define REG_DSP_CON_CONTINUE()              (U32REG(REG_ADDR_DSP_CON) &= (~REG_DSP_CON_CBIT_HOLD))
#define REG_DSP_CON_START_CHECK()           (U32REG(REG_ADDR_DSP_CON) & REG_DSP_CON_CBIT_START)
#define REG_DSP_CON_RUN_CHECK()             (U32REG(REG_ADDR_DSP_CON) & REG_DSP_CON_SBIT_RUN)
#define REG_DSP_CON_HOLD_CHECK()            (U32REG(REG_ADDR_DSP_CON) & REG_DSP_CON_CBIT_HOLD)
#define REG_DSP_CNT()                       (U32REG(REG_ADDR_DSP_CYCLE_CNT))
#define REG_DSP_DIT()                       (U32REG(REG_ADDR_DSP_INT) |= 0x1)        //MCU give dsp debug int
#define REG_DSP_MIT()                       (U32REG(REG_ADDR_DSP_INT) |= 0x2)        //MCU give dsp mcu int
#define REG_DSP_RESET()                     \
do {                                        \
    U32REG(REG_ADDR_DSP_RST) = 0;           \
    U32REG(REG_ADDR_DSP_RST) = 1;           \
} while(U32REG(REG_ADDR_DSP_RST) == 0)

/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup DSP_Exported_Functions
 * @{
 */
/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
