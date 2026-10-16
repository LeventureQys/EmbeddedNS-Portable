/**
  ******************************************************************************
  * @file    drv_spi_std_slave.c
  * @brief   Header file of DRV_SPI_STD_SLAVE module.
  * @version V1.0.0
  * @date    2021/10/28
  * @author  lijixuan
  ******************************************************************************
  */

/**
  * 版本日志：
  * V1.0.0, 2021/10/26
  * 1.做了spi做主机时的接收和发送的buff缓存接口
  */

#include "conf_drv_en.h"
#ifdef DRV_SPI_STD_ENABLED
/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_spi_flash.h"
#include "a0_hal_def.h"
#include "drv_spi_std_slave.h"

/** @addtogroup DRV_SPI_STD_SLAVE
  * @{
  */

/** @addtogroup SPI
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup Define_SPI_Slave_RingBuff
  * @{
  */
#if (DRV_SPI0_SLAVE_TX_BUFF_SIZE != 0)
uint8_t drvSPI0SlaveTxBuff[DRV_SPI0_SLAVE_TX_BUFF_SIZE];
#endif
#if (DRV_SPI0_SLAVE_RX_BUFF_SIZE != 0)
uint8_t drvSPI0SlaveRxBuff[DRV_SPI0_SLAVE_RX_BUFF_SIZE];
#endif
#if (DRV_SPI1_SLAVE_TX_BUFF_SIZE != 0)
uint8_t drvSPI1SlaveTxBuff[DRV_SPI1_SLAVE_TX_BUFF_SIZE];
#endif
#if (DRV_SPI1_SLAVE_RX_BUFF_SIZE != 0)
uint8_t drvSPI1SlaveRxBuff[DRV_SPI1_SLAVE_RX_BUFF_SIZE];
#endif
#if (DRV_SPI2_SLAVE_TX_BUFF_SIZE != 0)
uint8_t drvSPI2SlaveTxBuff[DRV_SPI2_SLAVE_TX_BUFF_SIZE];
#endif
#if (DRV_SPI2_SLAVE_RX_BUFF_SIZE != 0)
uint8_t drvSPI2SlaveRxBuff[DRV_SPI2_SLAVE_RX_BUFF_SIZE];
#endif
#if (DRV_SPI3_SLAVE_TX_BUFF_SIZE != 0)
uint8_t drvSPI3SlaveTxBuff[DRV_SPI3_SLAVE_TX_BUFF_SIZE];
#endif
#if (DRV_SPI3_SLAVE_RX_BUFF_SIZE != 0)
uint8_t drvSPI3SlaveRxBuff[DRV_SPI3_SLAVE_RX_BUFF_SIZE];
#endif


DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI0SlaveTxBuffPara = {
    .buffSize = DRV_SPI0_SLAVE_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI0SlaveTxBuff[0],
};

DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI0SlaveRxBuffPara = {
    .buffSize = DRV_SPI0_SLAVE_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI0SlaveRxBuff[0],
};

DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI1SlaveTxBuffPara = {
    .buffSize = DRV_SPI1_SLAVE_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI1SlaveTxBuff[0],
};

DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI1SlaveRxBuffPara = {
    .buffSize = DRV_SPI1_SLAVE_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI1SlaveRxBuff[0],
};

DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI2SlaveTxBuffPara = {
    .buffSize = DRV_SPI2_SLAVE_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI2SlaveTxBuff[0],
};

DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI2SlaveRxBuffPara = {
    .buffSize = DRV_SPI2_SLAVE_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI2SlaveRxBuff[0],
};

DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI3SlaveTxBuffPara = {
    .buffSize = DRV_SPI3_SLAVE_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI3SlaveTxBuff[0],
};

DRV_SPI_SLAVE_BUFF_TypeDef     drvSPI3SlaveRxBuffPara = {
    .buffSize = DRV_SPI3_SLAVE_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI3SlaveRxBuff[0],
};
DRV_SPI_SLAVE_BUFF_TypeDef  *pDrvSPISlaveBuffPara[4][2];         //Uart0~Uart3, rx/tx
uint8_t drvSpiSlaveIrqFlag[4][2] = {0};
/**
  * @}
  */

/**
  * @brief  初始化SPI从机收发缓冲区            
  */
void DrvSpiSlaveBuffInit(void)
{
#if DRV_SPI0_SLAVE_RX_BUFF_SIZE
    pDrvSPISlaveBuffPara[0][0] = &drvSPI0SlaveRxBuffPara;
#endif
#if DRV_SPI0_SLAVE_TX_BUFF_SIZE
    pDrvSPISlaveBuffPara[0][1] = &drvSPI0SlaveTxBuffPara;
#endif
#if DRV_SPI1_SLAVE_RX_BUFF_SIZE
    pDrvSPISlaveBuffPara[1][0] = &drvSPI1SlaveRxBuffPara;
#endif
#if DRV_SPI1_SLAVE_TX_BUFF_SIZE
    pDrvSPISlaveBuffPara[1][1] = &drvSPI1SlaveTxBuffPara;
#endif
#if DRV_SPI2_SLAVE_RX_BUFF_SIZE
    pDrvSPISlaveBuffPara[2][0] = &drvSPI2SlaveRxBuffPara;
#endif
#if DRV_SPI2_SLAVE_TX_BUFF_SIZE
    pDrvSPISlaveBuffPara[2][1] = &drvSPI2SlaveTxBuffPara;
#endif
#if DRV_SPI3_SLAVE_RX_BUFF_SIZE
    pDrvSPISlaveBuffPara[3][0] = &drvSPI3SlaveRxBuffPara;
#endif
#if DRV_SPI3_SLAVE_TX_BUFF_SIZE
    pDrvSPISlaveBuffPara[3][1] = &drvSPI3SlaveTxBuffPara;
#endif
}

/**
  * @brief  初始化SPI从机的中断选择           
  */
void DrvSpiSlaveDefineIrq()
{
#if DRV_SPI0_SLAVE_RX_BUFF_SIZE
    drvSpiSlaveIrqFlag[0][0] = 1;
#endif
#if DRV_SPI0_SLAVE_TX_BUFF_SIZE
    drvSpiSlaveIrqFlag[0][1] = 1; 
#endif
#if DRV_SPI1_SLAVE_RX_BUFF_SIZE
    drvSpiSlaveIrqFlag[1][0] = 1;;
#endif
#if DRV_SPI1_SLAVE_TX_BUFF_SIZE
    drvSpiSlaveIrqFlag[1][1] = 1;;
#endif
#if DRV_SPI2_SLAVE_RX_BUFF_SIZE
    drvSpiSlaveIrqFlag[2][0] = 1;;
#endif
#if DRV_SPI2_SLAVE_TX_BUFF_SIZE
    drvSpiSlaveIrqFlag[2][1] = 1;;
#endif
#if DRV_SPI3_SLAVE_RX_BUFF_SIZE
    drvSpiSlaveIrqFlag[3][0] = 1;;
#endif
#if DRV_SPI3_SLAVE_TX_BUFF_SIZE
    drvSpiSlaveIrqFlag[3][1] = 1;;
#endif    
}

/**
  * @brief  初始化SPI主机
  * @param  pInit  初始化SPI主机结构体.
  * @retval 0（初始化成功）;-1：传入的SPI号错误               
  */
int DrvSpiSlaveStdInit(SPI_InitTypeDef *pInit)
{
    int32_t result = 0;
    DrvSpiSlaveBuffInit();/*初始化缓冲区*/
    if ((pInit->spiIdx >= 1) && (pInit->spiIdx <= 3))
    {
        /*初始化控制寄存器*/
        SPI_CR_TypeDef cr = {
            .b = {
                .mode = pInit->spiMode,
                .bitDir = pInit->bitDir,
                .enable = 0,
                .byteDir = SPI_BYTE_LOW2HIGH,       /* default, little endian */
                .clkDiv = pInit->clkDiv,
                .slaveEn = 1,
            },
        };  
        HAL_SPI_CTRL_SET(SPIN(pInit->spiIdx), cr.w);
        HAL_SPI_TRANS_NUM_SET(SPIN(pInit->spiIdx),1024);
        
        /*初始化中断控制*/
        SPI_FCR_TypeDef fcr = {
        .b = {    
        .fifoRst = 1,
        .txFifoTrigCfg = 2,
        .rxFifoTrigCfg = 2,
        .rxFifoTrigEn = 1,
        .rxTimeOutEn = 1,       /* default, little endian */
        .txFifoTrigEn = 0,
        .rwMode = 3,
            },
        };
        HAL_SPI_FIFO_CTRL_SET(SPIN(pInit->spiIdx), fcr.w); 
        HAL_SPI_TIMEOUT_SET(SPIN(pInit->spiIdx),0x1);     
        DrvSpiSlaveDefineIrq();
        DRV_SPI_RX_IRQ_EN(1); 
        HAL_SPI_ENABLE(SPIN(pInit->spiIdx));
    } 
    else result = -1;
    return result;
}

/**
  * @brief  往SPI的发送缓冲区写入数据
  * @param  spiIdx  使用第几个spi模块.
  * @param  pu8Buff  要写入数据的指针
  * @param  num  写入的数据长度
  * @retval 0（写出成功）                     
  */
int32_t DrvSpiSlaveTxDataWrite(uint8_t spiIdx, uint8_t *pu8Buff, size_t num)
{
    int32_t result = -1;
    DRV_SPI_SLAVE_BUFF_TypeDef *pSpiPara = pDrvSPISlaveBuffPara[spiIdx][1];
    if (pSpiPara != NULL)
    {
        if (num > 0)
        {
            DRV_SPI_INT_CTRL_SET(SPIN(spiIdx),SPI_IT_RX_TRING_EN_MSK,SPI_IT_TX_RX_TRING_DIS);
            uint32_t size = pSpiPara->buffSize - pSpiPara->wrIdx;
            uint8_t *pTxBuff = pSpiPara->pBuffStart;
            /*发送缓冲区剩下的空间大小大于要往缓冲区写的字节数，只写入一次数据*/
            if (size > num)
            {
                memcpy(&pTxBuff[pSpiPara->wrIdx], pu8Buff, num);
                pSpiPara->wrIdx += num;
                result = 0;
            }
            /*发送缓冲区剩下的空间大小小于要往缓冲区写的字节数，分开写入*/
            else   
            {
                /*先写入剩下空余的字节数*/
                memcpy(&pTxBuff[pSpiPara->wrIdx], pu8Buff, size);
                pu8Buff += size;
                size = num - size;
                /*空余字节数写入完成后，从缓冲区的开始地址重新写入剩下的数据*/
                memcpy(&pTxBuff[0], pu8Buff, size);
                pSpiPara->wrIdx = size;
                result = 0;
            }
            DRV_SPI_INT_CTRL_SET(SPIN(spiIdx),SPI_IT_RX_TRING_EN_MSK,SPI_IT_TX_TRING_EN);  
        }
        else
        {
            result = -2;
        }
    }
    return result;  	
}

/**
  * @brief  从接收缓冲区读取数据
  * @param  spiIdx  使用第几个spi模块.
  * @param  pu8Buff  读出数据存放的指针
  * @param  num  读出的数据长度
  * @retval 0（读出成功）                     
  */
int32_t DrvSpiSlaveRxDataRead(uint8_t spiIdx, uint8_t *pu8Buff, size_t num)
{
    int32_t result = -1;
    DRV_SPI_SLAVE_BUFF_TypeDef *pSpiPara = pDrvSPISlaveBuffPara[spiIdx][0];
    /*判断接收缓冲区读写指针的位置情况*/
	  int32_t size = pSpiPara->wrIdx - pSpiPara->rdIdx;
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
                    memcpy(pu8Buff,&pSpiPara->pBuffStart[pSpiPara->rdIdx],num);
                    pSpiPara->rdIdx = pSpiPara->rdIdx + num;							
                    result = 0;		
                }
            }
            else
            {
                size = pSpiPara->buffSize - pSpiPara->rdIdx;
                if(size >= num)
                {
                    memcpy(pu8Buff,&pSpiPara->pBuffStart[pSpiPara->rdIdx],num);
                    pSpiPara->rdIdx = pSpiPara->rdIdx + num;                              
                }
                else
                {
                    size = pSpiPara->buffSize + pSpiPara->wrIdx - pSpiPara->rdIdx;
                    if(size >= num)
                    {
                        size = pSpiPara->buffSize - pSpiPara->rdIdx;
                        /*先从接收缓冲区接收读指针到尾部的数据*/
                        memcpy(pu8Buff,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size); 
                        /*再从接收缓冲区接收指针开始位置到写指针的数据*/
                        memcpy(pu8Buff,&pSpiPara->pBuffStart[0],pSpiPara->wrIdx);
                        pSpiPara->rdIdx = (pSpiPara->rdIdx + num) % pSpiPara->buffSize;
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
  * @brief  SPI做从机时发送中断处理函数
  * @param  spiIdx  使用第几个spi模块.
  * @retval none                    
  */
__RAM_CODE int32_t DrvSpiSlaveRxIrqHandle(uint8_t spiIdx)
{
    uint32_t fifoCnt = 0;
    int32_t result = 1;
    DRV_SPI_SLAVE_BUFF_TypeDef *pSpiParaTx = pDrvSPISlaveBuffPara[spiIdx][1];
    DRV_SPI_SLAVE_BUFF_TypeDef *pSpiParaRx = pDrvSPISlaveBuffPara[spiIdx][0];
    int32_t size = 0;
    int32_t size_tx = 0;
    fifoCnt = HAL_SPI_RX_CNT_GET(SPIN(spiIdx));
    /*判断有无数据接收*/
    if(fifoCnt > 0)
    {
        size = pSpiParaRx->buffSize - pSpiParaRx->wrIdx; 
        /*接收缓冲区剩余空间够*/        
        if(size > fifoCnt)
        {   
            size_tx = pSpiParaTx->wrIdx - pSpiParaTx->rdIdx;
            /*判断是否有数据写入*/
            if(size_tx != 0)
            {
                /*读缓冲区写指针大于读指针*/
                if(size_tx >0)    
                {                     
                    if(size_tx >= fifoCnt)
                    {
                        while(fifoCnt)
                        {
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                            fifoCnt--; 
                        }                  
                    }
                    else
                    {
                        while(size_tx)
                        {
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                            size_tx--;
                            fifoCnt--;                            
                        } 
                        while(fifoCnt)
                        {
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),0);
                            fifoCnt--; 
                        } 
                    }  
                } 
                /*读缓冲区读指针大于写指针*/
                else
                {
                   size_tx = pSpiParaTx->buffSize - pSpiParaTx->rdIdx;
                   {
                       
                        if(size_tx > fifoCnt)
                        {
                            while(fifoCnt)
                            {
                                pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                                HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                                fifoCnt--; 
                            }     
                        }
                        else
                        {
                            
                            size_tx = pSpiParaTx->wrIdx - pSpiParaTx->rdIdx + pSpiParaTx->buffSize; 
                            if(size_tx >fifoCnt)
                            {
                                size_tx = pSpiParaTx->buffSize - pSpiParaTx->rdIdx;
                                while(size_tx)
                                {
                                    pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                                    HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                                    size_tx--;
                                    fifoCnt--;                                 
                                } 
                                pSpiParaTx->rdIdx = 0;
                                while(fifoCnt)
                                {
                                    pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                                    HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                                    fifoCnt--;                                 
                                } 
                           }
                           else
                           {
                                while(size_tx)
                                {
                                    pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                                    HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                                    size_tx--;
                                    fifoCnt--;                                 
                                } 
                                pSpiParaTx->rdIdx = 0;
                                size_tx = pSpiParaTx->wrIdx;
                                while(size_tx)
                                {
                                    pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                                    HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                                    size_tx--;
                                    fifoCnt--;                                 
                                } 
                                while(fifoCnt)
                                {
                                    pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                                    HAL_SPI_DATA_WRITE(SPIN(spiIdx),0);
                                    fifoCnt--;                                 
                                }     

                           }                               
                        }
                   }                                                      
                }       
            }
            /*没有数据写入*/
            else  
            {
                while(fifoCnt)
                {
                    pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                    HAL_SPI_DATA_WRITE(SPIN(spiIdx),0);
                    fifoCnt--; 
                }                    
            }   
        }
        /*接收缓冲区剩余空间不够需要覆盖的情况*/
        else
        {     
            size_tx = pSpiParaTx->wrIdx - pSpiParaTx->rdIdx;           
            if(size_tx != 0)
            {
                if(size_tx > 0)
                {
                    if(size_tx >= size)
                    {
                        while(size)
                        {                           
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                            size--;   
                        }
                        pSpiParaRx->wrIdx = 0;                        
                    }
                    else
                    {
                        while(size_tx)
                        {                           
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                            size_tx--;
                            size--;   
                        }
                        while(size)
                        {                           
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),0);
                            size--;   
                        }
                        pSpiParaRx->wrIdx = 0;                          
                    }  
                }
                else
                {
                    size_tx = pSpiParaTx->buffSize - pSpiParaTx->rdIdx;  
                    if(size_tx > size)
                    {
                        while(size)
                        {                           
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                            size--;   
                        } 
                    }
                    else
                    {
                        while(size_tx)
                        {
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                            size_tx--;
                            size--;                                 
                        } 
                        pSpiParaTx->rdIdx = 0;
                        size_tx = pSpiParaTx->wrIdx;
                        while(size_tx)
                        {
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),pSpiParaTx->pBuffStart[pSpiParaTx->rdIdx++]);
                            size_tx--;
                            size--;                                 
                        } 
                        while(size)
                        {
                            pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                            HAL_SPI_DATA_WRITE(SPIN(spiIdx),0);
                            size--;                                 
                        }    
                        pSpiParaRx->wrIdx = 0;
                    }
                }   
            }
            else
            {
                while(size)
                {                           
                    pSpiParaRx->pBuffStart[pSpiParaRx->wrIdx++] = HAL_SPI_DATA_READ(SPIN(spiIdx)); 
                    HAL_SPI_DATA_WRITE(SPIN(spiIdx),0);
                    size--;                                
                } 
                pSpiParaRx->wrIdx = 0;  
            }
        }  
    }
    /*没有数据接收*/
    else
    {
        result = -2;   
    }
    return result;
}

/**
  * @brief  获取接收缓冲区还有多少数据没有读出
  * @param  spiIdx  使用第几个spi模块.
  * @retval 没有读出的数据的数量                     
  */
int32_t DrvSpiSlaveRxNumGet(uint8_t spiIdx)
{ 
	  int size;
	  DRV_SPI_SLAVE_BUFF_TypeDef *pSpiPara = pDrvSPISlaveBuffPara[spiIdx][0];
	  size = (pSpiPara->wrIdx - pSpiPara->rdIdx + pSpiPara->buffSize) % pSpiPara->buffSize; 
	  return size; 
}

/**
  * @brief  获取发送缓冲区还有多少数据没有发出
  * @param  spiIdx  使用第几个spi模块.
  * @retval 没有发出的数据的数量                     
  */
int32_t DrvSpiSlaveTxNumGet(uint8_t spiIdx)
{
    int32_t result = -1;
    DRV_SPI_SLAVE_BUFF_TypeDef *pSpiPara = pDrvSPISlaveBuffPara[spiIdx][1];
    if (pSpiPara != NULL)
    {
        int32_t size = (pSpiPara->wrIdx - pSpiPara->rdIdx);
        if (size < 0)
        {
            size += pSpiPara->buffSize;
        }
        result = size;
    }
    return (result);
}

#endif

/**
  * @}
  */

/**
  * @}
  */
/************************************************************************************************************************/
