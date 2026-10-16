/**
  ******************************************************************************
  * @file    demo_spi_std.h
  * @brief   Header file of DRV_SPI_STD DRV module.
  * @version V1.0.0
  * @date    2021/10/28
  * @author  lijixuan
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef _APP_SPI_STD_H_
#define _APP_SPI_STD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_spi_std.h"
#include "drv_spi_std.h"
#define 		DEMO_SPI

/** @addtogroup APP_SPI_STD
  * @{
  */

/** @addtogroup UART
  * @{
  */ 

/* Exported functions --------------------------------------------------------*/
/** @defgroup APP_SPI_STD_Exported_Functions SPI Application Exported Functions
  * @{
  */
void DemoSpi(void);
void DemoSpiMaster(void);
void DemoSpiSlave(void);
void DemoSpiStdFlash(void);
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


