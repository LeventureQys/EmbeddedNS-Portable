#ifndef __DEMO_DMA_H
#define __DEMO_DMA_H

#include "a0.h"
#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_dma.h"
#include "drv_dma.h"

void DemoDmaMemeryRefresh(void);
void DemoDmaInit(void);
void DemoDma0Irq(void);
void DemoDma1Irq(void);

#endif

