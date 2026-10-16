/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_kbd.c
 * @brief   Keyboard Scan HAL module.
 * @version V1.0.2
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.2, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.1, 2021/10/11,
 *      1. 头文件中的注释部分调整，适应Doxygen
 * V1.0.0, 2021/06/17,
 *      1. 头文件包含寄存器操作宏，并添加注释
 *      2. 添加几个键盘检测相关函数
 */

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup KSCAN
 * @{
 */

#ifdef HAL_KSCAN_MODULE_ENABLED
#include "a0_hal_kbd.h"
#include "a0_hal_def.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @defgroup KSCAN_Exported_Functions KSCAN Exported Functions
 * @{
 */

//键扫描时间配置计算，根据扫描周期时间和扫描周期占比，计算出写入寄存器的数值
//return -1, scan cycle out of range
//return -2, scan duty ratio out of range
//return > 0, time config value need to write to KSCAN->STR
int32_t HalKscanTimeCalc(float_t scanCycleInUs, float_t scanDutyRatio)
{
    uint32_t setV1 = (uint32_t)(scanCycleInUs / ((1000000.0f / (float_t)SystemPclk) * 8.0f * 64.0f) + 0.5f) - 1;
    if (setV1 >> 6)
    {
        return -1;
    }
    uint32_t setV2 = (uint32_t)((float_t)setV1 * scanDutyRatio + 0.5f);
    if (setV2 >> 5)
    {
        return -2;
    }
    return ((setV2 << 6) | setV1);
}

//定时去查询键盘检测结果，返回按键按下的序号和力度值，其中力度值已经按照力度曲线转化成Midi码，力度值非0表示按下，0表示释放
//*pKeyEvent, 指针指向2个16位的memory，用于返回按键事件(keyIdx, keyVel)
//return == 0, key event
//return != 0, no key event
int32_t HalKscanKeyEventCheck(uint16_t *pKeyEvent)
{
    KSCAN_STATE_TypeDef state;
    KSCAN_KEY_STATE_TypeDef keyVel;
    state.w = HAL_KSCAN_KBD_STATE_GET();
    if (state.b.krq != 0)
    {
        HAL_KSCAN_KRQ_CLR();
        uint32_t addr = state.b.keyNum;
        HAL_KSCAN_RAM_ADDR_SET(addr);
        keyVel.w = HAL_KSCAN_KBD_INFO_GET();
        *pKeyEvent++ = (uint16_t)addr;
        if (keyVel.b.on != 0)
        {
            //pressed
            *pKeyEvent++ = keyVel.b.time;
        }
        else
        {
            //released
            *pKeyEvent++ = 0;
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * @}
 */
#endif
/**
 * @}
 */
/**
 * @}
 */
