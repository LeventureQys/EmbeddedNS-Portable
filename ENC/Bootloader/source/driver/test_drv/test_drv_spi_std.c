#ifndef __TEST_DRV_SPI_STD_C_
#define __TEST_DRV_SPI_STD_C_

/**
  ******************************************************************************
  * @file    app_spi_std.c
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
   C.SPI模块使用流程（可参考app_spi()函数流程）
      a.初始化I/O配置
      b.先定义结构体变量SPI_InitTypeDef 
      c.调用初始化函数DrvSpiStdMasterInit() / DrvSpiSlaveStdInit(&pInitSpiSlave); 
      d.调用相关驱动层接口函数自行编写程序
	  e.把中断函数添加到a0_it.c文件中；中断入口函数在在该文件结尾
  */

/*Includes ------------------------------------------------------------------*/

/** @addtogroup TEST_DRV_SPI_STD
  * @{
  */

/** @addtogroup SPI
  * @{
  */
#include "a0_hal_conf.h"
#include "a0_hal_spi_flash.h"
#include "a0_hal_def.h"
#include "test_drv_spi_std.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup Change test mode through define
  * @{
  */
#define      SPI_MASTER                1    /*测试SPI主机写1*/
#define      SPI_SLAVE                 0    /*测试SPI从写1*/
#define      SPI_STD_FLASH             0    /*测试SPI读写外部flash写1（**测试外部flash时得写1）*/
#define      SPI_STD_FLASH_DEVICE_ID   0    /*测试SPI读写外部flash时，要测试设备ID值写1（**与TestDrvSpiStdFlash配合使用）*/
#define      SPI_STD_FLASH_READ_WRITE  0    /*测试SPI读写外部flash是，要测试flash读写模式写1（**与TestDrvSpiStdFlash配合使用）*/
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
  
#define UART_ID   					(0)
#define SPI_ID   					(1)  
  
void TestDrvSpi()
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
    TestDrvSpiMaster();
    #endif
    
    #if SPI_SLAVE
    TestDrvSpiSlave();
    #endif
    
    #if SPI_STD_FLASH
    TestDrvSpiStdFlash();
    #endif
}

/**
  * @brief  SPI做主机测试函数              
  */
void TestDrvSpiMaster()
{
    uint8_t rxData[10];
    uint16_t rxGetNum;
    uint16_t rxGetNumSpi;
    uint8_t  arry_Rx[32];
    TestDrvSpiUartInit(UART_ID); /*改变参数值，可更改串口模块测试*/  
    SPI_InitTypeDef pInitSpiMaster;
    pInitSpiMaster.bitDir = 0;
    pInitSpiMaster.clkDiv = 48;
    pInitSpiMaster.spiIdx = SPI_ID;
    pInitSpiMaster.spiMode = 0;    
    DrvSpiStdMasterInit(&pInitSpiMaster);  
    while (1)
    {
        /*获取串口数据*/
       rxGetNum  = DrvUartRxNumGet(UART_ID); /*获取是否有数据接收*/        
       if(rxGetNum >= 1) 
       {
           while(DrvUartRxDataRead(UART_ID,arry_Rx,rxGetNum)== 0) /*等待从接收缓冲区读取数据完成*/
           {              
               if(DRV_SPI1_TX_BUFF_SIZE - DrvSpiTxNumGet(SPI_ID) >= rxGetNum)  /*发送缓冲区是否有空间*/
                {
                    HAL_SPI_CS_CLR(SPIN(SPI_ID)); 
                    DrvSpiTxDataWrite(SPI_ID,arry_Rx,rxGetNum);
                    HAL_SPI_CS_SET(SPIN(SPI_ID));
                }
               break;               
           }   
       }
       /*获取SPI从机数据*/
       rxGetNumSpi = DrvSpiRxNumGet(SPI_ID);
       if(rxGetNumSpi >=1)
       {
            while(DrvSpiRxDataRead(SPI_ID,rxData,rxGetNumSpi) == 0)
            {
                if((DRV_UART0_TX_BUFF_SIZE - DrvUartTxNumGet(UART_ID)) >= rxGetNumSpi)
                {
                    DrvUartTxDataWrite(UART_ID,rxData,rxGetNumSpi);
                }
                break;
            }  
       }
    }      
}

/**
  * @brief  SPI做从机机测试函数              
  */
void TestDrvSpiSlave()
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
  * @brief  SPI测试外部flash函数              
  */
void TestDrvSpiStdFlash()
{
	#if(SPI_STD_FLASH == 1)	
	uint8_t rxfromcmddata[128] =  {0,1,2,3,4,5,6,7,8,9};  /*往flash写入的数据，定义该数组只为测试时使用，用户可定义其他数组*/
	uint8_t rxfromcmddata2[10] = {0};                    /*往flash读出的数据，定义该数组只为测试时使用，用户可定义其他数组*/
	#endif  
    /*测试的是SPI1，要测试其他SPI模块更改DrvSpiExtFlashInit(spiIdx)中参数*/
    DrvSpiExtFlashInit(SPI_ID);  /*此处可以更改参数*/
    #if SPI_STD_FLASH_DEVICE_ID
    DrvSpiExtFlashDeviceId(SPI_ID);
    #endif
 
    #if SPI_STD_FLASH_READ_WRITE
    DrvSpiExtFlashSectorErase(SPI_DI,0x00);
    DrvSpiExtFlashPageWrite(SPI_DI,rxfromcmddata,0x00,128);
    DrvSpiExtFlashReadData(SPI_DI,rxfromcmddata2,0x00,128);
    #endif
    while(1);     
}

/**
  * @brief  初始化串口函数，该函数做主机测试时用，可以通过串口写数据，主机会发送串口数据              
  */
void TestDrvSpiUartInit(uint8_t uartIdx)
{
	/*配置串口通讯的基本参数*/
	CONFIG_UART_TypeDef paraUart0;/*定义参数配置结构体*/
    paraUart0.uartIdx = uartIdx;
    paraUart0.baudrate = 115200; 
    paraUart0.bitNum = 3;
    paraUart0.parityStyle = 0; 
    paraUart0.stopNum = 0; 	
	/*传入要使用的串口和基本参数结构体*/
    DrvUartInit(&paraUart0);        
}

/**
  * @brief  This function handles SPI 1 TX Handler.
  */
void SPI1_TX_IRQHandler(void)
{
	#if SPI_MASTER
    DrvSpiTxIrqHandle(1);
	#endif
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
	#if SPI_MASTER
    DrvSpiTxIrqHandle(2); 
	#endif	
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
	#if SPI_MASTER
    DrvSpiTxIrqHandle(3);
	#endif
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

/**
  * @}
  */

/**
  * @}
  */

/******************************************************************************************************************/
#endif
