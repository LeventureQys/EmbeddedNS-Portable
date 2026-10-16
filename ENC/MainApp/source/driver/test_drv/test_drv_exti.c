
#ifndef __TEST_DRV_EXTI_C_
#define __TEST_DRV_EXTI_C_

#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_gpio.h"
#include "test_drv_exti.h"

uint32_t irqCnt0 = 0;
uint32_t irqCnt1 = 0;

void TestDrvExtiInit(void)
{
  HAL_GPIO_AF_SET(GPIO0, 26, GPIO_AF0);
  HAL_GPIO_EXTI_PIN_SEL_P0(EXTI0, BIT26);    
  HAL_GPIO_EXTI_MODE_SET(EXTI0, GPIO_FALLING_EDGE);
  HAL_GPIO_EXTI_ENABLE(EXTI0);
  NVIC_EnableIRQ(EXT0_INT_IRQn);
}

void Exti0Irq(void)
{
    irqCnt0++;
    if (irqCnt0 >= 100)
    {
        HAL_GPIO_EXTI_DISABLE(EXTI0);
    }
    HAL_GPIO_EXTI_CLR(EXTI0);
    return;
}

void Exti1Irq(void)
{
    irqCnt1++;
    if (irqCnt1 >= 100)
    {
        HAL_GPIO_EXTI_DISABLE(EXTI1);
    }
    HAL_GPIO_EXTI_CLR(EXTI1);
    return;
}

void EXT0_INT_IRQHandler(void)
{
    Exti0Irq();
    return;
}

void EXT1_INT_IRQHandler(void)
{
    Exti1Irq();
    return;
}


#endif
