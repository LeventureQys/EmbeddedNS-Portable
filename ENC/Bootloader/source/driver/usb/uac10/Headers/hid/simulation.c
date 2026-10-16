#include "USB\Device\hid\simulation.h"
#include "I2s\SynthSchedule\SynthSchedule.h"


#if defined(DRV_I2S_ENABLE_SYNTH_SCHEDULE) && defined(DRV_OTG_DEVICE_ENABLE_HID) && (DRV_OTG_DEVICE_HID_FUNCTION_SELECT == DRV_OTG_DEVICE_HID_FUNCTION_FOR_SIMULATION)

#include "a0_hal_dsp.h"
#include "Drv_Timer.h"

#define SRAM_ADDRESS_BASE				(0x10000000)
#define SRAM_ADDRESS_LEN				(0x00020000)
#define SRAM_ADDRESS_END				(SRAM_ADDRESS_BASE + SRAM_ADDRESS_LEN)

#define DEFAULT_SEND_REPORTID			(0x11)
#define DEFAULT_RECV_REPORTID			(0x51)

#define DSP_WAITHOLD_TIMEOUT_MS			(10)

#define DSP_PC_ADDRESS					(0xFFF)

#define SIMULATION_DEBUG_ENABLED

#ifdef SIMULATION_DEBUG_ENABLED


typedef struct {
	uint32_t eReportType;
	uint32_t eCommand;
	uint32_t eAddress;
	uint32_t eCount;
	uint32_t eSupport;
	uint32_t eUnknown;
	uint32_t nSuccess;
	uint32_t eHoldTimeout;
	int		 eProcessCode;
}SimulationDebugMonitor;

#define SMLT_DEBUG_INITIALIZER	{0,0,0,0,0,0,0,0};

SimulationDebugMonitor debug = SMLT_DEBUG_INITIALIZER;

#endif

static uint8_t sendReportId	= DEFAULT_SEND_REPORTID;
static uint8_t recvReportId = DEFAULT_RECV_REPORTID;

static int waitForDspHold(void){
	static int maxRetries = DSP_WAITHOLD_TIMEOUT_MS;
	
	while(isDspRunning() && maxRetries-- > 0)
		Timer_Delay(1);
	
	if(maxRetries == 0)
		return -1;
	
	return 0;
}

static int doPramRead(SimulationReport *report){

	uint16_t address = getDataAddress(report);
	uint16_t count = getDataCount(report);
	uint32_t data;
	int i = 0;
	
	for(i = 0; i < count; i++){
		data = DSP_PRAM(address + i);
		setData(report,i,data);
	}
	
	return 0;
}

static int doPramWrite(const SimulationReport *report){
	uint16_t address = getDataAddress(report);
	uint16_t count = getDataCount(report);
	uint32_t data;
	int i;
	
	for(i = 0; i < count; i++){
		data = getData(report,i);
		DSP_PRAM(address + i) = data;
	}
	
	return 0;
}

static int doReadPC(SimulationReport *report){

	uint32_t pc = REG_DSP_PC_RD();
	if(pc >= 2)
		pc -= 2;
	
	setData(report,0,pc << 16);
	return 0;
}
static int doIramRead(SimulationReport *report){
	uint16_t address = getDataAddress(report);
	uint16_t count = getDataCount(report);
	
	if(address == DSP_PC_ADDRESS){
		if(count != 1)
			return -1;
		
		return doReadPC(report);
	}
	
	int i;
	for(i = 0; i < count; i++){
		setData(report,i,DSP_IRAM(address + i));
	}
	
	return 0;
}

static int doWritePC(const SimulationReport *report){
	uint32_t pc = getData(report,0);
//	U32REG(REG_ADDR_DSP_PC) = (pc & 0xFFFF);
	REG_DSP_PC_WR(pc);
}
static int doIramWrite(const SimulationReport *report){
	uint16_t address = getDataAddress(report);
	uint16_t count = getDataCount(report);
	
	if(address == DSP_PC_ADDRESS){
		if(count != 1)
			return -1;
		
		return doWritePC(report);
	}
	
	int i;
	for(i = 0; i < count; i++){
		DSP_IRAM(address + i) = getData(report,i);
	}
	
	return 0;
}

static int doSramRead(SimulationReport *report){
	
	uint8_t *addr = (uint8_t*)(SRAM_ADDRESS_BASE + (getDataAddress(report) << 2));
	uint32_t size = getDataCount(report) << 2;
	
	memcpy(&report->mContent.mByte[DATA_OFFSET],addr,size);
	
	return 0;
}

static int doSramWrite(const SimulationReport *report){
	uint8_t *addr = (uint8_t*)(SRAM_ADDRESS_BASE + (getDataAddress(report) << 2));
	uint32_t size = getDataCount(report) << 2;
	memcpy(addr,&(report->mContent.mByte[DATA_OFFSET]),size);
	return 0;
}
	
static int doContinue(const SimulationReport *report){
	uint32_t pc;
	
	if(isDspRunning())
		return 0;
	
	if(isDspHold()){
		dspContinue();
//		while(!isDspRunning());
	}else if(isSynthProcessInProgress()){
		pc = REG_DSP_PC_RD();
		if(pc >= 2)
			pc -= 2;
		REG_DSP_PC_WR(pc);
		dspStart();
//		while(!isDspRunning());
	}else{
		startSynthSchedule();
	}

	return 0;
}

static int doReset(const SimulationReport *report){
	REG_DSP_PC_WR(0);
	REG_DSP_RESET();
	return 0;
}

static int doHold(const SimulationReport *report){
	
	dspHold();
	return waitForDspHold();
}

static int checkSimulationReport(const SimulationReport *report) {
	uint32_t sAddress;
	uint32_t eAddress;
	uint32_t count;
	
	if(report->mContent.mLayout.mCommand < SC_PRAM_READ || report->mContent.mLayout.mCommand > SC_HOLD)
		return E_COMMAND;
	
	if(report->mContent.mLayout.mCommand >= 0xC6 && report->mContent.mLayout.mCommand <= 0xC8)
		return E_UNSUPPORT;

	if((count = getDataCount(report)) > MAX_DATA_COUNT)
		return E_COUNT;


	if(report->mContent.mLayout.mCommand == SC_SRAM_READ || report->mContent.mLayout.mCommand == SC_SRAM_WRITE){
		count 	 = count << 2;
		sAddress = SRAM_ADDRESS_BASE + (getDataAddress(report) << 2);
		eAddress = SRAM_ADDRESS_BASE + (count << 2);
		
		if(sAddress >= SRAM_ADDRESS_END)
			return E_ADDRESS;
		
		if(eAddress > SRAM_ADDRESS_END)
			return E_ADDRESS;
	}

	return 0;
}

static void exceptionStatistics(int result){
#ifdef SIMULATION_DEBUG_ENABLED
	debug.eProcessCode = result;
	
	if(result < 0){
		switch(result){
			case E_COMMAND:
				debug.eCommand++;
				break;
			
			case E_ADDRESS:
				debug.eAddress++;
				break;
			
			case E_COUNT:
				debug.eCount++;
				break;
			
			case E_UNSUPPORT:
				debug.eSupport++;
				break;
			
			case E_REPORTID:
				debug.eReportType++;
				break;
			
			default:
				debug.eUnknown++;
		}
	}else{
		debug.nSuccess++;
	}
#endif
}

 /*
  * @retval 0: 处理成功，此次处理无需应答。
  * 		1: 处理成功，此次处理需要应答。
  *			2: 处理成功，有数据需要返回。
  *		   -n: 处理失败，-n代表错误码。
  *
  *
  */
int doProcessSimulationReport(SimulationReport *report){
	int result = 0;
		
	if(report->mReportId != recvReportId){
		result = E_REPORTID;
		goto process_exit;
	}
		
	if((result = checkSimulationReport(report)) < 0){
		goto process_exit;
	}
		
//	result = S_NOACK;
	switch(report->mContent.mLayout.mCommand){
		case SC_PRAM_READ:
			doPramRead(report);
			result = S_DATA;
			break;
			
		case SC_PRAM_WRITE:
			doPramWrite(report);
			result = S_DOACK;
			break;
			
		case SC_IRAM_READ:
			result = S_DATA;
			if(doIramRead(report) < 0)
				result = E_COMMAND_PC;
//			doIramRead(report);
//			result = S_DATA;
			break;
			
		case SC_IRAM_WRITE:
			result = S_DOACK;
			if(doIramWrite(report) < 0)
				result = E_COMMAND_PC;
//			doIramWrite(report);
//			result = S_DOACK;
			break;
			
		case SC_SRAM_READ:
			doSramRead(report);
			result = S_DATA;
			break;
			
		case SC_SRAM_WRITE:
			doSramWrite(report);
			result = S_DOACK;
			break;
			
		case SC_CONTINUE:
			doContinue(report);
			result = S_DOACK;
			break;
			
		case SC_RESET:
			doReset(report);
			result = S_DOACK;
			break;
			
		case SC_HOLD:
			result = S_HOLDACK;
			if(!isDspHold()){
				if(doHold(report) < 0)
					result = E_HOLDTIMEOUT;
			}
			break;
			
		default:
			result = E_COMMAND;
	}
	
process_exit:
	if(result != S_NOACK){
		if(result != S_DATA)
			memset(report,0,sizeof(SimulationReport));
		
		report->mReportId = sendReportId;
		
		if(result < 0)
			report->mContent.mLayout.mCommand = SC_ERR;
		else if(result == S_DOACK)
			report->mContent.mLayout.mCommand = SC_ACK;
		else if(result == S_HOLDACK)
			holdAckReport(report);
	}
	exceptionStatistics(result);
	
	return result;
}

#define SRAM_BASE_ADDR	(0x10000000)
#define SRAM(addr)		*((uint8_t*)(SRAM_BASE_ADDR + (addr)))

void simulation_init(const uint16_t sendId,const uint16_t recvId){
	
	if(sendId <= 0xff)
		sendReportId = (uint8_t)sendId;
	
	if(recvId <= 0xff)
		recvReportId = (uint8_t)recvId;
#if 0
	DSP_IRAM(0x0001) = 0x32333435;
	DSP_PRAM(0x0001) = 0x42434445;
	SRAM(0x00000004) = 0x52;
	SRAM(0x00000005) = 0x53;
	SRAM(0x00000006) = 0x54;
	SRAM(0x00000007) = 0x55;
#endif
}

void holdAckReport(SimulationReport *report){
	memset(report,0,sizeof(SimulationReport));
	report->mReportId = sendReportId;
	report->mContent.mHoldAck.mHoldAck0 = SC_HOLD_ACK0;
	report->mContent.mHoldAck.mHoldAck1 = SC_HOLD_ACK1;
}

void breakAckReport(SimulationReport *report){
	holdAckReport(report);
}

void errorReport(SimulationReport *report){
	memset(report,0,sizeof(SimulationReport));
	report->mReportId = sendReportId;
	report->mContent.mLayout.mCommand = SC_ERR;
}

#endif
