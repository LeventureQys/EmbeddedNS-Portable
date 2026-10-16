/**
  ******************************************************************************
  * @file    drv_uart.h
  * @brief   Header file of UART DRV module.
  * @version V1.0.0
  * @date    2021/09/16
  * @author  lijixuan
  ******************************************************************************
  */
	
/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DRV_UART_H
#define __DRV_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_uart.h"

/** @addtogroup UART_Drive_Driver
  * @{
  */
	
/** @addtogroup UART
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/
/** @defgroup UART_DRV_Exported_Types UART Exported Types
  * @{
  */

/**
  * @brief  UART Configuration Structure the Tx_Buff/Rx_Buff definition
  */
typedef struct{
    int32_t     buffSize;              /*!< Uart RingBuff size          */
    int32_t     wrIdx;                 /*!< Uart RingBuff of head       */
    int32_t     rdIdx;                 /*!< Uart RingBuff of tail       */
    uint8_t*    pBuffStart;            /*!< Uart RingBuff buffer begin */
}DRV_UART_BUFF_TypeDef;

/**
  * @brief  UART Configuration Structure the basic parameters
  */
typedef struct{
    uint32_t uartIdx;       /*!< UART IDx   0~3                                          */
    uint32_t baudrate;      /*!< UART baudrate                                           */
    uint32_t bitNum;        /*!< UART bitNum          
	                               This parameter can be a value of @ref UART data bit number*/
	
    uint32_t stopNum;       /*!< UART stopNum          
	                               This parameter can be a value of @ref UART stop bit number*/
	
    uint32_t parityStyle;   /*!< UART parityStyle          
	                               This parameter can be a value of @ref UART parity style*/
	
}CONFIG_UART_TypeDef;

/** 
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup UART_DRV_Exported_Constants UART Exported Constants
  * @{
  */

/** @defgroup Allocate_buffer_space Allocate buffer space for TxBuff/RxBuff
  * @brief Customize the size of the TxBuff/RxBuff
  * @{
  */
#define DRV_UART0_RX_BUFF_SIZE     (256)	
#define DRV_UART1_RX_BUFF_SIZE     (256) 
#define DRV_UART2_RX_BUFF_SIZE     (256) 
#define DRV_UART3_RX_BUFF_SIZE     (256) 
#define DRV_UART0_TX_BUFF_SIZE     (256) 
#define DRV_UART1_TX_BUFF_SIZE     (256) 
#define DRV_UART2_TX_BUFF_SIZE     (256) 
#define DRV_UART3_TX_BUFF_SIZE     (256) 

/**
  * @}
  */

/** @defgroup UART_DATA_BIT_ItTring_Ctrl UART interrupt control
  * @brief UART interrupt trigger
  * @{
  */
#define USART_It_Tx_Trig_En_Pos                     (5U)           
#define USART_It_Tx_Trig_En_Msk                     (0x00000020U)  
#define USART_It_Tx_Trig_Enable                     (0x00000020U)  /*!< UART interrupt enable          */
#define USART_It_Tx_Trig_Disable                    (0x00000000U)  /*!< UART interrupt disable         */
/**
  * @}
  */

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
extern uint8_t drvUart0TxBuff[DRV_UART0_TX_BUFF_SIZE];
extern uint8_t drvUart0RxBuff[DRV_UART0_RX_BUFF_SIZE];
extern uint8_t drvUart1TxBuff[DRV_UART1_TX_BUFF_SIZE];
extern uint8_t drvUart1RxBuff[DRV_UART1_RX_BUFF_SIZE];
extern uint8_t drvUart2TxBuff[DRV_UART2_TX_BUFF_SIZE];
extern uint8_t drvUart2RxBuff[DRV_UART2_RX_BUFF_SIZE];
extern uint8_t drvUart3TxBuff[DRV_UART3_TX_BUFF_SIZE];
extern uint8_t drvUart3RxBuff[DRV_UART3_RX_BUFF_SIZE];
extern DRV_UART_BUFF_TypeDef drvUart0TxBuffPara;
extern DRV_UART_BUFF_TypeDef drvUart0RxBuffPara;
extern DRV_UART_BUFF_TypeDef drvUart1TxBuffPara;
extern DRV_UART_BUFF_TypeDef drvUart2TxBuffPara;
extern DRV_UART_BUFF_TypeDef drvUart2RxBuffPara;
extern DRV_UART_BUFF_TypeDef drvUart3TxBuffPara;
extern DRV_UART_BUFF_TypeDef drvUart3RxBuffPara;
extern DRV_UART_BUFF_TypeDef *pDrvUartBuffPara[4][2];         //Uart0~Uart3, rx/tx

/* Exported macros -----------------------------------------------------------*/
/** @defgroup UART_DRV_Exported_Macros UART Exported Macros
  * @{
  */

/** @defgroup UART_DATA_BIT_ItTring_Ctrl UART interrupt control
  * @brief  UART IRQ interrupt control
	* @param  UARTx : The ID of UART
  *         This parameter can be one of the these values
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
	* @{
  */
#define DRV_UART_TX_IRQ_EN(UARTx)                   {NVIC_ClearPendingIRQ(((IRQn_Type)(UART0_TX_IRQn + ((UARTx) << 1))));NVIC_EnableIRQ(((IRQn_Type)(UART0_TX_IRQn + ((UARTx) << 1))));}
#define DRV_UART_RX_IRQ_EN(UARTx)                   NVIC_EnableIRQ(((IRQn_Type)(UART0_RX_IRQn + ((UARTx) << 1))))
#define DRV_UART_TX_IRQ_DIS(UARTx)                  NVIC_DisableIRQ(((IRQn_Type)(UART0_TX_IRQn + ((UARTx) << 1))))
#define DRV_UART_RX_IRQ_DIS(UARTx)                  NVIC_DisableIRQ(((IRQn_Type)(UART0_RX_IRQn + ((UARTx) << 1))))
/**
  * @}
  */

/** @defgroup UART_DRV_interrupt_trigger UART interrupt trigger
  * @brief  UART interrupt trigger
	* @{
  */
#define DRV_UART_INT_CTRL_SET(UARTn,ICV_Msk,ICV)    (UARTn)->ICR = (((UARTn)->ICR & (~ICV_Msk))|ICV)
/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup UART_DRV_Exported_Functions UART Drv Exported Functions
  * @{
  */
int32_t DrvUartDeInit(UART_TypeDef* UARTx);
void DrvUartStop(UART_TypeDef* UARTx);
void DrvUartStart(UART_TypeDef* UARTx);
void DrvUartSetBitNum(UART_TypeDef *UARTx,uint32_t bitNum);
void DrvUartSetBaudRate(UART_TypeDef *UARTx,uint32_t baudrate);
void DrvUartSetStopNum(UART_TypeDef *UARTx,uint32_t stopNum);
void DrvUartSetParityStyle(UART_TypeDef *UARTx,uint32_t parityStyle);
void DrvUartSetTxIntTrigleNum(UART_TypeDef *UARTx,uint32_t uartTxIntTringNum);
void DrvUartSetRxIntTrigleNum(UART_TypeDef *UARTx,uint32_t uartRxIntTringNum);
int DrvUartInit(CONFIG_UART_TypeDef* parUART);
void DrvUartBuffInit(void);
int32_t DrvUartTxNumGet(uint8_t uartIdx);
int32_t DrvUartTxDataWrite(uint8_t uartIdx, uint8_t *pu8Buff, size_t num);
int32_t DrvUartRxDataRead(uint8_t uartIdx, uint8_t *pu8Buff, size_t num);
int32_t DrvUartRxNumGet(uint8_t uartIdx);
__RAM_CODE int32_t DrvUartRxIrqHandle(uint8_t uartIdx);
__RAM_CODE void DrvUartTxIrqHandle(uint8_t uartIdx);
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

#endif /*__DRV_UART_H*/


