/*
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD. 
 *
 ******************************************************************************
 * @file    jat1821.h
 * @brief   Header file of chip jat1821(QFN40).
    | Version | Data       | Author     | Info           |
    | ------- | ---------- | ---------- | -------------- |
    | 1.0.0   | 2022/03/28 | huangzheng | First version  |
 * @version V1.0.0
 * @date    2022/03/28
 * @author  huangzheng
 * @details
 ******************************************************************************
 */

#ifndef __JAT1821_H
#define __JAT1821_H

#include "a0.h"
#include "a0_hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * JAT1821的通用IO及其复用如下：
 * | AF0       | AF1  | AF2      | AF3          | AF4       | AF5       | AF6       |
 * | --------- | ---- | -------- | ------------ | --------- | --------- | --------- |
 * |           |      |          |              |           |           |           |
 * | GPIO0[0]  |      | AIN13    |              | SWO       | PWM9      |           |
 * | GPIO0[1]  |      | AIN14    |              | DMIC_CLK  | PWM10     |           |
 * | GPIO0[18] |      |          |              | SPI2_MOSI | I2S_BCLK  |           |
 * | GPIO0[19] |      |          |              | SPI2_MISO | I2S_LRCLK |           |
 * | GPIO0[20] |      |          |              | SPI2_CLK  | I2S_MCLK  |           |
 * | GPIO0[24] |      | AIN5     | DMIC_IN1     | SPI1_MOSI | I2S_D3    | I2S_D0    |
 * | GPIO0[25] |      | AIN6     | DMIC_IN0     | SPI1_MISO | I2S_D4    | I2S_BCLK  |
 * | GPIO0[26] |      | AIN7     | I2C_M_SDA    | SPI1_CLK  | UART_RX1  | I2S_LRCLK |
 * | GPIO0[27] |      | AIN8     | I2C_M_SCL    | SPI1_CS   | UART_TX1  | I2S_MCLK  |
 * | GPIO0[28] |      | AIN9     | DMIC_CLK     | SPI2_MOSI | UART_RX2  |           |
 * | GPIO0[29] |      | AIN10    | SWO          | SPI2_MISO | UART_TX2  |           |
 * | GPIO0[30] |      | AIN11    | I2S_S1_BCLK  | SPI2_CLK  | I2C_M_SDA |           |
 * | GPIO0[31] |      | AIN12    | I2S_S1_LRCLK | SPI2_CS   | I2C_M_SCL |           |
 * | GPIO1[6]  | TCK  | UART_RX3 |              |           |           |           |
 * | GPIO1[7]  | TMS  | UART_TX3 |              |           |           |           |
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
#ifdef I2S_FIFO1
#undef I2S_FIFO1
#endif
#ifdef I2S_FIFO2
#undef I2S_FIFO2
#endif
#ifdef SPI3
#undef SPI3
#endif
#ifdef KSCAN
#undef KSCAN
#endif

/* Exported macros ----------------------------------------------------------*/
/**
 * @brief  If use chip JAT1821, this macro should be called when initial to config
 *         p0[2:17,21:23], p1[0:5,8:15] to pull-down, p1[6:7] to pull-up, other io no pull
 * @return None
 */
__STATIC_FORCEINLINE void JAT1821GpioPulldownConfig(void)
{
    GPIO0->PUPDR[0] = 0xAAAAAAA0;
    GPIO0->PUPDR[1] = 0x0000A80A;
    GPIO1->PUPDR[0] = 0xAAAA5AAA;
    return;
}

/* Exported functions -------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
