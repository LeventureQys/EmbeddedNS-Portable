#ifndef __DEMO_C_
#define __DEMO_C_

#include "conf_demo_or_test.h"
#include "demo.h"

#ifdef DEMO_ADC_ENABLED
#include "demo_adc.c"
#endif

#ifdef DEMO_DMA_ENABLED
#include "demo_dma.c"
#endif

#ifdef DEMO_DUAL_TIMER_ENABLED
#include "demo_dual_timer.c"
#endif

#ifdef DEMO_EXTI_ENABLED
#include "demo_exti.c"
#endif

#ifdef DEMO_PWM_ENABLED
#include "demo_pwm.c"
#endif

#ifdef DEMO_SPI_STD_ENABLED
#include "demo_spi_std.c"
#endif

#ifdef DEMO_UART_ENABLED
#include "demo_uart.c"
#endif

#ifdef DEMO_WDT_ENABLED
#include "demo_wdt.c"
#endif

void DemoProcess(void)
{
#ifdef DEMO_ADC_ENABLED
    DemoAdc();
#endif
#ifdef DEMO_DMA_ENABLED
    DemoDmaInit();
#endif
#ifdef DEMO_DUAL_TIMER_ENABLED
    DemoTimer0();
    DemoTimer1();
#endif
#ifdef DEMO_EXTI_ENABLED
    DemoExtiInit();
#endif
#ifdef DEMO_PWM_ENABLED
    DemoPwm();
#endif
#ifdef DEMO_SPI_STD_ENABLED
    DemoSpi();
#endif
#ifdef DEMO_UART_ENABLED
    DemoUart();
#endif
#ifdef DEMO_WDT_ENABLED
    DemoWdt();
#endif
}

#endif
