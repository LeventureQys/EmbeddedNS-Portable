#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include "USB\drv_usb_config.h"
#include "IIS\Drv_I2s_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(DRV_I2S_ENABLE_SYNTH_SCHEDULE) && defined(DRV_OTG_DEVICE_ENABLE_HID) && (DRV_OTG_DEVICE_HID_FUNCTION_SELECT == DRV_OTG_DEVICE_HID_FUNCTION_FOR_SIMULATION)
	
#include "USB\Device\hid\simulation_report.h"
#include "a0_hal_dsp.h"

#define E_REPORTID		(-1)
#define E_COMMAND		(-2)
#define E_ADDRESS		(-3)
#define E_COUNT			(-4)
#define E_UNSUPPORT		(-5)
#define E_HOLDTIMEOUT	(-6)
#define E_COMMAND_PC	(-7)

#define S_NOACK			(0)
#define S_DOACK			(1)
#define S_DATA			(2)
#define S_HOLDACK		(3)
	
typedef enum {
	ST_RUNNING	= 0,
	ST_HOLD 	= 1,
}DspHoldingState;


#define REPORTID_NONE	(0xffff)

int  doProcessSimulationReport(SimulationReport *report);

void holdAckReport(SimulationReport *report);

void breakAckReport(SimulationReport *report);

void errorReport(SimulationReport *report);

void simulation_init(const uint16_t sendId,const uint16_t recvId);

#endif

#ifdef __cplusplus
}
#endif

#endif
