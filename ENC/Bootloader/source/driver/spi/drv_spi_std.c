/**
  ******************************************************************************
  * @file    drv_spi_std.c
  * @brief   spi drv module.
  * @version V1.0.0
  * @date    2021/10/16
  * @author  lijixuan 
  ******************************************************************************
  */
 #include "conf_drv_en.h"
 #ifdef DRV_SPI_STD_ENABLED
 
/**
  * 版本日志：
  * V1.0.0, 2021/10/26
  *1.驱动层实现MX25l128flash的读写及擦除 
  *2.做了spi做主机时的接收和发送的buff缓存
  */

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_spi_flash.h"
#include "a0_hal_def.h"
#include "drv_spi_std.h"

/** @addtogroup DRV_SPI_STD
  * @{
  */

/** @addtogroup SPI
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup Define_UART_RingBuff
  * @{
  */
#if (DRV_SPI0_TX_BUFF_SIZE != 0)
uint8_t drvSPI0TxBuff[DRV_SPI0_TX_BUFF_SIZE];
#endif
#if (DRV_SPI0_RX_BUFF_SIZE != 0)
uint8_t drvSPI0RxBuff[DRV_SPI0_RX_BUFF_SIZE];
#endif
#if (DRV_SPI1_TX_BUFF_SIZE != 0)
uint8_t drvSPI1TxBuff[DRV_SPI1_TX_BUFF_SIZE];
#endif
#if (DRV_SPI1_RX_BUFF_SIZE != 0)
uint8_t drvSPI1RxBuff[DRV_SPI1_RX_BUFF_SIZE];
#endif
#if (DRV_SPI2_TX_BUFF_SIZE != 0)
uint8_t drvSPI2TxBuff[DRV_SPI2_TX_BUFF_SIZE];
#endif
#if (DRV_SPI2_RX_BUFF_SIZE != 0)
uint8_t drvSPI2RxBuff[DRV_SPI2_RX_BUFF_SIZE];
#endif
#if (DRV_SPI3_TX_BUFF_SIZE != 0)
uint8_t drvSPI3TxBuff[DRV_SPI3_TX_BUFF_SIZE];
#endif
#if (DRV_SPI3_RX_BUFF_SIZE != 0)
uint8_t drvSPI3RxBuff[DRV_SPI3_RX_BUFF_SIZE];
#endif

DRV_SPI_BUFF_TypeDef     drvSPI0TxBuffPara = {
    .buffSize = DRV_SPI0_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI0TxBuff[0],
};

DRV_SPI_BUFF_TypeDef     drvSPI0RxBuffPara = {
    .buffSize = DRV_SPI0_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI0RxBuff[0],
};

DRV_SPI_BUFF_TypeDef     drvSPI1TxBuffPara = {
    .buffSize = DRV_SPI1_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI1TxBuff[0],
};

DRV_SPI_BUFF_TypeDef     drvSPI1RxBuffPara = {
    .buffSize = DRV_SPI1_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI1RxBuff[0],
};

DRV_SPI_BUFF_TypeDef     drvSPI2TxBuffPara = {
    .buffSize = DRV_SPI2_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI2TxBuff[0],
};

DRV_SPI_BUFF_TypeDef     drvSPI2RxBuffPara = {
    .buffSize = DRV_SPI2_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI2RxBuff[0],
};

DRV_SPI_BUFF_TypeDef     drvSPI3TxBuffPara = {
    .buffSize = DRV_SPI3_TX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI3TxBuff[0],
};

DRV_SPI_BUFF_TypeDef     drvSPI3RxBuffPara = {
    .buffSize = DRV_SPI3_RX_BUFF_SIZE,
    .wrIdx = 0,
    .rdIdx = 0,
    .pBuffStart = &drvSPI3RxBuff[0],
};

DRV_SPI_BUFF_TypeDef  *pDrvSPIBuffPara[4][2];         //SPI0~SPI3, rx/tx
/**
  * @}
  */

/**
  * @brief  初始化SPI主机收发缓冲区            
  */
void DrvSpiBuffInit(void)
{
#if DRV_SPI0_RX_BUFF_SIZE
    pDrvSPIBuffPara[0][0] = &drvSPI0RxBuffPara;
#endif
#if DRV_SPI0_TX_BUFF_SIZE
    pDrvSPIBuffPara[0][1] = &drvSPI0TxBuffPara;
#endif
#if DRV_SPI1_RX_BUFF_SIZE
    pDrvSPIBuffPara[1][0] = &drvSPI1RxBuffPara;
#endif
#if DRV_SPI1_TX_BUFF_SIZE
    pDrvSPIBuffPara[1][1] = &drvSPI1TxBuffPara;
#endif
#if DRV_SPI2_RX_BUFF_SIZE
    pDrvSPIBuffPara[2][0] = &drvSPI2RxBuffPara;
#endif
#if DRV_SPI2_TX_BUFF_SIZE
    pDrvSPIBuffPara[2][1] = &drvSPI2TxBuffPara;
#endif
#if DRV_SPI3_RX_BUFF_SIZE
    pDrvSPIBuffPara[3][0] = &drvSPI3RxBuffPara;
#endif
#if DRV_SPI3_TX_BUFF_SIZE
    pDrvSPIBuffPara[3][1] = &drvSPI3TxBuffPara;
#endif
}

#define UART_FIFO_SIZE                      (16)    /*!< Uart fifo max number is 16 */

/** @defgroup MX25L128 flash conmand
  * @{
  */
#define MX25L128NotBusy         (1<<1)              /*等待繁忙状态完成值*/
uint8_t cmdRaedData = 0x03;                         /*读出数据状态命令*/
uint8_t cmdPageProgram = 0x02;                      /*写数据命令*/
uint8_t cmdSectorErase = 0x20;                      /*页擦除命令*/
uint8_t cmdGetDeviceId = 0x90;                      /*获取设备ID命令*/
uint8_t cmdWrEn = 0x06;                             /*写使能命令*/
uint8_t cmdWrDis = 0x04;                            /*写失能命令*/
uint8_t dumy0 = 0x00;                               /*dumy命令，dumy值为0*/
uint8_t dumy1 = 0xff;                               /*dumy命令，dumy值为*/
uint8_t rdsr = 0x05;                                /*读取状态寄存器*/
uint8_t rxfromcmd[2] = {0};                         /*命令获取到的状态值存于该数组*/
uint8_t rxfromcmddata[10] = {0,1,2,3,4,5,6,7,8,9};  /*往flash写入的数据，定义该数组只为测试时使用，用户可定义其他数组*/
uint8_t rxfromcmddata2[10] = {0};                   /*往flash读出的数据，定义该数组只为测试时使用，用户可定义其他数组*/
/**
  * @}
  */

/**
  * @brief  初始化SPI主机
  * @param  pInit  初始化SPI主机结构体.
  * @retval 0（初始化成功）;-1：传入的SPI号错误               
  */
int DrvSpiStdMasterInit(SPI_InitTypeDef *pInit)
{
    int32_t result = 0;
    DrvSpiBuffInit();/*初始化缓冲区*/
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
            },
        };       
        HAL_SPI_CTRL_SET(SPIN(pInit->spiIdx), cr.w);
        /*初始化中断控制*/
        HAL_SPI_TIMEOUT_SET(SPIN(pInit->spiIdx),0xf); 
        SPI_FCR_TypeDef fcr = {
        .b = {    
        .fifoRst = 1,
        .txFifoTrigCfg = 0,
        .rxFifoTrigCfg = 2,
        .rxFifoTrigEn = 1,
        .rxTimeOutEn = 1,       /* default, little endian */
        .txFifoTrigEn = 1,
        .rwMode = 3,
            },
        };
        HAL_SPI_FIFO_CTRL_SET(SPIN(pInit->spiIdx), fcr.w); 
        HAL_SPI_TRANS_NUM_SET(SPIN(pInit->spiIdx),1024);
        DRV_SPI_RX_IRQ_EN(1); 
        DRV_SPI_TX_IRQ_EN(1); 
        HAL_SPI_ENABLE(SPIN(pInit->spiIdx));
    }
    else result = -1;
    return result;
}

int32_t drv_spi_slave_init(uint8_t spiIdx)
{
    SPI_InitTypeDef pSpiFlashInit;
    /* config gpio for EXT Flash Access */
    HAL_GPIO_AF_SET(GPIO0,14, GPIO_AF3_SPI1_CS);
    HAL_GPIO_AF_SET(GPIO0,13, GPIO_AF3_SPI1_CLK);
    HAL_GPIO_AF_SET(GPIO0,12, GPIO_AF3_SPI1_MISO);
    HAL_GPIO_AF_SET(GPIO0,11, GPIO_AF3_SPI1_MOSI);
    
    /*中断寄存器配置*/
    HAL_SPI_TIMEOUT_SET(SPIN(spiIdx),0xffffffff);
    SPI_FCR_TypeDef paraInitFCR; 
    paraInitFCR.b.txFifoTrigCfg = 2;
    paraInitFCR.b.rxFifoTrigCfg = 2;
    paraInitFCR.b.rxFifoTrigEn = 0; 
    paraInitFCR.b.rxTimeOutEn = 1;
    paraInitFCR.b.txFifoTrigEn = 1;
    HAL_SPI_FIFO_CTRL_SET(SPIN(spiIdx), paraInitFCR.w); 

    pSpiFlashInit.bitDir = 0;  
    pSpiFlashInit.clkDiv  = 96;
    pSpiFlashInit.spiIdx = 1;
    pSpiFlashInit.spiMode = SPI_CPHA0CPOL0; 
    HalSpiStdInit(&pSpiFlashInit);   

    /*配置成从模式*/
    SPI_CR_TypeDef cr; 
    cr.w = HAL_SPI_CTRL_GET(SPIN(spiIdx));
    cr.b.slaveEn = 1;
    HAL_SPI_CTRL_SET(SPIN(spiIdx),cr.w); 

    DRV_SPI_RX_IRQ_EN(spiIdx);
		return 0;
}

/**
  * @brief  初始化SPI主机模式          
  */
int32_t DrvSpiExtFlashInit(uint8_t spiIdx)
{
    SPI_InitTypeDef pSpiFlashInit;
    pSpiFlashInit.bitDir = 0;  
    pSpiFlashInit.clkDiv  = 96;
    pSpiFlashInit.spiIdx = spiIdx;
    pSpiFlashInit.spiMode = SPI_CPHA0CPOL0; 
    HalSpiStdInit(&pSpiFlashInit);
	  return 0;
}

/**
  * @brief  MX25L128读取设备的ID值
  * @param  spiIdx  使用第几个spi模块.             
  */
void DrvSpiExtFlashDeviceId(uint8_t spiIdx)
{
    HAL_SPI_CS_CLR(SPIN(spiIdx));   
    HalSpiTxData(spiIdx,&cmdGetDeviceId,1);
    HalSpiTxData(spiIdx,&dumy0,3);     
    HalSpiRxData(spiIdx,rxfromcmd,2);
    HAL_SPI_CS_SET(SPIN(spiIdx));    
}

int32_t DrvSpiInitStd(SPI_InitTypeDef* pInitSpiStd)
{
    SPI_InitTypeDef pSpiFlashInit;
    pSpiFlashInit.bitDir = pInitSpiStd->bitDir;  
    pSpiFlashInit.clkDiv  = pInitSpiStd->clkDiv;
    pSpiFlashInit.spiIdx = pInitSpiStd->spiIdx;
    pSpiFlashInit.spiMode = pInitSpiStd->spiMode;    
    HalSpiStdInit(&pSpiFlashInit);
    return 0;     
}

void DrvSpiTest(void)
{
    DrvSpiExtFlashInit(1);
    DrvSpiExtFlashSectorErase(1,0x00);
    DrvSpiExtFlashPageWrite(1,rxfromcmddata,0x00,0x205);
    DrvSpiExtFlashReadData(1,rxfromcmddata2,0x00,10);    
    while(1); 
}

/**
  * @brief  MX25L128读取总线状态
  * @param  rdsrVar  寄存器状态.
  * @retval 0（读出寄存器状态为传入函数参数的状态值）;-1：不为传入的状态值               
  */
int32_t DrvSpiExtFlashBusyWait(uint8_t spiIdx,uint8_t rdsrVar)
{
    uint32_t timeout = 0xffffffff;
    int32_t result = 1;
    /*获取状态寄存器的值*/
    HAL_SPI_CS_CLR(SPIN(spiIdx));
    HalSpiTxData(spiIdx,&rdsr,1);/*写入rdsr命令*/
    do
    {
        HalSpiRxData(spiIdx,rxfromcmd,1);
        timeout--;
        if(timeout == 0)
        {
            break;
        }
    }while(rxfromcmd[0] != rdsrVar); 
    if(timeout == 0)
    {
        result = -1;     
    }
    else result = 0;
    HAL_SPI_CS_SET(SPIN(spiIdx));
    return (result);    
}

/**
  * @brief  MX25L128的写使能命令
  * @param  spiIdx  使用第几个spi模块.
  * @retval none                
  */
void drv_spi_ext_flash_write_enable(uint8_t spiIdx)
{ 
    HAL_SPI_CS_CLR(SPIN(spiIdx)); 
    HalSpiTxData(spiIdx,&cmdWrEn,1);/*传入写使能命令*/
    HAL_SPI_CS_SET(SPIN(spiIdx));
}

/**
  * @brief  MX25L128的页擦除
  * @param  spiIdx  使用第几个spi模块.
  * @param  flash_address  要擦除的页的地址，该地址所在页会被全部擦除.
  * @retval 0（擦除成功），其他失败                 
  */
int32_t DrvSpiExtFlashSectorErase(uint8_t spiIdx,uint32_t flashAddress)
{
    int32_t result = 1; 
    uint8_t cmdSE[4] = {0}; 
    
    cmdSE[0] = cmdSectorErase;
    cmdSE[1] = (uint8_t)(flashAddress >> 16);
    cmdSE[2] = (uint8_t)(flashAddress >> 8);
    cmdSE[3] = (uint8_t)(flashAddress);
   
    drv_spi_ext_flash_write_enable(spiIdx); 
    result = DrvSpiExtFlashBusyWait(spiIdx,MX25L128NotBusy);

    HAL_SPI_CS_CLR(SPIN(spiIdx));    
    HalSpiTxData(spiIdx,cmdSE,4);/*写入页擦除命令*/
    HAL_SPI_CS_SET(SPIN(spiIdx));
    
    result = DrvSpiExtFlashBusyWait(spiIdx,0);
    return result;
}

/**
  * @brief  MX25L128的写页入功能，只写入地址存在的页，超出页边缘数据将重新从0地址开始写该页
  * @param  spiIdx  使用第几个spi模块.
  * @param  pDataWrite  要写入数据的指针地址
  * @param  flash_address  要往flash写入的开始地址
  * @param  length  要写入的数据长度.
  * @retval 0（写入成功），其他失败                     
  */
int32_t DrvSpiExtFlashOncePageWrite(uint8_t spiIdx,uint8_t* pDataWrite,uint32_t flashAddress,uint32_t length)
{
    int32_t result = 1; 
    uint32_t count = 0;
    uint8_t cmdPP[4] = {0};
    cmdPP[0] = cmdPageProgram;/*页写入命令*/
    cmdPP[1] = (uint8_t)(flashAddress >> 16);/*地址16~24*/
    cmdPP[2] = (uint8_t)(flashAddress >> 8);/*地址8~15*/
    cmdPP[3] = (uint8_t)(flashAddress);/*地址0~7*/
    /*写使能后判断是否总线繁忙*/
    drv_spi_ext_flash_write_enable(spiIdx); 
    result = DrvSpiExtFlashBusyWait(spiIdx,MX25L128NotBusy);
    /*写命令*/
    HAL_SPI_CS_CLR(SPIN(spiIdx));
    HalSpiTxData(spiIdx,cmdPP,4);
    /*写入数据*/
    for(count = 1;count <= length;count++)
    {
        HalSpiTxData(spiIdx,pDataWrite,1);
        pDataWrite++;
    }
    HAL_SPI_CS_SET(SPIN(spiIdx));  
    result = DrvSpiExtFlashBusyWait(spiIdx,0);/*可能返回值错误*/  
    return result;
}

/**
  * @brief  MX25L128的写页入功能，写入该页地址后长度的数据
  * @param  spiIdx  使用第几个spi模块.
  * @param  pDataWrite  要写入数据的指针地址
  * @param  flash_address  要往flash写入的开始地址
  * @param  length  要写入的数据长度.
  * @retval 0（写入成功），2传入的长度参数错误                     
  */
int32_t DrvSpiExtFlashPageWrite(uint8_t spiIdx,uint8_t* pDataWrite,uint32_t flashAddress,uint32_t length)
{
    int32_t result = 1; 
    int32_t remainSizeFirstPage = 0;
    uint32_t cycle_time = 0;
    uint32_t len_each_times = 0; 
   
    remainSizeFirstPage = SPIF_PAGE_SIZE - (flashAddress  % SPIF_PAGE_SIZE);
    if(length > 0)
    {
        /*判断页剩余数据是否大于要写入的数据长度*/
        remainSizeFirstPage = remainSizeFirstPage - length;
        if(remainSizeFirstPage >= 0)
        {
            len_each_times = length;
            DrvSpiExtFlashOncePageWrite(spiIdx,pDataWrite,flashAddress,len_each_times);   
        }
        /*写入数据大于剩余的页数时*/
        else
        {
            /*首先把第一页剩余数据写满*/
            len_each_times = SPIF_PAGE_SIZE - (flashAddress  % SPIF_PAGE_SIZE);
            DrvSpiExtFlashOncePageWrite(spiIdx,pDataWrite,flashAddress,len_each_times);
            pDataWrite = pDataWrite + len_each_times;
            flashAddress = flashAddress + len_each_times;
            length = length - len_each_times;
            while(length != 0)
            {
                cycle_time = length / SPIF_PAGE_SIZE;/*剩余的数据够写几页整页*/
                /*写入不够整页的数据*/
                if(cycle_time == 0)
                {
                    len_each_times = length % SPIF_PAGE_SIZE;
                    DrvSpiExtFlashOncePageWrite(spiIdx,pDataWrite,flashAddress,len_each_times);
                    length = length - len_each_times;
                }
                /*写书整页数据*/
                else
                {
                    while(cycle_time--)
                    {
                        len_each_times = SPIF_PAGE_SIZE;
                        DrvSpiExtFlashOncePageWrite(spiIdx,pDataWrite,flashAddress,len_each_times);
                        pDataWrite = pDataWrite + SPIF_PAGE_SIZE;
                        flashAddress = flashAddress + SPIF_PAGE_SIZE;
                        length = length - len_each_times;
                    } 
                }  
            }    
        }
    }
    else result = 2; 
    return result;
}

/**
  * @brief  MX25L128读数据
  * @param  spiIdx  使用第几个spi模块.
  * @param  pDataRecive  存放读出数据的数据指针
  * @param  flash_address  从flash读出数据的地址
  * @param  length  读出数据的长度.
  * @retval 0（读出成功）                     
  */
int32_t DrvSpiExtFlashReadData(uint8_t spiIdx,uint8_t* pDataRecive,uint32_t flashAddress,uint32_t length)
{
    int32_t result = 1; 
    uint8_t cmdREAD[4] = {0};
    uint32_t count = 0;
    /*读数据命令包*/
    cmdREAD[0] = cmdRaedData;
    cmdREAD[1] = (uint8_t)(flashAddress >> 16);
    cmdREAD[2] = (uint8_t)(flashAddress >> 8);
    cmdREAD[3] = (uint8_t)(flashAddress);
    
    drv_spi_ext_flash_write_enable(spiIdx); 
    result = DrvSpiExtFlashBusyWait(spiIdx,MX25L128NotBusy);
    HAL_SPI_CS_CLR(SPIN(spiIdx));    
    HalSpiTxData(spiIdx,cmdREAD,4);
    for(count = 1;count <= length;count++)
    {
        HalSpiRxData(spiIdx,pDataRecive,1);
        pDataRecive++;
    }
    HAL_SPI_CS_SET(SPIN(spiIdx));
    result = DrvSpiExtFlashBusyWait(spiIdx,0);
    result = 0;
    return result;
}
/********************************************SPI做主机环形缓冲区*********************************************************/

/**
  * @brief  往SPI的发送缓冲区写入数据
  * @param  spiIdx  使用第几个spi模块.
  * @param  pu8Buff  要写入数据的指针
  * @param  num  写入的数据长度
  * @retval 0（读出成功）                     
  */
int32_t DrvSpiTxDataWrite(uint8_t spiIdx, uint8_t *pu8Buff, size_t num)
{
    int32_t result = -1;
    DRV_SPI_BUFF_TypeDef *pSpiPara = pDrvSPIBuffPara[spiIdx][1];
    if (pSpiPara != NULL)
    {
        if (num > 0)
        {
            DRV_SPI_INT_CTRL_SET(SPIN(spiIdx),SPI_IT_TX_TRING_EN_MSK,SPI_IT_TX_RX_TRING_DIS);
            uint32_t size = pSpiPara->buffSize - pSpiPara->wrIdx;
            uint8_t *pTxBuff = pSpiPara->pBuffStart;
            /*发送缓冲区剩下的空间大小大于要往缓冲区写的字节数，只写入一次数据*/
            if (size > num)
            {
                memcpy(&pTxBuff[pSpiPara->wrIdx], pu8Buff, num);
                pSpiPara->wrIdx += num;
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
            }
            DRV_SPI_INT_CTRL_SET(SPIN(spiIdx),SPI_IT_TX_TRING_EN_MSK,SPI_IT_TX_TRING_EN);  
        }
        else
        {
            result = -2;
        }
    }
    return result;  	
}

/**
  * @brief  SPI做主机时发送中断处理函数
  * @param  spiIdx  使用第几个spi模块.
  * @retval 0（读出成功）                     
  */
void DrvSpiTxIrqHandle(uint8_t spiIdx)
{
    #if 1 /*old verbsion  发送位之间不可以连续，只可以8位发送的方式*/
    uint32_t fifoCnt = 0;
    SPI_SR_TypeDef  sr;
	DRV_SPI_BUFF_TypeDef *pSpiPara = pDrvSPIBuffPara[spiIdx][1];
    DRV_SPI_BUFF_TypeDef *pSpiRxPara = pDrvSPIBuffPara[spiIdx][0]; 
    
    int32_t size_rx = 0;
    int32_t size = pSpiPara->wrIdx - pSpiPara->rdIdx;
    sr.w = HAL_SPI_STATE_GET(SPIN(spiIdx));
    fifoCnt = UART_FIFO_SIZE - sr.b.txFifoCnt;
    if (size != 0)
    {
        /*发送缓冲区写指针大于读指针*/
        if(size >0)
        {
            size_rx = pSpiRxPara->buffSize - pSpiRxPara->wrIdx;
            /*发送缓冲区剩余空间大于写入的数量*/
            if(size >= fifoCnt)
            {
                size = fifoCnt;
                /*接收缓冲区数量大于写入数量*/
                if(size_rx >= size)
                {
                    while(size)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    }     
                }
                /*接收缓冲区数量小于写入数量*/
                else
                {
                    size_rx =  pSpiRxPara->buffSize - pSpiRxPara->wrIdx; 
                    while(size_rx)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size_rx--;
                        size--;
                    } 
                    pSpiRxPara->wrIdx = 0;
                    while(size--)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    } 
                }
            }
            /*发送缓冲区剩余空间小于写入数量*/
            else
            {
                if(size_rx > size)
                {
                    while(size)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    }     
                }
                else
                {
                    size_rx =  pSpiRxPara->buffSize - pSpiRxPara->wrIdx; 
                    while(size_rx)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size_rx--;
                        size--;
                    } 
                    pSpiRxPara->wrIdx = 0;
                    while(size)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    } 
                }   
            }            
        }   
        else
        {
            size =  pSpiPara->buffSize - pSpiPara->rdIdx;
            if(fifoCnt <= size)
            {
                size = fifoCnt;  
                size_rx =  pSpiRxPara->buffSize - pSpiRxPara->wrIdx; 
                if(size_rx >= size)
                {
                    while(size)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    }     
                }
                else
                {
                    while(size_rx)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                        size_rx--;
                    }
                    pSpiRxPara->wrIdx = 0;  
                    while(size)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    }
                }
            }
            else
            {
                size_rx = pSpiRxPara->buffSize - pSpiRxPara->wrIdx;
                if(size_rx >= size)
                {
                    while(size)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    }
                    pSpiPara->rdIdx = 0;            
                }
                else
                {
                    while(size_rx)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                        size_rx--;
                    }
                    pSpiRxPara->wrIdx = 0;  
                    while(size)
                    {
                        DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                        DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                        size--;
                    }
                    pSpiPara->rdIdx = 0;
                }
                size = pSpiPara->wrIdx;
                {
                    if(size_rx >= size)
                    {
                        while(size)
                        {
                            DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                            DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                            size--;
                        }        
                    }
                    else
                    {
                        while(size_rx)
                        {
                            DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                            DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                            size--;
                            size_rx--;
                        }
                        pSpiRxPara->wrIdx = 0;  
                        while(size)
                        {
                            DrvSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx++],1);             
                            DrvSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx++],1);
                            size--;
                        }
                    }    
                } 
            }

        }
        
    }
    else
    {
        DRV_SPI_INT_CTRL_SET(SPIN(spiIdx),SPI_IT_TX_TRING_EN_MSK,SPI_IT_TX_RX_TRING_DIS);   
    }
    return; 
	#elif 0	   /*new verbsion  发送位可以连续，用于缓冲区*/
	uint32_t fifoCnt = 0;
    SPI_SR_TypeDef  sr;
    DRV_SPI_BUFF_TypeDef *pSpiPara = pDrvSPIBuffPara[spiIdx][1];
    DRV_SPI_BUFF_TypeDef *pSpiRxPara = pDrvSPIBuffPara[spiIdx][0]; 
    
    int32_t size_rx = 0;
    int32_t size = pSpiPara->wrIdx - pSpiPara->rdIdx;
    sr.w = HAL_SPI_STATE_GET(SPIN(spiIdx));
    fifoCnt = UART_FIFO_SIZE - sr.b.txFifoCnt;
    if (size != 0)
    {
        /*发送缓冲区写指针大于读指针*/
        if(size >0)
        {
            /*发送缓冲区剩余空间大于写入的数量*/
            if(size >= fifoCnt)
            {
                size = fifoCnt;
			}	
			/*接收缓冲区数量大于写入数量*/
			HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
			pSpiPara->rdIdx += size;  
        } 
        else
		{

			size += pSpiPara->buffSize;
			if(size >= fifoCnt)
			{				
				size = pSpiPara->buffSize - pSpiPara->rdIdx;
				if(size >= fifoCnt)
				{
					size = fifoCnt;
					HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
					pSpiPara->rdIdx += size;  
					
				}
				else
				{
					HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
					pSpiPara->rdIdx = 0;
					size = fifoCnt - size;
					HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
					pSpiPara->rdIdx += size; 
				}
			}
			else
			{
				uint8_t sizeTemp = pSpiPara->buffSize - pSpiPara->rdIdx;
				HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],sizeTemp);
				pSpiPara->rdIdx = 0;
				size = size - sizeTemp;
				HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
				pSpiPara->rdIdx += size;			
			}
		}			      
    }
    else
    {
        DRV_SPI_INT_CTRL_SET(SPIN(spiIdx),SPI_IT_TX_TRING_EN_MSK,SPI_IT_TX_RX_TRING_DIS);   
    }
    return; 
    #elif 0 /*other reverse verbsion  预留*/
    uint32_t fifoCnt = 0;
    SPI_SR_TypeDef  sr;
    DRV_SPI_BUFF_TypeDef *pSpiPara = pDrvSPIBuffPara[spiIdx][1];
    DRV_SPI_BUFF_TypeDef *pSpiRxPara = pDrvSPIBuffPara[spiIdx][0]; 
    
    int32_t size_rx = 0;
    int32_t size = pSpiPara->wrIdx - pSpiPara->rdIdx;
    sr.w = HAL_SPI_STATE_GET(SPIN(spiIdx));
    fifoCnt = UART_FIFO_SIZE - sr.b.txFifoCnt;
    if (size != 0)
    {
        /*发送缓冲区写指针大于读指针*/
        if(size >0)
        {
            size_rx = pSpiRxPara->buffSize - pSpiRxPara->wrIdx;
            /*发送缓冲区剩余空间大于写入的数量*/
            if(size >= fifoCnt)
            {
                size = fifoCnt;
                /*接收缓冲区数量大于写入数量*/
                if(size_rx >= size)
                {
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size;
                    while(size)
                    {
                        size--;
                    }     
                }
                /*接收缓冲区数量小于写入数量*/
                else
                {
                    size_rx =  pSpiRxPara->buffSize - pSpiRxPara->wrIdx;
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size_rx);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size_rx);
                    pSpiPara->rdIdx += size_rx; 
                    pSpiRxPara->wrIdx += size_rx;                    
                    while(size_rx)
                    {
                        size_rx--;
                        size--;
                    } 
                    pSpiRxPara->wrIdx = 0;
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size;
                    while(size--)
                    {
                        size--;
                    } 
                }
            }
            /*发送缓冲区剩余空间小于写入数量*/
            else
            {
                if(size_rx > size)
                {
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size;
                    while(size)
                    {
                        size--;
                    }     
                }
                else
                {
                    size_rx =  pSpiRxPara->buffSize - pSpiRxPara->wrIdx; 
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size_rx);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size_rx);
                    pSpiPara->rdIdx += size_rx; 
                    pSpiRxPara->wrIdx += size_rx;
                    while(size_rx)
                    {
                        size_rx--;
                        size--;
                    } 
                    pSpiRxPara->wrIdx = 0;
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size;
                    while(size)
                    {
                        size--;
                    } 
                }   
            }            
        }   
        else
        {
            size =  pSpiPara->buffSize - pSpiPara->rdIdx;
            if(fifoCnt <= size)
            {
                size = fifoCnt;  
                size_rx =  pSpiRxPara->buffSize - pSpiRxPara->wrIdx; 
                if(size_rx >= size)
                {
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size;
                    while(size)
                    {
                        size--;
                    }     
                }
                else
                {
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size_rx);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size_rx);
                    pSpiPara->rdIdx += size_rx; 
                    pSpiRxPara->wrIdx += size_rx;
                    while(size_rx)
                    {
                        size--;
                        size_rx--;
                    }
                    pSpiRxPara->wrIdx = 0; 
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size;                    
                    while(size)
                    {
                        size--;
                    }
                }
            }
            else
            {
                size_rx = pSpiRxPara->buffSize - pSpiRxPara->wrIdx;
                if(size_rx >= size)
                {
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size; 
                    while(size)
                    {
                        size--;
                    }
                    pSpiPara->rdIdx = 0;            
                }
                else
                {
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size_rx);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size_rx);
                    pSpiPara->rdIdx += size_rx; 
                    pSpiRxPara->wrIdx += size_rx; 
                    while(size_rx)
                    {
                        size--;
                        size_rx--;
                    }
                    pSpiRxPara->wrIdx = 0;  
                    HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                    HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                    pSpiPara->rdIdx += size; 
                    pSpiRxPara->wrIdx += size; 
                    while(size)
                    {
                        size--;
                    }
                    pSpiPara->rdIdx = 0;
                }
                size = pSpiPara->wrIdx;
                {
                    if(size_rx >= size)
                    {
                        HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                        HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                        pSpiPara->rdIdx += size; 
                        pSpiRxPara->wrIdx += size; 
                        while(size)
                        {
                            size--;
                        }        
                    }
                    else
                    {
                        HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size_rx);
                        HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size_rx);
                        pSpiPara->rdIdx += size_rx; 
                        pSpiRxPara->wrIdx += size_rx; 
                        while(size_rx)
                        {
                            size--;
                            size_rx--;
                        }
                        pSpiRxPara->wrIdx = 0; 
                        HalSpiTxData(spiIdx,&pSpiPara->pBuffStart[pSpiPara->rdIdx],size);
                        HalSpiRxData(spiIdx,&pSpiRxPara->pBuffStart[pSpiRxPara->wrIdx],size);
                        pSpiPara->rdIdx += size; 
                        pSpiRxPara->wrIdx += size;                         
                        while(size)
                        {
                            size--;
                        }
                    }    
                } 
            }

        }
        
    }
    else
    {
        DRV_SPI_INT_CTRL_SET(SPIN(spiIdx),SPI_IT_TX_TRING_EN_MSK,SPI_IT_TX_RX_TRING_DIS);   
    }
    return; 
    #endif
}

/**
  * @brief  从接收缓冲区读取数据
  * @param  spiIdx  使用第几个spi模块.
  * @param  pu8Buff  读出数据存放的指针
  * @param  num  读出的数据长度
  * @retval 0（读出成功）                     
  */
int32_t DrvSpiRxDataRead(uint8_t spiIdx, uint8_t *pu8Buff, size_t num)
{
    int32_t result = -1;
    DRV_SPI_BUFF_TypeDef *pSpiPara = pDrvSPIBuffPara[spiIdx][0];
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
  * @brief  获取接收缓冲区还有多少数据没有读出
  * @param  spiIdx  使用第几个spi模块.
  * @retval 没有读出的数据的数量                     
  */
int32_t DrvSpiRxNumGet(uint8_t spiIdx)
{ 
	  int size;
	  DRV_SPI_BUFF_TypeDef *pSpiPara = pDrvSPIBuffPara[spiIdx][0];
	  size = (pSpiPara->wrIdx - pSpiPara->rdIdx + pSpiPara->buffSize) % pSpiPara->buffSize; 
	  return size; 
}

/**
  * @brief  获取发送缓冲区还有多少数据没有发出
  * @param  spiIdx  使用第几个spi模块.
  * @retval 没有发出的数据的数量                     
  */
int32_t DrvSpiTxNumGet(uint8_t spiIdx)
{
    int32_t result = -1;
    DRV_SPI_BUFF_TypeDef *pSpiPara = pDrvSPIBuffPara[spiIdx][1];
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

/**
  * @brief  spi做主机发送数据，从寄存器发出，不带缓冲区
  * @param  spiIdx  使用第几个spi模块.
  * @param  pTxData  要写入的数据.
  * @param  byteSize  写入数据的长度.
  * @retval none                     
  */
void DrvSpiTxData(uint8_t spiIdx,uint8_t *pTxData,size_t byteSize)
{
    SPI_TypeDef *SPIx = SPIN(spiIdx);

    SPI_SR_TypeDef  sr;
    HAL_SPI_OP_MODE_SET(SPIx, SPI_OP_TXRX); 

    if (byteSize != 0)
    {
        HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPIx, SPI_8BIT);
        HAL_SPI_TRANS_WIDTH_SET(SPIx, SPI_8BIT);
        
        HAL_SPI_TRANS_NUM_SET(SPIx, byteSize);
        while (byteSize != 0)
        {
            sr.w = HAL_SPI_STATE_GET(SPIx);
            if (sr.b.txFifoCnt <= 15)
            {
                HAL_SPI_DATA_WRITE(SPIx, *pTxData++);
                byteSize--;
            }
        }
    }
    while (HAL_SPI_TRANS_NUM_GET(SPIx) != 0)
    {
        ;
    }
	return;
}

/**
  * @brief  spi做主机接收数据，从寄存器接收，不带缓冲区
  * @param  spiIdx  使用第几个spi模块.
  * @param  pTxData  存放接收数据的指针.
  * @param  byteSize  接收数据的长度.
  * @retval none                     
  */
void DrvSpiRxData(uint8_t spiIdx, uint8_t *pRxData, size_t byteSize)
{
    SPI_TypeDef *SPIx = SPIN(spiIdx);
    SPI_SR_TypeDef  sr;
	
    if (byteSize != 0)
    {
        HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPIx, SPI_8BIT);
        HAL_SPI_TRANS_WIDTH_SET(SPIx, SPI_8BIT);

        HAL_SPI_TRANS_NUM_SET(SPIx, byteSize);
        while (byteSize != 0)
        {
            sr.w = HAL_SPI_STATE_GET(SPIx);
            if (sr.b.rxFifoCnt != 0)
            {
                *pRxData++ = HAL_SPI_DATA_READ(SPIx);
            }
            byteSize--;
        }
    }
	return;
}

#endif

/**
  * @}
  */

/**
  * @}
  */
