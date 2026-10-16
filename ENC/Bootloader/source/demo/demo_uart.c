#ifndef __DEMO_UART_C_
#define __DEMO_UART_C_

/**
  ******************************************************************************
  * @file    app_uart.c
  * @brief   uart app module.
  * @version V1.0.0
  * @date    2021/09/23
  * @author  lijixuan 
  ******************************************************************************
  */
 
/**
  * 版本日志：
  * V1.0.0, 2021/09/23,
  * 1. 串口应用层函数测试例程
  */
 
 /**
   *串口应用层测试例程说明：Rx->Tx(串口0)
   *1.硬件连接：GPIO1.9(UART0_TX)发送引脚连接USB转串口模块RX；GPIO1.8(UART0_RX)发送引脚连接USB转串口模块TX
   *  A0—VCC->USB转串口模块VCC;A0—GND->USB转串口模块GND;
   *2.使用说明：初始化串口时候需先定义一个串口配置的结构体变量CONFIG_UART_TypeDef，可参考demo_uart()函数
   *3.把中断函数添加到a0_it.c文件中；中断入口函数在在该文件结尾
   */ 
/* Includes ------------------------------------------------------------------*/
#include "a0_hal_uart.h"
#include "a0_hal_gpio.h"
#include "drv_uart.h"
#include "demo_uart.h"

/** @addtogroup DEMO_UART
  * @{
  */

/** @addtogroup UART
  * @{
  */ 

/*UART_applicatino_functions-----------------------------------------------------*/
/** @addtogroup UART_applicatino_functions
  * @{
  */
  
/**
  * @brief  应用函数测试例程：测试时把相应的屏蔽宏定义置1
  *         例程1：串口收发测试（串口0）
  *         例程2：串口应用例程，读取接收缓冲区数据处理，把数据大小写转换数字屏蔽（串口0）
  *
  * @retval none
  */
#define UART_ID   					(0)
#define BAUDE_RATE   				(115200)

void DemoUart(void)
{
	/*配置GPIO模式为串口0*/
	HAL_GPIO_AF_SET(GPIO1, 9, GPIO_AF1_UART0_TX);
    HAL_GPIO_AF_SET(GPIO1, 8, GPIO_AF1_UART0_RX);
	/*配置串口通讯的基本参数*/
	CONFIG_UART_TypeDef paraUart;/*定义参数配置结构体*/
    paraUart.uartIdx = UART_ID;
    paraUart.baudrate = BAUDE_RATE; 
    paraUart.bitNum = 3;
    paraUart.parityStyle = 0; 
    paraUart.stopNum = 0; 	
	/*传入要使用的串口和基本参数结构体*/
    DrvUartInit(&paraUart); 
	
	uint8_t txFifoCnt;
	uint8_t rxFifoCnt;
	uint8_t rxBuff[16];
    while (1)
    {
		//发送部分
		txFifoCnt = UART_FIFO_SIZE - HAL_UART_TX_FIFO_CNT_GET(UARTN(UART_ID)); //发送fifo剩余多少空间
	    while(txFifoCnt--)
		{ 
			HAL_UART_DATA_SET(UARTN(UART_ID),0x33);//发送0x33
		}		
		rxFifoCnt = HAL_UART_RX_FIFO_CNT_GET(UARTN(UART_ID));//接收fifo接收了多少字节数据
		
		//接收部分
		memset(rxBuff,0xff,16);//重置接收缓存的数据
		while(rxFifoCnt--)
		{			
			rxBuff[rxFifoCnt] = HAL_UART_DATA_GET(UARTN(UART_ID));//读取接收的数据
		}
    }
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************************************************************************/

#endif
