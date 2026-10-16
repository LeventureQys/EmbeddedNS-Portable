

#ifndef	__TIMER_H
#define	__TIMER_H

#include "Main.h"

#define DELAY_MS(ms)                    do{uint32_t curMs = GetMs(0); while(GetMs(curMs) < ms);}while(0)

void TimerInitial(void);
void TimerHfInitial(void);
void ClrMs(void);
uint32_t GetMs(uint32_t lastTimeMs);
void Timer_Delay(uint32_t ms);
uint32_t TimeMsGet(void);

#endif

