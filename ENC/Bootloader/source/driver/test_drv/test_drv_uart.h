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
#ifndef __TEST_DRV_UART_H
#define __TEST_DRV_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_uart.h"

/** @addtogroup App_UART
  * @{
  */

/** @addtogroup UART
  * @{
  */

/*UART_applicatino_functions-----------------------------------------------------*/
/** @defgroup UART_applicatino_functions
  * @{
  */
void AppConversionChar(uint8_t *source,uint16_t num);
void TestDrvUart(void);
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

