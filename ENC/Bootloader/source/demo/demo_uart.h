/**
  ******************************************************************************
  * @file    app_uart.h
  * @brief   Header file of App UART module.
  * @version V1.0.0
  * @date    2021/09/023
  * @author  lijixuan
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __APP_UART_H
#define __APP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_uart.h"
#include "drv_uart.h"

#define DEMO_UART    

/** @addtogroup DEMO_UART
  * @{
  */

/** @addtogroup UART
  * @{
  */

/*UART_applicatino_functions-----------------------------------------------------*/
/** @defgroup UART_applicatino_functions
  * @{
  */
void demo_conversion_char(uint8_t *source,uint16_t num);
void DemoUart(void);
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

