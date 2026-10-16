/*
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD. 
 *
 ******************************************************************************
 * @file    jat1621.h
 * @brief   Header file of chip jat1621(QFN32).
    | Version | Data       | Author     | Info           |
    | ------- | ---------- | ---------- | -------------- |
    | 1.0.0   | 2024/04/15 | huangzheng | First version  |
 * @version V1.0.0
 * @date    2024/04/15
 * @author  huangzheng
 * @details
 ******************************************************************************
 */

#ifndef __JAT1621_H
#define __JAT1621_H

#include "a0.h"
#include "a0_hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * JAT1621的通用IO及其复用如下：
 * | AF0       | AF1  | AF2      | AF3      | AF4       | AF5      | AF6  |
 * | --------- | ---- | -------- | -------- | --------- | -------- | ---- |
 * | GPIO0[2]  |      | AIN15    |          |           | PWM11    |      |
 * | GPIO0[4]  |      | AIN17    |          |           | PWM13    |      |
 * | GPIO0[6]  |      | AIN19    |          |           | PWM15    |      |
 * | GPIO0[27] |      | AIN8     | I2CM_SCL |           | UART_TX1 |      |
 * | GPIO0[28] |      | AIN9     |          | SPI2_MOSI | UART_RX2 |      |
 * | GPIO0[29] |      | AIN10    | SWO      |           | UART_TX2 |      |
 * | GPIO1[4]  |      |          |          | UART_RX1  | I2CM_SDA |      |
 * | GPIO1[6]  | TCK  | UART_RX3 |          |           |          |      |
 * | GPIO1[7]  | TMS  | UART_TX3 |          |           |          |      |
 */

/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/*
 * undefine modules in a0.h
 */
#ifdef UART0
#undef UART0
#endif
#ifdef I2CSlave
#undef I2CSlave
#endif
#ifdef I2S_FIFO0
#undef I2S_FIFO0
#endif
#ifdef I2S_FIFO1
#undef I2S_FIFO1
#endif
#ifdef I2S_FIFO2
#undef I2S_FIFO2
#endif
#ifdef I2S_FIFO3
#undef I2S_FIFO3
#endif
#ifdef I2S_FIFO4
#undef I2S_FIFO4
#endif
#ifdef SPI1
#undef SPI1
#endif
#ifdef SPI3
#undef SPI3
#endif
#ifdef KSCAN
#undef KSCAN
#endif

/* Exported macros ----------------------------------------------------------*/
/**
 * @brief  If use chip JAT1621, this macro should be called when initial to config
 *         p0[0:1,3,5,7:26,30:31], p1[0:3,5,8:15] to pull-down, p1[6:7] to pull-up, other io no pull
 * @return None
 */
__STATIC_FORCEINLINE void JAT1621GpioPulldownConfig(void)
{
    GPIO0->PUPDR[0] = 0xAAAA888A;
    GPIO0->PUPDR[1] = 0xA02AAAAA;
    GPIO1->PUPDR[0] = 0xAAAA58AA;
    return;
}

/* Exported functions -------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
