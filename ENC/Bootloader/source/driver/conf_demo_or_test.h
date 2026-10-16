#ifndef __CONF_DEMO_OR_TEST_H_
#define __CONF_DEMO_OR_TEST_H_

//#define CONF_SEL_DEMO      //宏定义打开选择demo示例,不能和 #define CONF_SEL_TEST_DRV 一起打开
//#define CONF_SEL_TEST_DRV  //宏定义打开选择test_drv示例，不能和 #define CONF_SEL_DEMO 一起打开

#ifdef CONF_SEL_DEMO
//	#define DEMO_ADC_ENABLED             	//使能ADC_DEMO 模块
//	#define DEMO_DMA_ENABLED    			//使能DMA_DEMO 模块
//	#define DEMO_DUAL_TIMER_ENABLED  		//使能DUAL_TIMER_DEMO 模块
//	#define DEMO_EXTI_ENABLED   			//使能EXTI_DEMO 模块
//	#define DEMO_PWM_ENABLED				//使能PWM_DEMO 模块   
//	#define DEMO_SPI_STD_ENABLED     		//使能SPI_STD_DEMO 模块
//	#define DEMO_UART_ENABLED  				//使能UART_DEMO 模块
//	#define DEMO_WDT_ENABLED  				//使能WDT_DEMO 模块
#endif

#ifdef CONF_SEL_TEST_DRV
//	#define TEST_DRV_DMA_ENABLED         	//使能DMA_TEST_DRV 模块
//	#define TEST_DRV_EXTI_ENABLED 			//使能EXTI_TEST_DRV 模块
//	#define TEST_DRV_I2C_MASTER_ENABLED  	//使能I2C_TEST_DRV 模块
//	#define TEST_DRV_KEY_ADC_ENABLED  		//使能KEY_ADC_TEST_DRV 模块
//	#define TEST_DRV_KEY_IO_ENABLED  		//使能KEY_IO_TEST_DRV 模块
//	#define TEST_DRV_PWM_ENABLED   			//使能PWM_TEST_DRV 模块
//	#define TEST_DRV_SPI_STD_ENABLED 		//使能SPI_STD_TEST_DRV 模块
//	#define TEST_DRV_UART_ENABLED  			//使能UART_TEST_DRV 模块
#endif

#include "conf_drv_en.h"

#if ((defined CONF_SEL_DEMO) && (defined CONF_SEL_TEST_DRV))
    #error CONF_SEL_DEMO 和 CONF_SEL_TEST_DRV 只能二选一
#endif

#ifdef DEMO_UART_ENABLED
	#ifndef DRV_UART_ENABLED
	#error 没添加串口驱动模块
	#endif
#endif

#ifdef DEMO_ADC_ENABLED
	#ifndef DRV_KEY_ADC_ENABLED
	#error 没添加ADC驱动模块
	#endif
#endif

#ifdef DEMO_DMA_ENABLED
	#ifndef DRV_DMA_ENABLED
	#error 没添加DMA驱动模块，在conf_drv_en.h中打开define配置
	#endif
#endif

#ifdef DEMO_DUAL_TIMER_ENABLED
	#ifndef DRV_DUAL_TIMER_ENABLE
	#error 	没添加DUAL_TIMER驱动模块
	#endif
#endif

#ifdef DEMO_EXTI_ENABLED
	#ifndef DRV_EXTI_ENABLED
	#error 没添加EXTI驱动模块
	#endif
#endif

#ifdef DEMO_SPI_STD_ENABLED
	#ifndef DRV_SPI_STD_ENABLED
	#error 没添加SPI主从驱动模块
	#endif
#endif

#ifdef DEMO_WDT_ENABLED
	#ifdef SYSTEM_A0_H
	#error 没添加WDT驱动模块
	#endif
#endif

#ifdef TEST_DRV_DMA_ENABLED
	#ifndef DRV_DMA
	#error 没添加DMA驱动模块
	#endif
#endif

#ifdef TEST_DRV_EXTI_ENABLED
	#ifndef DRV_EXTI_ENABLED
	#error 没添加EXTI驱动模块
	#endif
#endif

#ifdef TEST_DRV_I2C_MASTER_ENABLED
	#ifndef DRV_I2C_MASTER_ENABLED
	#error 没添加IIC主从模块驱动模块
	#endif
#endif

#ifdef TEST_DRV_KEY_ADC_ENABLED
	#ifndef DRV_KEY_ADC_ENABLED
	#error 	没添加KEY_ADC驱动模块
	#endif
#endif

#ifdef TEST_DRV_KEY_IO_ENABLED
	#ifndef DRV_KEY_IO_ENABLED
	#error 没添加KEY_IO驱动模块
	#endif
#endif

#ifdef TEST_DRV_SPI_STD_ENABLED
	#ifndef DRV_SPI_STD_ENABLED
	#error 没添加SPI主从驱动模块
	#endif
#endif

#ifdef TEST_DRV_UART_ENABLED
	#ifndef DRV_UART_ENABLED
	#error 没添加UART驱动模块
	#endif
#endif

#ifdef TEST_DRV_PWM_ENABLED
	#ifndef DRV_PWM_ENABLED
	#error 没添加PWM驱动模块
	#endif
#endif

#include "demo.h"
#include "test_drv.h"

#endif







