#ifndef __SIMULATION_REPORT_H__
#define __SIMULATION_REPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#include "USB\Debug\drv_usb_debug.h"

#define MAX_DATA_COUNT 	(14)
#define DATA_OFFSET	  	(7)

typedef struct {
	uint8_t byte[4];
}SimulationData;

typedef union {
	uint8_t 				mByte[63];
	struct {
		uint8_t 			mCommand;
		uint8_t 			mUnused[2];
		uint8_t 			mAddress[2];
		uint8_t 			mCount[2];
		SimulationData 		mData[MAX_DATA_COUNT];
	}mLayout;
	struct {
		uint8_t				mHoldAck0;
		uint8_t				mHoldAck1;
	}mHoldAck;
}SimulationReportContent;

typedef struct {
	uint8_t 				mReportId;
	SimulationReportContent mContent;
}SimulationReport;

typedef enum {
	SC_PRAM_READ	= 0xC0,
	SC_PRAM_WRITE	= 0xC1,
	SC_IRAM_READ	= 0xC2,
	SC_IRAM_WRITE	= 0xC3,
	SC_SRAM_READ	= 0xC4,
	SC_SRAM_WRITE	= 0xC5,
	SC_CONTINUE		= 0xC9,
	SC_RESET		= 0xCA,
	SC_HOLD			= 0xCB,
	SC_ACK			= 0xF0,
	SC_ERR			= 0XE0,
	SC_HOLD_ACK0	= 0x55,
	SC_HOLD_ACK1	= 0xAA,
}SimulationCommand;

__inline static uint32_t bytes2WordBe32(const uint8_t byte[4]) {
	return (byte[0] << 24 | byte[1] << 16 | byte[2] << 8 | byte[3]);
}

__inline static uint32_t bytes2WordLe32(const uint8_t byte[4]) {
	return (byte[3] << 24 | byte[2] << 16 | byte[1] << 8 | byte[0]); 
}

__inline static void word2BytesBe32(const uint32_t word,uint8_t byte[4]){
	byte[0] = (word >> 24);
	byte[1] = (word >> 16) & 0xFF;
	byte[2] = (word >> 8 ) & 0xFF;
	byte[3] = (word & 0xFF);
}
__inline static void word2BytesLe32(const uint32_t word,uint8_t byte[4]){
	byte[0] = (word & 0xFF);
	byte[1] = (word >> 8 ) & 0xFF;
	byte[2] = (word >> 16) & 0xFF;
	byte[3] = (word >> 24);
}

__inline static uint16_t bytes2WordLe16(const uint8_t byte[2]){
	return ((byte[1] << 8) | byte[0]);
}

__inline static uint16_t bytes2WordBe16(const uint8_t byte[2]){
	return ((byte[0] << 8) | byte[1]);
}

__inline static void word2BytesLe16(const uint16_t word,uint8_t byte[2]){
	byte[0] = (word & 0xff);
	byte[1] = (word >> 8) & 0xff;
}

__inline static void word2BytesBe16(const uint16_t word,uint8_t byte[2]){
	byte[0] = (word >> 8) & 0xff;
	byte[1] = (word & 0xff);
}

__inline static uint16_t getDataAddress(const SimulationReport *report) {
	return bytes2WordLe32(report->mContent.mLayout.mAddress);
}

__inline static uint16_t getDataCount(const SimulationReport *report){
	return bytes2WordLe32(report->mContent.mLayout.mCount);
}

__inline static uint32_t getData(const SimulationReport *report,int index){
	// ASSERT(index < MAX_DATA_COUNT);
	return bytes2WordLe32(report->mContent.mLayout.mData[index].byte);
}

__inline static const uint8_t* getByteData(const SimulationReport *report,int index){
	return report->mContent.mLayout.mData[index].byte;
}

__inline static void setData(SimulationReport *report,int index,uint32_t data){
	word2BytesLe32(data,report->mContent.mLayout.mData[index].byte);
}

__inline static void setByteData(SimulationReport *report,int index,uint8_t byte[4]){
	
	report->mContent.mLayout.mData[index].byte[0] = byte[0];
	report->mContent.mLayout.mData[index].byte[1] = byte[1];
	report->mContent.mLayout.mData[index].byte[2] = byte[2];
	report->mContent.mLayout.mData[index].byte[3] = byte[3];
}

__inline static void setReportId(SimulationReport *report,const uint8_t reportId){
	report->mReportId = reportId;
}


#ifdef __cplusplus
}
#endif

#endif
