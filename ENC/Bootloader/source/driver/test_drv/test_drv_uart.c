#ifndef __TEST_DRV_UART_C_
#define __TEST_DRV_UART_C_

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
   *2.改变测试例程的宏定义可实现相应宏下的功能，详细说明见app_uart_test()函数说明
   *3.使用说明：初始化串口时候需先定义一个串口配置的结构体变量CONFIG_UART_TypeDef，可参考app_uart()函数
   *4.把中断函数添加到a0_it.c文件中；中断入口函数在在该文件结尾
   */ 
/* Includes ------------------------------------------------------------------*/
#include "a0_hal_uart.h"
#include "a0_hal_gpio.h"
#include "drv_uart.h"
#include "test_drv_uart.h"

/** @addtogroup App_UART
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
  
void TestDrvUart(void)
{
    uint16_t rxGetNum;
    uint8_t arryRx[32];
    /*该测试为把串口0接收到的数据存放到接收缓冲区，当接收缓冲区存放数据大于0时把数据通过发送缓冲区发送出去*/
    #if 1
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
    while (1)
    {
        rxGetNum  = DrvUartRxNumGet(UART_ID); /*获取是否有数据接收*/        
        if(rxGetNum >= 1) 
        {
            while(DrvUartRxDataRead(UART_ID,arryRx,rxGetNum)== UART_ID) /*等待从接收缓冲区读取数据完成*/
            { 
                if(DRV_UART0_TX_BUFF_SIZE - DrvUartTxNumGet(UART_ID) >= rxGetNum)/*判断发送缓冲区是否有足够的空间*/
                {
                    DrvUartTxDataWrite(UART_ID,arryRx,rxGetNum);/*把接收缓冲区的数据从发送缓冲区读取出去*/
                }
                break;               
            }
        }
      
    }
    #endif
    
    /*该测试为把串口0接收到的数据存放到接收缓冲区，缓冲区中读取数据然后进行处理，把字母大小写转换并屏蔽数字发出*/
    #if 0
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
    while (1)
    {
       rxGetNum  = DrvUartRxNumGet(UART_ID); /*获取是否有数据接收*/        
       if(rxGetNum >= 1) 
       {
           while(DrvUartRxDataRead(UART_ID,arryRx,rxGetNum)== 0) /*等待从接收缓冲区读取数据完成*/
           { 
               if((DRV_UART0_TX_BUFF_SIZE - DrvUartTxNumGet(UART_ID)) >= rxGetNum)/*判断发送缓冲区是否有足够的空间*/
               {
                   TestDrvConversionChar(arryRx,rxGetNum);/*获取到的数据进行处理大小写转换屏蔽数字*/
               }
               break;               
           }
       }
    }
    #endif
}
/**
  * @brief  传入数据，数据字母大小写转换并屏蔽数字然后发出.
  * @param  source  传入数据指针
  * @param  num  要转换数据的个数
  * @retval none
  */
void TestDrvConversionChar(uint8_t *source,uint16_t num)
{ 
    uint16_t count = 0;
    for(count=0;count<=num-1;count++)
    {
        if(*source >= 'A'&& *source<= 'Z')
		{
			*source = *source - 'A' + 'a';
            DrvUartTxDataWrite(0,source,1);
            source++;
		}
		else if(*source >= 'a'&& *source<= 'z')
		{
            *source = *source - 'a' + 'A';
            DrvUartTxDataWrite(0,source,1);
            source++;
		}
		else
        {
            source++;
        }             
    }
}

#ifndef TEST_DRV_SPI_STD_ENABLED

/**
  * @brief  This function handles UART 0 RX Handler.
  */
void UART0_RX_IRQHandler(void)
{
    DrvUartRxIrqHandle(0);
	return;
}	

/**
  * @brief  This function handles UART 0 TX Handler.
  */
void UART0_TX_IRQHandler(void)
{
    DrvUartTxIrqHandle(0);
	return;
}

/**
  * @brief  This function handles UART 1 RX Handler.
  */
void UART1_RX_IRQHandler(void)
{
	DrvUartRxIrqHandle(1);
	return;
}	

/**
  * @brief  This function handles UART 1 TX Handler.
  */
void UART1_TX_IRQHandler(void)
{
    DrvUartTxIrqHandle(1);
	return;
}

/**
  * @brief  This function handles UART 2 RX Handler.
  */
void UART2_RX_IRQHandler(void)
{
    DrvUartRxIrqHandle(2);
	return;
}	

/**
  * @brief  This function handles UART 2 TX Handler.
  */
void UART2_TX_IRQHandler(void)
{
    DrvUartTxIrqHandle(2);
	return;
}

/**
  * @brief  This function handles UART 3 RX Handler.
  */
void UART3_RX_IRQHandler(void)
{
    DrvUartRxIrqHandle(3);
	return;
}	

/**
  * @brief  This function handles UART 2 TX Handler.
  */
void UART3_TX_IRQHandler(void)
{
    DrvUartTxIrqHandle(3);
	return;
}
#endif

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
