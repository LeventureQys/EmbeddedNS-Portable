#ifndef __DRV_DMA_H
#define __DRV_DMA_H

#include "a0.h"
#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_dma.h"

typedef struct
{
    uint32_t            dmaChUse;
    uint32_t            srcAddress;
    uint32_t            dstAddress;
    uint32_t            lenInByte;
    uint32_t            selectIdx;
    uint32_t            selectSrc;
    uint32_t            lldAddress;
    DMA_CH_CCR_TypeDef  ccr;
    DMA_CH_CCFG_TypeDef ccfg;
}DMA_CFG_PARA_TypeDef;

typedef struct
{
    void                *pHVar;                     /*!< handle variables */    
    void                (*pCbTC)(void *pHandle);    /*!< callback function for dma irq terminal count */
    void                (*pCbErr)(void *pHandle);   /*!< callback function for dma irq error */
    void                (*pCbAbt)(void *pHandle);   /*!< callback function for dma irq abort */
    uint32_t            chIdx;
    DMA_CTRL_TypeDef    *InstanceGroup;             /*!< Group Register base address */
    DMA_CH_TypeDef      *InstanceCh;                /*!< Ch Register base address */
    DMA_SEL_TypeDef     *InstanceSelect;            /*!< Select Register base address */
}DMA_HANDLE_TypeDef;

typedef struct
{
    DMA_CH_CCFG_TypeDef ccfg;
    DMA_CH_CCR_TypeDef  ccr;
    DMA_CTRL_TypeDef    *dmaCtrl;
    DMA_CH_TypeDef      *dmaCh;
    uint32_t            srcAddr;
    uint32_t            dstAddr;
    uint32_t            lenInByte;
    DMA_LLD_TypeDef     lld;
}DRV_DMA_CH_PARA_TypeDef;

typedef struct
{
    DMA_CTRL_TypeDef    *dmaCtrl;
    uint8_t      dmaCh;
}DMA_CH_CFG_PARA_TypeDef;

typedef struct
{
    uint32_t            handleNumber;
    DMA_HANDLE_TypeDef   *pHandle;    
}DMA_INIT_TypeDef;

int32_t DrvDmaInit(DMA_CH_CFG_PARA_TypeDef* chCfgPara, uint32_t dstAdd, uint32_t sourceAdd, uint32_t transLen);

void DrvDmaChCfg(DRV_DMA_CH_PARA_TypeDef *pPara);

#endif

