
#include "conf_drv_en.h"
#ifdef DRV_DMA_ENABLED

#include "drv_dma.h"

const DMA_CH_CCR_TypeDef ccrDefault = {
    .b = {
        .tcMsk = 1,
        .chEn = 1,
        .dstCtrl = DMA_ADDR_INC,
        .srcCtrl = DMA_ADDR_INC,
        .mode = DMA_MODE_NORMAL,
        .dstWidth = DMA_WIDTH_32BIT,
        .srcWidth = DMA_WIDTH_32BIT,
        .srcSize = DMA_BURST_1,	//32位按照4字节传输
        .fifoTh = DMA_FIFO_TH_1, 
    },
};
const DMA_LLD_TypeDef   lldDefault = {
    .ctrl.b = {
        .srcCtrl = DMA_ADDR_INC,
        .dstCtrl = DMA_ADDR_INC,
        .srcWidth = DMA_WIDTH_32BIT,
        .dstWidth = DMA_WIDTH_32BIT,
        .fifoTh = DMA_FIFO_TH_1,
    },
};


int32_t DrvDmaInit(DMA_CH_CFG_PARA_TypeDef* chCfgPara, uint32_t dstAdd, uint32_t sourceAdd, uint32_t transLen)
{
	int32_t result;
	DRV_DMA_CH_PARA_TypeDef  dmaChPara;
	if(chCfgPara->dmaCtrl == DMA0CTRL)
	{
		NVIC_DisableIRQ(DMA0_IRQn);
		HAL_DMA_GROUP_DISABLE(DMA0CTRL);       //disable
		dmaChPara.dmaCh = (DMA0CH0 + chCfgPara->dmaCh);
	}
	else if(chCfgPara->dmaCtrl == DMA1CTRL)
	{		
		NVIC_DisableIRQ(DMA1_IRQn);
		HAL_DMA_GROUP_DISABLE(DMA1CTRL);       //disable
		dmaChPara.dmaCh = (DMA1CH0 + chCfgPara->dmaCh);
	}
	else
	{
		result = -1;
	}
	
	dmaChPara.dmaCtrl = chCfgPara->dmaCtrl;
	dmaChPara.ccr = ccrDefault;
	dmaChPara.lld = lldDefault;
	dmaChPara.srcAddr = sourceAdd;
	dmaChPara.dstAddr = dstAdd;
	dmaChPara.lenInByte = transLen;
	
	DrvDmaChCfg(&dmaChPara);
	
	HAL_DMA_GROUP_ENABLE(dmaChPara.dmaCtrl);       //enable
	
	if(chCfgPara->dmaCtrl == DMA0CTRL)
	{
		NVIC_EnableIRQ(DMA0_IRQn);
	}
	else if(chCfgPara->dmaCtrl == DMA1CTRL)
	{		
		NVIC_EnableIRQ(DMA1_IRQn);
	}
	
	return result;
}

void DrvDmaChCfg(DRV_DMA_CH_PARA_TypeDef *pPara)
{
    //长度分成2部分，前半部分直接DMACH传输，后半部分触发LLD传输
    HAL_DMA_CH_SRC_ADDR_SET(pPara->dmaCh, pPara->srcAddr);
    HAL_DMA_CH_DST_ADDR_SET(pPara->dmaCh, pPara->dstAddr);
    HAL_DMA_CH_CFG_SET(pPara->dmaCh, PACKED_U32(&pPara->ccfg));
    HAL_DMA_CH_CTRL_SET(pPara->dmaCh, PACKED_U32(&pPara->ccr));
    HAL_DMA_CH_TRANS_SIZE_SET(pPara->dmaCh, pPara->lenInByte/4); //总长度  /4
    
    pPara->lld.srcAddr = (pPara->srcAddr + (pPara->lenInByte/2));
    pPara->lld.dstAddr = (pPara->dstAddr + (pPara->lenInByte/2));
    pPara->lld.size = 1;
	
	#if 1    	//不使用链表传输方式时写    1
	pPara->lld.ctrl.w = 0;	
	#endif

    HAL_DMA_CH_LLP_ADDR_SET(pPara->dmaCh, (uint32_t)&pPara->lld);//总长度  /4

    return;
}

#endif
