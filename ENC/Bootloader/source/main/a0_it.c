#include "a0_hal_conf.h"
#include "a0_hal_def.h"

#include "a0_hal_sys_ctrl.h"
#include "a0_hal_timer.h"

#include "app_dma.h"
#include "app_i2s.h"

#include "drv_uac10.h"
#include "conf_demo_or_test.h"

extern void app_i2s_dma_irq(void);
extern void app_i2s_irq(void);

/**
  * @brief  This function handles NMI exception.
  */
void NMI_Handler(void)
{
    HalSysWdtSoftwareRst();
}

/**
  * @brief  This function handles Hard Fault exception.
  */
void HardFault_Handler(void)
{
    if (CoreDebug->DHCSR & 1) 
    {  //check C_DEBUGEN == 1 -> Debugger Connected  
        __breakpoint(0);  // halt program execution here         
    }
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
		
    }
}

/**
  * @brief  This function handles SVCall exception.
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  */
extern uint32_t FlagBits;
void SysTick_Handler(void)
{
	FlagBits = 0xFFFFFFFF;
    HAL_TickInc();
}

//void TIMER0_IRQHandler(void)
//{
//    hal_timer_irq_handler(0);
//    HAL_TIM_INT_CLR(TIM0);
//    return;
//}
//void TIMER1_IRQHandler(void)
//{
//    hal_timer_irq_handler(1);
//    HAL_TIM_INT_CLR(TIM1);
//    return;
//}
//void TIMER2_IRQHandler(void)
//{
//    hal_timer_irq_handler(2);
//    HAL_TIM_INT_CLR(TIM2);
//    return;
//}
//void TIMER3_IRQHandler(void)
//{
//    hal_timer_irq_handler(3);
//    HAL_TIM_INT_CLR(TIM3);
//    return;
//}

/**
  * @brief  This function handles SYNTH Handler. When dsp go to end (run code STOP), DSPEND irq will occur
  */
void DSP_END_IRQHandler(void)
{
    return;
}

void DSP_INT_IRQHandler(void)
{
    return;
}

#if !(defined(CONF_SEL_DEMO) || defined(CONF_SEL_TEST_DRV))
void DMA0_IRQHandler(void)
{
    hal_dma_irq_handler(DMA0CTRL);
    return;
}

void DMA1_IRQHandler(void)
{
    hal_dma_irq_handler(DMA1CTRL);
    return;
}
#endif

void USB_IRQHandler(void)
{
#ifdef HAL_OTG_DEV_MODULE_ENABLED
    DrvUac10IrqHandler();
#endif
    return;
}

