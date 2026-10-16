/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_gpio.h
 * @brief   Header file of GPIO HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_GPIO_H
#define __A0_HAL_GPIO_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup GPIO
 * @brief  General Purpose I/O (GPIO) with 2 port, 32 pins in GPIO0 and 16 pins in GPIO1
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup GPIO_Exported_Constants GPIO Exported Constants
 * @{
 */
/**
 * @defgroup GPIO_EXT_INT_TYPE GPIO External Interrupt Trigger Type
 * @brief GPIO external interrupt trigger type, use for 'MODE' in macro @ref HAL_GPIO_EXTI_MODE_SET
 * @{
 */
#define GPIO_LOW_LEVEL                      (0x0U)  /*!< Gpio external interrupt low level      */
#define GPIO_FALLING_EDGE                   (0x1U)  /*!< Gpio external interrupt falling edge   */
#define GPIO_HIGH_LEVEL                     (0x2U)  /*!< Gpio external interrupt high level     */
#define GPIO_RISING_EDGE                    (0x3U)  /*!< Gpio external interrupt rising edge    */
/**
 * @}
 */
/**
 * @defgroup GPIO_Output_Drive GPIO Ouput Drive Config
 * @brief GPIO maximum current per pin in output mode, include source current and sink current, use for 'DRV' in macro @ref HAL_GPIO_DRV_SET
 * @{
 */
#define GPIO_DRV_4MA                        (0x0U)  /*!< Drive max 4ma, default                 */
#define GPIO_DRV_8MA                        (0x1U)  /*!< Drive max 8ma                          */
#define GPIO_DRV_12MA                       (0x2U)  /*!< Drive max 12ma                         */
#define GPIO_DRV_16MA                       (0x3U)  /*!< Drive max 16ma                         */
/**
 * @}
 */
/**
 * @defgroup GPIO_Pull GPIO Pull Config
 * @brief GPIO Pull-Up or Pull-Down Activation, use for 'PUPD' in macro @ref HAL_GPIO_PUPD_SET
 * @{
 */
#define GPIO_NOPULL                         (0x0U)  /*!< No Pull-up or Pull-down activation     */
#define GPIO_PULLUP                         (0x1U)  /*!< Pull-up activation                     */
#define GPIO_PULLDOWN                       (0x2U)  /*!< Pull-down activation                   */
/**
 * @}
 */
/**
 * @defgroup GPIO_OUT_TYPE GPIO Output Type
 * @brief GPIO output cmos or open drain
 * @{
 */
#define GPIO_OUT_CMOS                       (0x0U)  /*!< CMOS output                            */
#define GPIO_OUT_OPEN_DRAIN                 (0x1U)  /*!< Open Drain output                      */
/**
 * @}
 */
/**
 * @defgroup GPIO_Alternate_Function_Selection GPIO Alternate Function Selection
 * @brief Alternate function selection of all 48 GPIO pins
 * @details
    | AF0       | AF1       | AF2       | AF3        | AF4        | AF5       | AF6       |
    | --------- | --------- | --------- | ---------- | ---------- | --------- | --------- |
    | GPIO0[0]  | BR0       | AIN13     | D0         | SWO        | PWM9      |           |
    | GPIO0[1]  | BR1       | AIN14     | D1         | MIC_CLK    | PWM10     |           |
    | GPIO0[2]  | BR2       | AIN15     | D2         | TRACE_CLK  | PWM11     |           |
    | GPIO0[3]  | BR3       | AIN16     | D3         | TRACE_D0   | PWM12     |           |
    | GPIO0[4]  | BR4       | AIN17     | D4         | TRACE_D1   | PWM13     |           |
    | GPIO0[5]  | BR5       | AIN18     | D5         | TRACE_D2   | PWM14     | SD_CD     |
    | GPIO0[6]  | BR6       | AIN19     | D6         | TRACE_D3   | PWM15     | SD_WP     |
    | GPIO0[7]  | BR7       | AIN20     | D7         |            |           |           |
    | GPIO0[8]  | MK0       | A10       | SPI3_MISO  | PMW2       | D0        |           |
    | GPIO0[9]  | MK1       | A09       | SPI3_CLK   | PMW1       | D1        |           |
    | GPIO0[10] | MK2       | A08       | SPI3_CS    | PMW0       | D2        |           |
    | GPIO0[11] | MK3       | A07       | SPI1_MOSI  | I2S_DATA4  | D3        |           |
    | GPIO0[12] | MK4       | A06       | SPI1_MISO  | I2S_DATA3  | D4        |           |
    | GPIO0[13] | MK5       | A05       | SPI1_CLK   | I2S_DATA2  | D5        |           |
    | GPIO0[14] | MK6       | A04       | SPI1_CS    | I2S_DATA1  | D6        |           |
    | GPIO0[15] | MK7       | A03       | UART2_RX   | I2S_DATA0  | D7        |           |
    | GPIO0[16] | ROWS0     | SRAM_CS   | SD_DATA0   | PMW7       | I2C_DATAs |           |
    | GPIO0[17] | ROWS1     | BANK      | SD_DATA1   | PMW6       | I2C_CLKs  |           |
    | GPIO0[18] | ROWS2     | ROW0      | SD_DATA2   | SPI2_MOSI  | I2S_BCLK  |           |
    | GPIO0[19] | ROWS3     | ROW1      | SD_DATA3   | SPI2_MISO  | I2S_LRCLK |           |
    | GPIO0[20] | ROWS4     | ROW2      | SD_CLK     | SPI2_CLK   | I2S_MCLK  |           |
    | GPIO0[21] | ROWS5     | KBDIO     | SD_CMD     | SPI2_CS    | I2S_DATA0 |           |
    | GPIO0[22] | ROWS6     | AIN3      | SD_DATA0   | UART2_RX   | I2S_DATA1 | SD_CD     |
    | GPIO0[23] | ROWS7     | AIN4      | SD_DATA1   | UART2_TX   | I2S_DATA2 | SD_WP     |
    | GPIO0[24] | ROWT0     | AIN5      | MIC_DIN1   | SPI1_MOSI  | I2S_DATA3 | I2S_DATA0 |
    | GPIO0[25] | ROWT1     | AIN6      | MIC_DIN0   | SPI1_MISO  | I2S_DATA4 | I2S_BCLK  |
    | GPIO0[26] | ROWT2     | AIN7      | I2CM_SDA   | SPI1_CLK   | UART_RX1  | I2S_LRCLK |
    | GPIO0[27] | ROWT3     | AIN8      | I2CM_SCL   | SPI1_CS    | UART_TX1  | I2S_MCLK  |
    | GPIO0[28] | ROWT4     | AIN9      | MIC_CLK    | SPI2_MOSI  | UART_RX2  | ROW0      |
    | GPIO0[29] | ROWT5     | AIN10     | SWO        | SPI2_MISO  | UART_TX2  | ROW1      |
    | GPIO0[30] | ROWT6     | AIN11     | I2S1_BCLK  | SPI2_CLK   | I2CM_SDA  | ROW2      |
    | GPIO0[31] | ROWT7     | AIN12     | I2S1_LRCLK | SPI2_CS    | I2CM_SCL  | KBDIO     |
    | GPIO1[0]  | SPI_CS    | SDRAM_CKE | SD_DATA0   | SPI3_CS    | I2CS_SDA  |           |
    | GPIO1[1]  | SPI_CLK   | SDRAM_CLK | SD_DATA1   | SPI3_CLK   | I2CS_SCL  |           |
    | GPIO1[2]  | SPI_TX    | SDRAM_WE  | SD_DATA2   | SPI3_MOSI  | UART2_RX  |           |
    | GPIO1[3]  | SPI_RX    | SDRAM_CS  | SD_DATA3   | SPI3_MISO  | UART2_TX  |           |
    | GPIO1[4]  | SPI_WP    | SDRAM_RAS | SD_CLK     | UART1_RX   | I2CM_SDA  |           |
    | GPIO1[5]  | SPI_HOLD  | SDRAM_CAS | SD_CMD     | UART1_TX   | I2CM_SCL  |           |
    | GPIO1[6]  | TCK       | UART3_RX  |            |            |           |           |
    | GPIO1[7]  | TMS       | UART3_TX  |            |            |           |           |
    | GPIO1[8]  | UART0_RX  | SRAM_WR   | I2CS_SDA   | MIC_DIN1   |           |           |
    | GPIO1[9]  | UART0_TX  | SRAM_RD   | I2CS_SCL   | MIC_DIN0   |           |           |
    | GPIO1[10] | BR8       | AIN2      | UART1_RX   | SD_CD      | I2S_DATA5 |           |
    | GPIO1[11] | BR9       | AIN1      | UART1_TX   | SD_WP      | I2S_DATA6 |           |
    | GPIO1[12] | BR10      | AIN0      | SPI3_MOSI  | PWM8       |           |           |
    | GPIO1[13] | MK8       | A02       | UART2_TX   | I2S_LRCLK  | PMW5      |           |
    | GPIO1[14] | MK9       | A01       | UART3_RX   | I2S_BCLK   | PMW4      |           |
    | GPIO1[15] | MK10      | A00       | UART3_TX   | I2S_MCLK   | PMW3      |           |
 * @{
 */
/* AF 0 */
#define GPIO_AF0                            ((uint8_t)0U)   /*!< AF0: GPIO                                  */
/* AF 1 */
#define GPIO_AF1_MK10                       ((uint8_t)1U)   /*!< AF1: MK10 Alternate Function mapping       */
#define GPIO_AF1_MK9                        ((uint8_t)1U)   /*!< AF1: MK0 Alternate Function mapping        */
#define GPIO_AF1_MK8                        ((uint8_t)1U)   /*!< AF1: MK8 Alternate Function mapping        */
#define GPIO_AF1_MK7                        ((uint8_t)1U)   /*!< AF1: MK7 Alternate Function mapping        */
#define GPIO_AF1_MK6                        ((uint8_t)1U)   /*!< AF1: MK6 Alternate Function mapping        */
#define GPIO_AF1_MK5                        ((uint8_t)1U)   /*!< AF1: MK5 Alternate Function mapping        */
#define GPIO_AF1_MK4                        ((uint8_t)1U)   /*!< AF1: MK4 Alternate Function mapping        */
#define GPIO_AF1_MK3                        ((uint8_t)1U)   /*!< AF1: MK3 Alternate Function mapping        */
#define GPIO_AF1_MK2                        ((uint8_t)1U)   /*!< AF1: MK2 Alternate Function mapping        */
#define GPIO_AF1_MK1                        ((uint8_t)1U)   /*!< AF1: MK1 Alternate Function mapping        */
#define GPIO_AF1_MK0                        ((uint8_t)1U)   /*!< AF1: MK0 Alternate Function mapping        */
#define GPIO_AF1_BR10                       ((uint8_t)1U)   /*!< AF1: BR10 Alternate Function mapping       */
#define GPIO_AF1_BR9                        ((uint8_t)1U)   /*!< AF1: BR0 Alternate Function mapping        */
#define GPIO_AF1_BR8                        ((uint8_t)1U)   /*!< AF1: BR8 Alternate Function mapping        */
#define GPIO_AF1_BR7                        ((uint8_t)1U)   /*!< AF1: BR7 Alternate Function mapping        */
#define GPIO_AF1_BR6                        ((uint8_t)1U)   /*!< AF1: BR6 Alternate Function mapping        */
#define GPIO_AF1_BR5                        ((uint8_t)1U)   /*!< AF1: BR5 Alternate Function mapping        */
#define GPIO_AF1_BR4                        ((uint8_t)1U)   /*!< AF1: BR4 Alternate Function mapping        */
#define GPIO_AF1_BR3                        ((uint8_t)1U)   /*!< AF1: BR3 Alternate Function mapping        */
#define GPIO_AF1_BR2                        ((uint8_t)1U)   /*!< AF1: BR2 Alternate Function mapping        */
#define GPIO_AF1_BR1                        ((uint8_t)1U)   /*!< AF1: BR1 Alternate Function mapping        */
#define GPIO_AF1_BR0                        ((uint8_t)1U)   /*!< AF1: BR0 Alternate Function mapping        */
#define GPIO_AF1_ROWT7                      ((uint8_t)1U)   /*!< AF1: ROWT7 Alternate Function mapping      */
#define GPIO_AF1_ROWT6                      ((uint8_t)1U)   /*!< AF1: ROWT6 Alternate Function mapping      */
#define GPIO_AF1_ROWT5                      ((uint8_t)1U)   /*!< AF1: ROWT5 Alternate Function mapping      */
#define GPIO_AF1_ROWT4                      ((uint8_t)1U)   /*!< AF1: ROWT4 Alternate Function mapping      */
#define GPIO_AF1_ROWT3                      ((uint8_t)1U)   /*!< AF1: ROWT3 Alternate Function mapping      */
#define GPIO_AF1_ROWT2                      ((uint8_t)1U)   /*!< AF1: ROWT2 Alternate Function mapping      */
#define GPIO_AF1_ROWT1                      ((uint8_t)1U)   /*!< AF1: ROWT1 Alternate Function mapping      */
#define GPIO_AF1_ROWT0                      ((uint8_t)1U)   /*!< AF1: ROWT0 Alternate Function mapping      */
#define GPIO_AF1_ROWS7                      ((uint8_t)1U)   /*!< AF1: ROWS7 Alternate Function mapping      */
#define GPIO_AF1_ROWS6                      ((uint8_t)1U)   /*!< AF1: ROWS6 Alternate Function mapping      */
#define GPIO_AF1_ROWS5                      ((uint8_t)1U)   /*!< AF1: ROWS5 Alternate Function mapping      */
#define GPIO_AF1_ROWS4                      ((uint8_t)1U)   /*!< AF1: ROWS4 Alternate Function mapping      */
#define GPIO_AF1_ROWS3                      ((uint8_t)1U)   /*!< AF1: ROWS3 Alternate Function mapping      */
#define GPIO_AF1_ROWS2                      ((uint8_t)1U)   /*!< AF1: ROWS2 Alternate Function mapping      */
#define GPIO_AF1_ROWS1                      ((uint8_t)1U)   /*!< AF1: ROWS1 Alternate Function mapping      */
#define GPIO_AF1_ROWS0                      ((uint8_t)1U)   /*!< AF1: ROWS0 Alternate Function mapping      */
#define GPIO_AF1_UART0_TX                   ((uint8_t)1U)   /*!< AF1: UART0_TX Alternate Function mapping   */
#define GPIO_AF1_UART0_RX                   ((uint8_t)1U)   /*!< AF1: UART0_RX Alternate Function mapping   */
#define GPIO_AF1_TMS                        ((uint8_t)1U)   /*!< AF1: TMS Alternate Function mapping        */
#define GPIO_AF1_TCK                        ((uint8_t)1U)   /*!< AF1: TCK Alternate Function mapping        */
#define GPIO_AF1_SPI_WP                     ((uint8_t)1U)   /*!< AF1: SPI_WP Alternate Function mapping     */
#define GPIO_AF1_SPI_HOLD                   ((uint8_t)1U)   /*!< AF1: SPI_HOLD Alternate Function mapping   */
#define GPIO_AF1_SPI_RX                     ((uint8_t)1U)   /*!< AF1: SPI_RX Alternate Function mapping     */
#define GPIO_AF1_SPI_TX                     ((uint8_t)1U)   /*!< AF1: SPI_TX Alternate Function mapping     */
#define GPIO_AF1_SPI_CLK                    ((uint8_t)1U)   /*!< AF1: SPI_CLK Alternate Function mapping    */
#define GPIO_AF1_SPI_CS                     ((uint8_t)1U)   /*!< AF1: SPI_CS Alternate Function mapping     */
/* AF 2 */
#define GPIO_AF2_A00                        ((uint8_t)2U)   /*!< AF2: A00 Alternate Function mapping        */
#define GPIO_AF2_A01                        ((uint8_t)2U)   /*!< AF2: A01 Alternate Function mapping        */
#define GPIO_AF2_A02                        ((uint8_t)2U)   /*!< AF2: A02 Alternate Function mapping        */
#define GPIO_AF2_A03                        ((uint8_t)2U)   /*!< AF2: A03 Alternate Function mapping        */
#define GPIO_AF2_A04                        ((uint8_t)2U)   /*!< AF2: A04 Alternate Function mapping        */
#define GPIO_AF2_A05                        ((uint8_t)2U)   /*!< AF2: A05 Alternate Function mapping        */
#define GPIO_AF2_A06                        ((uint8_t)2U)   /*!< AF2: A06 Alternate Function mapping        */
#define GPIO_AF2_A07                        ((uint8_t)2U)   /*!< AF2: A07 Alternate Function mapping        */
#define GPIO_AF2_A08                        ((uint8_t)2U)   /*!< AF2: A08 Alternate Function mapping        */
#define GPIO_AF2_A09                        ((uint8_t)2U)   /*!< AF2: A09 Alternate Function mapping        */
#define GPIO_AF2_A10                        ((uint8_t)2U)   /*!< AF2: A10 Alternate Function mapping        */
#define GPIO_AF2_AIN0                       ((uint8_t)2U)   /*!< AF2: AIN0 Alternate Function mapping       */
#define GPIO_AF2_AIN1                       ((uint8_t)2U)   /*!< AF2: AIN1 Alternate Function mapping       */
#define GPIO_AF2_AIN2                       ((uint8_t)2U)   /*!< AF2: AIN2 Alternate Function mapping       */
#define GPIO_AF2_AIN20                      ((uint8_t)2U)   /*!< AF2: AIN20 Alternate Function mapping      */
#define GPIO_AF2_AIN19                      ((uint8_t)2U)   /*!< AF2: AIN19 Alternate Function mapping      */
#define GPIO_AF2_AIN18                      ((uint8_t)2U)   /*!< AF2: AIN18 Alternate Function mapping      */
#define GPIO_AF2_AIN17                      ((uint8_t)2U)   /*!< AF2: AIN17 Alternate Function mapping      */
#define GPIO_AF2_AIN16                      ((uint8_t)2U)   /*!< AF2: AIN16 Alternate Function mapping      */
#define GPIO_AF2_AIN15                      ((uint8_t)2U)   /*!< AF2: AIN15 Alternate Function mapping      */
#define GPIO_AF2_AIN14                      ((uint8_t)2U)   /*!< AF2: AIN14 Alternate Function mapping      */
#define GPIO_AF2_AIN13                      ((uint8_t)2U)   /*!< AF2: AIN13 Alternate Function mapping      */
#define GPIO_AF2_AIN12                      ((uint8_t)2U)   /*!< AF2: AIN12 Alternate Function mapping      */
#define GPIO_AF2_AIN11                      ((uint8_t)2U)   /*!< AF2: AIN11 Alternate Function mapping      */
#define GPIO_AF2_AIN10                      ((uint8_t)2U)   /*!< AF2: AIN10 Alternate Function mapping      */
#define GPIO_AF2_AIN9                       ((uint8_t)2U)   /*!< AF2: AIN9 Alternate Function mapping       */
#define GPIO_AF2_AIN8                       ((uint8_t)2U)   /*!< AF2: AIN8 Alternate Function mapping       */
#define GPIO_AF2_AIN7                       ((uint8_t)2U)   /*!< AF2: AIN7 Alternate Function mapping       */
#define GPIO_AF2_AIN6                       ((uint8_t)2U)   /*!< AF2: AIN6 Alternate Function mapping       */
#define GPIO_AF2_AIN5                       ((uint8_t)2U)   /*!< AF2: AIN5 Alternate Function mapping       */
#define GPIO_AF2_AIN4                       ((uint8_t)2U)   /*!< AF2: AIN4 Alternate Function mapping       */
#define GPIO_AF2_AIN3                       ((uint8_t)2U)   /*!< AF2: AIN3 Alternate Function mapping       */
#define GPIO_AF2_KBDIO                      ((uint8_t)2U)   /*!< AF2: KBDIO Alternate Function mapping      */
#define GPIO_AF2_ROW2                       ((uint8_t)2U)   /*!< AF2: ROW2 Alternate Function mapping       */
#define GPIO_AF2_ROW1                       ((uint8_t)2U)   /*!< AF2: ROW1 Alternate Function mapping       */
#define GPIO_AF2_ROW0                       ((uint8_t)2U)   /*!< AF2: ROW0 Alternate Function mapping       */
#define GPIO_AF2_BANK                       ((uint8_t)2U)   /*!< AF2: BANK Alternate Function mapping       */
#define GPIO_AF2_SRAM_CS                    ((uint8_t)2U)   /*!< AF2: SRAM_CS Alternate Function mapping    */
#define GPIO_AF2_SRAM_RD                    ((uint8_t)2U)   /*!< AF2: SRAM_RD Alternate Function mapping    */
#define GPIO_AF2_SRAM_WR                    ((uint8_t)2U)   /*!< AF2: SRAM_WR Alternate Function mapping    */
#define GPIO_AF2_UART3_TX                   ((uint8_t)2U)   /*!< AF2: UART3_TX Alternate Function mapping   */
#define GPIO_AF2_UART3_RX                   ((uint8_t)2U)   /*!< AF2: UART3_RX Alternate Function mapping   */
#define GPIO_AF2_SDRAM_CAS                  ((uint8_t)2U)   /*!< AF2: SDRAM_CAS Alternate Function mapping  */
#define GPIO_AF2_SDRAM_RAS                  ((uint8_t)2U)   /*!< AF2: SDRAM_RAS Alternate Function mapping  */
#define GPIO_AF2_SDRAM_CS                   ((uint8_t)2U)   /*!< AF2: SDRAM_CS Alternate Function mapping   */
#define GPIO_AF2_SDRAM_WE                   ((uint8_t)2U)   /*!< AF2: SDRAM_WE Alternate Function mapping   */
#define GPIO_AF2_SDRAM_CLK                  ((uint8_t)2U)   /*!< AF2: SDRAM_CLK Alternate Function mapping  */
#define GPIO_AF2_SDRAM_CKE                  ((uint8_t)2U)   /*!< AF2: SDRAM_CKE Alternate Function mapping  */
/* AF 3*/
#define GPIO_AF3_UART3_TX                   ((uint8_t)3U)   /*!< AF3: UART3_TX Alternate Function mapping   */
#define GPIO_AF3_UART3_RX                   ((uint8_t)3U)   /*!< AF3: UART3_RX Alternate Function mapping   */
#define GPIO_AF3_UART2_TX                   ((uint8_t)3U)   /*!< AF3: UART2_TX Alternate Function mapping   */
#define GPIO_AF3_UART2_RX                   ((uint8_t)3U)   /*!< AF3: UART2_RX Alternate Function mapping   */
#define GPIO_AF3_SPI1_CS                    ((uint8_t)3U)   /*!< AF3: SPI1_CS Alternate Function mapping    */
#define GPIO_AF3_SPI1_CLK                   ((uint8_t)3U)   /*!< AF3: SPI1_CLK Alternate Function mapping   */
#define GPIO_AF3_SPI1_MISO                  ((uint8_t)3U)   /*!< AF3: SPI1_MISO Alternate Function mapping  */
#define GPIO_AF3_SPI1_MOSI                  ((uint8_t)3U)   /*!< AF3: SPI1_MOSI Alternate Function mapping  */
#define GPIO_AF3_SPI3_CS                    ((uint8_t)3U)   /*!< AF3: SPI3_CS Alternate Function mapping    */
#define GPIO_AF3_SPI3_CLK                   ((uint8_t)3U)   /*!< AF3: SPI3_CLK Alternate Function mapping   */
#define GPIO_AF3_SPI3_MISO                  ((uint8_t)3U)   /*!< AF3: SPI3_MISO Alternate Function mapping  */
#define GPIO_AF3_SPI3_MOSI                  ((uint8_t)3U)   /*!< AF3: SPI3_MOSI Alternate Function mapping  */
#define GPIO_AF3_UART1_TX                   ((uint8_t)3U)   /*!< AF3: UART1_TX Alternate Function mapping   */
#define GPIO_AF3_UART1_RX                   ((uint8_t)3U)   /*!< AF3: UART1_RX Alternate Function mapping   */
#define GPIO_AF3_D7                         ((uint8_t)3U)   /*!< AF3: D7 Alternate Function mapping         */
#define GPIO_AF3_D6                         ((uint8_t)3U)   /*!< AF3: D6 Alternate Function mapping         */
#define GPIO_AF3_D5                         ((uint8_t)3U)   /*!< AF3: D5 Alternate Function mapping         */
#define GPIO_AF3_D4                         ((uint8_t)3U)   /*!< AF3: D4 Alternate Function mapping         */
#define GPIO_AF3_D3                         ((uint8_t)3U)   /*!< AF3: D3 Alternate Function mapping         */
#define GPIO_AF3_D2                         ((uint8_t)3U)   /*!< AF3: D2 Alternate Function mapping         */
#define GPIO_AF3_D1                         ((uint8_t)3U)   /*!< AF3: D1 Alternate Function mapping         */
#define GPIO_AF3_D0                         ((uint8_t)3U)   /*!< AF3: D0 Alternate Function mapping         */
#define GPIO_AF3_I2S_S1_LRCLK               ((uint8_t)3U)   /*!< AF3: I2S_S1_LR Alternate Function mapping  */
#define GPIO_AF3_I2S_S1_BCLK                ((uint8_t)3U)   /*!< AF3: I2S_S1_B Alternate Function mapping   */
#define GPIO_AF3_SWO                        ((uint8_t)3U)   /*!< AF3: SWO Alternate Function mapping        */
#define GPIO_AF3_MIC_CLK                    ((uint8_t)3U)   /*!< AF3: DMIC_CLK Alternate Function mapping   */
#define GPIO_AF3_I2CM_SCL                   ((uint8_t)3U)   /*!< AF3: I2CM_SCL Alternate Function mapping   */
#define GPIO_AF3_I2CM_SDA                   ((uint8_t)3U)   /*!< AF3: I2CM_SDA Alternate Function mapping   */
#define GPIO_AF3_MIC_DIN0                   ((uint8_t)3U)   /*!< AF3: DMIC_DIN0 Alternate Function mapping  */
#define GPIO_AF3_MIC_DIN1                   ((uint8_t)3U)   /*!< AF3: DMIC_DIN1 Alternate Function mapping  */
#define GPIO_AF3_SD_CMD                     ((uint8_t)3U)   /*!< AF3: SD_CMD Alternate Function mapping     */
#define GPIO_AF3_SD_CLK                     ((uint8_t)3U)   /*!< AF3: SD_CLK Alternate Function mapping     */
#define GPIO_AF3_SD_DATA3                   ((uint8_t)3U)   /*!< AF3: SD_DATA3 Alternate Function mapping   */
#define GPIO_AF3_SD_DATA2                   ((uint8_t)3U)   /*!< AF3: SD_DATA2 Alternate Function mapping   */
#define GPIO_AF3_SD_DATA1                   ((uint8_t)3U)   /*!< AF3: SD_DATA1 Alternate Function mapping   */
#define GPIO_AF3_SD_DATA0                   ((uint8_t)3U)   /*!< AF3: SD_DATA0 Alternate Function mapping   */
#define GPIO_AF3_I2CS_SCL                   ((uint8_t)3U)   /*!< AF3: I2CS_SCL Alternate Function mapping   */
#define GPIO_AF3_I2CS_SDA                   ((uint8_t)3U)   /*!< AF3: I2CS_SDA Alternate Function mapping   */
/* AF 4 */
#define GPIO_AF4_I2S_MCLK                   ((uint8_t)4U)   /*!< AF4: I2S_MCLK Alternate Function mapping   */
#define GPIO_AF4_I2S_BCLK                   ((uint8_t)4U)   /*!< AF4: I2S_BCLK Alternate Function mapping   */
#define GPIO_AF4_I2S_LRCLK                  ((uint8_t)4U)   /*!< AF4: I2S_LRCLK Alternate Function mapping  */
#define GPIO_AF4_I2S_DATA0                  ((uint8_t)4U)   /*!< AF4: I2S_DATA0 Alternate Function mapping  */
#define GPIO_AF4_I2S_DATA1                  ((uint8_t)4U)   /*!< AF4: I2S_DATA1 Alternate Function mapping  */
#define GPIO_AF4_I2S_DATA2                  ((uint8_t)4U)   /*!< AF4: I2S_DATA2 Alternate Function mapping  */
#define GPIO_AF4_I2S_DATA3                  ((uint8_t)4U)   /*!< AF4: I2S_DATA3 Alternate Function mapping  */
#define GPIO_AF4_I2S_DATA4                  ((uint8_t)4U)   /*!< AF4: I2S_DATA4 Alternate Function mapping  */
#define GPIO_AF4_PWM0                       ((uint8_t)4U)   /*!< AF4: PWM0 Alternate Function mapping       */
#define GPIO_AF4_PWM1                       ((uint8_t)4U)   /*!< AF4: PWM1 Alternate Function mapping       */
#define GPIO_AF4_PWM2                       ((uint8_t)4U)   /*!< AF4: PWM2 Alternate Function mapping       */
#define GPIO_AF4_PWM8                       ((uint8_t)4U)   /*!< AF4: PWM8 Alternate Function mapping       */
#define GPIO_AF4_SD_WP                      ((uint8_t)4U)   /*!< AF4: SD_WP Alternate Function mapping      */
#define GPIO_AF4_SD_CD                      ((uint8_t)4U)   /*!< AF4: SD_CD Alternate Function mapping      */
#define GPIO_AF4_TRACE_D3                   ((uint8_t)4U)   /*!< AF4: TRACE_D3 Alternate Function mapping   */
#define GPIO_AF4_TRACE_D2                   ((uint8_t)4U)   /*!< AF4: TRACE_D2 Alternate Function mapping   */
#define GPIO_AF4_TRACE_D1                   ((uint8_t)4U)   /*!< AF4: TRACE_D1 Alternate Function mapping   */
#define GPIO_AF4_TRACE_D0                   ((uint8_t)4U)   /*!< AF4: TRACE_D0 Alternate Function mapping   */
#define GPIO_AF4_TRACK_CLK                  ((uint8_t)4U)   /*!< AF4: TRACE_CLK Alternate Function mapping  */
#define GPIO_AF4_MIC_CLK                    ((uint8_t)4U)   /*!< AF4: DMIC_CLK Alternate Function mapping   */
#define GPIO_AF4_SWO                        ((uint8_t)4U)   /*!< AF4: SWO Alternate Function mapping        */
#define GPIO_AF4_SPI2_CS                    ((uint8_t)4U)   /*!< AF4: SPI2_CS Alternate Function mapping    */
#define GPIO_AF4_SPI2_CLK                   ((uint8_t)4U)   /*!< AF4: SPI2_CLK Alternate Function mapping   */
#define GPIO_AF4_SPI2_MISO                  ((uint8_t)4U)   /*!< AF4: SPI2_MISO Alternate Function mapping  */
#define GPIO_AF4_SPI2_MOSI                  ((uint8_t)4U)   /*!< AF4: SPI2_MOSI Alternate Function mapping  */
#define GPIO_AF4_SPI1_CS                    ((uint8_t)4U)   /*!< AF4: SPI1_CS Alternate Function mapping    */
#define GPIO_AF4_SPI1_CLK                   ((uint8_t)4U)   /*!< AF4: SPI1_CLK Alternate Function mapping   */
#define GPIO_AF4_SPI1_MISO                  ((uint8_t)4U)   /*!< AF4: SPI1_MISO Alternate Function mapping  */
#define GPIO_AF4_SPI1_MOSI                  ((uint8_t)4U)   /*!< AF4: SPI1_MOSI Alternate Function mapping  */
#define GPIO_AF4_UART2_TX                   ((uint8_t)4U)   /*!< AF4: UART2_TX Alternate Function mapping   */
#define GPIO_AF4_UART2_RX                   ((uint8_t)4U)   /*!< AF4: UART2_RX Alternate Function mapping   */
#define GPIO_AF4_PWM6                       ((uint8_t)4U)   /*!< AF4: PWM6 Alternate Function mapping       */
#define GPIO_AF4_PWM7                       ((uint8_t)4U)   /*!< AF4: PWM7 Alternate Function mapping       */
#define GPIO_AF4_MIC_DIN0                   ((uint8_t)4U)   /*!< AF4: DMIC_DIN0 Alternate Function mapping  */
#define GPIO_AF4_MIC_DIN1                   ((uint8_t)4U)   /*!< AF4: DMIC_DIN1 Alternate Function mapping  */
#define GPIO_AF4_UART1_TX                   ((uint8_t)4U)   /*!< AF4: UART1_TX Alternate Function mapping   */
#define GPIO_AF4_UART1_RX                   ((uint8_t)4U)   /*!< AF4: UART1_RX Alternate Function mapping   */
#define GPIO_AF4_SPI3_CS                    ((uint8_t)4U)   /*!< AF4: SPI3_CS Alternate Function mapping    */
#define GPIO_AF4_SPI3_CLK                   ((uint8_t)4U)   /*!< AF4: SPI3_CLK Alternate Function mapping   */
#define GPIO_AF4_SPI3_MISO                  ((uint8_t)4U)   /*!< AF4: SPI3_MISO Alternate Function mapping  */
#define GPIO_AF4_SPI3_MOSI                  ((uint8_t)4U)   /*!< AF4: SPI3_MOSI Alternate Function mapping  */
/* AF 5 */
#define GPIO_AF5_PWM3                       ((uint8_t)5U)   /*!< AF5: PWM3 Alternate Function mapping       */
#define GPIO_AF5_PWM4                       ((uint8_t)5U)   /*!< AF5: PWM4 Alternate Function mapping       */
#define GPIO_AF5_PWM5                       ((uint8_t)5U)   /*!< AF5: PWM5 Alternate Function mapping       */
#define GPIO_AF5_D7                         ((uint8_t)5U)   /*!< AF5: D7 Alternate Function mapping         */
#define GPIO_AF5_D6                         ((uint8_t)5U)   /*!< AF5: D6 Alternate Function mapping         */
#define GPIO_AF5_D5                         ((uint8_t)5U)   /*!< AF5: D5 Alternate Function mapping         */
#define GPIO_AF5_D4                         ((uint8_t)5U)   /*!< AF5: D4 Alternate Function mapping         */
#define GPIO_AF5_D3                         ((uint8_t)5U)   /*!< AF5: D3 Alternate Function mapping         */
#define GPIO_AF5_D2                         ((uint8_t)5U)   /*!< AF5: D2 Alternate Function mapping         */
#define GPIO_AF5_D1                         ((uint8_t)5U)   /*!< AF5: D1 Alternate Function mapping         */
#define GPIO_AF5_D0                         ((uint8_t)5U)   /*!< AF5: D0 Alternate Function mapping         */
#define GPIO_AF5_I2S_DATA6                  ((uint8_t)5U)   /*!< AF5: I2S_DATA6 Alternate Function mapping  */
#define GPIO_AF5_I2S_DATA5                  ((uint8_t)5U)   /*!< AF5: I2S_DATA5 Alternate Function mapping  */
#define GPIO_AF5_PWM15                      ((uint8_t)5U)   /*!< AF5: PWM15 Alternate Function mapping      */
#define GPIO_AF5_PWM14                      ((uint8_t)5U)   /*!< AF5: PWM14 Alternate Function mapping      */
#define GPIO_AF5_PWM13                      ((uint8_t)5U)   /*!< AF5: PWM13 Alternate Function mapping      */
#define GPIO_AF5_PWM12                      ((uint8_t)5U)   /*!< AF5: PWM12 Alternate Function mapping      */
#define GPIO_AF5_PWM11                      ((uint8_t)5U)   /*!< AF5: PWM11 Alternate Function mapping      */
#define GPIO_AF5_PWM10                      ((uint8_t)5U)   /*!< AF5: PWM10 Alternate Function mapping      */
#define GPIO_AF5_PWM9                       ((uint8_t)5U)   /*!< AF5: PWM9 Alternate Function mapping       */
#define GPIO_AF5_I2CM_SCL                   ((uint8_t)5U)   /*!< AF5: I2CM_SCL Alternate Function mapping   */
#define GPIO_AF5_I2CM_SDA                   ((uint8_t)5U)   /*!< AF5: I2CM_SDA Alternate Function mapping   */
#define GPIO_AF5_UART2_TX                   ((uint8_t)5U)   /*!< AF5: UART2_TX Alternate Function mapping   */
#define GPIO_AF5_UART2_RX                   ((uint8_t)5U)   /*!< AF5: UART2_RX Alternate Function mapping   */
#define GPIO_AF5_UART1_TX                   ((uint8_t)5U)   /*!< AF5: UART1_TX Alternate Function mapping   */
#define GPIO_AF5_UART1_RX                   ((uint8_t)5U)   /*!< AF5: UART1_RX Alternate Function mapping   */
#define GPIO_AF5_I2S_DATA0                  ((uint8_t)5U)   /*!< AF5: I2S_DATA0 Alternate Function mapping  */
#define GPIO_AF5_I2S_DATA1                  ((uint8_t)5U)   /*!< AF5: I2S_DATA1 Alternate Function mapping  */
#define GPIO_AF5_I2S_DATA2                  ((uint8_t)5U)   /*!< AF5: I2S_DATA2 Alternate Function mapping  */
#define GPIO_AF5_I2S_DATA3                  ((uint8_t)5U)   /*!< AF5: I2S_DATA3 Alternate Function mapping  */
#define GPIO_AF5_I2S_DATA4                  ((uint8_t)5U)   /*!< AF5: I2S_DATA4 Alternate Function mapping  */
#define GPIO_AF5_I2S_MCLK                   ((uint8_t)5U)   /*!< AF5: I2S_MCLK Alternate Function mapping   */
#define GPIO_AF5_I2S_BCLK                   ((uint8_t)5U)   /*!< AF5: I2S_BCLK Alternate Function mapping   */
#define GPIO_AF5_I2S_LRCLK                  ((uint8_t)5U)   /*!< AF5: I2S_LRCLK Alternate Function mapping  */
#define GPIO_AF5_I2CS_SCL                   ((uint8_t)5U)   /*!< AF5: I2CS_SCL Alternate Function mapping   */
#define GPIO_AF5_I2CS_SDA                   ((uint8_t)5U)   /*!< AF5: I2CS_SDA Alternate Function mapping   */
/* AF 6 */
#define GPIO_AF6_SD_WP                      ((uint8_t)6U)   /*!< AF6: SD_WP Alternate Function mapping      */
#define GPIO_AF6_SD_CD                      ((uint8_t)6U)   /*!< AF6: SD_CD Alternate Function mapping      */
#define GPIO_AF6_KBDIO                      ((uint8_t)6U)   /*!< AF6: KBDIO Alternate Function mapping      */
#define GPIO_AF6_ROW2                       ((uint8_t)6U)   /*!< AF6: ROW2 Alternate Function mapping       */
#define GPIO_AF6_ROW1                       ((uint8_t)6U)   /*!< AF6: ROW1 Alternate Function mapping       */
#define GPIO_AF6_ROW0                       ((uint8_t)6U)   /*!< AF6: ROW0 Alternate Function mapping       */
#define GPIO_AF6_I2S_MCLK                   ((uint8_t)6U)   /*!< AF6: I2S_MCLK Alternate Function mapping   */
#define GPIO_AF6_I2S_LRCLK                  ((uint8_t)6U)   /*!< AF6: I2S_LRCLK Alternate Function mapping  */
#define GPIO_AF6_I2S_BCLK                   ((uint8_t)6U)   /*!< AF6: I2S_BCLK Alternate Function mapping   */
#define GPIO_AF6_I2S_DATA0                  ((uint8_t)6U)   /*!< AF6: I2S_DATA0 Alternate Function mapping  */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup GPIO_Exported_Macros GPIO Exported Macros
 * @{
 */
/**
 * @brief  Gpio input data read, read register 'IDR' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @return Gpio input Data
 */
#define HAL_GPIO_IN_DATA_GET(GPIO)                                      ((GPIO)->IDR)

/**
 * @brief  Gpio output data write, write register 'ODR' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  DATA    Write gpio output data to register (e.g. 0x05 to config Px.0 and Px.2 to output high level, other pin output low level)
 * @return None
 */
#define HAL_GPIO_OUT_DATA_SET(GPIO, DATA)                               ((GPIO)->ODR = (DATA))

/**
 * @brief  Gpio output data read, read register 'ODR' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @return Gpio output Data
 */
#define HAL_GPIO_OUT_DATA_GET(GPIO)                                     ((GPIO)->ODR)

/**
 * @brief  Gpio output bit set, write register 'BSR' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Set gpio bit (e.g. 0x05 to config Px.0 and Px.2 to output high level)
 * @return None
 */
#define HAL_GPIO_BIT_SET(GPIO, BITV)                                    ((GPIO)->BSR = (BITV))

/**
 * @brief  Gpio output bit clr, write register 'BCR' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Clr gpio bit (e.g. 0x05 to config Px.0 and Px.2 to output low level)
 * @return None
 */
#define HAL_GPIO_BIT_CLR(GPIO, BITV)                                    ((GPIO)->BCR = (BITV))

/**
 * @brief  Gpio output enable set, set bits (BITV) of register 'OER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Config the pin of gpio to output (e.g. 0x05 to config Px.0 and Px.2 to output without change other pin config)
 * @return None
 */
#define HAL_GPIO_OUT_EN(GPIO, BITV)                                     ((GPIO)->OER |= (BITV))

/**
 * @brief  Gpio input enable set, clr bits (BITV) of register 'OER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Config the pin of gpio to input (e.g. 0x05 to config Px.0 and Px.2 to output without change other pin config)
 * @return None
 */
#define HAL_GPIO_IN_EN(GPIO, BITV)                                      ((GPIO)->OER &= (~(BITV)))

/**
 * @brief  Gpio input/output direction set, write register 'OER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  DATA    Config the pin direction of gpio, '1' means output, '0' means input (e.g. 0x03 to config Px.0~Px.1 output and other pin (Px.2~Px.31) input )
 * @return None
 */
#define HAL_GPIO_DIR_SET(GPIO, DATA)                                    ((GPIO)->OER = (DATA))

/**
 * @brief  Gpio input/output direction get, read register 'OER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @return Gpio input/output direction value, '1' means output, '0' means input (e.g. 0x03 means Px.0~Px.1 are output and other pin (Px.2~Px.31) are input )
 */
#define HAL_GPIO_DIR_GET(GPIO)                                          ((GPIO)->OER)

/**
 * @brief  Gpio pin config to be open drain output, set bits (BITV) of register 'OTYPER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Config the pin of gpio to be open drain mode (e.g. 0x05 to config Px.0 and Px.2 open drain output without change other pin config)
 * @return None
 */
#define HAL_GPIO_OUT_OD(GPIO, BITV)                                     ((GPIO)->OTYPER |= (BITV))

/**
 * @brief  Gpio pin config to be CMOS output, clr bits (BITV) of register 'OTYPER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Config the pin of gpio to be CMOS mode (e.g. 0x05 to config Px.0 and Px.2 CMOS output without change other pin config)
 * @return None
 */
#define HAL_GPIO_OUT_CMOS(GPIO, BITV)                                   ((GPIO)->OTYPER &= (~(BITV)))

/**
 * @brief  Config the group of gpio pin to be open drain or CMOS output, write register 'OTYPER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  DATA    '1' means open drain, '0' means CMOS (e.g. 0x05 to config Px.0 and Px.2 open drain output and other pin CMOS output)
 * @return None
 */
#define HAL_GPIO_OUT_TYPE_SET(GPIO, DATA)                               ((GPIO)->OTYPER = (DATA))

/**
 * @brief  Get the output mode config value of the gpio pin, read register 'OTYPER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @return '1' means open drain, '0' means CMOS (e.g. 0x05 means Px.0 and Px.2 open drain output and other pin CMOS output)
 */
#define HAL_GPIO_OUT_TYPE_GET(GPIO)                                     ((GPIO)->OTYPER)

/**
 * @brief  Gpio pin config input schmitt trigger enable, set bits (BITV) of register 'ISMTER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Config the pin of gpio to enable input schmitt trigger (e.g. 0x05 to config Px.0 and Px.2 enable input schmitt trigger without change other pin config)
 * @return None
 */
#define HAL_GPIO_IN_SCHMITT_ENABLE(GPIO, BITV)                          ((GPIO)->ISMTER |= (BITV))

/**
 * @brief  Gpio pin config input schmitt trigger disable, clr bits (BITV) of register 'ISMTER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  BITV    Config the pin of gpio to disable input schmitt trigger (e.g. 0x05 to config Px.0 and Px.2 disable input schmitt trigger without change other pin config)
 * @return None
 */
#define HAL_GPIO_IN_SCHMITT_DISABLE(GPIO, BITV)                         ((GPIO)->ISMTER &= (~(BITV)))

/**
 * @brief  Config the group of gpio pin input schmitt trigger enable or disable, write register 'ISMTER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  DATA    '1' means enable, '0' means disable (e.g. 0x05 to config Px.0 and Px.2 enable input schmitt trigger and other pin disable)
 * @return None
 */
#define HAL_GPIO_IN_SCHMITT_SET(GPIO, DATA)                             ((GPIO)->ISMTER = (DATA))

/**
 * @brief  Get the input schmitt trigger config value of the gpio pin, read register 'ISMTER' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @return '1' means enable, '0' means disable (e.g. 0x05 means Px.0 and Px.2 enable input schmitt trigger and other pin disable)
 */
#define HAL_GPIO_IN_SCHMITT_GET(GPIO)                                   ((GPIO)->ISMTER)

/**
 * @brief  Gpio alternate function config, update the PIN of GPIO with AFV in registers 'AFR[]' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  PIN     0~31 means Px.0 to Px.31
 * @param  AFV     Gpio alternate function config value, @ref GPIO_Alternate_Function_Selection
 * @return None
 */
#define HAL_GPIO_AF_SET(GPIO, PIN, AFV)                                 \
do {                                                                    \
    uint32_t idx = (((PIN) >> 3U) & 0x3U);                              \
    uint32_t offset = (((PIN) & 0x7U) << 2U);                           \
    __IO uint32_t afr = (GPIO)->AFR[idx];                               \
    (GPIO)->AFR[idx] = (afr & (~(0x7U << offset))) | ((AFV) << offset); \
} while(0U)

/**
 * @brief  Gpio pull-up and pull-down config, update the PIN of GPIO with PUPD in registers 'PUPDR[]' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  PIN     0~31 means Px.0 to Px.31
 * @param  PUPD    Gpio pull-up/pull-down config value, @ref GPIO_Pull
 * @return None
 */
#define HAL_GPIO_PUPD_SET(GPIO, PIN, PUPD)                                      \
do {                                                                            \
    uint32_t idx = (((PIN) >> 4U) & 0x1U);                                      \
    uint32_t offset = (((PIN) & 0xFU) << 1U);                                   \
    __IO uint32_t pupdr = (GPIO)->PUPDR[idx];                                   \
    (GPIO)->PUPDR[idx] = (pupdr & (~(0x3U << offset))) | ((PUPD) << offset);    \
} while(0U)

/**
 * @brief  Gpio output drive config, update the PIN of GPIO with DRV in registers 'DRVR[]' in @ref GPIO_TypeDef
 * @param  GPIO    Gpio instance, GPIO0/GPIO1, @ref Peripheral_Declaration
 * @param  PIN     0~31 means Px.0 to Px.31
 * @param  DRV     Gpio output drive config value, @ref GPIO_Output_Drive
 * @return None
 */
#define HAL_GPIO_DRV_SET(GPIO, PIN, DRV)                                        \
do {                                                                            \
    uint32_t idx = (((PIN) >> 4U) & 0x1U);                                      \
    uint32_t offset = (((PIN) & 0xFU) << 1U);                                   \
    __IO uint32_t drvr = (GPIO)->DRVR[idx];                                     \
    (GPIO)->DRVR[idx] = (drvr & (~(0x3U << offset))) | ((DRV) << offset);       \
} while(0U)

/**
 * @brief  GPIO external interrupt mode config, write bits 'intMode' in register 'CR', @ref EXTINT_CR_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @param  MODE    GPIO external interrupt mode value, @ref GPIO_EXT_INT_TYPE
 * @return None
 */
#define HAL_GPIO_EXTI_MODE_SET(EXTI, MODE)                              ((EXTI)->CR = ((EXTI)->CR & (~0x30U)) | ((MODE) << 4))

/**
 * @brief  GPIO external interrupt mode config, read bits 'intMode' in register 'CR', @ref EXTINT_CR_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @return Value current external interrupt mode, @ref GPIO_EXT_INT_TYPE
 */
#define HAL_GPIO_EXTI_MODE_GET(EXTI)                                    (((EXTI)->CR >> 4) & 0x3U)

/**
 * @brief  GPIO external interrupt function enable, set bit 'intEn' in register 'CR', @ref EXTINT_CR_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_GPIO_EXTI_ENABLE(EXTI)                                      ((EXTI)->CR |= ( BIT1))

/**
 * @brief  GPIO external interrupt function disable, clr bit 'intEn' in register 'CR', @ref EXTINT_CR_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_GPIO_EXTI_DISABLE(EXTI)                                     ((EXTI)->CR &= (~BIT1))

/**
 * @brief  GPIO external interrupt flag clear, set bit 'intState' in register 'CR', @ref EXTINT_CR_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_GPIO_EXTI_CLR(EXTI)                                         ((EXTI)->CR |= ( BIT0))

/**
 * @brief  GPIO external interrupt pin select, write registers 'SELR[]' in @ref EXTINT_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @param  P1SEL   GPIO external interrupt source pin select from GPIO1, e.g. 0x00000105 means P1.0, P1.2 and P1.8 are selected to be the source of external interrupt pins, other pins of GPIO1 are not selected
 * @param  P0SEL   GPIO external interrupt source pin select from GPIO0, e.g. 0x00030000 means P0.16 and P0.17 are selected to be the source of external interrupt pins, other pins of GPIO0 are not selected
 * @return None
 */
#define HAL_GPIO_EXTI_PIN_SEL(EXTI, P1SEL, P0SEL)                       \
do {                                                                    \
    (EXTI)->SELR[0] = (P0SEL);                                          \
    (EXTI)->SELR[1] = (P1SEL);                                          \
} while(0U)

/**
 * @brief  GPIO External interrupt pin select, only select from GPIO0, write register 'SELR[0]' in @ref EXTINT_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @param  SEL     GPIO external interrupt source pin select from GPIO0, e.g. 0x00030000 means P0.16 and P0.17 are selected to be the source of external interrupt pins, other pins of GPIO0 are not selected
 * @return None
 */
#define HAL_GPIO_EXTI_PIN_SEL_P0(EXTI, SEL)                             ((EXTI)->SELR[0] = (SEL))

/**
 * @brief  GPIO external interrupt pin select, only select from GPIO0, write register 'SELR[1]' in @ref EXTINT_TypeDef
 * @param  EXTI    GPIO external interrupt instance, EXTI0/EXTI1, @ref Peripheral_Declaration
 * @param  SEL     GPIO external interrupt source pin select from GPIO1, e.g. 0x00000105 means P1.0, P1.2 and P1.8 are selected to be the source of external interrupt pins, other pins of GPIO1 are not selected
 * @return None
 */
#define HAL_GPIO_EXTI_PIN_SEL_P1(EXTI, SEL)                             ((EXTI)->SELR[1] = (SEL))
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
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
