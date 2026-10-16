/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_uart.c
 * @brief   UART HAL module.
 * @version V1.0.5
 * @date    2022/11/15
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.5, 2022/11/15, by huangzheng
 *      1. 头文件中HAL_UART_TIMEOUT_SET接收超时寄存器写入宏更新
 * V1.0.4, 2022/04/07, by huangzheng
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.3, 2021/09/22, by huangzheng
 *      1. 将A0.h中的Constants移到该头文件中
 * V1.0.2, 2021/09/15, by huangzheng
 *      1. 头文件中的注释部分调整，适应Doxygen
 *      2. UART_InitTypeDef中修改，串口直接改成index的序号，0~3，方便处理
 * V1.0.1, 2021/05/17, by huangzheng
 *      1. 头文件中添加寄存器操作宏及注释
 *      2. 修改部分HAL函数接口
 * V1.0.0, 2021/02/02, by Tao
 *      1. 初版，UART控制器HAL层函数接口，不配置IO复用，需要上层应用控制IO复用
 */
/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup UART
 * @{
 */

#ifdef HAL_UART_MODULE_ENABLED
#include "a0_hal_uart.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/**
 * @defgroup UART_Exported_Functions UART Exported Functions
 * @{
 */
/*
    目前使用a0_hal_uart.h中的宏，基本够用，不需要额外的hal层代码，除非用户特殊应用，可自行添加
    hal_uart_init仅仅是写了4个寄存器，CR/ICR/BRCR/RTOR，其他中断函数等根据用户应用自行编写对应代码
    可以参考a0_test_uart.c中的测试代码，进行编写。
*/
void HalUartInit(UART_InitTypeDef *pInit)
{
    UART_TypeDef    *UARTx = UARTN(pInit->uartIdx);
    HAL_UART_BAUD_SET(UARTx, HAL_UART_BAUD_VALUE(SystemPclk, pInit->baudrate));
    HAL_UART_INT_CTRL_SET(UARTx, pInit->icr.w);
    HAL_UART_CTRL_SET(UARTx, pInit->cr.w);
    HAL_UART_TIMEOUT_SET(UARTx, pInit->timeout);
    return;
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
