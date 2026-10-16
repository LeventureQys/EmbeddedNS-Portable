/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_kbd.h
 * @brief   Header file of Keyboard Scan HAL module.
 * @version V1.0.2
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_KBD_H
#define __A0_HAL_KBD_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup KSCAN
 * @brief Keyboard scan, function key scan, and LED driver module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup KSCAN_Exported_Constants KSCAN Exported Constants
 * @{
 */

/**
 * @defgroup KSCAN_RAM_ADDR KSCAN Ram Address
 * @brief KSCAN module ram address
 * @details
    | Ram Address | Description        |
    | ----------- | ------------------ |
    | [0x00:0x57] | keyboard state     |
    | [0x58:0x5F] | led light on ctrl  |
    | [0x60:0x67] | function key state |
    | [0x80:~]    | krq clear          |
 * @{
 */
#define KSCAN_RAM_ADDR_KEYBOARD             (0x00)  /*!< ram address of keyboard, offset 0~87 */
#define KSCAN_RAM_ADDR_LED                  (0x58)  /*!< ram address of led, offset 0~7 */
#define KSCAN_RAM_ADDR_FKEY                 (0x60)  /*!< ram address of function key, offset 0~7 */
#define KSCAN_RAM_ADDR_KRQ_CLR              (0x80)  /*!< use to clear krq bit */
/**
 * @}
 */

/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup KSCAN_Exported_Macros KSCAN Exported Macros
 * @{
 */

/**
 * @brief  Write address to KSCAN module, write register 'CR' in @ref KSCAN_TypeDef
 * @param  ADDRESS     ram address of KSCAN module, @ref KSCAN_RAM_ADDR
 * @return None
 */
#define HAL_KSCAN_RAM_ADDR_SET(ADDRESS)                                 (KSCAN->CR.ADDR = (ADDRESS))

/**
 * @brief  Read state of KSCAN module, read register 'CR' in @ref KSCAN_TypeDef
 * @return State of KSCAN module, @ref KSCAN_STATE_TypeDef
 */
#define HAL_KSCAN_KBD_STATE_GET()                                       (KSCAN->CR.STATE)

/**
 * @brief  Write the led ctrl value to light on/off led, write register 'DR' in @ref KSCAN_TypeDef
 * @param  VAL     led ctrl value write, '0' to light on, '1' to light off, BIT10~BIT0 means MK10~MK0 output state
 * @return None
 */
#define HAL_KSCAN_LED_CTRL_SET(VAL)                                     (KSCAN->DR.LED = (VAL))

/**
 * @brief  Read information of keyboard or function key state, read register 'DR' in @ref KSCAN_TypeDef
 * @return Info of keyboard (ram address[0x00:0x57]) @ref KSCAN_KEY_STATE_TypeDef, or function key (ram address[0x60:0x67])
 */
#define HAL_KSCAN_KBD_INFO_GET()                                        (KSCAN->DR.KEY)

/**
 * @brief  KSCAN time config value write, write register 'STR' in @ref KSCAN_TypeDef
 * @param  VAL     Config value. @ref HalKscanTimeCalc, use this function to calculate the config value.
 * @return None
 */
#define HAL_KSCAN_SCAN_TIME_CFG_SET(VAL)                                (KSCAN->STR = (VAL))

/**
 * @brief  KSCAN time config value read, read register 'STR' in @ref KSCAN_TypeDef
 * @return Read the config value write to KSCAN time config register.
 */
#define HAL_KSCAN_SCAN_TIME_CFG_GET()                                   (KSCAN->STR)

/**
 * @brief  Clear KRQ bit when keyboard event scaned, write '0x80' to register 'CR' in @ref KSCAN_TypeDef
 * @return None
 */
#define HAL_KSCAN_KRQ_CLR()                                             (KSCAN->CR.ADDR = KSCAN_RAM_ADDR_KRQ_CLR)

/**
 * @brief  Update LED display value, write register 'DR' in @ref KSCAN_TypeDef
 * @param  IDX     0 to 7, means ROWS0 to ROWS7.
 * @param  VAL     BIT10 to BIT0 means MK10 to MK0 output state, '1' is light off, '0' is light on
 * @return None
 */
#define HAL_KSCAN_LED_DISP(IDX, VAL)                                    \
do {                                                                    \
    KSCAN->CR.ADDR = (KSCAN_RAM_ADDR_LED + (IDX));                      \
    KSCAN->DR.LED = (VAL);                                              \
} while(0U)

/**
 * @brief  Get function key scan value, write register 'CR' of fkey address[0x60:0x67], read register 'DR' in @ref KSCAN_TypeDef
 * @param  IDX     0 to 7, means ROWS0 to ROWS7.
 * @return BIT10 to BIT0 means BR10 to BR0 input state
 */
#define HAL_KSCAN_FKEY_STATE(IDX)                                       \
do {                                                                    \
    KSCAN->CR.ADDR = (KSCAN_RAM_ADDR_FKEY + (IDX));                     \
    return (KSCAN->DR.KEY);                                             \
} while(0U)

/**
 * @brief  Config as velocity keyboard mode, and enable, write '0' to register 'CFGR' in @ref KSCAN_TypeDef
 * @return None
 */
#define HAL_KSCAN_KEY_VEL_EN()                                          (KSCAN->CFGR = 0U)

/**
 * @brief  Config as no velocity keyboard (switch key) mode, and enable, write '2' to register 'CFGR' in @ref KSCAN_TypeDef
 * @return None
 */
#define HAL_KSCAN_KEY_SW_EN()                                           (KSCAN->CFGR = BIT2)

/**
 * @brief  Config to velocity keyboard mode, clr BIT2 of register 'CFGR' in @ref KSCAN_TypeDef
 * @return None
 */
#define HAL_KSCAN_KEY_VEL_CFG()                                         (KSCAN->CFGR &= (~BIT2))

/**
 * @brief  Config to no velocity keyboard (switch key) mode, set BIT2 of register 'CFGR' in @ref KSCAN_TypeDef
 * @return None
 */
#define HAL_KSCAN_KEY_SW_CFG()                                          (KSCAN->CFGR |= ( BIT2))

/**
 * @brief  Config to enable keyboard scan, clr BIT0 of register 'CFGR' in @ref KSCAN_TypeDef
 * @return None
 */
#define HAL_KSCAN_ENABLE()                                              (KSCAN->CFGR &= (~BIT0))

/**
 * @brief  Config to disable keyboard scan, set BIT0 of register 'CFGR' in @ref KSCAN_TypeDef
 * @return None
 */
#define HAL_KSCAN_DISABLE()                                             (KSCAN->CFGR |= ( BIT0))

/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup KSCAN_Exported_Functions KSCAN Exported Functions
 * @{
 */
/**
 * @brief  calculate the config value according to the scan cycle in microsecond and scan duty ratio
 * @param  scanCycleInUs   scan cycle in microsecond
 * @param  scanDutyRatio   0.0f < duty ratio < 1.0f
 * @retval > 0     time config value need to write to KSCAN->STR, @ref HAL_KSCAN_SCAN_TIME_CFG_SET
 * @retval == -1   error, scan cycle out of range
 * @retval == -2   error, scan duty ratio out of range
 */
int32_t HalKscanTimeCalc(float_t scanCycleInUs, float_t scanDutyRatio);

/**
 * @brief  check keyboard result, it will return vaild keyIdx and keyVal if key event occured, user should call this function periodically
 * @param  pKeyEvent   pointer to 16bit memory, for return keyIdx and keyVal
 * @retval == 0    key event occured
 * @retval != 0    no key event occured
 */
int32_t HalKscanKeyEventCheck(uint16_t *pKeyEvent);

/* for code compatible */
#define hal_kscan_time_calc                 HalKscanTimeCalc
#define hal_kscan_key_event_check           HalKscanKeyEventCheck

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

