/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_i2s.h
 * @brief   Header file of I2S HAL module.
 * @version V1.0.4
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_I2S_H
#define __A0_HAL_I2S_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup I2S
 * @brief Integrated-Interchip Sound (I2S) module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup I2S_Exported_Types I2S Exported Types
 * @{
 */
/**
 * @brief  I2S Init Configuration Structure definition
 */
typedef struct
{
    I2S_CR_TypeDef      cr;
    I2S_CCR0_TypeDef    ccr0;
    I2S_CCR1_TypeDef    ccr1;
} I2S_InitTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup I2S_Exported_Constants I2S Exported Constants
 * @{
 */
/**
 * @defgroup I2S_Sample_Rate I2S Sample Rate Config
 * @brief I2S sample rate config value, use for 'lrclkDiv' in @ref I2S_CR_TypeDef
 * @{
 */
#define I2S_FS_12K                          (0)     /*!< i2s sample rate is 12k or 11.025k */
#define I2S_FS_16K                          (1)     /*!< i2s sample rate is 16k or 14.7k */
#define I2S_FS_24K                          (2)     /*!< i2s sample rate is 24k or 22.05k */
#define I2S_FS_32K                          (3)     /*!< i2s sample rate is 32k or 29.4k */
#define I2S_FS_48K                          (4)     /*!< i2s sample rate is 48k or 44.1k */
#define I2S_FS_96K                          (5)     /*!< i2s sample rate is 96k or 88.2k */
#define I2S_FS_192K                         (6)     /*!< i2s sample rate is 192k or 176.4k */
#define I2S_FS_384K                         (7)     /*!< i2s sample rate is 384k or 352.8k */
/**
 * @}
 */

/**
 * @defgroup I2S_MCLK_Div I2S MCLK Div
 * @brief I2S mclk div from the clk to i2s module, use for 'mclkDiv' in @ref I2S_CR_TypeDef
 * @{
 */
#define I2S_MCLK_DIV2                       (1)     /*!< i2s mclk = i2s_clk / 2 */
#define I2S_MCLK_DIV1                       (2)     /*!< i2s mclk = i2s_clk / 1 */
#define I2S_MCLK_DIV4                       (3)     /*!< i2s mclk = i2s clk / 4 */
/**
 * @}
 */

/**
 * @defgroup I2S_Clk_Group I2S Clk Group
 * @brief I2S clk group config value, use for 'd0Clk', 'd1Clk', 'd2Clk', 'd3Clk', 'd4Clk', 'd5Clk', 'd6Clk', in @ref I2S_CCR1_TypeDef
 * @{
 */
#define I2S_CLK_DEFAULT                     (0)     /*!< i2s default clk group */
#define I2S_CLK_S1                          (1)     /*!< i2s slave 1 clk group */
#define I2S_CLK_S2                          (2)     /*!< i2s slave 2 clk group */
#define I2S_CLK_NONE                        (3)     /*!< i2s clk none */
/**
 * @}
 */

/**
 * @defgroup I2S_Mode I2S Mode
 * @brief I2S mode config value, use for 'mode' in @ref I2S_CR_TypeDef
 * @{
 */
#define I2S_MODE_LJ                         (0)     /*!< i2s mode is left justified */
#define I2S_MODE_I2S                        (1)     /*!< i2s mode is i2s */
#define I2S_MODE_DSP1                       (2)     /*!< i2s mode is dsp mode in rise edge */
#define I2S_MODE_DSP2                       (3)     /*!< i2s mode is dsp mode in fall edge */
/**
 * @}
 */

/**
 * @defgroup I2S_Data_Pin_Out_Enable I2S Data Pin Out Enable
 * @brief I2S data pin output enable config value, use for 'dOutEn' in @ref I2S_CCR0_TypeDef
 * @{
 */
#define I2S_D0_OUT_EN                       (BIT0)  /*!< i2s d0 output enable */
#define I2S_D1_OUT_EN                       (BIT1)  /*!< i2s d1 output enable */
#define I2S_D2_OUT_EN                       (BIT2)  /*!< i2s d2 output enable */
#define I2S_D3_OUT_EN                       (BIT3)  /*!< i2s d2 output enable */
#define I2S_D4_OUT_EN                       (BIT4)  /*!< i2s d4 output enable */
#define I2S_D5_OUT_EN                       (BIT5)  /*!< i2s d5 output enable */
#define I2S_D6_OUT_EN                       (BIT6)  /*!< i2s d6 output enable */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup I2S_Exported_Macros I2S Exported Macros
 * @{
 */
/**
 * @brief  write i2s ctrl register, write register 'CR' in @ref I2S_TypeDef
 * @param  CTRL    value wrtie to the register
 * @return None
 */
#define HAL_I2S_CTRL_SET(CTRL)                                          (I2S_CTRL->CR = (CTRL))

/**
 * @brief  read i2s ctrl register, read register 'CR' in @ref I2S_TypeDef
 * @return value read from the ctrl register
 */
#define HAL_I2S_CTRL_GET()                                              (I2S_CTRL->CR)

/**
 * @brief  write i2s channel ctrl 0 register, write register 'CCR[0]' in @ref I2S_TypeDef
 * @param  CTRL    value wrtie to the register
 * @return None
 */
#define HAL_I2S_CH_CTRL0_SET(CTRL)                                      (I2S_CTRL->CCR[0] = (CTRL))

/**
 * @brief  read i2s channel ctrl 0 register, read register 'CCR[0]' in @ref I2S_TypeDef
 * @return value read from the channel ctrl 0 register
 */
#define HAL_I2S_CH_CTRL0_GET()                                          (I2S_CTRL->CCR[0])

/**
 * @brief  write i2s channel ctrl 1 register, write register 'CCR[1]' in @ref I2S_TypeDef
 * @param  CTRL    value wrtie to the register
 * @return None
 */
#define HAL_I2S_CH_CTRL1_SET(CTRL)                                      (I2S_CTRL->CCR[1] = (CTRL))

/**
 * @brief  read i2s channel ctrl 1 register, read register 'CCR[1]' in @ref I2S_TypeDef
 * @return value read from the channel ctrl 1 register
 */
#define HAL_I2S_CH_CTRL1_GET()                                          (I2S_CTRL->CCR[1])

/**
 * @brief  write data to i2s fifo left, write register 'FR[FIFOIDX][0]' in @ref I2S_TypeDef
 * @param  FIFOIDX     0~6, fifo0~fifo6
 * @param  DATA        pcm data value write to fifo
 * @return None
 */
#define HAL_I2S_FIFO_WR_L(FIFOIDX, DATA)                                (I2S_CTRL->FR[FIFOIDX][0] = (DATA))

/**
 * @brief  read data to i2s fifo left, read register 'FR[FIFOIDX][0]' in @ref I2S_TypeDef
 * @param  FIFOIDX     0~6, fifo0~fifo6
 * @return pcm data value read from fifo
 */
#define HAL_I2S_FIFO_RD_L(FIFOIDX)                                      (I2S_CTRL->FR[FIFOIDX][0])

/**
 * @brief  write data to i2s fifo right, write register 'FR[FIFOIDX][1]' in @ref I2S_TypeDef
 * @param  FIFOIDX     0~6, fifo0~fifo6
 * @param  DATA        pcm data value write to fifo
 * @return None
 */
#define HAL_I2S_FIFO_WR_R(FIFOIDX, DATA)                                (I2S_CTRL->FR[FIFOIDX][1] = (DATA))

/**
 * @brief  read data to i2s fifo right, read register 'FR[FIFOIDX][1]' in @ref I2S_TypeDef
 * @param  FIFOIDX     0~6, fifo0~fifo6
 * @return pcm data value read from fifo
 */
#define HAL_I2S_FIFO_RD_R(FIFOIDX)                                      (I2S_CTRL->FR[FIFOIDX][1])
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup I2S_Exported_Functions I2S Exported Functions
 * @{
 */

/**
 * @brief  config the i2s clk source, if i2s is selected from pll2 out, pll2 clk will be configed, and enable pll2, otherwise, pll2 will be powerdown
 * @param  adj     pll2 fine adjust, -1, N of pll2 -1 (pll2 out freq decrease), +1, N of pll2 +1 (pll2 out freq increase)
 * @return NONE
 */
void HalI2sClkFineAdjust(int32_t adj);

/**
 * @brief  config the i2s clk source, if i2s is selected from pll2 out, pll2 clk will be configed, and enable pll2, otherwise, pll2 will be powerdown
 * @param  i2sClkSrc       i2s clk source select[0,1], 0 is from pll2 out, 1 is from SystemClk/4
 * @param  pll2ClkSrc      pll2 clk source select[0,1], 0, is from pll0 out, 1 is from pll1 out
 * @param  pll2ClkMode     pll2 clk mode select[0,1], 0, pll2 out is 45.1584M (for 44.1K sample rate), 1, pll2 out is 49.152M (for 48k sample rate)
 * @return NONE
 */
void HalI2sClkSelect(uint8_t i2sClkSrc, uint8_t pll2ClkSrc, uint8_t pll2ClkMode);

/**
 * @brief  i2s module initial process function
 * @param  pInit    pointer to structure of @ref I2S_InitTypeDef
 * @return None
 */
void HalI2sInit(I2S_InitTypeDef *pInit);

/* for code compatible */
#define hal_i2s_clk_fine_adjust                 HalI2sClkFineAdjust
#define hal_i2s_clk_select                      HalI2sClkSelect
#define hal_i2s_init                            HalI2sInit
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
