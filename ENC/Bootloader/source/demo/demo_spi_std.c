#ifndef __DEMO_SPI_STD_C_
#define __DEMO_SPI_STD_C_


/**
  ******************************************************************************
  * @file    demo_spi_std.c
  * @brief   spi app module.
  * @version V1.0.0
  * @date    2021/10/16
  * @author  lijixuan 
  ******************************************************************************
  */

/**
  * 版本日志：
  * V1.0.0, 2021/10/28
  *1.驱动层实现MX25l128flash的读写及擦除 
  *2.做了spi做主机时的接收和发送的buff缓存接口功能，往发送和接收缓存区写数据后发送接收都在中断进行
  *3.测试说明（主从测试需要两块开发板，该测试两块开发板都选择了spi1；只在一个开发板做不同spi收发实验用户自行更改程序）：
   A.硬件连接
      a.主机使用SPI1: cs0.14；clk：0.13；miso：0.12；mosi：0.11
      b.从机使用SPI1: cs0.14；clk：0.13；miso：0.12；mosi：0.11
      c.主机串口0：1.9（TX）;1.8（RX）
   B.使用流程
      a.把主机测试编译宏命令打开，烧录程序，烧录完后宏编译命令关闭
      b.把从机测试编译宏命令打开，烧录程序，烧录完后宏编译命令关闭
      c.串口发送数据后，数据会返回串口PC端（主机串口接数据->主spi发数据->从spi接数据->主机串口发数据）
   C.SPI模块使用流程（可参考demo_spi()函数流程）
      a.初始化I/O配置
      b.先定义结构体变量SPI_InitTypeDef 
      c.调用初始化函数DrvSpiStdMasterInit() / DrvSpiSlaveStdInit(&pInitSpiSlave); 
      d.调用相关驱动层接口函数自行编写程序
	  e.把中断函数添加到a0_it.c文件中；中断入口函数在在该文件结尾
  */

/*Includes ------------------------------------------------------------------*/

/** @addtogroup APP_SPI_STD
  * @{
  */

/** @addtogroup SPI
  * @{
  */
#include "a0_hal_conf.h"
#include "a0_hal_spi_flash.h"
#include "a0_hal_def.h"
#include "drv_spi_std.h"
#include "drv_spi_std_slave.h"
#include "drv_uart.h"
#include "demo_spi_std.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup Change test mode through define
  * @{
  */
#define      SPI_MASTER                1    /*测试SPI主机写1*/
#define      SPI_SLAVE                 0    /*测试SPI从写1*/
/**
  * @}
  */

/** @defgroup 测试时定义的变量
  * @{
  */


/**
  * @}
  */

/**
  * @brief  测试函数，该函数包含了3种测试模式；通过改变宏定义值置1可以改变测试模式              
  */
#define SPI_ID   					(1)  

void DemoSpi()
{ 
    /*配置串口，该测试例程使用串口0*/
    #if SPI_MASTER
    HAL_GPIO_AF_SET(GPIO1, 9, GPIO_AF1_UART0_TX);
    HAL_GPIO_AF_SET(GPIO1, 8, GPIO_AF1_UART0_RX);
    #endif
    
    /*配置SPI I/O口，该测试例程使用SPI1*/
    HAL_GPIO_AF_SET(GPIO0,14, GPIO_AF3_SPI1_CS);
    HAL_GPIO_AF_SET(GPIO0,13, GPIO_AF3_SPI1_CLK);
    HAL_GPIO_AF_SET(GPIO0,12, GPIO_AF3_SPI1_MISO);
    HAL_GPIO_AF_SET(GPIO0,11, GPIO_AF3_SPI1_MOSI);
    
    #if SPI_MASTER
    DemoSpiMaster();
    #endif
    
    #if SPI_SLAVE
    DemoSpiSlave();
    #endif
    
    #if SPI_STD_FLASH
    DemoSpiStdFlash();
    #endif
}
    uint8_t rxData[10];
/**
  * @brief  SPI做主机测试函数              
  */
void DemoSpiMaster()
{
    SPI_InitTypeDef pInitSpiMaster;
    pInitSpiMaster.bitDir = 0;
    pInitSpiMaster.clkDiv = 48;
    pInitSpiMaster.spiIdx = SPI_ID;
    pInitSpiMaster.spiMode = 0;    
    DrvSpiStdMasterInit(&pInitSpiMaster); 
    uint8_t dataSend[9] = {1,2,3,4,5,6,7,8,9};
	uint8_t cntSend;
    while (1)
    {	
		if(cntSend > 9)
		{
			cntSend = 0;
		}
		HAL_SPI_CS_CLR(SPIN(SPI_ID));//片选
		HalSpiTxData(SPI_ID,&dataSend[cntSend],1);//发数据	
	    //接收部分		
		HalSpiRxData(SPI_ID,rxData,1);			
		HAL_SPI_CS_SET(SPIN(SPI_ID));
		cntSend++;	
    }      
}

/**
  * @brief  SPI做从机例程，该例程为收到什么数据发什么数据              
  */
void DemoSpiSlave()
{
    uint8_t  arry_Rx[32];
    uint16_t rxSpiGetNum = 0;
    SPI_InitTypeDef pInitSpiSlave;
    pInitSpiSlave.bitDir = 0;
    pInitSpiSlave.clkDiv = 8;
    pInitSpiSlave.spiIdx = SPI_ID;
    pInitSpiSlave.spiMode = 0;
    DrvSpiSlaveStdInit(&pInitSpiSlave);       
    while(1)
    {
        rxSpiGetNum = DrvSpiSlaveRxNumGet(SPI_ID);/*获取主机发送的数据*/
        if(rxSpiGetNum >= 1)
        {
            DrvSpiSlaveRxDataRead(SPI_ID,arry_Rx,rxSpiGetNum);
            if(256 - DrvSpiSlaveTxNumGet(SPI_ID) >= rxSpiGetNum)/*发送缓冲区是否有空间*/
            {
                DrvSpiSlaveTxDataWrite(SPI_ID,arry_Rx,rxSpiGetNum);/*发送数据到主机*/
            }
        }
    }          
}


/**
  * @brief  This function handles SPI 1 TX Handler.
  */
void SPI1_TX_IRQHandler(void)
{
	return;
}

/**
  * @brief  This function handles SPI 1 RX Handler.
  */
void SPI1_RX_IRQHandler(void)
{
	#if SPI_SLAVE
    DrvSpiSlaveRxIrqHandle(1); 
	#endif	
	return;
}

/**
  * @brief  This function handles SPI 2 TX Handler.
  */
void SPI2_TX_IRQHandler(void)
{

	return;
}

/**
  * @brief  This function handles SPI 2 TX Handler.
  */
 void SPI2_RX_IRQHandler(void)
{
	#if SPI_SLAVE
    DrvSpiSlaveRxIrqHandle(2); 
	#endif	
	return;
} 
  
/**
  * @brief  This function handles SPI 3 TX Handler.
  */
void SPI3_TX_IRQHandler(void)
{

	return;
}

/**
  * @brief  This function handles SPI 3 RX Handler.
  */
void SPI3_RX_IRQHandler(void)
{
	#if SPI_SLAVE
    DrvSpiSlaveRxIrqHandle(3); 
	#endif	
	return;
}

/**
  * @}
  */

/**
  * @}
  */

/******************************************************************************************************************/

#endif
