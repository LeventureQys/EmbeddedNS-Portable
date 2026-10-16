/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_def.h
 * @brief   HAL defination file.
    | Version | Data       | Author     | Info                       |
    | ------- | ---------- | ---------- | -------------------------- |
    | 1.0.0   | 2021/09/17 | huangzheng | First version              |
    | 1.0.1   | 2021/10/11 | huangzheng | Add version information    |
    | 1.0.2   | 2022/04/07 | huangzheng | Adjust include files       |
 * @version V1.0.2
 * @date    2022/04/07
 * @author  huangzheng
 *
 ******************************************************************************
 */

#ifndef __A0_HAL_DEF_H
#define __A0_HAL_DEF_H

/**
 * @addtogroup A0_CHIP
 * @{
 */

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"
#include "system_a0.h"                      /* System Header */
#include "a0_hal_conf.h"

#include <string.h>
#ifdef ITM_ENABLE
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/**
 * @addtogroup GLOBAL_MACROS
 * @{
 */

/* 下列函数如需使用别的替代，必须按照HAL_TickDelay, HAL_TickGet原型，提供毫秒计数 */
/* for usb delay function */
#define HalOtgDevTickDelay                  HAL_TickDelay
#define HalOtgHostTickDelay                 HAL_TickDelay
#define HalOtgHostTickGet                   HAL_TickGet
/* for spi flash delay function */
#define HalSpiFlashTickDelay                HAL_TickDelay
#define HalSpiFlashTickGet                  HAL_TickGet
/* for i2c master delay function */
#define HalI2cMasterTickGet                 HAL_TickGet

#ifndef ITM_ENABLE
#define DBG_PRINT(...)
#define DBG_SCAN(...)
#else
#define DBG_PRINT(...)                      printf(__VA_ARGS__)
#define DBG_SCAN(...)                       scanf(__VA_ARGS__)
#endif

#define UNUSED(x)                           ((void)(x))
#define SET_BIT(REG, BIT)                   ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)                 ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)                  ((REG) & (BIT))
#define CLEAR_REG(REG)                      ((REG) = (0x0))
#define WRITE_REG(REG, VAL)                 ((REG) = (VAL))
#define READ_REG(REG)                       ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) \
WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

/**
 * @}
 */

/* Peripheral Control functions  *********************************************/
/**
 * @addtogroup GLOBAL_FUNCTIONS Global Functions
 * @{
 */
void            HAL_TickInit(void);
void            HAL_TickInc(void);
void            HAL_TickDelay(volatile uint32_t Delay);
void            HAL_TickClr(void);
uint32_t        HAL_TickGet(void);
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
