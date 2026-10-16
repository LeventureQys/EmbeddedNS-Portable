
#include "a0_hal_conf.h"

#ifdef ITM_ENABLE

#include "a0_hal_def.h"
#include "stdio.h"

struct __FILE { int handle; /* Add whatever you need here */ };

FILE __stdout;
FILE __stdin;

volatile int32_t ITM_RxBuffer;

//重定义printf功能
int fputc(int ch,FILE *f)
{
    return ITM_SendChar(ch);
}

//重定义scanf功能
int fgetc(FILE *f)
{
    while(ITM_CheckChar() != 1)   __NOP();
    int32_t ch = ITM_ReceiveChar();
    if (ch != -1)
    {
        ITM_SendChar(ch);
    }
    return (ch);
}

int ferror(FILE *f) 
{
    /* Your implementation of ferror */
    return EOF;
}

void _ttywrch(int ch) 
{
	fputc(ch, &__stdout);
}

int __backspace()
{
	return 0;
}

void _sys_exit(int return_code) 
{
    while (1);    /* endless loop */
}

#if 0
//测试通过keil的窗口进行输入输出测试
void itmTest(void)
{
    int32_t value = 0;
    while (1)
    {
        printf("input value\n");
        scanf("%d", &value);
        printf("\nvalue = %d\n", value);
    }
}
#endif

void SWO_Init(uint32_t portBits, uint32_t cpuCoreFreqHz)
{
    uint32_t SWOSpeed = 6000000; /* default 6MHz baud rate */
    uint32_t SWOPrescaler = (cpuCoreFreqHz / SWOSpeed) - 1; /* SWOSpeed in Hz, note that cpuCoreFreqHz is expected to be match the CPU core clock */

    /* enable access to SWO registers */
    CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk; /* enable trace in core debug */
    ITM->LAR = 0xC5ACCE55;      /* ITM Lock Access Register, C5ACCE55 enables more write access to Control Register 0xE00 :: 0xFFC */

    /* initially disable ITM and stimulus port, to make sure that nothing transferred via SWO when changing the SWI prescaler */
    ITM->TER &= (~portBits);    /* disable ITM stimulus port */
    ITM->TCR = 0;               /* disable TIM */
    
    /* initialize SWO prescaler */
    TPI->SPPR = 0x00000002;     /* "Selected PIN Protocol Register": Select which protocol to use for trace output (2: SWO NRZ, 1: SWO Manchester encoding) */
    TPI->ACPR = SWOPrescaler;   /* "Async Clock Prescaler Register". Scale the baud rate of the asynchronous output */
    ITM->TPR = ITM_TPR_PRIVMASK_Msk; /* ITM Trace Privilege Register */
    DWT->CTRL = 0x400003FE;     /* DWT_CTRL */
    TPI->FFCR = 0x00000100;     /* Formatter and Flush Control Register */
    
    /* enable ITM and stimulus port */
    ITM->TCR = ITM_TCR_TraceBusID_Msk | ITM_TCR_SWOENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk; /* ITM Trace Control Register */
    ITM->TER |= portBits;       /* ITM Trace Enable Register. Enabled tracing on stimulus ports. One bit per stimulus port. */
}

#endif

