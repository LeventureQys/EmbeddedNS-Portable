/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_sys_ctrl.h
 * @brief   SYS_CTRL HAL module. Include external spi controller, external sram
 *          and sdram interface, pll, clk, codec, efuse, usb phy, wdt, etc.
 * @version V1.0.5
 * @date    2023/12/01
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_SYS_CTRL_H
#define __A0_HAL_SYS_CTRL_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup SYS_CTRL
 * @brief System Control module, it include clock, pll, sdram, sram, usb, wdt, codec, efuse, exteranl spi flash access, and so on.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup Sys_Ctrl_Exported_Constants Sys Ctrl Exported Constants
 * @{
 */

/**
 * @defgroup Sys_Spi_Ext_Addr_Width Spi Ext Addr Width
 * @brief SPI external flash address bytes config value, for 'addrWidth' in @ref SYS_SPI_TypeDef
 * @{
 */
#define SPI_EXT_ADDR_3BYTES                 (0x0)   /*!< spi external flash address is 3 bytes  */
#define SPI_EXT_ADDR_4BYTES                 (0x1)   /*!< spi external flash address is 4 bytes  */
/**
 * @}
 */

/**
 * @defgroup Sys_Codec_Mclk_Mode Codec Mclk Mode
 * @brief Codec mclk mode config value, for 'mclkMode' in @ref SYS_CODEC_TypeDef
 * @details
 *  | MCLK                   | FS     | LRCLK           |
 *  | ---------------------- | ------ | --------------- |
 *  | 12.288MHz / 11.2896MHz | 256FS  | 48KHz/44.1KHz   |
 *  | 12.288MHz / 11.2896MHz | 128FS  | 96KHz/88.2KHz   |
 *  | 12MHz                  | 272FS  | 44.1KHz         |
 *  | 12MHz                  | 136FS  | 88.2KHz         |
 *  | 12.288MHz / 11.2896MHz | 1024FS | 12KHz/11.025KHz |
 *  | 12.288MHz / 11.2896MHz | 512FS  | 24KHz/22.05KHz  |
 *  | 12MHz                  | 1088FS | 11.025KHz       |
 *  | 12MHz                  | 544FS  | 22.05KHz        |
 *  | 12.288MHz / 11.2896MHz | 384FS  | 32KHz           |
 *  | 12MHz                  | 250FS  | 48KHz           |
 *  | 12MHz                  | 500FS  | 24KHz           |
 *  | 12MHz                  | 1500FS | 8KHz            |
 *  | 12.288MHz              | 1536FS | 8KHz            |
 *  | 12.288MHz              | 768FS  | 16KHz           |
 *  | 12MHz                  | 750FS  | 16KHz           |
 *  | 12MHz                  | 375FS  | 32KHz           |
 * @{
 */
#define CODEC_256FS                         (0xF)   /*!< MCLK/LRCLK = 256, Default              */
#define CODEC_128FS                         (0xE)   /*!< MCLK/LRCLK = 128                       */
#define CODEC_272FS                         (0xD)   /*!< MCLK/LRCLK = 272                       */
#define CODEC_136FS                         (0xC)   /*!< MCLK/LRCLK = 136                       */
#define CODEC_1024FS                        (0xB)   /*!< MCLK/LRCLK = 1024                      */
#define CODEC_512FS                         (0xA)   /*!< MCLK/LRCLK = 512                       */
#define CODEC_1088FS                        (0x9)   /*!< MCLK/LRCLK = 1088                      */
#define CODEC_544FS                         (0x8)   /*!< MCLK/LRCLK = 544                       */
#define CODEC_384FS                         (0x7)   /*!< MCLK/LRCLK = 384                       */
#define CODEC_250FS                         (0x6)   /*!< MCLK/LRCLK = 250                       */
#define CODEC_500FS                         (0x5)   /*!< MCLK/LRCLK = 500                       */
#define CODEC_1500FS                        (0x4)   /*!< MCLK/LRCLK = 1500                      */
#define CODEC_1536FS                        (0x3)   /*!< MCLK/LRCLK = 1536                      */
#define CODEC_768FS                         (0x2)   /*!< MCLK/LRCLK = 768                       */
#define CODEC_750FS                         (0x1)   /*!< MCLK/LRCLK = 750                       */
#define CODEC_375FS                         (0x0)   /*!< MCLK/LRCLK = 375                       */
/**
 * @}
 */

/**
 * @defgroup Sys_Codec_In_Sel Codec Adc Input Select
 * @brief Codec input selection config value, for 'lInSel' and 'rInSel' in @ref SYS_CODEC_ADC0_TypeDef and @ref SYS_CODEC_ADC1_TypeDef
 * @{
 */
#define CODEC_SEL_ADC0                      (0x1)   /*!< Codec input select ADC0 pin            */
#define CODEC_SEL_ADC1                      (0x2)   /*!< Codec input select ADC1 pin            */
#define CODEC_SEL_LINEIN                    (0x3)   /*!< Codec input select LINEIN pin, Default */
/**
 * @}
 */

/**
 * @defgroup Sys_Codec_De_Emphasis Codec De Emphasis Filter
 * @brief Codec de-emphasis filter config value, for 'deEmphasis' in @ref SYS_CODEC_TypeDef
 * @{
 */
#define CODEC_DEEMPH_OFF                    (0x0)   /*!< Codec de-emphasis filter: off          */
#define CODEC_DEEMPH_32K_384FS              (0x1)   /*!< Codec de-emphasis filter: 32K, 384FS   */
#define CODEC_DEEMPH_44K_256FS              (0x2)   /*!< Codec de-emphasis filter: 44.1K, 256FS */
#define CODEC_DEEMPH_48K_256FS              (0x3)   /*!< Codec de-emphasis filter: 48K, 256FS   */
/**
 * @}
 */

/**
 * @defgroup Sys_Codec_Audio_Mode Codec Audio Interface Mode
 * @brief Codec audio interface mode config value, for 'audioIfMode' in @ref SYS_CODEC_TypeDef
 * @{
 */
#define CODEC_PARALLEL_MODE                 (0x0)   /*!< Codec data is parallel mode            */
#define CODEC_I2S_MODE                      (0x1)   /*!< Codec data is i2s mode, Default        */
#define CODEC_RIGHT_JUSTIFIED               (0x2)   /*!< Codec data is right-justified mode     */
#define CODEC_LEFT_JUSTIFIED                (0x3)   /*!< Codec data is left-justified mode      */
/**
 * @}
 */

/**
 * @defgroup Sys_Pll_Clock_Range Pll Clock Range
 * @brief PLL output frequency range config value, for 'pll1Range' in @ref SYS_PLL_CFG1_TypeDef
 * @{
 */
#define PLL_RANGE_20M_40M                   (0x0)   /*!< PLL out range is 20MHz to 40MHz        */
#define PLL_RANGE_40M_80M                   (0x1)   /*!< PLL out range is 40MHz to 80MHz        */
#define PLL_RANGE_80M_160M                  (0x2)   /*!< PLL out range is 80MHz to 160MHz       */
#define PLL_RANGE_160M_300M                 (0x3)   /*!< PLL out range is 160MHz to 300MHz      */
/**
 * @}
 */

/**
 * @defgroup Sys_Efuse_Check_Result Efuse Check Result
 * @brief Efuse check result
 * @{
 */
#define EFUSE_CHECK_ERROR                   (0x0)   /*!< eFuse ck0 or ck1 check error           */
/**
 * @}
 */

/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup Sys_Ctrl_Exported_Macros Sys Ctrl Exported Macros
 * @{
 */

/**
 * @brief  system ctrl sram rd/wr and cs cycle config set, write regsister 'SYS_SRAM' in @ref SYS_TypeDef by CRW and CCS input
 * @param  CRW     HCLK Cycle of RD and WR signal width, (CRW) >= 1
 * @param  CCS     HCLK Cycle of CS signal width, (CCS) >= (CRW) + 3
 * @return None
 */
#define HAL_SYS_SRAM_CYCLE_CFG_SET(CRW, CCS)                            \
do {                                                                    \
    uint32_t ws = (CRW) - 1;                                            \
    uint32_t fws = (CCS) - (CRW) - 3;                                   \
    SYS_CTRL->SYS_SRAM = ((ws << 8) | fws);                             \
} while (0U)

/**
 * @brief  system ctrl sram ctrl register write, write register 'SYS_SRAM' in @ref SYS_TypeDef by VAL input
 * @param  VAL     Sram rd/wr and cs cycle config value write to the reigster
 * @return None
 */
#define HAL_SYS_SRAM_CTRL_SET(VAL)                                      (SYS_CTRL->SYS_SRAM = (VAL))

/**
 * @brief  system ctrl sdram auto refresh cycle config set and enable sdram, write register 'SYS_SDRAM' in @ref SYS_TypeDef by CAR input
 * @param  CAR     HCLK Cycle of Auto Refresh, (CAR) <= 0x1FFFE
 * @return None
 */
#define HAL_SYS_SDRAM_CYCLE_CFG_EN(CAR)                                 (SYS_CTRL->SYS_SDRAM = (0x0001FFFFUL - ((CAR) >> 1)))

/**
 * @brief  system ctrl sdram enable ctrl, set bit16 of register 'SYS_SDRAM' in @ref SYS_TypeDef
 * @return None
 */
#define HAL_SYS_SDRAM_ENABLE()                                          (SYS_CTRL->SYS_SDRAM |= ( BIT16))

/**
 * @brief  system ctrl sdram disable ctrl, clr bit16 of register 'SYS_SDRAM' in @ref SYS_TypeDef
 * @return None
 */
#define HAL_SYS_SDRAM_DISABLE()                                         (SYS_CTRL->SYS_SDRAM &= (~BIT16))

/**
 * @brief  system ctrl sdram ctrl register write, write register 'SYS_SDRAM' in @ref SYS_TypeDef by VAL input
 * @param  VAL     Sdram auto refresh cycle and enable or disable config value write to the reigster
 * @return None
 */
#define HAL_SYS_SDRAM_CTRL_SET(VAL)                                     (SYS_CTRL->SYS_SDRAM = (VAL))

/**
 * @brief  system ctrl external spi interface config set, write register 'SYS_SPI' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_SPI, @ref SYS_SPI_TypeDef
 * @return None
 */
#define HAL_SYS_EXT_SPI_CFG_SET(CFG)                                    (SYS_CTRL->SYS_SPI = (CFG))

/**
 * @brief  system ctrl external spi interface config get, read register 'SYS_SPI' in @ref SYS_TypeDef
 * @return config value read from SYS_SPI, @ref SYS_SPI_TypeDef
 */
#define HAL_SYS_EXT_SPI_CFG_GET()                                       (SYS_CTRL->SYS_SPI)

/**
 * @brief  config SYS_SPI access external flash, clr bit31 of register 'SYS_SPI' in @ref SYS_TypeDef
 * @return None
 */
#define HAL_SYS_EXT_SPI_CTL_SEL0()                                      (SYS_CTRL->SYS_SPI &= (~BIT31))

/**
 * @brief  config SPIFC access external flash in CS1, set bit31 of register 'SYS_SPI' in @ref SYS_TypeDef
 * @return None
 */
#define HAL_SYS_EXT_SPI_CTL_SEL1()                                      (SYS_CTRL->SYS_SPI |= ( BIT31))

/**
 * @brief  system ctrl registers lock, include registers offset 0x00,0x1C,0x20,0x34,0x38,0x3C,0x50,0x54,0x58,0x5C.
 * @return None
 */
#define HAL_SYS_REG_LOCK()                                              (SYS_CTRL->SYS_LOCK = 0)

/**
 * @brief  system ctrl registers unlock, include registers offset 0x00,0x1C,0x20,0x34,0x38,0x3C,0x50,0x54,0x58,0x5C.
 * @return None
 */
#define HAL_SYS_REG_UNLOCK()                                            (SYS_CTRL->SYS_LOCK = 1)

/**
 * @brief  system ctrl pll0 config register0 set, write register 'SYS_PLL_CFG00' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_PLL_CFG00, @ref SYS_PLL_CFG00_TypeDef
 * @return None
 */
#define HAL_SYS_PLL_REG0_SET(CFG)                                       (SYS_CTRL->SYS_PLL_CFG00 = (CFG))

/**
 * @brief  system ctrl pll0 config register1 set, write register 'SYS_PLL_CFG01' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_PLL_CFG01, @ref SYS_PLL_CFG00_TypeDef
 * @return None
 */
#define HAL_SYS_PLL_REG1_SET(CFG)                                       (SYS_CTRL->SYS_PLL_CFG01 = (CFG))

/**
 * @brief  system ctrl pll0 config register2 set, write register 'SYS_PLL_CFG02' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_PLL_CFG02, @ref SYS_PLL_CFG00_TypeDef
 * @return None
 */
#define HAL_SYS_PLL_REG2_SET(CFG)                                       (SYS_CTRL->SYS_PLL_CFG02 = (CFG))

/**
 * @brief  system ctrl pll0 config register0 get, read register 'SYS_PLL_CFG00' in @ref SYS_TypeDef
 * @return config value read from SYS_PLL_CFG00, @ref SYS_PLL_CFG00_TypeDef
 */
#define HAL_SYS_PLL_REG0_GET()                                          (SYS_CTRL->SYS_PLL_CFG00)

/**
 * @brief  system ctrl pll0 config register1 get, read register 'SYS_PLL_CFG01' in @ref SYS_TypeDef
 * @return config value read from SYS_PLL_CFG01, @ref SYS_PLL_CFG01_TypeDef
 */
#define HAL_SYS_PLL_REG1_GET()                                          (SYS_CTRL->SYS_PLL_CFG01)

/**
 * @brief  system ctrl pll0 config register2 get, read register 'SYS_PLL_CFG02' in @ref SYS_TypeDef
 * @return config value read from SYS_PLL_CFG02, @ref SYS_PLL_CFG02_TypeDef
 */
#define HAL_SYS_PLL_REG2_GET()                                          (SYS_CTRL->SYS_PLL_CFG02)

/**
 * @brief  system ctrl pll1 and pll2 config register set, write register 'SYS_PLL_CFG1' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_PLL_CFG1, @ref SYS_PLL_CFG1_TypeDef
 * @return None
 */
#define HAL_SYS_PLL_CODEC_SET(CFG)                                      (SYS_CTRL->SYS_PLL_CFG1 = (CFG))

/**
 * @brief  system ctrl pll1 and pll2 config register get, read register 'SYS_PLL_CFG1' in @ref SYS_TypeDef
 * @return config value read from SYS_PLL_CFG1, @ref SYS_PLL_CFG1_TypeDef
 */
#define HAL_SYS_PLL_CODEC_GET()                                         (SYS_CTRL->SYS_PLL_CFG1)

/**
 * @brief  system ctrl clk control register set, write register 'SYS_CLK' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_CLK, @ref SYS_CLK_TypeDef
 * @return None
 */
#define HAL_SYS_CLK_SET(CFG)                                            (SYS_CTRL->SYS_CLK = (CFG))

/**
 * @brief  system ctrl clk control register get, read register 'SYS_CLK' in @ref SYS_TypeDef
 * @return config value read from SYS_CLK, @ref SYS_CLK_TypeDef
 */
#define HAL_SYS_CLK_GET()                                               (SYS_CTRL->SYS_CLK)

/**
 * @brief  system ctrl codec control register set, write register 'SYS_CODEC_CTRL' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_CODEC_CTRL, @ref SYS_CODEC_TypeDef
 * @return None
 */
#define HAL_SYS_CODEC_CTRL_SET(CFG)                                     (SYS_CTRL->SYS_CODEC_CTRL = (CFG))

/**
 * @brief  system ctrl codec control register get, read register 'SYS_CODEC_CTRL' in @ref SYS_TypeDef
 * @return config value read from SYS_CODEC_CTRL, @ref SYS_CODEC_TypeDef
 */
#define HAL_SYS_CODEC_CTRL_GET()                                        (SYS_CTRL->SYS_CODEC_CTRL)

/**
 * @brief  system ctrl codec adc control registers set, write register 'SYS_CODEC_ADC0' and 'SYS_CODEC_ADC1' in @ref SYS_TypeDef by CFG0 and CFG1 input
 * @param  CFG0    config value write to SYS_CODEC_ADC0, @ref SYS_CODEC_ADC0_TypeDef
 * @param  CFG1    config value write to SYS_CODEC_ADC1, @ref SYS_CODEC_ADC1_TypeDef
 * @return None
 */
#define HAL_SYS_CODEC_ADC_SET(CFG0, CFG1)                               \
do {                                                                    \
    SYS_CTRL->SYS_CODEC_ADC0 = (CFG0);                                  \
    SYS_CTRL->SYS_CODEC_ADC1 = (CFG1);                                  \
} while(0U)

/**
 * @brief  system ctrl codec adc control register0 get, read register 'SYS_CODEC_ADC0' in @ref SYS_TypeDef
 * @return config value read from SYS_CODEC_ADC0, @ref SYS_CODEC_ADC0_TypeDef
 */
#define HAL_SYS_CODEC_ADC0_GET()                                        (SYS_CTRL->SYS_CODEC_ADC0)

/**
 * @brief  system ctrl codec adc control register1 get, read register 'SYS_CODEC_ADC1' in @ref SYS_TypeDef
 * @return config value read from SYS_CODEC_ADC1, @ref SYS_CODEC_ADC1_TypeDef
 */
#define HAL_SYS_CODEC_ADC1_GET()                                        (SYS_CTRL->SYS_CODEC_ADC1)

/**
 * @brief  system ctrl codec dac control register set, write register 'SYS_CODEC_DAC' in @ref SYS_TypeDef by CFG input
 * @param  CFG     config value write to SYS_CODEC_DAC, @ref SYS_CODEC_DAC_TypeDef
 * @return None
 */
#define HAL_SYS_CODEC_DAC_SET(CFG)                                      (SYS_CTRL->SYS_CODEC_DAC = (CFG))

/**
 * @brief  system ctrl codec dac control register1 get, read register 'SYS_CODEC_DAC' in @ref SYS_TypeDef
 * @return config value read from SYS_CODEC_DAC, @ref SYS_CODEC_DAC_TypeDef
 */
#define HAL_SYS_CODEC_DAC_GET()                                         (SYS_CTRL->SYS_CODEC_DAC)

/**
 * @brief  system ctrl eFuse check data0 registers write, for user, write register 'SYS_EFUSE_CK0' in @ref SYS_TypeDef by CK input
 * @param  CK      efuse value write to SYS_EFUSE_CK0
 * @return None
 */
#define HAL_SYS_EFUSE_CHECK0_WR(CK)                                     (SYS_CTRL->SYS_EFUSE_CK0 = (CK))

/**
 * @brief  system ctrl eFuse check data1 registers write, for jinaudio, write register 'SYS_EFUSE_CK1' in @ref SYS_TypeDef by CK input
 * @param  CK      efuse value write to SYS_EFUSE_CK1
 * @return None
 */
#define HAL_SYS_EFUSE_CHECK1_WR(CK)                                     (SYS_CTRL->SYS_EFUSE_CK1 = (CK))

/**
 * @brief  system ctrl eFuse data0 check result read, read bit0 of 'SYS_EFUSE_STATE' in @ref SYS_TypeDef
 * @return eFuse check result, @ref Sys_Efuse_Check_Result
 */
#define HAL_SYS_EFUSE_RESULT0_RD()                                      (SYS_CTRL->SYS_EFUSE_STATE & BIT0)

/**
 * @brief  system ctrl eFuse data1 check result read, read bit1 of 'SYS_EFUSE_STATE' in @ref SYS_TypeDef
 * @return eFuse check result, @ref Sys_Efuse_Check_Result
 */
#define HAL_SYS_EFUSE_RESULT1_RD()                                      (SYS_CTRL->SYS_EFUSE_STATE & BIT1)

/**
 * @brief  system ctrl efuse ctrl register write, write register 'SYS_EFUSE_CTRL' in @ref SYS_TypeDef by VAL input
 * @param  VAL     set value write to SYS_EFUSE_CTRL
 * @return None
 */
#define HAL_SYS_EFUSE_CTRL_SET(VAL)                                     (SYS_CTRL->SYS_EFUSE_CTRL = (VAL))

/**
 * @brief  system ctrl eFuse program bit start singal send, set bit2 of register 'SYS_EFUSE_CTRL' in @ref SYS_TypeDef
 * @return None
 */
#define HAL_SYS_EFUSE_PROGRAM_BIT_START()                               (SYS_CTRL->SYS_EFUSE_CTRL |= ( (BIT2)))

/**
 * @brief  system ctrl eFuse program bit stop singal send, clr bit2 of register 'SYS_EFUSE_CTRL' in @ref SYS_TypeDef
 * @return None
 */
#define HAL_SYS_EFUSE_PROGRAM_BIT_STOP()                                (SYS_CTRL->SYS_EFUSE_CTRL &= (~(BIT2)))

/**
 * @brief  system ctrl eFuse program bit number set, write register 'SYS_EFUSE_CTRL' in @ref SYS_TypeDef
 * @param  BIT     bit number to program
 * @return None
 */
#define HAL_SYS_EFUSE_PROGRAM_BIT_SET(BIT)                              (SYS_CTRL->SYS_EFUSE_CTRL = ((BIT) << 8))

/**
 * @brief  system ctrl current reset source check whether it is software reset, read register 'SYS_RST_SRC' in @ref SYS_TypeDef
 * @retval == 0    hardware reset or watchdog reset
 * @retval != 0    software reset
 */
#define HAL_SYS_SOFT_RESET_CHECK()                                      (SYS_CTRL->SYS_RST_SRC)

/**
 * @brief  watchdog registers lock
 * @return None
 */
#define HAL_SYS_WDT_LOCK()                                              (U32REG(WDT_LOCK_BASE) = 0x0)

/**
 * @brief  watchdog registers unlock
 * @return None
 */
#define HAL_SYS_WDT_UNLOCK()                                            (U32REG(WDT_LOCK_BASE) = 0x1ACCE551)

/**
 * @brief  watchdog registers lock state get
 * @retval == 0    watchdog registers is unlocked
 * @retval != 0    watchdog registers is locked
 */
#define HAL_SYS_WDT_LOCK_STATE()                                        (U32REG(WDT_LOCK_BASE))

/**
 * @brief  watchdog load value set. count with PCLK, write register 'WDOGLOAD' in @ref WDT_TypeDef by VAL input
 * @param  VAL     VAL = (WdtTime(s) * PCLK), max time is about 44s while PCLK = 96MHz
 * @return None
 */
#define HAL_SYS_WDT_LOAD_SET(VAL)                                       (WDT->WDOGLOAD = (VAL))

/**
 * @brief  watchdog mask interrupt status get, read register 'WDOGMIS' in @ref WDT_TypeDef
 * @retval == 0    no interrupt occured
 * @retval != 0    interrupt occured
 */
#define HAL_SYS_WDT_MIS_GET()                                           (WDT->WDOGMIS)

/**
 * @brief  watchdog interrupt clear, write any data to register to clear interrupt flag, write '0' to register 'WDOGINTCLR' in @ref WDT_TypeDef
 * @return None
 */
#define HAL_SYS_WDT_INT_CLR()                                           (WDT->WDOGINTCLR = 0)

/**
 * @brief  watchdog NMI interruput enable, user can rewrite NMI_Handler to process, write '1' to register 'WDOGCONTROL' in @ref WDT_TypeDef
 * @return None
 */
#define HAL_SYS_WDT_INT_EN()                                            (WDT->WDOGCONTROL = 0x1)

/**
 * @brief  watchdog NMI interrupt and hardware reset enable, user do not need to rewrite NMI_Handler, write '3' to register 'WDOGCONTROL' in @ref WDT_TypeDef
 * @return None
 */
#define HAL_SYS_WDT_RST_EN()                                            (WDT->WDOGCONTROL = 0x3)

/**
 * @brief  usb device send wakeup signal to host, wait host to send resume cmd, e.g. usb device wakeup by key event
 * @return None
 */
#define HAL_SYS_USB_REMOTE_WAKEUP_SET()                                 (SYS_CTRL->SYS_USB_CFG0 |= ( (BIT5)))

/**
 * @brief  after host send resume to usb device, clear the singal register
 * @return None
 */
#define HAL_SYS_USB_REMOTE_WAKEUP_CLR()                                 (SYS_CTRL->SYS_USB_CFG0 &= (~(BIT5)))

/**
 * @brief  config otg ID to GND, use for host
 * @return None
 */
#define HAL_SYS_USB_ID_CLR()                                            ((SYS_CTRL->SYS_USB_CFG0) = (SYS_CTRL->SYS_USB_CFG0 & (~(BIT4|BIT3))))

/**
 * @brief  config otg ID to VCC, use for device
 * @return None
 */
#define HAL_SYS_USB_ID_SET()                                            ((SYS_CTRL->SYS_USB_CFG0) = (SYS_CTRL->SYS_USB_CFG0 & (~(BIT3)) | (BIT4)))

/**
 * @brief  config usb device vbus disable
 * @return None
 */
#define HAL_SYS_USB_VBUS_DIS()                                          ((SYS_CTRL->SYS_USB_CFG0) = (SYS_CTRL->SYS_USB_CFG0 & (~(BIT1|BIT0))))

/**
 * @brief  config usb device vbus enable
 * @return None
 */
#define HAL_SYS_USB_VBUS_EN()                                           ((SYS_CTRL->SYS_USB_CFG0) = (SYS_CTRL->SYS_USB_CFG0 & (~(BIT0)) | (BIT1)))

/**
 * @brief  config usb device vbus connect to p0.19
 * @return None
 */
#define HAL_SYS_USB_VBUS_IO()                                           ((SYS_CTRL->SYS_USB_CFG0) = (SYS_CTRL->SYS_USB_CFG0 | (BIT0 | BIT1)))

/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup Sys_Ctrl_Exported_Functions Sys Ctrl Exported Functions
 * @{
 */
/**
 * @brief  watchdog initial function
 * @param  cycle   watchdog count cycle
 * @return None
 * @details
 * WatchDog use pclk to count, so input parameter cycle = SystemPclk * time (in seconds)
 * e.g. config watchdog count in 500ms, cycle = SystemPclk * 500 / 1000;
 * Default config watchdog as hardware reset, NMI_Handler is not necessary
 */
void HalSysWdtInit(uint32_t cycle);

/**
 * @brief  watchdog reload count value
 * @param  cycle   watchdog count cycle
 * @return None
 * @details
 * WatchDog use pclk to count, so input parameter cycle = SystemPclk * time (in seconds)
 * e.g. config watchdog count in 500ms, cycle = SystemPclk * 500 / 1000;
 * Default config watchdog as hardware reset, NMI_Handler is not necessary
 */
void HalSysWdtReload(uint32_t cycle);

/**
 * @brief  should be called in NMI handler
 * @return None
 */
void HalSysWdtSoftwareRst(void);

/**
 * @brief  record system cycle by DWT->CYCNT, need to use 5 words memory
 * @param  pRecordMem pointer to 5 words memory
 * @return None
 * @details 
 * memory[5] = {
 *      current start cycle, 
 *      cycle between 2 HalSysCycleRecordStart called, 
 *      cycle between 2 HalSysCycleRecordStart called max recorded, 
 *      cycle between HalSysCycleRecordStart and HalSysCycleRecordEnd called, 
 *      cycle between HalSysCycleRecordStart and HalSysCycleRecordEnd called max recorded }
 */
void HalSysCycleRecordStart(uint32_t *pRecordMem);
void HalSysCycleRecordEnd(uint32_t *pRecordMem);

/**
 * @}
 */

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
