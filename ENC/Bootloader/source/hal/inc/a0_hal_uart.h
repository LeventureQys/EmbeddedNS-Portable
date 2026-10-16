/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_uart.h
 * @brief   Header file of UART HAL module.
 * @version V1.0.5
 * @date    2022/11/15
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_UART_H
#define __A0_HAL_UART_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup UART
 * @brief 4 group Universal Asynchronous Receiver Transmitter (UART) module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup UART_Exported_Types UART Exported Types
 * @{
 */
/**
 * @brief  UART Configuration Structure definition
 */
typedef struct
{
    UART_CR_TypeDef     cr;                 /*!< uart ctrl register, @ref UART_CR_TypeDef                   */
    UART_ICR_TypeDef    icr;                /*!< uart interrupt ctrl register, @ref UART_ICR_TypeDef        */
    uint32_t            baudrate;           /*!< uart baudrate value, e.g. baudrate = 19200;                */
    uint32_t            timeout;            /*!< uart rx timeout value write to register, uart cycle number */
    uint32_t            uartIdx;            /*!< uart index number to init, 0~3                             */
}UART_InitTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup UART_Exported_Constants UART Exported Constants
 * @{
 */
#define UART_FIFO_SIZE                      (16)    /*!< Uart fifo max number is 16 */

/**
 * @defgroup UART_DATA_BIT UART Data Bit Number
 * @brief UART data bit number, for 'bitNum' in @ref UART_CR_TypeDef
 * @{
 */
#define UART_5BIT                           (0x0U)  /*!< Uart bit numebr is 5                   */
#define UART_6BIT                           (0x1U)  /*!< Uart bit number is 6                   */
#define UART_7BIT                           (0x2U)  /*!< Uart bit number is 7                   */
#define UART_8BIT                           (0x3U)  /*!< Uart bit number is 8                   */
/**
 * @}
 */

/**
 * @defgroup UART_STOP_BIT UART Stop Bit Number
 * @brief UART stop bit number, for 'stopNum' in @ref UART_CR_TypeDef
 * @{
 */
#define UART_STOP_1BIT                      (0x0U)  /*!< Uart stop bit is 1 bit                 */
#define UART_STOP_2BIT                      (0x1U)  /*!< Uart stop bit is 2 bit                 */
/**
 * @}
 */

/**
 * @defgroup UART_PARITY_STYLE UART Parity Style
 * @brief UART parity style, for 'parityStyle' in @ref UART_CR_TypeDef
 * @{
 */
#define UART_NO_PARITY                      (0x0U)  /*!< No parity, value 0~3 are the same      */
#define UART_ODD_PARITY                     (0x4U)  /*!< Odd parity                             */
#define UART_EVEN_PARITY                    (0x5U)  /*!< Even parity                            */
#define UART_FORCE_ONE                      (0x6U)  /*!< Force '1'                              */
#define UART_FORCE_ZERO                     (0x7U)  /*!< Force '0'                              */
/**
 * @}
 */

/**
 * @defgroup UART_DMA_MODE UART Dma Mode
 * @brief UART dma mode, for 'dmaMode' in @ref UART_CR_TypeDef
 * @{
 */
#define UART_DMA_UNUSED                     (0x0U)  /*!< Dma unused                             */
#define UART_DMA_TX                         (0x1U)  /*!< Dma use for tx                         */
#define UART_DMA_RX                         (0x2U)  /*!< Dma use for rx                         */
#define UART_DMA_TXRX                       (0x3U)  /*!< Dma use for tx and rx                  */
/**
 * @}
 */

/**
 * @defgroup UART_FIFO_RST_CTRL UART Fifo Reset Ctrl
 * @brief UART fifo reset ctrl, for RCV in macro @ref HAL_UART_FIFO_RST
 * @{
 */
#define UART_FIFO_RST_OFFSET                (8)     /*!< reset fifo ctrl bit offset in 'CR'     */
#define UART_FIFO_RST_RX                    (0x1U)  /*!< reset rx fifo only                     */
#define UART_FIFO_RST_TX                    (0x2U)  /*!< reset tx fifo only                     */
#define UART_FIFO_RST_TXRX                  (0x3U)  /*!< reset tx and rx fifo                   */
/**
 * @}
 */

/**
 * @defgroup UART_RX_FIFO_TRIG UART Rx Fifo Trigger
 * @brief UART rx fifo trigger config value, for 'rxTrigNum' in @ref UART_ICR_TypeDef
 * @{
 */
#define UART_RX_NE0                         (0x0U)  /*!< data number in rx fifo != 0            */
#define UART_RX_BE4                         (0x1U)  /*!< data number in rx fifo >= 4            */
#define UART_RX_BE8                         (0x2U)  /*!< data number in rx fifo >= 8            */
#define UART_RX_FULL                        (0x3U)  /*!< data number in rx fifo == 16           */
/**
 * @}
 */

/**
 * @defgroup UART_TX_FIFO_TRIG UART Tx Fifo Trigger
 * @brief UART tx fifo trigger config value, for 'txTrigNum' in @ref UART_ICR_TypeDef
 * @{
 */
#define UART_TX_E0                          (0x0U)  /*!< data number in tx fifo == 0            */
#define UART_TX_SE4                         (0x1U)  /*!< data number in tx fifo <= 4            */
#define UART_TX_SE8                         (0x2U)  /*!< data number in tx fifo <= 8            */
#define UART_TX_NEFULL                      (0x3U)  /*!< data number in tx fifo != 16           */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup UART_Exported_Macros UART Exported Macros
 * @{
 */
/**
 * @brief  Config uart rx timeout value, write register 'RTOR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @param  TOV     Time out value write to the register
 * @return None
 */
#define HAL_UART_TIMEOUT_SET(UART, TOV)                                 ((UART)->RTOR = (TOV))

/**
 * @brief  Calculate baud set value
 * @param  CLK     UART clock, use pclk
 * @param  BAUD    Baud rate set by the user.
 * @return Baudrate set value
 */
#define HAL_UART_BAUD_VALUE(CLK, BAUD)                                  (((CLK) + 8 * (BAUD)) / (16 * (BAUD)) - 1)

/**
 * @brief  Uart baud set, write register 'BRCR' in @ref UART_TypeDef
 * @param  UART        Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @param  BAUDSET     Baud rate set value write to register
 * @return None
 */
#define HAL_UART_BAUD_SET(UART, BAUDSET)                                ((UART)->BRCR = (BAUDSET))

/**
 * @brief  Uart interrupt ctrl value set, write register 'ICR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @param  ICV     Interrupt ctrl value write to register, @ref UART_ICR_TypeDef
 * @return None
 */
#define HAL_UART_INT_CTRL_SET(UART, ICV)                                ((UART)->ICR = (ICV))

/**
 * @brief  Uart interrupt ctrl value set, read register 'ICR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return Value read from intterrupt ctrl register, @ref UART_ICR_TypeDef
 */
#define HAL_UART_INT_CTRL_GET(UART)                                     ((UART)->ICR)

/**
 * @brief  Uart interrupt rx error flag clear, set bit 'rxErrClr' of register 'ICR' ( @ref UART_ICR_TypeDef ) in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_UART_INT_RX_ERR_CLR(UART)                                   ((UART)->ICR |= (BIT8))

/**
 * @brief  Uart ctrl value set, write register 'CR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @param  CV      Ctrl value write to register, @ref UART_CR_TypeDef
 * @return None
 */
#define HAL_UART_CTRL_SET(UART, CV)                                     ((UART)->CR = (CV))

/**
 * @brief  Uart ctrl value get, read register 'CR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return Value read from ctrl register, @ref UART_CR_TypeDef
 */
#define HAL_UART_CTRL_GET(UART)                                         ((UART)->CR)

/**
 * @brief  Uart fifo reset ctrl, set bit 'rxFifoRst' and 'txFifoRst' of 'CR' ( @ref UART_CR_TypeDef ) in @ref UART_TypeDef by RCV input
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @param  RCV     Reset ctrl value, @ref UART_FIFO_RST_CTRL
 * @return Value read from ctrl register
 */
#define HAL_UART_FIFO_RST(UART, RCV)                                    ((UART)->CR |= ((RCV) << UART_FIFO_RST_OFFSET))

/**
 * @brief  Uart clk enable ctrl, set bit 'clkEn' of 'CR' ( @ref UART_CR_TypeDef ) in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_UART_CLK_ENABLE(UART)                                       ((UART)->CR |= ( BIT10))

/**
 * @brief  Uart clk disable ctrl, clr bit 'clkEn' of 'CR' ( @ref UART_CR_TypeDef ) in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_UART_CLK_DISABLE(UART)                                      ((UART)->CR &= (~BIT10))

/**
 * @brief  Uart tx data write, write register 'DR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @param  VAL     Uart data register write TX value
 * @return None
 */
#define HAL_UART_DATA_SET(UART, VAL)                                    ((UART)->DR = VAL)

/**
 * @brief  Uart rx data read, read register 'DR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return RX Data
 */
#define HAL_UART_DATA_GET(UART)                                         ((UART)->DR)

/**
 * @brief  Uart fifo address get
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return Uart fifo address, for DMA config use
 */
#define HAL_UART_FIFO_ADDR_GET(UART)                                    ((uint32_t)(&((UART)->DR)))

/**
 * @brief  Uart state value get, read register 'SR' in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return Value read from state register, @ref UART_SR_TypeDef
 */
#define HAL_UART_STATE_GET(UART)                                        ((UART)->SR)

/**
 * @brief  Uart rx fifo data count get from state register, read 'rxFifoCnt' of 'SR' ( @ref UART_SR_TypeDef ) in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return count number of data in rx fifo
 */
#define HAL_UART_RX_FIFO_CNT_GET(UART)                                  (((UART)->SR >> 4) & 0x1F)
/**
 * @brief  Uart rx fifo data count get from state register, read 'txFifoCnt' of 'SR' ( @ref UART_SR_TypeDef ) in @ref UART_TypeDef
 * @param  UART    Uart instance, UART0/UART1/UART2/UART3, @ref Peripheral_Declaration
 * @return count number of data in tx fifo
 */
#define HAL_UART_TX_FIFO_CNT_GET(UART)                                  (((UART)->SR >> 11) & 0x1F)
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup UART_Exported_Functions UART Exported Functions
 * @{
 */
/**
 * @brief  uart module initial process function
 * @param  pInit    pointer to structure of @ref UART_InitTypeDef
 * @return None
 */
void HalUartInit(UART_InitTypeDef *pInit);

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
