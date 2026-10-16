
#ifndef __DEMO_DMA_C_
#define __DEMO_DMA_C_

#include "demo_dma.h"

#define DMA_SOURCE_ADDR         (0x00000000)
#define DMA_DST_ADDR   			(0x10002000)
#define DMA_TRANS_LENTH         (0x180)

uint8_t dmaCompleteFlag;  

void DemoDmaInit(void)
{
	//1选择DMA0或者DMA1控制，2选择DMA的通道
	DMA_CH_CFG_PARA_TypeDef appDmaCfg;
	
	appDmaCfg.dmaCh = 0; //通道0
	appDmaCfg.dmaCtrl = DMA0CTRL; //选择dma0控制
	DemoDmaMemeryRefresh();	//刷新目标地址
	DrvDmaInit(&appDmaCfg,DMA_DST_ADDR,DMA_SOURCE_ADDR,DMA_TRANS_LENTH);
	
	while(1)
	{	
		if (dmaCompleteFlag == 0x01) //DMA传输完成
		{	
			//DMA传输完成，进行RAM数据校验
			uint32_t *pSrc = (uint32_t *)(DMA_SOURCE_ADDR);
			uint32_t *pDst = (uint32_t *)(DMA_DST_ADDR);
			uint32_t totalLen = DMA_TRANS_LENTH / 4;
			for (uint32_t idx = 0; idx < totalLen; idx++)
			{
				uint32_t srcData = *pSrc++;
				uint32_t dstData = *pDst++;
				if (srcData != dstData)
				{//数据校验出错
					__breakpoint(0);
				}
				dmaCompleteFlag = 2;
			}
		}
		else if(dmaCompleteFlag == 2)
		{
			__breakpoint(0);        //测试完成			
		}		
	}
}

void DemoDmaMemeryRefresh(void)
{
    uint8_t *pWr = (uint8_t *)0x10002000;
    for (uint32_t i = 0; i < DMA_TRANS_LENTH; i++)
    {
        *pWr++ = (uint8_t)i;
    }
    return;
}

void DemoDma0Irq(void)
{
	if (HAL_DMA_GROUP_INT_GET(DMA0CTRL) != 0)
    {
        uint32_t tcIsr = HAL_DMA_GROUP_TC_INT_GET(DMA0CTRL);
        if (tcIsr != 0)
        {
            HAL_DMA_GROUP_TC_INT_CLR(DMA0CTRL, tcIsr);
			dmaCompleteFlag = 1;
        }
        else
        {
            uint32_t eaIsr = HAL_DMA_GROUP_EA_INT_GET(DMA0CTRL);
            if (eaIsr != 0)
            {
                HAL_DMA_GROUP_EA_INT_CLR(DMA0CTRL,eaIsr);
            }
        }
    }
    return;
}

void DemoDma1Irq(void)
{
  	if (HAL_DMA_GROUP_INT_GET(DMA1CTRL) != 0)
    {
        uint32_t tcIsr = HAL_DMA_GROUP_TC_INT_GET(DMA1CTRL);
        if (tcIsr != 0)
        {
            HAL_DMA_GROUP_TC_INT_CLR(DMA1CTRL, tcIsr);
			dmaCompleteFlag = 1;
        }
        else
        {
            uint32_t eaIsr = HAL_DMA_GROUP_EA_INT_GET(DMA1CTRL);
            if (eaIsr != 0)
            {
                HAL_DMA_GROUP_EA_INT_CLR(DMA1CTRL,eaIsr);
            }
        }
    }
    return;
}

void DMA0_IRQHandler(void)
{
	DemoDma0Irq();
    return;
}

void DMA1_IRQHandler(void)
{
	DemoDma1Irq();
    return;
}

#endif
