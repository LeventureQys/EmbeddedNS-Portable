
/**
  ******************************************************************************
  * @file    a0_hal_conf.h
  * @author  huangzheng
  * @brief   HAL configuration file.
  * 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __A0_HAL_CONF_H
#define __A0_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif
     
/* Exported types ------------------------------------------------------------*/
/* Exported definations ------------------------------------------------------*/
/* ########################## Module Configuration ########################## */
/**
  * @brief This is the list of modules configuration of the HAL 
  */
#define SWO_PIN_SELECT                          (29)    // 0, P0.0,      29, P0.29
//#define ITM_ENABLE
//#define TIM_RELEASE_EN                                  // 用于脱机时使能SWO输出打印信息时使能

/* a0_hal_spi_flash.c 配置 */
#define INT_FLASH_TYPE                          (2)     // 0, 512KByte, 1, 2MByte(only JAT1823), 2, 256KByte, 3, 128KByte
#if ((INT_FLASH_TYPE == 2) || (INT_FLASH_TYPE == 3))
#define INT_FLASH_CONTINUE_RD_DISABLE                   // 禁止内部Flash的连续读模式时打开定义，常规模式下请勿打开
#endif
//#define DBG_IN_RAM                                      // 原厂调试模式下使用，常规模式下请勿打开
//#define INT_FLASH_PROTECT_UNLOCK                        // 需要支持解锁内部Flash保护时打开定义，常规模式下请勿打开
//#define EXT_FLASH_SUPPORT                               // 需要控制器支持CS1的外部Flash时打开定义，仅限JAT1823 */
#ifdef EXT_FLASH_SUPPORT
#define EXT_FLASH_TYPE                          (0)     // 0, 3字节地址(小于等于16MByte）, 1, 4字节地址（大于16MByte）
#endif

/* a0_hal_sys_ctrl.c 配置 */
//#define EFUSE_WRITE_EN                                  // 需要使能eFuse烧写函数，大部分情况下不使用
//#define HAL_SYS_WDT_SOFT_RST_EN                         // 使能时，看门狗复位使用软复位，否则进行硬复位

/* a0_hal_otg_device.c 配置，下列定义建议在脱机模式下禁止，来提高软件效率 */  
//#define HAL_OTG_PRINT_EN                                // 需要调试USB，并打印输出时使能
#define USB_MEM_ADDR_CHECK_EN                           // 需要调式USB，并对FIFO操作的memory地址做检查时使能，访问范围[0x10000000:0x1001FFFF]、[0x020000000:0x20007FFF]

/* a0_hal_otg_host.c 配置 */  
//#define USB_MALLOC_EN                                   // 使能malloc/free时，需要在startup_a0.s中定义heap，禁止时需要占用0x400(Bytes)的全局变量给主机获取配置描述符使用
//#define HAL_OTG_PRINT_EN                                // 需要调试USB，并打印输出时使能，脱机模式下禁止，来提高软件效率
#define HAL_USBH_DSC_STR_LANGUAGE_MAX_LEN       (10)    // 最大255
#define HAL_USBH_DSC_STR_MANUFACTURE_MAX_LEN    (128)   // 最大255
#define HAL_USBH_DSC_STR_PRODUCT_MAX_LEN        (128)   // 最大255
#define HAL_USBH_DSC_STR_SERIAL_MAX_LEN         (128)   // 最大255

/* ########################## Module Selection ############################## */
/**
  * @brief This is the list of modules to be used in the HAL driver 
  */

#define HAL_OTG_USAGE                           (1)     // 0, no usb, 1, for device, 2, for host, 3, for otg(need to be added)

#define HAL_SPI_FLASH_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_TIMER_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED  
#define HAL_SPI_STD_MODULE_ENABLED
#define HAL_I2C_SLAVE_MODULE_ENABLED
#define HAL_I2C_MASTER_MODULE_ENABLED
#define HAL_PWM_MODULE_ENABLED
#define HAL_I2S_MODULE_ENABLED
#define HAL_KSCAN_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#if (HAL_OTG_USAGE == 1)
#define HAL_OTG_DEV_MODULE_ENABLED
#elif (HAL_OTG_USAGE == 2)
#define HAL_OTG_HOST_MODULE_ENABLED
#endif

#ifdef __cplusplus
}
#endif

#endif 
