#ifndef __TEST_DRV_H_
#define __TEST_DRV_H_

#include "conf_demo_or_test.h"
#include "test_drv_dma.h"
#include "test_drv_pwm.h"
#include "test_drv_exti.h"
#include "test_drv_uart.h"
#include "test_drv_spi_std.h"

#define TEST_DRV

extern uint8_t testDrvSpiSlave;
extern uint8_t testDrvSpiMaster;

void TestDrvProcess(void);

#endif
