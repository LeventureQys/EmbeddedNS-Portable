#ifndef  __TEST_DRV_C_
#define  __TEST_DRV_C_

#include "conf_demo_or_test.h"
#include "test_drv.h"

#ifdef TEST_DRV_DMA_ENABLED
	#include "test_drv_dma.c"
#endif

#ifdef TEST_DRV_EXTI_ENABLED
	#include "test_drv_exti.c"
#endif

#ifdef TEST_DRV_I2C_MASTER_ENABLED
	#include "test_drv_i2c_master.c"
#endif

#ifdef TEST_DRV_KEY_ADC_ENABLED
	#include "test_drv_key_adc.c"
#endif

#ifdef TEST_DRV_KEY_IO_ENABLED
	#include "test_drv_key_io.c"
#endif

#ifdef TEST_DRV_PWM_ENABLED
	#include "test_drv_pwm.c"
#endif

#ifdef TEST_DRV_SPI_STD_ENABLED
	#include "test_drv_spi_std.c"
#endif

#ifdef TEST_DRV_UART_ENABLED
	#include "test_drv_uart.c"
#endif

void TestDrvProcess(void)
{
	#ifdef TEST_DRV_DMA_ENABLED
	TestDrvDmaInit();
	#endif
	
	#ifdef TEST_DRV_EXTI_ENABLED
	TestDrvExtiInit();
	#endif
	
	#ifdef TEST_DRV_I2C_MASTER_ENABLED
	TestDrvI2C();
	#endif
	
	#ifdef TEST_DRV_KEY_ADC_ENABLED
	TestDrvKeyAcd();
	#endif
	
	#ifdef TEST_DRV_KEY_IO_ENABLED
	TestDrvKeyIo();
	#endif
	
	#ifdef TEST_DRV_PWM_ENABLED
	TestDrvPwm();
	#endif
	
	#ifdef TEST_DRV_SPI_STD_ENABLED
	TestDrvSpi();
	#endif
	
	#ifdef TEST_DRV_UART_ENABLED
	TestDrvUart();
	#endif
}

#endif
