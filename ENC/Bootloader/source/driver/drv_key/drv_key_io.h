
/**
  ******************************************************************************
  * @file    drv_key_io.h
  * @brief    Header file of  Independent Key DRV module.
  * @version V1.0.0
  * @date    2021/09/20
  * @author  ziyingli
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DRV_KEY_IO_H__
#define __DRV_KEY_IO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 该模块不支持多按键同时按下响应，每次仅响应一个按键，包括长按等 */
    
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"  
#include "a0_hal_def.h"
#include "a0_hal_gpio.h"

/* Exported constants --------------------------------------------------------*/   
#define IO_PORT_MAX                     (2)         //和MCU的GPIO模块相关
#define IO_KEY_ON_STATE                 (0)         //0，按键按下低电平，非0，按键按下高电平
#define IO_KEY_PUPD                     (1)         //0, IO无需上下拉，1，内部上拉，2，内部下拉    
    
/* Exported types ------------------------------------------------------------*/   
 /** @defgroup Independent_Key_Exported_Types Key Exported Types
  * @{
  */
    
#define GPIO_DIR_IN(port,pin) 				    (HAL_GPIO_IN_EN(port,pin))
#define GPIO_IN_GET(port, pin)				    (HAL_GPIO_IN_DATA_GET(port)&pin)	
	
 /**
* @brief Independent Key Configuration Structure definition
*/
typedef struct {
    uint16_t    keyPressedInMs;                     //按键长按响应时间，毫秒为单位，如果配置成0，则表示无长按响应
    uint32_t    ioNum[IO_PORT_MAX];                 //2组Port口，每组使用IO作为按键数量
    uint32_t    *pIoInfo[IO_PORT_MAX];              //指向每组使用Pin的数组，ioNum为0时，此指针无效
    FUNCP       *pKeyOnCb[IO_PORT_MAX];             //指向按键Callback函数
    FUNCP       *pKeyOffCb[IO_PORT_MAX];            //指向按键Callback函数
    FUNCP       *pKeyPressedCb[IO_PORT_MAX];        //指向按键Callback函数
    uint32_t    ioMask[IO_PORT_MAX];                //初始化会根据IO使用情况得到Mask值
    uint32_t    state[IO_PORT_MAX+2];               //模块使用的变量，0~1，2组PORT口状态，2，扫描毫秒计数，3，长按毫秒计数
}KEY_IO_PARA_TypeDef;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup Independent_Key_Exported_Functions  Independent Key Exported Functions
  * @{
  */
void DrvKeyIoInit(KEY_IO_PARA_TypeDef *pPara);
void DrvKeyIoProcess(KEY_IO_PARA_TypeDef *pPara);


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
