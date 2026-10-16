/**
  ******************************************************************************
  * @file    a0_drv_uart.c
  * @brief   uart drv module.
  * @version V1.0.0
  * @date    2021/09/16
  * @author  lijixuan 
  ******************************************************************************
  */
 
/**
  * 版本日志：
  * V1.0.0, 2021/09/16
  
  */

#include "conf_drv_en.h"
#ifdef DRV_UART_ENABLED


/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "drv_uart.h"
#include <string.h>

/** @addtogroup UART_Drive_Driver
  * @{
  */

/** @addtogroup UART
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup Define_UART_RingBuff
  * @{
  */

#if (DRV_UART0_TX_BUFF_SIZE != 0)
uint8_t drvUart0TxBuff[DRV_UART0_TX_BUFF_SIZE];
#endif
#if (DRV_UART0_RX_BUFF_SIZE != 0)
uint8_t drvUart0RxBuff[DRV_UART0_RX_BUFF_SIZE];
#endif
#if (DRV_UART1_TX_BUFF_SIZE != 0)
uint8_t drvUart1TxBuff[DRV_UART1_TX_BUFF_SIZE];
#endif
#if (DRV_UART1_RX_BUFF_SIZE != 0)
uint8_t drvUart1RxBuff[DRV_UART1_RX_BUFF_SIZE];
#endif
#if (DRV_UART2_TX_BUFF_SIZE != 0)
uint8_t drvUart2TxBuff[DRV_UART2_TX_BUFF_SIZE];
#endif
#if (DRV_UART2_RX_BUFF_SIZE != 0)
uint8_t drvUart2RxBuff[DRV_UART2_RX_BUFF_SIZE];
#endif
#if (DRV_UART3_TX_BUFF_SIZE != 0)
uint8_t drvUart3TxBuff[DRV_UART3_TX_BUFF_SIZE];
#endif
#if (DRV_UART3_RX_BUFF_SIZE != 0)
uint8_t drvUart3RxBuff[DRV_UART3_RX_BUFF_SIZE];
#endif

DRV_UART_BUFF_TypeDef     drvUart0TxBuffPara = {
    .buffSize = DRV_UART0_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart0TxBuff[0],
};

DRV_UART_BUFF_TypeDef     drvUart0RxBuffPara = {
    .buffSize = DRV_UART0_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart0RxBuff[0],
};

DRV_UART_BUFF_TypeDef     drvUart1TxBuffPara = {
    .buffSize = DRV_UART1_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart1TxBuff[0],
};

DRV_UART_BUFF_TypeDef     drvUart1RxBuffPara = {
    .buffSize = DRV_UART1_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart1RxBuff[0],
};

DRV_UART_BUFF_TypeDef     drvUart2TxBuffPara = {
    .buffSize = DRV_UART2_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart2TxBuff[0],
};

DRV_UART_BUFF_TypeDef     drvUart2RxBuffPara = {
    .buffSize = DRV_UART2_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart2RxBuff[0],
};

DRV_UART_BUFF_TypeDef     drvUart3TxBuffPara = {
    .buffSize = DRV_UART3_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart3TxBuff[0],
};

DRV_UART_BUFF_TypeDef     drvUart3RxBuffPara = {
    .buffSize = DRV_UART3_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvUart3RxBuff[0],
};

DRV_UART_BUFF_TypeDef  *pDrvUartBuffPara[4][2];         //Uart0~Uart3, rx/tx

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup UART_DRV_Exported_Functions
  * @{
  */

/**
  * @brief  Initialize the UART ring buffer.
  * @retval none      
  */
void DrvUartBuffInit(void)
{
#if DRV_UART0_RX_BUFF_SIZE
    pDrvUartBuffPara[0][0] = &drvUart0RxBuffPara;
#endif
#if DRV_UART0_TX_BUFF_SIZE
    pDrvUartBuffPara[0][1] = &drvUart0TxBuffPara;
#endif
#if DRV_UART1_RX_BUFF_SIZE
    pDrvUartBuffPara[1][0] = &drvUart1RxBuffPara;
#endif
#if DRV_UART1_TX_BUFF_SIZE
    pDrvUartBuffPara[1][1] = &drvUart1TxBuffPara;
#endif
#if DRV_UART2_RX_BUFF_SIZE
    pDrvUartBuffPara[2][0] = &drvUart2RxBuffPara;
#endif
#if DRV_UART2_TX_BUFF_SIZE
    pDrvUartBuffPara[2][1] = &drvUart2TxBuffPara;
#endif
#if DRV_UART3_RX_BUFF_SIZE
    pDrvUartBuffPara[3][0] = &drvUart3RxBuffPara;
#endif
#if DRV_UART3_TX_BUFF_SIZE
    pDrvUartBuffPara[3][1] = &drvUart3TxBuffPara;
#endif
}

/**
  * @brief  DeInitializes the UART peripheral.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3
  * @retval        This parameter can be one of the following values:
  *                @arg 1:DeInitializes the UART peripheral successful
  */
int32_t DrvUartDeInit(UART_TypeDef* UARTx)
{
    int32_t result = -1;
    UART_CR_TypeDef cr;
    cr.w = HAL_UART_CTRL_GET(UARTx);
    cr.b.rxFifoRst = UART_FIFO_RST_RX;
    cr.b.txFifoRst = UART_FIFO_RST_TX;
    HAL_UART_CTRL_SET(UARTx,cr.w);
    HAL_UART_CLK_DISABLE(UARTx); 
    UARTx = NULL;
    result = 1;
    return (result);
}

/**
  * @brief  Stop ongoing transfers.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3
  * @retval none
  */
void DrvUartStop(UART_TypeDef* UARTx)
{
    HAL_UART_CLK_DISABLE(UARTx);
}

/**
  * @brief  Start continuing the UART transfer.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3
  * @retval none
  */
void DrvUartStart(UART_TypeDef* UARTx)
{
    HAL_UART_CLK_ENABLE(UARTx); 
}

/**
  * @brief  Set the UART data bit number.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3
  * @param  Bit_Num  UART data bit number
  *                This parameter can be one of the following values:
  *                @arg UART_5BIT:Uart bit numebr is 5
  *                @arg UART_6BIT:Uart bit numebr is 6
  *                @arg UART_7BIT:Uart bit numebr is 7
  *                @arg UART_8BIT:Uart bit numebr is 8
  * @retval none
  */
void DrvUartSetBitNum(UART_TypeDef *UARTx,uint32_t bitNum)
{
    UART_CR_TypeDef cr;
    cr.w = HAL_UART_CTRL_GET(UARTx);
    cr.b.bitNum = bitNum;
    HAL_UART_CTRL_SET(UARTx,cr.w);
}

/**
  * @brief  Set the UART baudrate.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3
  * @param  baudrate  This member configures the UART communication baud rate.
  * @retval none
  */
void DrvUartSetBaudRate(UART_TypeDef *UARTx,uint32_t baudrate)
{
    HAL_UART_BAUD_SET(UARTx, baudrate);
}

/**
  * @brief  Set the UART stopNum.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3
  * @param  stopNum  Specifies the number of stop bits transmitted.
  *                This parameter can be one of the following values:
  *                @arg UART_STOP_1BIT:Uart stop bit is 1 bit
  *                @arg UART_STOP_2BIT:Uart stop bit is 2 bit            
  * @retval none
  */
void DrvUartSetStopNum(UART_TypeDef *UARTx,uint32_t stopNum)
{
    UART_CR_TypeDef cr;
    cr.w = HAL_UART_CTRL_GET(UARTx);
    cr.b.stopNum = stopNum;
    HAL_UART_CTRL_SET(UARTx,cr.w);
}

/**
  * @brief  Set the UART parityStyle.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3
  * @param  parityStyle  Specifies the parity mode.
  *                This parameter can be one of the following values:
  *                @arg UART_NO_PARITY:No parity
  *                @arg UART_ODD_PARITY:Odd parity  
  *                @arg UART_EVEN_PARITY:Even parity
  *                @arg UART_FORCE_ONE:Force '1'  
  *                @arg UART_FORCE_ZERO:Force '0'       
  * @retval none
  */
void DrvUartSetParityStyle(UART_TypeDef *UARTx,uint32_t parityStyle)
{
    UART_CR_TypeDef cr;
    cr.w = HAL_UART_CTRL_GET(UARTx);
    cr.b.parityStyle = parityStyle;
    HAL_UART_CTRL_SET(UARTx,cr.w);
}

/**
  * @brief  UART_TX_TRIG_CTRL UART tx trigger ctrl.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3 
  * @param  uartTxIntTringNum UART_TX_TRIG_CTRL UART tx trigger bit ctrl
  *                This parameter can be one of the following values:
  *                @arg UART_TX_E0:data number in tx fifo == 0 
  *                @arg UART_TX_SE4:data number in tx fifo <= 4 
  *                @arg UART_TX_SE8:data number in tx fifo <= 8 
  *                @arg UART_TX_NEFULL:data number in tx fifo != 16
  * @retval none
  */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
void DrvUartSetTxIntTrigleNum(UART_TypeDef *UARTx,uint32_t uartTxIntTringNum)
{
    UART_ICR_TypeDef icr;
    icr.w = HAL_UART_INT_CTRL_GET(UARTx);
    icr.b.txTrigNum = uartTxIntTringNum;
    HAL_UART_INT_CTRL_SET(UARTx, icr.w);
}

/**
  * @brief  UART_RX_TRIG_CTRL UART rx trigger ctrl.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *                This parameter can be one of the following values:
  *                @arg UART0:the Uart instance of UART0
  *                @arg UART1:the Uart instance of UART1
  *                @arg UART2:the Uart instance of UART2
  *                @arg UART3:the Uart instance of UART3 
  * @param  uartRxIntTringNum UART_RX_TRIG_CTRL UART rx trigger bit ctrl
  *                This parameter can be one of the following values:
  *                @arg UART_RX_NE0:data number in rx fifo != 0
  *                @arg UART_RX_BE4:data number in rx fifo >= 4 
  *                @arg UART_RX_BE8:data number in rx fifo >= 8  
  *                @arg UART_RX_FULL:data number in rx fifo == 16
  * @retval none
  */    
void DrvUartSetRxIntTrigleNum(UART_TypeDef *UARTx,uint32_t uartRxIntTringNum)
{
    UART_ICR_TypeDef icr;
    icr.w = HAL_UART_INT_CTRL_GET(UARTx);
    icr.b.rxTrigNum = uartRxIntTringNum;
    HAL_UART_INT_CTRL_SET(UARTx, icr.w);
}

/**
  * @brief  Get the remaining quantity data in Tx_Buff 
  * @param  uartIdx : The ID of UART
  *         This parameter can be one of the these values
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
  * @retval The remaining quantity data in Tx_Buff 
  */
int32_t DrvUartTxNumGet(uint8_t uartIdx)
{
    int32_t result = -1;
    DRV_UART_BUFF_TypeDef *pUartPara = pDrvUartBuffPara[uartIdx][1];
    if (pUartPara != NULL)
    {
        int32_t size = (pUartPara->wrIdx - pUartPara->rdIdx);
        if (size < 0)
        {
            size += pUartPara->buffSize ;
        }
        result = size;
    }
    return (result);
}

/**
  * @brief  从接收缓冲区中读取num个数据放到pu8Buff数组中
  * @param  uartIdx : The ID of UART
  *         This parameter can be one of the these values:
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
  * @param  pu8Buff:User define one buffer pointer will point to the read pointer in Rx_Buff
  * @param  num:The amount of data after reading the pointer in Rx_Buff
  * @retval This parameter can be one of the following values:
  *         @arg 0: 读取成功
  *         @arg 1: 读取失败
  *         @arg 2: 传入的获取数量参数小于0
  */
int32_t DrvUartRxDataRead(uint8_t uartIdx, uint8_t *pu8Buff, size_t num)
{
    int32_t result = -1;
    DRV_UART_BUFF_TypeDef *pUartPara = pDrvUartBuffPara[uartIdx][0];
    /*判断接收缓冲区读写指针的位置情况*/
	int32_t size = pUartPara->wrIdx - pUartPara->rdIdx;
    if(num > 0)
    {
        if(size != 0)
        { 
            if(size > 0)
            {
                if(size < num)
                {
                    result = 1;  
                }
                /*当要读出的数据的数量小于接收缓冲区写入的数据量时，把数据读出num个*/
                else
                {
                    memcpy(pu8Buff,&pUartPara->pBuffStart[pUartPara->rdIdx],num);
                    pUartPara->rdIdx = pUartPara->rdIdx + num;							
                    result = 0;		
                }
            }
            else
            {				
                size = pUartPara->buffSize - pUartPara->rdIdx;
                if(size >= num)
                {
                    memcpy(pu8Buff,&pUartPara->pBuffStart[pUartPara->rdIdx],num);
                    pUartPara->rdIdx = pUartPara->rdIdx + num;
                    result = 0;                    
                }
                else
                {
                    size = pUartPara->buffSize + pUartPara->wrIdx - pUartPara->rdIdx;
                    if(size >= num)
                    {
                        size = pUartPara->buffSize - pUartPara->rdIdx;
                        /*先从接收缓冲区接收读指针到尾部的数据*/
                        memcpy(pu8Buff,&pUartPara->pBuffStart[pUartPara->rdIdx],size); 
                        pUartPara->rdIdx = 0;
                        size = num - size;
                        /*再从接收缓冲区接收指针开始位置到写指针的数据*/
                        memcpy(pu8Buff,&pUartPara->pBuffStart[0],size);
                        pUartPara->rdIdx += size;
                        result = 0;
                    }    
                    else
                    {
                        result = 1; 
                    }
                }
	
            }
        }
        else
        {
            result = 1;
        }
    }
    else
    {
       result = 2;   
    }
	return (result);	
}

/**
  * @brief  Gets the amount of data that has been received in in Rx_Buff
  * @param  token : Endpoint Type
  *         This parameter can be one of the these values:
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
  * @retval The amount of data that has been received in in Rx_Buff
  */
int32_t DrvUartRxNumGet(uint8_t uartIdx)
{ 
    int size;
    DRV_UART_BUFF_TypeDef *pUartPara = pDrvUartBuffPara[uartIdx][0];
    size = (pUartPara->wrIdx - pUartPara->rdIdx + pUartPara->buffSize) % pUartPara->buffSize ; 
    return size; 
}

/**
  * @brief  传入串口基地址及串口通讯参数串口功能初始化.
  * @param  UARTx  Pointer to a UART_TypeDef structure the Uart instance.
  *         This parameter can be one of the these values:
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
  * @param  CONFIG_UART_TypeDef  配置串口通讯基本参数的结构体.
  *         This parameter can be a value of @ref SetUartTypeDef_Alternate_function_selection
  *         @arg 0:初始化成功
  *         @arg -1:初始化传入参数不正确
  */ 
int DrvUartInit(CONFIG_UART_TypeDef* parUART)
{
    int32_t result = -1;
	if(parUART != NULL)
	{
        UART_InitTypeDef pInit_uart = {
        .uartIdx = parUART->uartIdx,
        .cr.b = {
		.bitNum= parUART->bitNum,
		.stopNum= parUART->stopNum,
		.parityStyle= parUART->parityStyle,
		.dmaMode= UART_DMA_UNUSED,    /*!< dma mode config, 0, dms unused, 1, dma for tx, 2, dma for rx, 3, dma for tx/rx */
		.rxFifoRst= UART_FIFO_RST_RX, /*!<This parameter can be a value of @ref UART fifo reset ctrl                      */
		.txFifoRst= UART_FIFO_RST_TX, /*!<This parameter can be a value of @ref UART fifo reset ctrl                      */
        .clkEn= 1,                    /*!< 0:disable uart clock;  1:enable uart clock                                     */
		},
        .icr.b = {
		.rxTimeOutEn = 1,             /*!< rx timeout int enable ctrl, 1, enable, 0, disable                              */
		.rxTrigEn= 1,                 /*!< rx trigger int enable ctrl, 1, enable, 0, disable                              */
		.txTrigEn= 1,                 /*!< tx trigger int enable ctrl, 1, enable, 0, disable                              */
		.rxTrigNum = UART_TX_SE8,     /*!<This parameter can be a value of @ref UART tx trigger ctrl                      */
		.txTrigNum = UART_RX_BE4,     /*!<This parameter can be a value of @ref UART rx trigger ctrl                      */
		},
		.baudrate = parUART->baudrate,
		.timeout = 0xffffffff,
		};
		HalUartInit(&pInit_uart);
		DrvUartBuffInit();/*初始化串口发送接收缓冲区*/
		DRV_UART_RX_IRQ_EN(parUART->uartIdx);/*配置接收NVIC中断*/
        DRV_UART_TX_IRQ_EN(parUART->uartIdx);/*配置发送NVIC中断*/
		result = 0;
	}
    return result;	
}

/**
  * @brief  接收中断函数
  * @param  uartIdx : The ID of UART
  *         This parameter can be one of the these values:
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
  * @retval Null
  */
__RAM_CODE int32_t DrvUartRxIrqHandle(uint8_t uartIdx)
{
    DRV_UART_BUFF_TypeDef *pUartPara = pDrvUartBuffPara[uartIdx][0];
    uint8_t rxNum = HAL_UART_RX_FIFO_CNT_GET(UARTN(uartIdx));
    int32_t result = -1;
    int32_t size = 0;
	  /*获取到串口fifo中要发送的数据量并逐一发送出去*/  
    if(rxNum >0)
    {
        size = pUartPara->buffSize - pUartPara->wrIdx;
        if(size > rxNum)
        {
            size = rxNum;  
            while(size)
            {
                pUartPara->pBuffStart[pUartPara->wrIdx++] = HAL_UART_DATA_GET(UARTN(uartIdx));  
                size--;
            }           
        }
        else
        {
            size = pUartPara->buffSize - pUartPara->wrIdx; 
            while(size)
            {
                pUartPara->pBuffStart[pUartPara->wrIdx++] = HAL_UART_DATA_GET(UARTN(uartIdx));  
                size--;
            }
            pUartPara->wrIdx = 0; 
        }       
    }
    else
    {
        result = -2;  
    }
	return result;
}

/**
  * @brief  发送中断函数
  * @param  uartIdx : The ID of UART
  *         This parameter can be one of the these values:
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
  * @retval Null
  */
__RAM_CODE void DrvUartTxIrqHandle(uint8_t uartIdx)
{
    uint32_t fifoCnt = 0;
	DRV_UART_BUFF_TypeDef *pUartPara = pDrvUartBuffPara[uartIdx][1];
    int32_t size = pUartPara->wrIdx - pUartPara->rdIdx;
    if (size != 0)
    {
        /*获取发送fifo中剩下的空间大小*/
        fifoCnt = UART_FIFO_SIZE - HAL_UART_TX_FIFO_CNT_GET(UARTN(uartIdx));
        /*缓冲区指针读写循环，缓冲区的写指针大于读指针情况*/
            if (size > 0)
            {
                if (size > fifoCnt)
                {
                    size = fifoCnt;
                }
                while (size)
                {
                    HAL_UART_DATA_SET(UARTN(uartIdx), pUartPara->pBuffStart[pUartPara->rdIdx]);
                    pUartPara->rdIdx++;
                    size--;
                }
            }
            /*缓冲区指针读写循环，当缓冲区的写指针小于读指针情况*/
            else
            {
                size += pUartPara->buffSize ;
                if (size > fifoCnt)
                {
                    size = fifoCnt;
                }
                int32_t size2End = pUartPara->buffSize  - pUartPara->rdIdx;
                /*当读指针到缓冲区尾部所剩的空间大小大于串口发送fifo空余字节数*/
                if (size2End >= size)
                {
                    while (size)
                    {
                        HAL_UART_DATA_SET(UARTN(uartIdx), pUartPara->pBuffStart[pUartPara->rdIdx]);
                        pUartPara->rdIdx++;
                        size--;
                    }
                }
                /*当读指针到缓冲区尾部所剩的空间大小小于串口发送fifo空余字节数*/
                else
                {
                    /*读指针到发送缓冲区尾部的数据发完*/
                    size -= size2End;
                    while (size2End)
                    {
                        HAL_UART_DATA_SET(UARTN(uartIdx), pUartPara->pBuffStart[pUartPara->rdIdx]);
                        pUartPara->rdIdx++;
                        size2End--;
                    }
                    pUartPara->rdIdx = 0;
                    /*把剩余数据发完*/
                    while (size)
                    {
                        HAL_UART_DATA_SET(UARTN(uartIdx), pUartPara->pBuffStart[pUartPara->rdIdx]);
                        pUartPara->rdIdx++;
                        size--;
                    }
                }
            }
        
    }
    else
    {
        /*缓冲区数据发送完成关闭中断*/
        DRV_UART_INT_CTRL_SET(UARTN(uartIdx),USART_It_Tx_Trig_En_Msk,USART_It_Tx_Trig_Disable);
    }
    return;
}

/**
  * @brief  Write data to Tx_Buff
  * @param  uartIdx : The ID of UART
  *         This parameter can be one of the these values:
  *         @arg 0: UART0
  *         @arg 1: UART1
  *         @arg 2: UART2
  *         @arg 3: UART3
  * @param  pu8Buff:Buffer pointer from which the Data will be send to Tx_Buff
  * @param  num: Total quantity will write to the Tx_Buff
  * @retval This parameter can be one of the following values:
  *         @arg -1:没有开启发送缓冲区
  *         @arg -2：传入参数num不大于0，num需大于0
  */
int32_t DrvUartTxDataWrite(uint8_t uartIdx, uint8_t *pu8Buff, size_t num)
{

    int32_t result = -1;
    DRV_UART_BUFF_TypeDef *pUartPara = pDrvUartBuffPara[uartIdx][1];
    if (pUartPara != NULL)
    {
        if (num > 0)
        {
            DRV_UART_INT_CTRL_SET(UARTN(uartIdx),USART_It_Tx_Trig_En_Msk,USART_It_Tx_Trig_Disable);
            uint32_t size = pUartPara->buffSize  - pUartPara->wrIdx;
            uint8_t *pTxBuff = pUartPara->pBuffStart;
            /*发送缓冲区剩下的空间大小大于要往缓冲区写的字节数，只写入一次数据*/
            if (size > num)
            {
                memcpy(&pTxBuff[pUartPara->wrIdx], pu8Buff, num);
                pUartPara->wrIdx += num;
            }
            /*发送缓冲区剩下的空间大小小于要往缓冲区写的字节数，分开写入*/
            else   
            {
                /*先写入剩下空余的字节数*/
                memcpy(&pTxBuff[pUartPara->wrIdx], pu8Buff, size);
			    pUartPara->wrIdx = 0;
                pu8Buff += size;
                size = num - size;
                /*空余字节数写入完成后，从缓冲区的开始地址重新写入剩下的数据*/
                memcpy(&pTxBuff[0], pu8Buff, size);
                pUartPara->wrIdx = size;
            }
            DRV_UART_INT_CTRL_SET(UARTN(uartIdx),USART_It_Tx_Trig_En_Msk,USART_It_Tx_Trig_Enable);   
        }
        else
        {
            result = -2;
        }
    }
    return result;  	
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
