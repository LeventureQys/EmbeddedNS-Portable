/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0.h
 * @brief   Header file of chip a0.
    | Version | Data       | Author     | Info                                                     |
    | ------- | ---------- | ---------- | -------------------------------------------------------- |
    | 1.0.0   | 2021/09/15 | huangzheng | First version                                            |
    | 1.0.1   | 2021/09/27 | huangzheng |                                                          |
    | 1.0.2   | 2021/10/15 | huangzheng |                                                          |
    | 1.0.3   | 2021/11/09 | huangzheng | 1. Add user irq<br/>2. SPIFC registers update            |
    | 1.0.4   | 2022/03/25 | huangzheng | 1. Adjust register of SYS_Typedef                        |
    |         |            |            | 2. Adjust include files                                  |
    | 1.0.5   | 2022/04/11 | huangzheng | 1. Add macro to get bit-band region address of registers |
    | 1.0.6   | 2022/07/13 | huangzheng | 1. The details of SYS_TypeDef is modified                |
    | 1.0.7   | 2023/12/01 | huangzheng | 1. DMA base address and pointer support channel index    |
 * @version V1.0.7
 * @date    2023/12/01
 * @author  huangzheng
 * @details
    This file include:
    1. The definition of interrupt number, @ref IRQn_Type
    2. Macros
    3. Memory map address
    4. Peripheral declaration
 ******************************************************************************
 */
#ifndef A0_H
#define A0_H

/**
 * @addtogroup A0_CHIP A0 Chip
 * @{
 */
/**
 * @addtogroup A0_HAL_Driver A0 HAL Driver
 * @{
 */
/**
 * @}
 */

/* -------------------------  Interrupt Number Definition  ------------------------ */
/**
 * @addtogroup IRQ_NUMBER_DEFINITION IRQ Number Definition
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum IRQn
{
/* -------------------  Cortex-M4 Processor Exceptions Numbers  ------------------- */
    NonMaskableInt_IRQn         = -14,      /*  2 Non Maskable Interrupt */
    HardFault_IRQn              = -13,      /*  3 HardFault Interrupt */
    MemoryManagement_IRQn       = -12,      /*  4 Memory Management Interrupt */
    BusFault_IRQn               = -11,      /*  5 Bus Fault Interrupt */
    UsageFault_IRQn             = -10,      /*  6 Usage Fault Interrupt */
    SVCall_IRQn                 =  -5,      /* 11 SV Call Interrupt */
    DebugMonitor_IRQn           =  -4,      /* 12 Debug Monitor Interrupt */
    PendSV_IRQn                 =  -2,      /* 14 Pend SV Interrupt */
    SysTick_IRQn                =  -1,      /* 15 System Tick Interrupt */

/* ----------------------    A0 Specific Interrupt Numbers    --------------------- */
    TIMER0_IRQn                 =  0,       /*!< Timer 0 Interrupt */
    TIMER1_IRQn                 =  1,       /*!< Timer 1 Interrupt */
    TIMER2_IRQn                 =  2,       /*!< Timer 2 Interrupt */
    TIMER3_IRQn                 =  3,       /*!< Timer 3 Interrupt */
    TIMER_DUAL_IRQn             =  4,       /*!< Timer Dual Interrupt */
    UART0_TX_IRQn               =  5,       /*!< Uart 0 TX Interrupt */
    UART0_RX_IRQn               =  6,       /*!< Uart 0 RX Interrupt */
    UART1_TX_IRQn               =  7,       /*!< Uart 1 TX Interrupt */
    UART1_RX_IRQn               =  8,       /*!< Uart 1 RX Interrupt */
    UART2_TX_IRQn               =  9,       /*!< Uart 2 RX Interrupt */
    UART2_RX_IRQn               = 10,       /*!< Uart 2 RX Interrupt */
    UART3_TX_IRQn               = 11,       /*!< Uart 3 RX Interrupt */
    UART3_RX_IRQn               = 12,       /*!< Uart 3 RX Interrupt */
    I2C_M_TX_IRQn               = 13,       /*!< I2c Master Tx Interrupt */
    I2C_M_RX_IRQn               = 14,       /*!< I2c Master Rx Interrupt */
    I2C_S_IRQn                  = 15,       /*!< I2c Slave Interrupt */
    SPI1_TX_IRQn                = 16,       /*!< Spi 1 Tx Interrupt */
    SPI1_RX_IRQn                = 17,       /*!< Spi 1 Rx Interrupt */
    SPI2_TX_IRQn                = 18,       /*!< Spi 2 Tx Interrupt */
    SPI2_RX_IRQn                = 19,       /*!< Spi 2 Rx Interrupt */
    SPI3_TX_IRQn                = 20,       /*!< Spi 3 Tx Interrupt */
    SPI3_RX_IRQn                = 21,       /*!< Spi 3 Rx Interrupt */
    PWM_IRQn                    = 22,       /*!< Pwm Interrupt */
    EXT0_INT_IRQn               = 23,       /*!< Ext 0 Interrupt */
    EXT1_INT_IRQn               = 24,       /*!< Ext 1 Interrupt */
    ADC_IRQn                    = 25,       /*!< ADC Interrupt */
    I2S_IRQn                    = 26,       /*!< I2s Master Interrupt */
    I2S_S1_IRQn                 = 27,       /*!< I2s Slave1 Interrupt */
    I2S_S2_IRQn                 = 28,       /*!< I2s Slave2 Interrupt */
    SPI_IRQn                    = 29,       /*!< Spi Flash Controller Interrupt */
    USB_IRQn                    = 30,       /*!< Usb Interrupt */
    SDC_IRQn                    = 31,       /*!< Sdc Interrupt */
    DMA0_IRQn                   = 32,       /*!< Dma 0 Interrupt */
    DMA1_IRQn                   = 33,       /*!< Dma 1 Interrupt */
    DSP_INT_IRQn                = 34,       /*!< Dsp Interrupt */
    DSP_END_IRQn                = 35,       /*!< Dsp End Interrupt */
    USER0_IRQn                  = 36,       /*!< User Defined 0 Interrupt */
    USER1_IRQn                  = 37,       /*!< User Defined 1 Interrupt */
    USER2_IRQn                  = 38,       /*!< User Defined 2 Interrupt */
    USER3_IRQn                  = 39,       /*!< User Defined 3 Interrupt */
    IRQn_MAX                    = 40,
} IRQn_Type;

#ifdef __cplusplus
}
#endif

/**
 * @}
 */
/**
 * @}
 */

/* ================================================================================ */
/* ================       Processor and Core Peripheral Section     =============== */
/* ================================================================================ */

/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

/* --------  Configuration of the Cortex-M4 Processor and Core Peripherals  ------- */
//#define ARM_MATH_CM4
#define __CM4_REV                 0x0001U   /* Core revision r0p1 */
#define __MPU_PRESENT             1U        /* MPU present */
#define __VTOR_PRESENT            1U        /* VTOR present */
#define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels, max bit is 8 */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1U        /* FPU present */

#include "core_cm4.h"                       /* Processor and core peripherals */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup A0_CHIP A0 Chip
 * @{
 */
/**
 * @addtogroup GLOBAL_MACROS Global Macros
 * @{
 */
#define u32RegPort(address)                 *((__IO uint32_t *)(address))                       /*!< 32bit register access */

#define U8REG(address)                      *((__IO uint8_t *)(address))                        /*!< 8bit register access */
#define U32REG(address)                     *((__IO uint32_t *)(address))                       /*!< 32bit register access */

#define PACKED_U16(address)                 (*((__packed uint16_t *)(address)))                 /*!< 16bit packed memery access */
#define PACKED_U32(address)                 (*((__packed uint32_t *)(address)))                 /*!< 32bit packed memery access */

/*
 * regAddr range is 0x40000000~0x400FFFFF
 * bit range is 0~31
 * user can use this macro to get the address of bit in register
 */
#define BITBAND_ADDR_GET(regAddr, bit)      ((((regAddr) & 0xFFFFF) << 5) + 0x42000000 + ((bit) << 2))

#define __RAM_CODE                          __attribute__((used, section("ram_code_section")))  /*!< user can use this section to place code in ram */
#define __RAM00                             __attribute__((used, section("ram00_section")))     /*!< Range: 0x10000000:0x10007FFF */
#define __RAM01                             __attribute__((used, section("ram01_section")))     /*!< Range: 0x10008000:0x1000FFFF */
#define __RAM02                             __attribute__((used, section("ram02_section")))     /*!< Range: 0x10010000:0x1001FFFF */
#define __RAM1                              __attribute__((used, section("ram1_section")))      /*!< Range: 0x20000000:0x20007FFF */
#define __RAM2                              __attribute__((used, section("ram2_section")))      /*!< Range: 0x28000000:0x28007FFF */
/**
 * @}
 */
/**
 * @}
 */
/* ================================================================================ */
/* ================       Device Specific Peripheral Section       ================ */
/* ================================================================================ */

/**
 * @addtogroup ADC
 * @brief  10 bit Analog Digithal Convert (ADC) module
 * @{
 */
/**
 * @defgroup ADC_Register_Types ADC Register Types
 * @brief  ADC register information
 * @{
 */
/* ================================================================================ */
/* ================         Analog Digital Convert (ADC)           ================ */
/* ================================================================================ */
/**
 * @brief  ADC control register bit field definition, 'CR' in @ref ADC_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t data:10;               /*!< [9:0],     convert data in one channel convert mode                    */
        uint32_t chSel:5;               /*!< [14:10],   channel select in one channel convert mode                  */
        uint32_t :1;
        uint32_t mode:1;                /*!< [16],      1, one channel mode, 0, multi channel mode                  */
        uint32_t fCnt:6;                /*!< [22:17],   ADCClk = PCLK / (2*(fCnt+1)), ADCClk range is 1MHz to 16MHz */
        uint32_t sCnt:4;                /*!< [26:23],   delay cycle (ADCCLK) before adc convert, range is 5 to 15   */
        uint32_t rst:1;                 /*!< [27],      write 1 to reset adc module                                 */
        uint32_t start:1;               /*!< [28],      write 1 to start adc convert                                */
        uint32_t powerDown:1;           /*!< [29],      write 1 to power down adc module                            */
        uint32_t busy:1;                /*!< [30],      1 means adc module is converting                            */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
}ADC_CR_TypeDef;
/**
 * @brief  ADC registers structure definition
 */
typedef struct
{
    __IO uint32_t CR;                   /*!< Offset: 0x00 (R/W) adc control register, @ref ADC_CR_TypeDef           */
    __IO uint32_t MCHSR;                /*!< Offset: 0x04 (R/W) adc channel enable in multi channel convert mode    */
    __IO uint32_t DR[21];               /*!< Offset: 0x08 (R/ ) adc channel convert data register                   */
} ADC_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup GPIO
 * @brief  General Purpose I/O (GPIO) with 2 port, 32 pins in GPIO0 and 16 pins in GPIO1
 * @{
 */
/**
 * @defgroup GPIO_Register_Types GPIO Register Types
 * @brief  GPIO register information
 * @{
 */
/* ================================================================================ */
/* ================         Exteranl Interrupt (EXTI)              ================ */
/* ================================================================================ */
/**
 * @brief  External Interrupt of GPIO control register bit field definition, 'CR' in @ref EXTINT_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t intState:1;            /*!< [0],       int state of external interrupt, write '1' to clear         */
        uint32_t intEn:1;               /*!< [1],       int enable of external interrupt                            */
        uint32_t :2;
        uint32_t intMode:2;             /*!< [5:4],     int trigger mode of external interrupt                      */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} EXTINT_CR_TypeDef;
/**
 * @brief  External Interrupt of GPIO registers structure definition
 */
typedef struct
{
    __IO uint32_t CR;                   /*!< Offset: 0x00 (R/W) external interrupt control register, @ref EXTINT_CR_TypeDef */
    __IO uint32_t SELR[2];              /*!< Offset: 0x04-0x08 (R/W) external interrupt io select register                  */
} EXTINT_TypeDef;
/* ================================================================================ */
/* ================          General Purpose I/O (GPIO)            ================ */
/* ================================================================================ */
/**
 * @brief  GPIO registers structure definition
 */
typedef struct
{
    __I  uint32_t IDR;                  /*!< Offset: 0x00 (R/ ) GPIO port input data register                       */
    __IO uint32_t ODR;                  /*!< Offset: 0x04 (R/W) GPIO port output data register                      */
    __IO uint32_t OER;                  /*!< Offset: 0x08 (R/W) GPIO port output enable register                    */
    __O  uint32_t BSR;                  /*!< Offset: 0x0C ( /W) GPIO port output bit set register, write 1 to set   */
    __O  uint32_t BCR;                  /*!< Offset: 0x10 ( /W) GPIO port output bit clr register, write 1 to clr   */
    __IO uint32_t OTYPER;               /*!< Offset: 0x14 (R/W) GPIO port output type register                      */
    __IO uint32_t AFR[4];               /*!< Offset: 0x18-0x24 (R/W) GPIO port alternate function register          */
    __IO uint32_t DRVR[2];              /*!< Offset: 0x28-0x2C (R/W) GPIO port output drive register                */
    __IO uint32_t PUPDR[2];             /*!< Offset: 0x30-0x34 (R/W) GPIO port input pull-up/pull-down register     */
    __IO uint32_t ISMTER;               /*!< Offset: 0x38 (R/W) GPIO port input schmitt trigger enable register     */
} GPIO_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/* ================================================================================ */
/* ================         Inter-Integrated Circuit (I2C)         ================ */
/* ================================================================================ */
//Master
/**
 * @addtogroup I2C_Master
 * @brief Inter-Integrated Circuit (I2C) master module
 * @{
 */
/**
 * @defgroup I2C_Master_Register_Types I2C Master Register Types
 * @brief  I2C master register information
 * @{
 */
/**
 * @brief  I2C master control register bit field definition, 'CR' in @ref I2C_M_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t ackTx:1;               /*!< [0],       write "0" to send ACK, write "1" to send NACK, auto reset to "0" */
        uint32_t wr:1;                  /*!< [1],       wrtie "1" to do WRITE operation, auto reset to "0" */
        uint32_t rd:1;                  /*!< [2],       wrtie "1" to do READ operation, auto reset to "0" */
        uint32_t restart:1;             /*!< [3],       wrtie "1" to do RESTART operation, auto reset to "0" */
        uint32_t stop:1;                /*!< [4],       write "1" to do STOP operation, auto reset to "0" */
        uint32_t start:1;               /*!< [5],       write "1" to do START operation, auto reset to "0" */
        uint32_t :6;
        uint32_t empty:1;               /*!< [12],      0, send buffer is not empty, 1, send buffer is empty */
        uint32_t tip:1;                 /*!< [13],      0, transport complete, 1, transporting */
        uint32_t busy:1;                /*!< [14],      0, bus idle, 1, bus busy */
        uint32_t ackRx:1;               /*!< [15],      0, get nack, 1, get ack */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} I2C_M_CR_TypeDef;
/**
 * @brief  I2C master registers structure definition
 */
typedef struct
{
    __IO uint32_t CR;                   /*!< Offset: 0x00 (R/W) Ctrl Register, @ref I2C_M_CR_TypeDef    */
    __O  uint32_t CSR;                  /*!< Offset: 0x04 ( /W) Clock Set Register                      */
    __I  uint32_t RDR:8;                /*!< Offset: 0x08 (R/ ) Rx Data Register                        */
    __O  uint32_t TDR:8;                /*!< Offset: 0x08 ( /W) Tx Data Register                        */
} I2C_M_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

//Slave
/**
 * @addtogroup I2C_Slave
 * @brief Inter-Integrated Circuit (I2C) slave module
 * @{
 */
/**
 * @defgroup I2C_Slave_Register_Types I2C Slave Register Types
 * @brief  I2C slave register information
 * @{
 */
/**
 * @brief  I2C slave control register bit field definition, 'CR' in @ref I2C_S_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t intEn:1;               /*!< [0],       1, int enable, 0, int disable */
        uint32_t dmaEn:1;               /*!< [1],       1, dma enable, 0, dma disable */
        uint32_t intFlag:1;             /*!< [2],       interrupt flag, write "0" to clear */
        uint32_t :1;
        uint32_t wrAckEn:1;             /*!< [4],       enable once (master)write transport */
        uint32_t rdAckEn:1;             /*!< [5],       enable once (master)read transport */
        uint32_t rdHoldEn:1;            /*!< [6],       1, enable sck hold, 0, disable sck hold */
        uint32_t rdHoldFlag:1;          /*!< [7],       1, sck is in hold state */
        uint32_t wrByteNum:5;           /*!< [12:8],    rx byte data number */
        uint32_t :3;
        uint32_t rdByteNum:5;           /*!< [20:16],   tx byte data number */
        uint32_t state:3;               /*!< [23:21],   state of irq source */
        uint32_t isRdFlag:1;            /*!< [24],      0, irq from write, 1, irq from read */
        uint32_t devAddr:7;             /*!< [31:25],   7 bit device address */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} I2C_S_CR_TypeDef;
/**
 * @brief  I2C slave registers structure definition
 */
typedef struct
{
    __IO uint32_t CR;                   /*!< Offset: 0x00 (R/W) Ctrl Register, @ref I2C_S_CR_TypeDef    */
    __I  uint32_t RDR[4U];              /*!< Offset: 0x04 (R/ ) Rx Data Register                        */
    __IO uint32_t TDR[4U];              /*!< Offset: 0x14 (R/W) Tx Data Register                        */
} I2C_S_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup I2S
 * @brief Integrated-Interchip Sound (I2S) module
 * @{
 */
/**
 * @defgroup I2S_Register_Types I2S Register Types
 * @brief  I2S register information
 * @{
 */
/* ================================================================================ */
/* ================        Integrated-Interchip Sound (I2S)        ================ */
/* ================================================================================ */
/**
 * @brief  I2S control register bit field definition, 'CR' in @ref I2S_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t clkEn:1;               /*!< [0],       clk enable in master mode, enable(1)/disable(0) */
        uint32_t mclkDiv:2;             /*!< [2:1],     mclk div select in master mode, reserved/div2/div1/div4(0/1/2/3) */
        uint32_t isMaster:1;            /*!< [3],       master(1)/slave(0) */
        uint32_t mode:2;                /*!< [4:5],     mode value 0~3 is LeftJustified/IIS/DspModeRiseEdge/DspModeFallEgde */
        uint32_t mclkSrc:1;             /*!< [6],       mclk source of codec in slave mode, 0, from pll of i2s, 1, from external mclk pin */
        uint32_t mclkOutDisable:1;      /*!< [7],       mclk output disable, 0, mclk will output when the AF of GPIO configed as mclk, 1, mclk will not output */
        uint32_t lrclkDiv:3;            /*!< [10:8],    lrclk div select in master mode, clkSrc/lrclk = 64 * 64/48/32/24/16/8/4/2 (0~7)
                                                        when pclk is 45.1884MHz, the fs of configed value 0~7 is 11.025/14.7/22.05/29.4/44.1/88.2/176.4/352.8k
                                                        when pclk is 49.152MHz, ths fs of configed value 0~7 is 12/16/24/32/48/96/192/384k */
        uint32_t :1;
        uint32_t frameNum:5;            /*!< [16:12],   i2s irq will occur every (frameNum+1) samples */
        uint32_t :3;
        uint32_t clkEnS1:1;             /*!< [20],      i2s slave1 enable, enable(1)/disable(0), clk pin in AF p0.30/p0.31 */
        uint32_t intEnS1:1;             /*!< [21],      i2s slave1 int enable, enable(1)/disable(0) */
        uint32_t dmaEnS1:1;             /*!< [22],      i2s slave1 dma enable, enable(1)/disable(0) */
        uint32_t :1;
        uint32_t clkEnS2:1;             /*!< [24],      i2s slave2 enable, enable(1)/disable(0) */
        uint32_t intEnS2:1;             /*!< [25],      i2s slave2 int enable, enable(1)/disable(0) */
        uint32_t dmaEnS2:1;             /*!< [26],      i2s slave2 dma enable, enable(1)/disable(0) */
        uint32_t :1;
        uint32_t d6OutDacEn:1;          /*!< [28],      i2s d6 pin output dac data enable(1)/disable(0) */
        uint32_t d6FifoSel:1;           /*!< [29],      i2s d6 fifo usage select, 0 for dac, 1 for i2s d6 */
        uint32_t d5FifoSel:1;           /*!< [30],      i2s d5 fifo usage select, 0 for adc0, 1 for i2s d5 */
        uint32_t d4FifoSel:1;           /*!< [31],      i2s d4 fifo usage select, 0 for adc1, 1 for i2s d4 */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} I2S_CR_TypeDef;
/**
 * @brief  I2S channel control register0 bit field definition, 'CCR[0]' in @ref I2S_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t fifoReset:7;           /*!< [6:0],     i2s fifo reset, write 1 to clear, from MSB to LSB is Fifo6/Fifo5/Fifo4/Fifo3/Fifo2/Fifo1/Fifo0 */
        uint32_t :5;
        uint32_t dOutEn:7;              /*!< [18:12],   i2s external data pin output enable, wirte 1 to config as output, from MSB to LSB is D6/D5/D4/D3/D2/D1/D0 */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} I2S_CCR0_TypeDef;
/**
 * @brief  I2S channel control register1 bit field definition, 'CCR[1]' in @ref I2S_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t d0Clk:2;               /*!< [1:0],     data0 clk group, 0, default clk group, 1, slave1 clk group, 2, slave2 clk group, 3, close */
        uint32_t d1Clk:2;               /*!< [3:2],     data1 clk group, 0, default clk group, 1, slave1 clk group, 2, slave2 clk group, 3, close */
        uint32_t d2Clk:2;               /*!< [5:4],     data2 clk group, 0, default clk group, 1, slave1 clk group, 2, slave2 clk group, 3, close */
        uint32_t d3Clk:2;               /*!< [7:6],     data3 clk group, 0, default clk group, 1, slave1 clk group, 2, slave2 clk group, 3, close */
        uint32_t d4Clk:2;               /*!< [9:8],     data4 clk group, 0, default clk group, 1, slave1 clk group, 2, slave2 clk group, 3, close */
        uint32_t d5Clk:2;               /*!< [11:10],   data5 clk group, 0, default clk group, 1, slave1 clk group, 2, slave2 clk group, 3, close */
        uint32_t d6Clk:2;               /*!< [13:12],   data6 clk group, 0, default clk group, 1, slave1 clk group, 2, slave2 clk group, 3, close */
        uint32_t :2;
        uint32_t bclkSelS2:3;           /*!< [18:16],   pin of bclk of slave2, 0~6 is from pin D0~D6, 7 is from pin mclk */
        uint32_t :1;
        uint32_t lrclkSelS2:3;          /*!< [22:20],   pin of lrclk of slave2, 0~6 is from pin D0~D6, 7 is from pin mclk */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} I2S_CCR1_TypeDef;
/**
 * @brief  I2S registers structure definition
 */
typedef struct
{
    __IO uint32_t CR;                   /*!< offset 0x00, ctrl register */
    __IO uint32_t CCR[2];               /*!< offset 0x04, channel ctrl register */
    __IO uint32_t FR[7][2];             /*!< offset 0x0C, fifo d0~d6, left/right register, if it is configed to output, wirte/read for tx */
} I2S_TypeDef;
/**
 * @brief  I2S FIFO registers structure definition
 */
typedef struct
{
    union
    {
        __I uint32_t Rx;
        __O uint32_t Tx;
    }ChL;
    union
    {
        __I uint32_t Rx;
        __O uint32_t Tx;
    }ChR;
} I2S_FifoTypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup SYS_CTRL
 * @brief System Control module, it include clock, pll, sdram, sram, usb, wdt, codec, efuse, exteranl spi flash access, and so on.
 * @{
 */
/**
 * @defgroup SYS_CTRL_Register_Types Sys Ctrl Register Types
 * @brief  Sys Ctrl register information
 * @{
 */
/* ================================================================================ */
/* ================             System Ctrl Registers              ================ */
/* ================================================================================ */
/**
 * @brief  System control spi interface register bit field definition, 'SYS_SPI' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t addrWidth:1;           /*!< [0],       spi controller access external flash with the address width, 0, 3 bytes address, 1, 4 bytes address */
        uint32_t :3;
        uint32_t dummyCycle:4;          /*!< [7:4],     dummy cycles of external flash read, cycles = (dummyCycle+1) */
        uint32_t modeValue:8;           /*!< [15:8],    mode value of flash read */
        uint32_t clkDiv:3;              /*!< [18:16],   spi clock div from HCLK, SpiClk = HCLK/(2*(clkDIv+1)) */
        uint32_t :12;
        uint32_t ctrlSrc:1;             /*!< [31],      0, SYS_SPI access external flash, 1, SPIFC access external flash in CS1 */
    } b;
    uint32_t w;
} SYS_SPI_TypeDef;
/**
 * @brief  System control sram interface register bit field definition, 'SYS_SRAM' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t fws:8;                 /*!< [7:0],     config the cs signal width, the CS signal width is alse relatived with ws, CS cycle is (ws + fws + 4) HCLK cycles */
        uint32_t ws:8;                  /*!< [15:8],    config the RD or WR signal witdh, RD/WR cycle is (ws + 1) HCLK cycles */
    } b;
    uint32_t w;
} SYS_SRAM_TypeDef;
/**
 * @brief  System control sdram interface register bit field definition, 'SYS_SDRAM' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t refreshCnt:16;         /*!< [15:0],    config the auto refresh time of sdram, auto refresh cycle is ((0xFFFF-refreshCnt)*2) HCLK cycle */
        uint32_t sdramEn:1;             /*!< [16],      0, sdram disable, 1, sdram enable */
    }b;
    uint32_t w;
} SYS_SDRAM_TypeDef;
/**
 * @brief  System control pll0 config register0 bit field definition, 'SYS_PLL_CFG00' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t pdExt:1;               /*!< [0],       pll0 power-down control, 0, normal, 1, powerdown, default: 0 */
        uint32_t ssen:1;                /*!< [1],       pll0 SSCG mode control, 0, normal pll, 1, sscg mode, default: 0 */
        uint32_t testExt:1;             /*!< [2],       pll0 test mode control, 0, normal, 1, test mode, default: 0 */
        uint32_t type:2;                /*!< [4:3],     pll0 spreading type control, 0, down spread, 1, center spread, default: 0 */
        uint32_t mp:1;                  /*!< [5],       pll0 filter function enable, 0, off, 1, on, default: 0 */
        uint32_t fir:1;                 /*!< [6],       pll0 filter function enable, 0, off, 1, on, default: 0 */
        uint32_t ns:9;                  /*!< [15:7],    pll0 spreading frequency control, default: 0x1E */
        uint32_t cpa:9;                 /*!< [24:16],   pll0 current control, default: 0x100 */
        uint32_t nr:7;                  /*!< [31:25],   pll0 division ratio control of reference divider, default: 0x2 */
    }b;
    uint32_t w;
} SYS_PLL_CFG00_TypeDef;
/**
 * @brief  System control pll0 config register1 bit field definition, 'SYS_PLL_CFG01' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t nf:24;                 /*!< [23:0],    pll0 division ratio control of the feedback divider, default: 0x800000 */
        uint32_t od:3;                  /*!< [26:24],   pll0 division ratio control of the output divider, default: 0x3 */
        uint32_t cpax:2;                /*!< [28:27],   pll0 current control, default: 0x0 */
        uint32_t :3;
    }b;
    uint32_t w;
} SYS_PLL_CFG01_TypeDef;
/**
 * @brief  System control pll0 config register2 bit field definition, 'SYS_PLL_CFG02' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t nv:24;                 /*!< [23:0],    pll0 spreading depth (Slope) control, default: 0x002222 */
        uint32_t rs:4;                  /*!< [27:24],   pll0 resistor control, default: 0x8 */
    }b;
    uint32_t w;
} SYS_PLL_CFG02_TypeDef;
/**
 * @brief  System control pll config register1 bit field definition, 'SYS_PLL_CFG1' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t pll1En:1;              /*!< [0],       pll1 enable config, pll1 is used for pll2 */
        uint32_t pll1Range:2;           /*!< [2:1],     pll1 clk out range config */
        uint32_t pll1M:6;               /*!< [8:3],     pll1 M value, must be 1 */
        uint32_t pll1N:6;               /*!< [14:9],    pll1 N value, pll1 clk out = ((Fosc * pllN) / pllM) */
        uint32_t :1;
        uint32_t pll2En:1;              /*!< [16],      pll2 enable config, pll2 is used for i2s module */
        uint32_t pll2M:5;               /*!< [21:17],   pll2 M value, M can be 20 or 25 */
        uint32_t pll2N:9;               /*!< [30:22],   pll2 N value, N can be 128 or 147, if user want to fine adjust the i2s clock, pllN can +1 or -1 */
    }b;
    uint32_t w;
} SYS_PLL_CFG1_TypeDef;
/**
 * @brief  System control clock config register bit field definition, 'SYS_CLK' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t clkLowSel0:1;          /*!< [0],       low frequency srouce select from OSC input or pre low frequency input */
        uint32_t clkSysSel:1;           /*!< [1],       system clock select from pll0 out or low frequency source */
        uint32_t clkI2SSel:1;           /*!< [2],       i2s module input clock select from pll2 out or (system clock / 4) */
        uint32_t clkPll2Sel:1;          /*!< [3],       pll2 input clock select from pll1 out or pll0 out */
        uint32_t clkLowSel1:4;          /*!< [7:4],     pre low frequency clock select from different source */
        uint32_t clkEnPclk:1;           /*!< [8],       PCLK enable ctrl. 1, enable, 0, disable */
        uint32_t clkEnSpi:1;            /*!< [9],       SPI module clock enable ctrl. 1, enable, 0, disable */
        uint32_t clkEnSdc:1;            /*!< [10],      SDC module clock enable ctrl. 1, enable, 0, disable */
        uint32_t clkEnDsp:1;            /*!< [11],      DSP module clock enable ctrl. 1, enable, 0, disable */
        uint32_t clkEnDma:1;            /*!< [12],      DMA module clock enable ctrl. 1, enable, 0, disable */
        uint32_t clkEnUsb:1;            /*!< [13],      USB module clock enable ctrl. 1, enable, 0, disable */
        uint32_t clkEnSysTick:1;        /*!< [14],      Tick clock enable ctrl. 1, enable, 0, disable */
        uint32_t clkEnUsbPhy:1;         /*!< [15],      USB PHY clock(12MHz) enable ctrl. 1, enable, 0, disable */
        uint32_t ramEnDsp:1;            /*!< [16],      SRAM2 (A0_SRAM2_BASE) enable ctrl. 1, enable, 0, disable */
        uint32_t ramEnCache:1;          /*!< [17],      SRAM in cache enable ctrl. 1, enable, 0, disable */
        uint32_t ramEn1:1;              /*!< [18],      SRAM1 (A0_SRAM1_BASE) enable ctrl. 1, enable, 0, disable */
        uint32_t ramEnKbd:1;            /*!< [19],      SRAM in keyboard scan module enable ctrl. 1, enable, 0, disable */
        uint32_t ramEn00:1;             /*!< [20],      part 1 of SRAM0 (A0_SRAM0_BASE+0) enable ctrl. 1, enable, 0, disable */
        uint32_t ramEn01:1;             /*!< [21],      part 2 of SRAM0 (A0_SRAM0_BASE+0x8000) enable ctrl. 1, enable, 0, disable */
        uint32_t ramEn02:1;             /*!< [22],      part 3 of SRAM0 (A0_SRAM0_BASE+0x10000) enable ctrl. 1, enable, 0, disable */
    }b;
    uint32_t w;
} SYS_CLK_TypeDef;
/**
 * @brief  System control codec config register bit field definition, 'SYS_CODEC_CTRL' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t codecRst:1;            /*!< [0],       codec module reset ctrl. 0, hold codec in reset state, 1, ctrl codec in normal state */
        uint32_t mclkMode:4;            /*!< [4:1],     mclk of codec config. Default is 0xF, 256FS */
        uint32_t deEmphasis:2;          /*!< [6:5],     de-emphasis filter config */
        uint32_t reback:1;              /*!< [7],       the dac input digital data returns to the adc digital output function enable ctrl. 0, normal mode, 1, enable reback */
        uint32_t audioIfMode:2;         /*!< [9:8],     codec audio interface mode, 0, parallel, 1, i2s, 2, right-justfied, 3, left-justfied */
        uint32_t dMic0En:1;             /*!< [10],      adc digital mic0 enable ctrl. 0, disable digital mic and select analog line-in path, 1, enable digital mic */
        uint32_t dMic1En:1;             /*!< [11],      adc digital mic1 enable ctrl. 0, disable digital mic, 1, enable digital mic */
        uint32_t vcmPd:1;               /*!< [12],      power down ctrl of the headphone VCM driver. 0, normal mode, 1, power down mode */
        uint32_t micBiasPd:1;           /*!< [13],      mic bias power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t isoEn:1;               /*!< [14],      isolation cell ctrl. 0, normal mode, 1, enable isolation cell to prevent floating signals between the analog and digital interface only when powering down the digital circuit */
        uint32_t digCorePd:1;           /*!< [15],      digital core power down ctrl. 0, normal mode, 1, power down digital core power */
        uint32_t tDigCtrl:6;            /*!< [21:16],   for test only, default = 0 */
    }b;
    uint32_t w;
} SYS_CODEC_TypeDef;
/**
 * @brief  System control codec adc config register0 bit field definition, 'SYS_CODEC_ADC0' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t lInVol:6;              /*!< [5:0],     adc left channel gain ctrl. 0~63 means -27dB~36dB, 27 is 0dB */
        uint32_t lInSel:2;              /*!< [7:6],     adc left channel input select. 0, mute, 1, select X_LMICIN0, 2, select X_LMICIN1, 3, select X_LLINEIN */
        uint32_t lBypassVol:6;          /*!< [13:8],    left channel bypass amplifer analog volume ctrl. 0x11~0x3F means -40dB~+6dB, 0x39 is 0dB */
        uint32_t lBypassSel:1;          /*!< [14],      adc left channel bypass path ctrl. 0, normal mode, 1, X_LLINEIN to headphone amplifer is selected */
        uint32_t lMicBoost:1;           /*!< [15],      adc left channel mic gain boost ctrl. 0, 0dB (default), 1, 20dB boost */
        uint32_t lAdcPd:1;              /*!< [16],      adc left channel power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t lBypassPd:1;           /*!< [17],      adc left channel bypass amplifier power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t lFEPd:1;               /*!< [18],      adc left channel front-end power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t adcDwaEn:1;            /*!< [19],      adc modulator DWA mode enable ctrl. 0, disable, 1, enable */
        uint32_t tAdcGain:2;            /*!< [21:20],   for test only, default = 1 */
    }b;
    uint32_t w;
} SYS_CODEC_ADC0_TypeDef;
/**
 * @brief  System control codec adc config register1 bit field definition, 'SYS_CODEC_ADC1' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t rInVol:6;              /*!< [5:0],     adc right channel gain ctrl. 0~63 means -27dB~36dB, 27 is 0dB */
        uint32_t rInSel:2;              /*!< [7:6],     adc right channel input select. 0, mute, 1, select X_RMICIN0, 2, select X_RMICIN1, 3, select X_RLINEIN */
        uint32_t rBypassVol:6;          /*!< [13:8],    right channel bypass amplifer analog volume ctrl. 0x11~0x3F means -40dB~+6dB, 0x39 is 0dB */
        uint32_t rBypassSel:1;          /*!< [14],      adc right channel bypass path ctrl. 0, normal mode, 1, X_LLINEIN to headphone amplifer is selected */
        uint32_t rMicBoost:1;           /*!< [15],      adc right channel mic gain boost ctrl. 0, 0dB (default), 1, 20dB boost */
        uint32_t rAdcPd:1;              /*!< [16],      adc right channel power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t rBypassPd:1;           /*!< [17],      adc right channel bypass amplifier power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t rFEPd:1;               /*!< [18],      adc right channel front-end power down ctrl. 0, normal mode, 1, power down mode */
    }b;
    uint32_t w;
} SYS_CODEC_ADC1_TypeDef;
/**
 * @brief  System control codec dac config register bit field definition, 'SYS_CODEC_DAC' in @ref SYS_TypeDef
 */
typedef union {
    struct {
        uint32_t lHpVol:6;              /*!< [5:0],     left headphone driver analog volume ctrl. 0x11~0x39 means -40dB~+0dB, 0x39 is 0dB */
        uint32_t lHpSel:1;              /*!< [6],       left headphone driver signal ctrl. 0, left channel DAC to X_LHPOUT is disable, 1, enable */
        uint32_t lMute:1;               /*!< [7],       dac left channel digital mute ctrl. 0, normal mode, 1, mute enable */
        uint32_t lHpPd:1;               /*!< [8],       left headphone driver power down ctrl, 0, normal, 1, power down mode */
        uint32_t lDacPd:1;              /*!< [9],       dac left channel power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t lOutPd:1;              /*!< [10],      X_LOUT driver power down ctrl, 0, normal mode, 1, power down mode */
        uint32_t r2lHpSel:1;            /*!< [11],      left headphone driver signal ctrl. 0, right channel DAC to X_LHPOUT is disable, 1, enable */
        uint32_t dacDemEn:1;            /*!< [12],      dac modulator DEM mode enable ctrl. 0, disable (default), 1, enable */
        uint32_t dacDwaEn:1;            /*!< [13],      dac modulator DWA mode enable ctrl. 0, disable (default), 1, enable */
        uint32_t tDacGain:2;            /*!< [15:14],   for test only, default = 1 */
        uint32_t rHpVol:6;              /*!< [21:16],   right headphone driver analog volume ctrl. 0x11~0x39 means -40dB~+0dB, 0x39 is 0dB */
        uint32_t rHpSel:1;              /*!< [22],      right headphone driver signal ctrl. 0, right channel DAC to X_RHPOUT is disable, 1, enable */
        uint32_t rMute:1;               /*!< [23],      dac right channel digital mute ctrl. 0, notmal mode, 1, mute enable */
        uint32_t rHpPd:1;               /*!< [24],      right headphone driver power down ctrl, 0, normal, 1, power down mode */
        uint32_t rDacPd:1;              /*!< [25],      dac right channel power down ctrl. 0, normal mode, 1, power down mode */
        uint32_t rOutPd:1;              /*!< [26],      X_ROUT driver power down ctrl, 0, normal mode, 1, power down mode */
    }b;
    uint32_t w;
} SYS_CODEC_DAC_TypeDef;
/**
 * @brief  system control registers structure definition
 * @details
 *     When software reset occur, the registers below will not reset.
 *     SYS_PLL_CFG1(0x1C), SYS_CLK(0x20), SYS_EFUSE_CK0(0x34), SYS_EFUSE_CK1(0x38), SYS_EFUSE_STATE(0x40), SYS_PLL_CFG00(0x54), SYS_PLL_CFG01(0x58), SYS_PLL_CFG02(0x5C)
 *     When SYS_LOCK register is configed to lock register, the registers below will be locked and can not be modified.
 *     SYS_REMAP(0x00), SYS_PLL_CFG1(0x1C), SYS_CLK(0x20), SYS_EFUSE_CK0(0x34), SYS_EFUSE_CK1(0x38), SYS_EFUSE_CTRL(0x3C), SYS_EFUSE_TRIM(0x50), 
 *     SYS_PLL_CFG00(0x54), SYS_PLL_CFG01(0x58), SYS_PLL_CFG02(0x5C), 
 */
typedef struct
{
    __IO uint32_t SYS_REMAP;            /*!< offset 0x00, system remap mode register        */
    __IO uint32_t SYS_SPI;              /*!< offset 0x04, external spi config register, @ref SYS_SPI_TypeDef        */
    __IO uint32_t SYS_SRAM;             /*!< offset 0x08, sram interface config register, @ref SYS_SRAM_TypeDef     */
    __IO uint32_t SYS_SDRAM;            /*!< offset 0x0C, sdram interface config register, @ref SYS_SDRAM_TypeDef   */
         uint32_t RSVD;
    __IO uint32_t SYS_LOCK;             /*!< offset 0x14, system register lock ctrl register */
         uint32_t RSVD1;
    __IO uint32_t SYS_PLL_CFG1;         /*!< offset 0x1C, system pll config register1, @ref SYS_PLL_CFG1_TypeDef    */
    __IO uint32_t SYS_CLK;              /*!< offset 0x20, system clk config register, @ref SYS_CLK_TypeDef          */
    __IO uint32_t SYS_CODEC_CTRL;       /*!< offset 0x24, system codec ctrl register, @ref SYS_CODEC_TypeDef        */
    __IO uint32_t SYS_CODEC_ADC0;       /*!< offset 0x28, system codec adc register0, @ref SYS_CODEC_ADC0_TypeDef   */
    __IO uint32_t SYS_CODEC_ADC1;       /*!< offset 0x2C, system codec adc register1, @ref SYS_CODEC_ADC1_TypeDef   */
    __IO uint32_t SYS_CODEC_DAC;        /*!< offset 0x30, system codec dac register, @ref SYS_CODEC_DAC_TypeDef     */
    __O  uint32_t SYS_EFUSE_CK0;        /*!< offset 0x34, system eFuse check register0 */
    __O  uint32_t SYS_EFUSE_CK1;        /*!< offset 0x38, system eFuse check register1 */
    __IO uint32_t SYS_EFUSE_CTRL;       /*!< offset 0x3C, system eFuse ctrl register */
    __I  uint32_t SYS_EFUSE_STATE;      /*!< offset 0x40, system eFuse state register */
    __IO uint32_t SYS_USB_CFG0;         /*!< offset 0x44, system usb config reigster0 */
    __IO uint32_t SYS_USB_CFG1;         /*!< offset 0x48, system usb config register1 (for test) */
    __I  uint32_t SYS_RST_SRC;          /*!< offset 0x4C, system reset source register */
    __IO uint32_t SYS_EFUSE_TRIM;       /*!< offset 0x50, system eFuse trim register (for test) */
    __IO uint32_t SYS_PLL_CFG00;        /*!< offset 0x54, system pll0 config register0, @ref SYS_PLL_CFG00_TypeDef */
    __IO uint32_t SYS_PLL_CFG01;        /*!< offset 0x58, system pll0 config register1, @ref SYS_PLL_CFG01_TypeDef */
    __IO uint32_t SYS_PLL_CFG02;        /*!< offset 0x5C, system pll0 config register2, @ref SYS_PLL_CFG02_TypeDef */
} SYS_TypeDef;
/* ================================================================================ */
/* ================              Watch Dog Timer(WDT)              ================ */
/* ================================================================================ */
/**
 * @brief  WDT registers structure definition
 */
typedef struct {
    __IO uint32_t WDOGLOAD;             /*!< offset 0x00, watchdog load register                    */
    __I  uint32_t WDOGVALUE;            /*!< offset 0x04, watchdog value register                   */
    __IO uint32_t WDOGCONTROL;          /*!< offset 0x08, watchdog control reigster                 */
    __O  uint32_t WDOGINTCLR;           /*!< offset 0x0C, watchdog clear interrupt register         */
    __I  uint32_t WDOGRIS;              /*!< offset 0x10, watchdog raw interrupt status register    */
    __I  uint32_t WDOGMIS;              /*!< offset 0x14, watchdog interrupt status registers       */
} WDT_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup DMA
 * @brief  2 groups (16 channels in all) Direct Memory Access (DMA) modlue
 * @{
 */
/**
 * @defgroup DMA_Register_Types DMA Register Types
 * @brief  DMA register information
 * @{
 */
/* ================================================================================ */
/* ================          Direct Memery Access (DMA)            ================ */
/* ================================================================================ */
/**
 * @brief  DMA link list description bit field definition
 */
typedef struct
{
    uint32_t            srcAddr;
    uint32_t            dstAddr;
    uint32_t            nextLLDAddr;
    union {
        struct {
            uint32_t    :16;
            uint32_t    dstSel:1;   /*!< [16],      the same as CCR[1],     */
            uint32_t    srcSe1:1;   /*!< [17],      the same as CCR[2],     */
            uint32_t    dstCtrl:2;  /*!< [19:18],   the same as CCR[4:3],   */
            uint32_t    srcCtrl:2;  /*!< [21:20],   the same as CCR[6:5],   */
            uint32_t    dstWidth:3; /*!< [24:22],   the same as CCR[10:8],  */
            uint32_t    srcWidth:3; /*!< [27:25],   the same as CCR[13:11], */
            uint32_t    tcMsk:1;    /*!< [28],      the same as CCR[31],    */
            uint32_t    fifoTh:3;   /*!< [31:29],   the same as CCR[26:24], */
        }b;
        uint32_t w;
    }ctrl;
    uint32_t            size;
} DMA_LLD_TypeDef;
/**
 * @brief  DMA channel control register bit field definition, 'CCR' in @ref DMA_CH_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t chEn:1;                /*!< [0],       0, channel disable, 1, channel enable */
        uint32_t dstSel:1;              /*!< [1],       must set 0 */
        uint32_t srcSel:1;              /*!< [2],       must set 0 */
        uint32_t dstCtrl:2;             /*!< [4:3],     dst addr: 0, inc, 1, dec, 2, fixed */
        uint32_t srcCtrl:2;             /*!< [6:5],     src addr: 0, inc, 1, dec, 2, fixed */
        uint32_t mode:1;                /*!< [7],       mode: 0, normal, 1, hardware handshake mode (uart/spi(1)/spi(4)/sdc) */
        uint32_t dstWidth:3;            /*!< [10:8],    dst transfer width: 0, 8bit, 1, 16bit, 2, 32bit */
        uint32_t srcWidth:3;            /*!< [13:11],   src transfer width: 0, 8bit, 1, 16bit, 2, 32bit */
        uint32_t :1;
        uint32_t abort:1;               /*!< [15],      write 1 to abort transaction */
        uint32_t srcSize:3;             /*!< [18:16],   src burst size, 0~7 is 1/4/8/16/32/64/128/256 */
        uint32_t prot:3;                /*!< [21:19],   default 0 */
        uint32_t :2;
        uint32_t fifoTh:3;              /*!< [26:24],   dma fifo threshold, 0~4 is 1/2/4/8/16 */
        uint32_t :4;
        uint32_t tcMsk:1;               /*!< [31],      0, tc will be set when terminal count happens, 1, will not set */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} DMA_CH_CCR_TypeDef;
/**
 * @brief  DMA channel config register bit field definition, 'CCFG' in @ref DMA_CH_TypeDef
 */
typedef union
{
    struct
    {
        uint32_t intTcMsk:1;            /*!< [0],       terminal interrupt, 1, mask(default), 0, no mask */
        uint32_t intErrMsk:1;           /*!< [1],       error interrupt, 1, mask(default), 0, no mask */
        uint32_t intAbtMsk:1;           /*!< [2],       abort interrupt, 1, mask(default), 0, no mask */
        uint32_t srcRequest:4;          /*!< [6:3],     src request select, 0~15 */
        uint32_t srcHardwareEn:1;       /*!< [7],       src hardware handshake enable, 0, disable, 1, enable */
        uint32_t busy:1;                /*!< [8],       dma channel busy */
        uint32_t dstRequest:4;          /*!< [12:9],    dst request select, 0~15 */
        uint32_t dstHardwareEn:1;       /*!< [13],      dst hardware handshake enable, 0, disable, 1, enable */
        uint32_t :2;
        uint32_t llpCnt:4;              /*!< [19:16],   RO, chain transfer counter */
    }b;                                 /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} DMA_CH_CCFG_TypeDef;
/**
 * @brief  DMA registers structure definition
 */
typedef struct
{
    __I  uint32_t ISR;                  /*!< Offset: 0x00 (R/ ) Interrupt Status Register */
    __I  uint32_t TCISR;                /*!< Offset: 0x04 (R/ ) Terminal Count Interrupt Status Register */
    __IO uint32_t TCICR;                /*!< Offset: 0x08 (R/W) Terminal Count Interrupt Clear Register */
    __I  uint32_t EAISR;                /*!< Offset: 0x0C (R/ ) Error/Abort Interrupt Status Register */
    __IO uint32_t EAICR;                /*!< Offset: 0x10 (R/W) Error/Abort Interrupt Clear Register */
    __I  uint32_t TCSR;                 /*!< Offset: 0x14 (R/ ) Terminal Count Status Register */
    __I  uint32_t EASR;                 /*!< Offset: 0x18 (R/ ) Error/Abort Status Register */
    __I  uint32_t CHEN;                 /*!< Offset: 0x1C (R/ ) Channel Enable Status Register */
    __I  uint32_t CHBUSY;               /*!< Offset: 0x20 (R/ ) Channel Busy Status Register */
    __IO uint32_t CR;                   /*!< Offset: 0x24 (R/W) Configuration Register */
    __IO uint32_t SYNC;                 /*!< Offset: 0x28 (R/W) Sync Register */
} DMA_CTRL_TypeDef;
/**
 * @brief  DMA channel registers structure definition
 */
typedef struct
{
    __IO uint32_t CCR;                  /*!< Offset: 0x00 (R/W) Channel Ctrl Register, @ref DMA_CH_CCR_TypeDef              */
    __IO uint32_t CCFG;                 /*!< Offset: 0x04 (R/W) Channel Configuration Register, @ref DMA_CH_CCFG_TypeDef    */
    __IO uint32_t CSRC;                 /*!< Offset: 0x08 (R/W) Channel Source Address Register                             */
    __IO uint32_t CDST;                 /*!< Offset: 0x0C (R/W) Channel Destination Address Register                        */
    __IO uint32_t CLLP;                 /*!< Offset: 0x10 (R/W) Channel Link List Pointer Register                          */
    __IO uint32_t CTSR;                 /*!< Offset: 0x14 (R/W) Channel Transfer Size Register                              */
         uint32_t NOUSE[2];
} DMA_CH_TypeDef;
/**
 * @brief  DMA select source config registers structure definition
 */
typedef struct
{
    __IO uint32_t SSCFG[16];            /*!< Offset: 0x00 (R/W) Select Source Configuration. */
} DMA_SEL_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup PWM
 * @brief 16 channels Pulse Width Modulation (PWM) module, it include several working @ref PWM_Mode
 * @{
 */
/* ================================================================================ */
/* ================          Pulse Width Modulation (PWM)          ================ */
/* ================================================================================ */
/**
 * @defgroup PWM_Register_Types PWM Register Types
 * @brief  PWM register information
 * @{
 */
/**
 * @brief  PWM channel control register bit field definition, 'CCR' in @ref PWM_CHR_TypeDef
 */
typedef struct
{
    uint32_t preDiv:16;                 /*!< offset 0x00, bit15~bit00, [0x0001, 0xFFFF], div from pclk */
    uint32_t cntValue:16;               /*!< offset 0x00, bit31~bit16, [0x0001, 0xFFFF], count value, */
    uint32_t cnt2Low:16;                /*!< offset 0x04, bit15~bit00, count change to output low */
    uint32_t cnt2High:16;               /*!< offset 0x04, bit31~bit16, count change to output high */
    uint32_t stepStart:16;              /*!< offset 0x08, bit15~bit00, step start */
    uint32_t stepEnd:16;                /*!< offset 0x08, bit31~bit16, step end */
    uint32_t mode:3;                    /*!< offset 0x0C, bit02~bit00, pwm mode, [0, 4] */
    uint32_t :5;
    uint32_t changeCnt:8;               /*!< offset 0x0C, bit15~bit07, pwm change count */
    uint32_t :16;
} PWM_CCR_TypeDef;
/**
 * @brief  PWM channel registers structure definition
 */
typedef struct
{
    __IO uint32_t CCR[4];               /*!< offset 0x00~0x0C, channel ctrl register, @ref PWM_CCR_TypeDef                                          */
    __I  uint32_t CCVR;                 /*!< offset 0x10, channel count value register, bit15~bit00 is fall edge cnt, bit31~bit16 is rise edge cnt  */
    __IO uint32_t CSR;                  /*!< offset 0x14, channel step register                                                                     */
         uint32_t RSVD[2];              /*!< offset 0x18-0x1C, reserved                                                                             */
} PWM_CHR_TypeDef;
/**
 * @brief  PWM registers structure definition
 */
typedef struct
{
    __IO uint32_t CR;                   /*!< offset 0x200, bit15~bit00, pwm start, bit31~bit16, pwm output inverse                                  */
    __IO uint32_t ICR;                  /*!< offset 0x204, bit15~bit00, pwm interrupt enable, bit31~bit16, pwm interrupt flag, write 0 to clear     */
} PWM_TypeDef;
/**
 * @}
 */

/**
 * @defgroup PWM_Exported_Constants PWM Exported Constants
 * @{
 */
/**
 * @defgroup PWM_REG_POS_CR PWM Register Position of CR
 * @brief  PWM channel start and output inverse position in CR of @ref PWM_TypeDef
 * @{
 */
#define PWM_CR_START_POS                (0U)
#define PWM_CR_OUTINV_POS               (16U)
/**
 * @}
 */
/**
 * @defgroup PWM_REG_POS_ICR PWM Register Position of ICR
 * @brief  PWM channel interrupt enable and interrupt flag position in ICR of @ref PWM_TypeDef
 * @{
 */
#define PWM_ICR_INTEN_POS               (0U)
#define PWM_ICR_INTF_POS                (16U)
/**
 * @}
 */
/**
 * @defgroup PWM_REG_POS_CCVR PWM Register Position of CCVR
 * @brief  PWM channel falling edge count and rising edge count position in CCVR of @ref PWM_CHR_TypeDef
 * @{
 */
#define PWM_CNT_FALL_POS                (0U)
#define PWM_CNT_RISE_POS                (16U)
/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup SPI_FLASH
 * @brief Serial Peripheral Interface (SPI) flash access controller module
 * @{
 */
/* ================================================================================ */
/* ==============               QSPI Flash Controller                ============== */
/* ================================================================================ */
/**
 * @defgroup SPI_FLASH_Register_Types SPI Flash Register Types
 * @brief  SPI flash controller register information
 * @{
 */
/**
 * @brief  SPI flash controller command registers bit field definition, 'CMDR' in @ref SPIFC_TypeDef
 */
typedef union {
    struct {
        uint32_t address;               /*!< CMD0,          flash operation address */
        uint32_t addrLen:3;             /*!< CMD1[2:0],     address byte number */
        uint32_t :13;
        uint32_t dumCycle:8;            /*!< CMD1[23:16],   second dummy state cycle */
        uint32_t insLen:2;              /*!< CMD1[25:24],   instruction code byte number,<br/>0/1/2, 0~2 byte */
        uint32_t :2;
        uint32_t modeBitsInsert:1;      /*!< CMD1[28],      <br/>0, mode bits have no insertion,<br/>1, mode bits have insertion, the value is set at bit[23:16] of CMDR[3](Offset=0x0C) */
        uint32_t :3;
        uint32_t dataCounter;           /*!< CMD2,          Read/Write data counter */
        uint32_t cmdSeparateDisable:1;  /*!< CMD3[0],       <br/>0, commands can be separated into small operations in XIP mode,<br/>1, disable */
        uint32_t operationMode:3;       /*!< CMD3[3:1],     <br/>0, read,<br/>1, write,<br/>2, auto polling FLASH WIP status,<br/>3, erase */
        uint32_t DTRMode:1;             /*!< CMD3[4],       <br/>0, disable,<br/>1, enable double transfer rate, only valid in read data operation */
        uint32_t spiMode:3;             /*!< CMD3[7:5],     <br/>mode, 0~6<br/>0, serial(1-1-1)<br/>1, Dual(1-1-2)<br/>2, Quad(1-1-4)<br/>3, DualIO(1-2-2)<br/>4, QuadIO(1-4-4)<br/>5, Ins2Bit(2-2-2)<br/>6, QPI(4-4-4)*/
        uint32_t startCs:2;             /*!< CMD3[9:8],     cs0~cs3,<br/>0~3, only support 0 in A0 */
        uint32_t :6;
        uint32_t modeBitValue:8;        /*!< CMD3[23:16],   mode bits value M7-0 */
        uint32_t insCode:8;             /*!< CMD3[31:24],   instruction code */
    } b;                                /*!< bit field access */
    uint32_t w[4];                      /*!< 32bit word access */
} SPIFC_CMDR_TypeDef;
/**
 * @brief  SPI flash controller command register bit field definition, 'CR' in @ref SPIFC_TypeDef
 */
typedef union {
    struct {
        uint32_t clkDiv:2;              /*!< [1:0](R/W),    spi clk divider, divider by 2/4/6/8, 0~3 */
        uint32_t :2;
        uint32_t clkMode:1;             /*!< [4](R/W),      spi clk mode, sck_out will be low/high at IDEL state, 0~1 */
        uint32_t :2;
        uint32_t xipIdle:1;             /*!< [7](R/ ),      <br/>0, xip port is busy,<br/>1 xip port is idle */
        uint32_t :8;
        uint32_t wipLoc:3;              /*!< [18:16](R/W),  busy bit of SPI status in hardware read status, BIT0-BIT7, 0~7 */
    } b;                                /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} SPIF_CR_TypeDef;
/**
 * @brief  SPI flash controller command register bit field definition, 'XIPCMD' in @ref SPIFC_TypeDef
 */
typedef union {
    struct {
        uint32_t xipDumyCycle:8;        /*!< [7:0],         dummy cycle for xip port, 0-255 */
        uint32_t xipOpMode:3;           /*!< [10:8],        serial(1-1-1)/Dual(1-1-2)/Quad(1-1-4)/DualIO(1-2-2)/QuadIO(1-4-4)/Ins2Bit(2-2-2)/QPI(4-4-4) mode, 0~6 */
        uint32_t xipAddrLen:1;          /*!< [11],          <br/>0, 3byte address,<br/>1, 4byte address */
        uint32_t xipInsCode:8;          /*!< [19:12],       instruction code for xip port */
        uint32_t xipModeBit:8;          /*!< [27:20],       mode bits in Quad/Dual I/O mode for xip port */
        uint32_t xipModeBitInser:1;     /*!< [28],          <br/>0, mode bits have no insertion,<br/>1, mode bits have insertion */
        uint32_t xipInsLen:2;           /*!< [30:29],       <br/>0, no instruction code,<br/>1, 1-byte instruction code,<br/>2, 2-byte instruction code */
    } b;                                /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} SPIF_XIPCMD_TypeDef;
/**
 * @brief  SPI flash controller registers structure definition
 */
typedef struct
{
    __IO uint32_t CMDR[4];              /*!< Offset: 0x00 (R/W) CMD Register0 ~ CMD Register3, @ref SPIFC_CMDR_TypeDef  */
    __IO uint32_t CR;                   /*!< Offset: 0x10 (R/W) Control Register, @ref SPIF_CR_TypeDef                  */
    __IO uint32_t ACTR;                 /*!< Offset: 0x14 (R/W) AC Timing Register                                      */
    __I  uint32_t SR;                   /*!< Offset: 0x18 (R/ ) State Register                                          */
         uint32_t RESERVED0;
    __IO uint32_t CR2;                  /*!< Offset: 0x20 (R/W) Control Register 2                                      */
    __IO uint32_t ISR;                  /*!< Offset: 0x24 (R/W) Interrupt Status Register                               */
         uint32_t RESERVED1[2];
    __IO uint32_t XIPCMD;               /*!< Offset: 0x30 (R/W) XIP Command Word, @ref SPIF_XIPCMD_TypeDef              */
    __IO uint32_t XIPCR;                /*!< Offset: 0x34 (R/W) XIP Control Register                                    */
    __IO uint32_t RESERVED2[2];
    __IO uint32_t RSTTIME1;             /*!< Offset: 0x40 (R/W) RESET Timing 1                                          */
    __IO uint32_t RSTTIME2;             /*!< Offset: 0x44 (R/W) RESET Timing 2                                          */
    __IO uint32_t RSTCTRL;              /*!< Offset: 0x48 (R/W) RESET Control                                           */
    __IO uint32_t CRANGE;               /*!< Offset: 0x4C (R/W) Cacheable address, unit is 8 bytes.                     */
} SPIFC_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup UART
 * @brief 4 group Universal Asynchronous Receiver Transmitter (UART) module
 * @{
 */
/* ================================================================================ */
/* ============== Universal Asynchronous Receiver Transmitter (UART) ============== */
/* ================================================================================ */
/**
 * @defgroup UART_Register_Types UART Register Types
 * @brief  UART register information
 * @{
 */
/**
 * @brief  UART control register bit field definition, 'CR' in @ref UART_TypeDef
 */
typedef union {
    struct {
        uint32_t bitNum:2;              /*!< [1:0],     data bit number config, 0~3 means 5/6/7/8bit */
        uint32_t stopNum:1;             /*!< [2],       stop bit config,<br/>0, 1bit stop,<br/>1, 2bit stop */
        uint32_t parityStyle:3;         /*!< [5:3],     parity style,<br/>0~3, no parity,<br/>4, odd parity,<br/>5, even parity,<br/>6, force '1',<br/>7, force '0' */
        uint32_t dmaMode:2;             /*!< [7:6],     dma mode config,<br/>0, dms unused,<br/>1, dma for tx,<br/>2, dma for rx,<br/>3, dma for tx/rx */
        uint32_t rxFifoRst:1;           /*!< [8],       write '1' to reset and clear rx fifo */
        uint32_t txFifoRst:1;           /*!< [9],       write '1' to reset and clear tx fifo */
        uint32_t clkEn:1;               /*!< [10],      uart clk enable ctrl,<br/>0, clk disable,<br/>1, clk enable */
    } b;                                /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} UART_CR_TypeDef;                      /* Read/Write */
/**
 * @brief  UART interrupt control register bit field definition, 'ICR' in @ref UART_TypeDef
 */
typedef union {
    struct {
        uint32_t rxErrEn:1;             /*!< [0],       rx error int enable ctrl,       <br/>0, disable,<br/>1, enable */
        uint32_t rxFullEn:1;            /*!< [1],       rx fifo full int enable ctrl,   <br/>0, disable,<br/>1, enable */
        uint32_t rxTimeOutEn:1;         /*!< [2],       rx timeout int enable ctrl,     <br/>0, disable,<br/>1, enable */
        uint32_t rxTrigEn:1;            /*!< [3],       rx trigger int enable ctrl,     <br/>0, disable,<br/>1, enable */
        uint32_t :1;
        uint32_t txTrigEn:1;            /*!< [5],       tx trigger int enable ctrl,     <br/>0, disable,<br/>1, enable */
        uint32_t :1;
        uint32_t txFullEn:1;            /*!< [7],       tx fifo full int enable ctrl,   <br/>0, disable,<br/>1, enable */
        uint32_t rxErrClr:1;            /*!< [8],       write '1' to clear rx parity and stop error */
        uint32_t :3;
        uint32_t rxTrigNum:2;           /*!< [13:12],   rx trigger number config,<br/>0, rx_num != 0,<br/>1, rx_num >= 4,<br/>2, rx_num >= 8,<br/>3, rx_num == 16(full) */
        uint32_t txTrigNum:2;           /*!< [15:14],   tx trigger number config,<br/>0, tx_num == 0,<br/>1, tx_num <= 4,<br/>2, tx_num <= 8,<br/>3, tx_num != 16(full) */
    } b;                                /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} UART_ICR_TypeDef;                     /* Read/Write */
/**
 * @brief  UART state register bit field definition, 'SR' in @ref UART_TypeDef
 */
typedef union {
    struct {
        uint32_t rxErrParity:1;         /*!< [0],       rx error for parity */
        uint32_t rxErrStop:1;           /*!< [1],       rx error for stop bit */
        uint32_t rxTimeOut:1;           /*!< [2],       rx timeout */
        uint32_t rxTrig:1;              /*!< [3],       rx trigger flag */
        uint32_t rxFifoCnt:5;           /*!< [8:4],     rx fifo data count number */
        uint32_t txTrig:1;              /*!< [9],       tx trigger flag */
        uint32_t txFifoFull:1;          /*!< [10],      tx fifo data full flag */
        uint32_t txFifoCnt:5;           /*!< [15:11],   tx fifo data count number */
    } b;                                /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} UART_SR_TypeDef;                      /* Read */
/**
 * @brief  UART registers structure definition
 */
typedef struct
{
    __IO uint32_t DR;                   /*!< Offset: 0x00 (R/W) Data Register                                   */
    __IO uint32_t CR;                   /*!< Offset: 0x04 (R/W) Ctrl Register, @ref UART_CR_TypeDef             */
    __IO uint32_t ICR;                  /*!< Offset: 0x08 (R/W) Interrupt Ctrl Register, @ref UART_ICR_TypeDef  */
    __I  uint32_t SR;                   /*!< Offset: 0x0C (R/ ) State Register, @ref UART_SR_TypeDef            */
    __IO uint32_t BRCR;                 /*!< Offset: 0x10 (R/W) Baud Rate Config Register                       */
    __O  uint32_t RTOR;                 /*!< Offset: 0x14 ( /W) Rx Time Out Register                            */
} UART_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/* ================================================================================ */
/* ================                   Timer (TIM)                  ================ */
/* ================================================================================ */
/**
 * @addtogroup TIMER
 * @brief 4 simple Timer module
 * @{
 */
/**
 * @defgroup TIMER_Register_Types TIMER Register Types
 * @brief  Timer register information
 * @{
 */
/**
 * @brief  Timer control register bit field definition, 'CR' in @ref TIM_TypeDef
 */
typedef union {
    struct {
        uint32_t enable:1;              /*!< [0],       <br/>1:enable,<br/>0:disable */
        uint32_t extClkEn:1;            /*!< [1],       external clk enable */
        uint32_t extClkSel:1;           /*!< [2],       external clk select */
        uint32_t intEn:1;               /*!< [3],       interrupt enable */
    } b;                                /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
} TIM_CR_TypeDef;
/**
 * @brief  Timer registers structure definition
 */
typedef struct
{
    __IO uint32_t CR;                   /*!< Offset: 0x00 (R/W) Ctrl Register, [3:0], @ref TIM_CR_TypeDef                   */
    __IO uint32_t CNTR;                 /*!< Offset: 0x04 (R/W) Count Register, [31:0]                                      */
    __IO uint32_t RLDR;                 /*!< Offset: 0x08 (R/W) Reload Register, [31:0]                                     */
    __IO uint32_t ISR;                  /*!< Offset: 0x0C (R/W) Interrupt State Register, write '1' to clear int flag, [0]  */
} TIM_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup DUAL_TIMER
 * @brief 1 dual-timer module
 * @{
 */
/**
 * @defgroup DUAL_TIMER_Register_Types DUAL TIMER Register Types
 * @brief  Dual-Timer register information
 * @{
 */
/**
 * @brief Dual-Timer control register bit field definition, 'CR' in @ref DTIM_TypeDef
 */
typedef union {
    struct {
        uint32_t oneShotCount:1;        /*!< [0],       <br/>1, One-shot mode,<br/>0, Wrapping mode(default) */
        uint32_t timerSize:1;           /*!< [1],       <br/>1, 32-bit counter,<br/>0, 16-bit counter(default) */
        uint32_t timerPre:2;            /*!< [3:2],     <br/>0, div1(default),<br/>1, div16,<br/>2, div256 */
        uint32_t :1;
        uint32_t intEn:1;               /*!< [5],       <br/>1:timer int enable,<br/>0:timer int disable(default) */
        uint32_t timerMode:1;           /*!< [6],       <br/>1:periodic mode,<br/>0:free-running mode(default) */
        uint32_t enable:1;              /*!< [7],       <br/>1:timer enable,<br/>0:timer disable,default */
    } b;                                /*!< bit field access */
    uint32_t w;                         /*!< 32bit word access */
}DTIM_CR_TypeDef;                       /* Read/Write */
/**
 * @brief  Dual-Timer registers structure definition
 */
typedef struct
{
    __IO uint32_t LR;                   /*!< Offset: 0x00 (R/W) Load Register, [31:0]                               */
    __I  uint32_t CNTR;                 /*!< Offset: 0x04 (R/ ) Count Register, [31:0]                              */
    __IO uint32_t CR;                   /*!< Offset: 0x08 (R/W) Ctrl Register, [7:0], @ref DTIM_CR_TypeDef          */
    __O  uint32_t ICR;                  /*!< Offset: 0x0C ( /W) Interrupt Clear Register, write any value to clear  */
    __I  uint32_t RISR;                 /*!< Offset: 0x10 (R/ ) Raw Interrupt Status Register, [0]                  */
    __I  uint32_t ISR;                  /*!< Offset: 0x14 (R/ ) Interrupt Status Register, [0]                      */
    __IO uint32_t BLR;                  /*!< Offset: 0x18 (R/W) Background Load Register, [31:0]                    */
} DTIM_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup SPI
 * @brief 3 groups Serial Peripheral Interface (SPI) module
 * @{
 */
/**
 * @defgroup SPI_Register_Types SPI Register Types
 * @brief  Spi register information
 * @{
 */
/* ================================================================================ */
/* ==============               Standard SPI Flash Controller        ============== */
/* ================================================================================ */
/**
 * @brief SPI state register bit field definition, 'SR' in @ref SPI_TypeDef
 */
typedef union {
    struct {
        uint32_t rxFifoCnt:5;           /*!< [4:0],     data count number by byte in rx fifo */
        uint32_t rxFifoFull:1;          /*!< [5],       '1' means rx fifo is full of data */
        uint32_t rxFifoEmpty:1;         /*!< [6],       '1' means rx fifo is empty */
        uint32_t rxTimeOut:1;           /*!< [7],       '1' means rx fifo is not empty time out */
        uint32_t txFifoCnt:5;           /*!< [12:8],    data count number by byte in tx fifo */
        uint32_t txFifoEmpty:1;         /*!< [13],      '1' means tx fifo is empty */
        uint32_t txFifoFull:1;          /*!< [14],      '1' means tx fifo is full of data */
        uint32_t :3;                    /*!< [17:15],   Reserved */
        uint32_t csState:1;             /*!< [18],      cs selected (1) state in slave mode, cs level state in master mode */
    } b;
    uint32_t w;
} SPI_SR_TypeDef;
/**
 * @brief SPI fifo control register bit field definition, 'FCR' in @ref SPI_TypeDef
 */
typedef union {
    struct {
        uint32_t rxFifoTrigCfg:2;       /*!< [1:0],     rx fifo trig config,<br/>0, rxNum != 0,<br/>1, rxNum >= 4,<br/>2, rxNum >= 8,<br/>3, rxNum == 16(full) */
        uint32_t txFifoTrigCfg:2;       /*!< [3:2],     tx fifo trig config,<br/>0, txNum == 0,<br/>1, txNum <= 4,<br/>2, txNum <= 8,<br/>3, txNum <= 16 */
        uint32_t rxTimeOutEn:1;         /*!< [4],       rx time out interrupt enable ctrl,<br/>0, disable,<br/>1, enable */
        uint32_t rxFifoTrigEn:1;        /*!< [5],       rx fifo trig interrupt enable ctrl,<br/>0, disable,<br/>1, enable */
        uint32_t txFifoTrigEn:1;        /*!< [6],       tx fifo trig interrupt enable ctrl,<br/>0, disable,<br/>1, enable */
        uint32_t rwMode:2;              /*!< [8:7],     transfer read/write mode config,<br/>0, no read/write,<br/>1, only write,<br/>2, only read,<br/>3, both read and write */
        uint32_t fifoRst:1;             /*!< [9],       write '1' to clear all fifo */
    } b;
    uint32_t w;
} SPI_FCR_TypeDef;
/**
 * @brief SPI control register bit field definition, 'CR' in @ref SPI_TypeDef
 */
typedef union {
    struct {
        uint32_t mode:2;                /*!< [1:0],     CPHA:CPOL config */
        uint32_t bitDir:1;              /*!< [2],       <br/>0, transfer data from high bit to low bit,<br/>1, transfer data from low bit to high bit */
        uint32_t slaveEn:1;             /*!< [3],       <br/>0, spi master mode,<br/>1, spi slave mode */
        uint32_t enable:1;              /*!< [4],       <br/>0, disable,<br/>1, enable */
        uint32_t csSel:1;               /*!< [5],       only can write 0 */
        uint32_t fifoAccWidth:2;        /*!< [7:6],     fifo access data width,<br/>0, 8bit,<br/>1, 16bit,<br/>2, 32bit */
        uint32_t byteDir:1;             /*!< [8],       <br/>0, transfer data from high byte to low byte,<br/>1, transfer data from low byte to high byte */
        uint32_t :1;                    /*!< [9],       Reserved*/
        uint32_t transWidth:2;          /*!< [11:10],   transfer data width,<br/>0, 8bit,<br/>1, 16bit,<br/>2, 32bit */
        uint32_t csCtrl:1;              /*!< [12],      cs output level ctrl in master mode,<br/>0, output low level,<br/>1, output high level */
        uint32_t :1;                    /*!< [13],      Reserved */
        uint32_t dmaMode:2;             /*!< [15:14],   dma mode config,<br/>0, dms unused,<br/>1, dma for rx,<br/>2, dma for tx,<br/>3, dma for tx/rx */
        uint32_t clkDiv:16;             /*!< [31:16],   spi clock div from PCLK config. min value is 8 */
    } b;
    uint32_t w;
} SPI_CR_TypeDef;
/**
 * @brief  SPI registers structure definition
 */
typedef struct
{
    __IO uint32_t DR;                   /*!< Offset: 0x00 (R/W) Data Register                               */
    __IO uint32_t CR;                   /*!< Offset: 0x04 (R/W) Ctrl Register, @ref SPI_CR_TypeDef          */
    __IO uint32_t TSR;                  /*!< Offset: 0x08 (R/W) Transfer Size Register                      */
    __IO uint32_t RTOR;                 /*!< Offset: 0x0C (R/W) Rx Time Out Register                        */
    __IO uint32_t FCR;                  /*!< Offset: 0x10 (R/W) Fifo Ctrl Register, @ref SPI_FCR_TypeDef    */
    __I  uint32_t SR;                   /*!< Offset: 0x14 (R/ ) State Register, @ref SPI_SR_TypeDef         */
} SPI_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup KSCAN
 * @brief Keyboard scan, function key scan, and LED driver module
 * @{
 */
/**
 * @defgroup KSCAN_Register_Types KSCAN Register Types
 * @brief  Keyboard scan and led driver register information
 * @{
 */
/* ================================================================================ */
/* ==============               Keyboard Scan and LED                ============== */
/* ================================================================================ */
/**
 * @brief KSCAN state register bit field definition, 'STATE' of 'CR' in @ref KSCAN_TypeDef
 */
typedef union {
    struct {
        uint32_t keyNum:7;
        uint32_t krq:1;
    } b;
    uint32_t w;
} KSCAN_STATE_TypeDef;
/**
 * @brief KSCAN state register bit field definition, 'KEY' of 'DR' in @ref KSCAN_TypeDef
 */
typedef union {
    struct {
        uint32_t time:12;
        uint32_t busy:1;
        uint32_t on:1;
        uint32_t srq:1;
    } b;
    uint32_t w;
} KSCAN_KEY_STATE_TypeDef;
/**
 * @brief  KSCAN registers structure definition
 */
typedef struct
{
    union {
        __O uint32_t ADDR;              /*!< Offset: 0x00 (W/ ) Write ram address */
        __I uint32_t STATE;             /*!< Offset: 0x00 ( /R) Read keyboard state and key number, @ref KSCAN_STATE_TypeDef */
    }CR;
    union {
        __O uint32_t LED;               /*!< Offset: 0x04 (W/ ) Write led ctrl value, write '0' to light on the LED */
        __I uint32_t KEY;               /*!< Offset: 0x04 ( /R) Read keyboard state or key scan value, @ref KSCAN_KEY_STATE_TypeDef */
    }DR;
    __IO uint32_t STR;                  /*!< Offset: 0x08 (W/R) Scan Time Register */
    __IO uint32_t CFGR;                 /*!< Offset: 0x0C (W/R) Config Register */
} KSCAN_TypeDef;
/**
 * @}
 */
/**
 * @}
 */

/* --------  End of section using anonymous unions and disabling warnings  -------- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

/**
 * @addtogroup A0_CHIP
 * @{
 */
/**
 * @addtogroup Memory_Map Memory Map
 * @{
 */
/* ================================================================================ */
/* ================              CPU memory map                    ================ */
/* AHB0: 0x28000000-0x3F000000                                                      */
/* AHB1: 0x40000000-0x4F000000                                                      */
/* AHB2: 0x50000000-0x5000FFFF                                                      */
/* ================================================================================ */
#define A0_INT_FLASH_BASE           (0x00000000UL)                  //base of internal flash address
#define A0_SRAM0_BASE               (0x10000000UL)                  //base of sram0 address (128KByte, for ins/data)
#define A0_SRAM1_BASE               (0x20000000UL)                  //base of sram1 address (32KByte, dma1 and dsp can not access)
#define A0_SRAM2_BASE               (0x28000000UL)                  //base of sram2 address (32KByte, for DSP iram/pram if DSP is used)

/* ================================================================================ */
/* ================              Peripheral memory map             ================ */
/* ================================================================================ */
//APB
#define A0_TIMER_BASE               (0x40000000UL)                  //0x0000/0x1000/0x2000/0x3000/0x4000, T0/T1/T2/T3/DualT
#define A0_UART_BASE                (0x40005000UL)                  //base of uart
#define A0_I2C_BASE                 (0x40009000UL)                  //base of i2c, include master and slave
#define A0_SPI_BASE                 (0x4000B000UL)                  //base of spi, 1 line
#define A0_SYS_BASE                 (0x4000E000UL)                  //base of system
#define A0_KEY_BASE                 (0x4000F000UL)                  //base of keyscan
#define A0_WDT_BASE                 (0x40010000UL)                  //base of watch dog timer
#define A0_GPIO_BASE                (0x40011000UL)                  //base of gpio/extint/adc
#define A0_PWM_BASE                 (0x40012000UL)                  //base of pwm
//AHB
#define A0_DMA0_BASE                (0x40020000UL)                  //base of dma0
#define A0_DMA1_BASE                (0x40030000UL)                  //base of dma1
#define A0_USB_BASE                 (0x40040000UL)                  //base of usb
#define A0_SPIFC_BASE               (0x40050000UL)                  //base of spi flash controller
#define A0_SDC_BASE                 (0x40060000UL)                  //base of sd controller
#define A0_I2S_BASE                 (0x50000000UL)                  //base of i2s
#define A0_DSP_BASE                 (0x28000000UL)                  //base of dsp
#define A0_EXT_RAM_BASE             (0x29000000UL)                  //base of external sram/sdram address
#define A0_EXT_FLASH_BASE           (0x30000000UL)                  //base of external flash address
//WDT
#define WDT_CTRL_BASE               (A0_WDT_BASE    + 0x00000000UL)     //Watchdog ctrl
#define WDT_LOCK_BASE               (A0_WDT_BASE    + 0x00000C00UL)     //Watchdog lock
//SYS_CTRL
#define SYS_CTRL_BASE               (A0_SYS_BASE    + 0x00000000UL)     //System ctrl
//GPIO & EXTI
#define GPIO0_BASE                  (A0_GPIO_BASE   + 0x00000000UL)     //Gpio port 0
#define GPIO1_BASE                  (A0_GPIO_BASE   + 0x0000003CUL)     //Gpio port 1
#define EXTI0_BASE                  (A0_GPIO_BASE   + 0x00000100UL)     //Ext int0
#define EXTI1_BASE                  (A0_GPIO_BASE   + 0x0000010CUL)     //Ext int1
//ADC
#define ADC_BASE                    (A0_GPIO_BASE   + 0x00000200UL)     //adc
//TIMER
#define TIM0_BASE                   (A0_TIMER_BASE  + 0x00000000UL)     //normal timer 0
#define TIM1_BASE                   (A0_TIMER_BASE  + 0x00001000UL)     //normal timer 1
#define TIM2_BASE                   (A0_TIMER_BASE  + 0x00002000UL)     //normal timer 2
#define TIM3_BASE                   (A0_TIMER_BASE  + 0x00003000UL)     //normal timer 3
#define TIM_BASE(n)                 (A0_TIMER_BASE  + 0x1000UL*(n))
//DTIMER
#define DTIM0_BASE                  (A0_TIMER_BASE  + 0x00004000UL)     //dual timer 0
#define DTIM1_BASE                  (A0_TIMER_BASE  + 0x00004020UL)     //dual timer 1
//UART
#define UART0_BASE                  (A0_UART_BASE   + 0x00000000UL)     //uart 0
#define UART1_BASE                  (A0_UART_BASE   + 0x00001000UL)     //uart 1
#define UART2_BASE                  (A0_UART_BASE   + 0x00002000UL)     //uart 2
#define UART3_BASE                  (A0_UART_BASE   + 0x00003000UL)     //uart 3
#define UART_BASE(n)                (A0_UART_BASE   + 0x1000UL*(n))
//I2C Master & I2C Slave
#define I2C_MASTER_BASE             (A0_I2C_BASE    + 0x00000000UL)
#define I2C_SLAVE_BASE              (A0_I2C_BASE    + 0x00001000UL)
//SPI
#define SPI1_BASE                   (A0_SPI_BASE    + 0x00000000UL)     //spi 1
#define SPI2_BASE                   (A0_SPI_BASE    + 0x00001000UL)     //spi 2
#define SPI3_BASE                   (A0_SPI_BASE    + 0x00002000UL)     //spi 3
#define SPI_BASE(n)                 (A0_SPI_BASE    + 0x1000UL*((n)-1))
//PWM
#define PWMCH00_BASE                (A0_PWM_BASE    + 0x00000000UL)
#define PWMCH01_BASE                (A0_PWM_BASE    + 0x00000020UL)
#define PWMCH02_BASE                (A0_PWM_BASE    + 0x00000040UL)
#define PWMCH03_BASE                (A0_PWM_BASE    + 0x00000060UL)
#define PWMCH04_BASE                (A0_PWM_BASE    + 0x00000080UL)
#define PWMCH05_BASE                (A0_PWM_BASE    + 0x000000A0UL)
#define PWMCH06_BASE                (A0_PWM_BASE    + 0x000000C0UL)
#define PWMCH07_BASE                (A0_PWM_BASE    + 0x000000E0UL)
#define PWMCH08_BASE                (A0_PWM_BASE    + 0x00000100UL)
#define PWMCH09_BASE                (A0_PWM_BASE    + 0x00000120UL)
#define PWMCH10_BASE                (A0_PWM_BASE    + 0x00000140UL)
#define PWMCH11_BASE                (A0_PWM_BASE    + 0x00000160UL)
#define PWMCH12_BASE                (A0_PWM_BASE    + 0x00000180UL)
#define PWMCH13_BASE                (A0_PWM_BASE    + 0x000001A0UL)
#define PWMCH14_BASE                (A0_PWM_BASE    + 0x000001C0UL)
#define PWMCH15_BASE                (A0_PWM_BASE    + 0x000001E0UL)
#define PWM_BASE                    (A0_PWM_BASE    + 0x00000200UL)
//I2S
#define I2S_CTRL_BASE               (A0_I2S_BASE    + 0x00000000UL)
#define I2S_FIFO_BASE               (A0_I2S_BASE    + 0x0000000CUL)
#define I2S_FIFO0_BASE              (I2S_FIFO_BASE  + 0x08 * 0)
#define I2S_FIFO1_BASE              (I2S_FIFO_BASE  + 0x08 * 1)
#define I2S_FIFO2_BASE              (I2S_FIFO_BASE  + 0x08 * 2)
#define I2S_FIFO3_BASE              (I2S_FIFO_BASE  + 0x08 * 3)
#define I2S_FIFO4_BASE              (I2S_FIFO_BASE  + 0x08 * 4)
#define I2S_FIFO5_BASE              (I2S_FIFO_BASE  + 0x08 * 5)
#define I2S_FIFO6_BASE              (I2S_FIFO_BASE  + 0x08 * 6)
#define I2S_FIFOIN_BASE             (I2S_FIFO5_BASE)                    //for codec adc or dmic0
#define I2S_FIFOIN1_BASE            (I2S_FIFO4_BASE)                    //for codec dmic1
#define I2S_FIFOOUT_BASE            (I2S_FIFO6_BASE)                    //for codec dac
//SPI Flash Controller
#define SPIFC_DR_BASE               (A0_SPIFC_BASE  + 0x100)
//DMA Group0
#define DMA0_CTRL_BASE              (A0_DMA0_BASE   + 0x00000000UL)
#define DMA0_CH_BASE                (A0_DMA0_BASE   + 0x00000100UL)
#define DMA0_SELECT_BASE            (A0_DMA0_BASE   + 0x00000200UL)
#define DMA0_CH0_BASE               (DMA0_CH_BASE   + 0x000UL)
#define DMA0_CH1_BASE               (DMA0_CH_BASE   + 0x020UL)
#define DMA0_CH2_BASE               (DMA0_CH_BASE   + 0x040UL)
#define DMA0_CH3_BASE               (DMA0_CH_BASE   + 0x060UL)
#define DMA0_CH4_BASE               (DMA0_CH_BASE   + 0x080UL)
#define DMA0_CH5_BASE               (DMA0_CH_BASE   + 0x0A0UL)
#define DMA0_CH6_BASE               (DMA0_CH_BASE   + 0x0C0UL)
#define DMA0_CH7_BASE               (DMA0_CH_BASE   + 0x0E0UL)
//DMA Group1
#define DMA1_CTRL_BASE              (A0_DMA1_BASE   + 0x00000000UL)
#define DMA1_CH_BASE                (A0_DMA1_BASE   + 0x00000100UL)
#define DMA1_SELECT_BASE            (A0_DMA1_BASE   + 0x00000200UL)
#define DMA1_CH0_BASE               (DMA1_CH_BASE   + 0x000UL)
#define DMA1_CH1_BASE               (DMA1_CH_BASE   + 0x020UL)
#define DMA1_CH2_BASE               (DMA1_CH_BASE   + 0x040UL)
#define DMA1_CH3_BASE               (DMA1_CH_BASE   + 0x060UL)
#define DMA1_CH4_BASE               (DMA1_CH_BASE   + 0x080UL)
#define DMA1_CH5_BASE               (DMA1_CH_BASE   + 0x0A0UL)
#define DMA1_CH6_BASE               (DMA1_CH_BASE   + 0x0C0UL)
#define DMA1_CH7_BASE               (DMA1_CH_BASE   + 0x0E0UL)
// n is dma ch 0~15
#define DMA_CTRL_BASE(n)            ((A0_DMA0_BASE + (((n) & 0x8) << 13)))
#define DMA_SELECT_BASE(n)          ((A0_DMA0_BASE + (((n) & 0x8) << 13)) + 0x200UL)
#define DMA_CH_BASE(n)              ((A0_DMA0_BASE + (((n) & 0x8) << 13)) + 0x100UL + 0x20UL * ((n) & 0x7))
//KSCAN
#define KSCAN_BASE                  (A0_KEY_BASE    + 0x00000000UL)
/**
 * @}
 */
/**
 * @addtogroup Peripheral_Declaration Peripheral Declaration
 * @{
 */
/* ================================================================================ */
/* ================             Peripheral Declaration             ================ */
/* ================================================================================ */
#define ADC                         ((ADC_TypeDef       *)ADC_BASE          )
#define EXTI0                       ((EXTINT_TypeDef    *)EXTI0_BASE        )
#define EXTI1                       ((EXTINT_TypeDef    *)EXTI1_BASE        )
#define GPIO0                       ((GPIO_TypeDef      *)GPIO0_BASE        )
#define GPIO1                       ((GPIO_TypeDef      *)GPIO1_BASE        )
#define I2CMaster                   ((I2C_M_TypeDef     *)I2C_MASTER_BASE   )
#define I2CSlave                    ((I2C_S_TypeDef     *)I2C_SLAVE_BASE    )
#define I2S_CTRL                    ((I2S_TypeDef       *)I2S_CTRL_BASE     )
#define I2S_FIFO0                   ((I2S_FifoTypeDef   *)I2S_FIFO0_BASE    )
#define I2S_FIFO1                   ((I2S_FifoTypeDef   *)I2S_FIFO1_BASE    )
#define I2S_FIFO2                   ((I2S_FifoTypeDef   *)I2S_FIFO2_BASE    )
#define I2S_FIFO3                   ((I2S_FifoTypeDef   *)I2S_FIFO3_BASE    )
#define I2S_FIFO4                   ((I2S_FifoTypeDef   *)I2S_FIFO4_BASE    )
#define I2S_FIFO5                   ((I2S_FifoTypeDef   *)I2S_FIFO5_BASE    )
#define I2S_FIFO6                   ((I2S_FifoTypeDef   *)I2S_FIFO6_BASE    )
#define I2S_FIFOIN                  ((I2S_FifoTypeDef   *)I2S_FIFOIN_BASE   )
#define I2S_FIFOIN1                 ((I2S_FifoTypeDef   *)I2S_FIFOIN1_BASE  )
#define I2S_FIFOOUT                 ((I2S_FifoTypeDef   *)I2S_FIFOOUT_BASE  )
#define SYS_CTRL                    ((SYS_TypeDef       *)SYS_CTRL_BASE     )
#define WDT                         ((WDT_TypeDef       *)WDT_CTRL_BASE     )

#define PWM00                       ((PWM_CHR_TypeDef   *)PWMCH00_BASE      )
#define PWM01                       ((PWM_CHR_TypeDef   *)PWMCH01_BASE      )
#define PWM02                       ((PWM_CHR_TypeDef   *)PWMCH02_BASE      )
#define PWM03                       ((PWM_CHR_TypeDef   *)PWMCH03_BASE      )
#define PWM04                       ((PWM_CHR_TypeDef   *)PWMCH04_BASE      )
#define PWM05                       ((PWM_CHR_TypeDef   *)PWMCH05_BASE      )
#define PWM06                       ((PWM_CHR_TypeDef   *)PWMCH06_BASE      )
#define PWM07                       ((PWM_CHR_TypeDef   *)PWMCH07_BASE      )
#define PWM08                       ((PWM_CHR_TypeDef   *)PWMCH08_BASE      )
#define PWM09                       ((PWM_CHR_TypeDef   *)PWMCH09_BASE      )
#define PWM10                       ((PWM_CHR_TypeDef   *)PWMCH10_BASE      )
#define PWM11                       ((PWM_CHR_TypeDef   *)PWMCH11_BASE      )
#define PWM12                       ((PWM_CHR_TypeDef   *)PWMCH12_BASE      )
#define PWM13                       ((PWM_CHR_TypeDef   *)PWMCH13_BASE      )
#define PWM14                       ((PWM_CHR_TypeDef   *)PWMCH14_BASE      )
#define PWM15                       ((PWM_CHR_TypeDef   *)PWMCH15_BASE      )
#define PWM                         ((PWM_TypeDef       *)PWM_BASE          )
#define PWMCH(n)                    (PWM00 + (n))

#define SPIFC                       ((SPIFC_TypeDef     *)A0_SPIFC_BASE     )

#define UART0                       ((UART_TypeDef      *)UART0_BASE        )
#define UART1                       ((UART_TypeDef      *)UART1_BASE        )
#define UART2                       ((UART_TypeDef      *)UART2_BASE        )
#define UART3                       ((UART_TypeDef      *)UART3_BASE        )
#define UARTN(n)                    ((UART_TypeDef      *)UART_BASE(n)      )

#define TIM0                        ((TIM_TypeDef       *)TIM0_BASE         )
#define TIM1                        ((TIM_TypeDef       *)TIM1_BASE         )
#define TIM2                        ((TIM_TypeDef       *)TIM2_BASE         )
#define TIM3                        ((TIM_TypeDef       *)TIM3_BASE         )
#define TIMN(n)                     ((TIM_TypeDef       *)TIM_BASE(n)       )
#define DTIM0                       ((DTIM_TypeDef      *)DTIM0_BASE        )
#define DTIM1                       ((DTIM_TypeDef      *)DTIM1_BASE        )

#define SPI1                        ((SPI_TypeDef       *)SPI1_BASE         )
#define SPI2                        ((SPI_TypeDef       *)SPI2_BASE         )
#define SPI3                        ((SPI_TypeDef       *)SPI3_BASE         )
#define SPIN(n)                     ((SPI_TypeDef       *)SPI_BASE(n)       )

#define DMA0CTRL                    ((DMA_CTRL_TypeDef  *)DMA0_CTRL_BASE    )
#define DMA0SELECT                  ((DMA_SEL_TypeDef   *)DMA0_SELECT_BASE  )
#define DMA0CH0                     ((DMA_CH_TypeDef    *)DMA0_CH0_BASE     )
#define DMA0CH1                     ((DMA_CH_TypeDef    *)DMA0_CH1_BASE     )
#define DMA0CH2                     ((DMA_CH_TypeDef    *)DMA0_CH2_BASE     )
#define DMA0CH3                     ((DMA_CH_TypeDef    *)DMA0_CH3_BASE     )
#define DMA0CH4                     ((DMA_CH_TypeDef    *)DMA0_CH4_BASE     )
#define DMA0CH5                     ((DMA_CH_TypeDef    *)DMA0_CH5_BASE     )
#define DMA0CH6                     ((DMA_CH_TypeDef    *)DMA0_CH6_BASE     )
#define DMA0CH7                     ((DMA_CH_TypeDef    *)DMA0_CH7_BASE     )

#define DMA1CTRL                    ((DMA_CTRL_TypeDef  *)DMA1_CTRL_BASE    )
#define DMA1SELECT                  ((DMA_SEL_TypeDef   *)DMA1_SELECT_BASE  )
#define DMA1CH0                     ((DMA_CH_TypeDef    *)DMA1_CH0_BASE     )
#define DMA1CH1                     ((DMA_CH_TypeDef    *)DMA1_CH1_BASE     )
#define DMA1CH2                     ((DMA_CH_TypeDef    *)DMA1_CH2_BASE     )
#define DMA1CH3                     ((DMA_CH_TypeDef    *)DMA1_CH3_BASE     )
#define DMA1CH4                     ((DMA_CH_TypeDef    *)DMA1_CH4_BASE     )
#define DMA1CH5                     ((DMA_CH_TypeDef    *)DMA1_CH5_BASE     )
#define DMA1CH6                     ((DMA_CH_TypeDef    *)DMA1_CH6_BASE     )
#define DMA1CH7                     ((DMA_CH_TypeDef    *)DMA1_CH7_BASE     )

#define DMACTRL(n)                  ((DMA_CTRL_TypeDef  *)DMA_CTRL_BASE(n)  )
#define DMASELECT(n)                ((DMA_SEL_TypeDef   *)DMA_SELECT_BASE(n))
#define DMACH(n)                    ((DMA_CH_TypeDef    *)DMA_CH_BASE(n)    )

#define KSCAN                       ((KSCAN_TypeDef     *)KSCAN_BASE        )

/**
 * @}
 */
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
