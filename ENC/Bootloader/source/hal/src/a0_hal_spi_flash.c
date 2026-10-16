/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_spi_flash.c
 * @brief   SPI FLASH HAL module.
 * @version V1.0.6
 * @date    2023/10/07
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.6, 2023/10/07
 *      1. 增加支持256KByte的Flash，INT_FLASH_TYPE值为2时选择。
 *      2. 增加支持128KByte的Flash，INT_FLASH_TYPE值为3时选择。
 * V1.0.5, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.4, 2021/11/09
 *      1. HalSpiFlashWipCheck函数中进行WIP查询，发送的命令有误，应该是0x05，修正BUG。
 *      2. HalSpiFlashQuadEnable函数中解除保护设置，最后读状态寄存器进行校验时，发送命令有误，应该是0x05，修正BUG。
 * V1.0.3, 2021/09/22
 *      1. 将A0.h中的Constans移到该头文件中
 * V1.0.2, 2021/09/14
 *      1. 头文件中的注释部分调整，适应Doxygen
 * V1.0.1, 2021/05/06
 *      1. 添加'EXT_FLASH_SUPPORT'作为选编译，用于选择是否支持外部Flash，如支持外部Flash，则增加对应接口函数
 *      2. 添加定义'EXT_FLASH_TYPE'值，0(最大16MByte的Flash，3字节地址), 2(超过16MByte的Flash
 *      2. 添加定义'INT_FLASH_TYPE'值，0(512KByte内部Flash), 1(2MByte内部Flash), 2(32MByte内部MXFlash，仅用于FPGA)
 *      3. Flash控制器访问使用宏
 *      4. 内部Flash的XIP访问，开启4XIO模式，连续读模式
 *      5. 若开启外部Flash的XIP访问，该外部Flash必须支持连续读模式和4XIO模式
 *      6. 该模块使用到系统Tick计数器接口函数，如HAL_TickDelay(), HAL_TickGet()
 * V1.0.0,
 *      1. 初版，支持内部，外部Flash的CMD访问
 */

/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup SPI_FLASH
 * @{
 */

#ifdef HAL_SPI_FLASH_MODULE_ENABLED
#include "a0_hal_spi_flash.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
#define SPIFC_FIFO_DEPTH                (64)        /* 64x32Bit */

#define SPIF_READ                       (0x03)      /* READ     */
#define SPIF_READ_2XIO                  (0xBB)      /* 2READ    */
#define SPIF_READ_4XIO                  (0xEB)      /* 4READ    */
#define SPIF_RESET_ENABLE               (0x66)      /* RSTEN    */
#define SPIF_RESET                      (0x99)      /* RST      */
#define SPIF_WRITE_ENABLE               (0x06)      /* WREN     */
#define SPIF_READ_STATUS                (0x05)      /* RDSR     */
#define SPIF_READ_STATUS1               (0x35)      /* RDSR1    */
#define SPIF_WRITE_STATUS               (0x01)      /* WRSR     */
#define SPIF_WRITE_STATUS1              (0x31)      /* WRSR1    */

#define SPIF_PAGE_PROGRAM               (0x02)      /* PP       */
#define SPIF_CHIP_ERASE                 (0x60)      /* CE(0xC7) */
#define SPIF_PAGE_ERASE                 (0x81)      /* PE       */
#define SPIF_SECTOR_ERASE               (0x20)      /* SE       */
#define SPIF_BLOCK_ERASE_32K            (0x52)      /* BE32     */
#define SPIF_BLOCK_ERASE_64K            (0xD8)      /* BE64     */
#define SPIF_QUAD_PAGE_PROGRAM          (0x32)      /* QPP      */

#define SPIF_SRP0                       (1<<7)      /* status register protect bit 0 */
#define SPIF_SRP1                       (1<<0)      /* status register protect bit 1 */
#define SPIF_QE                         (1<<1)      /* status register quad enable bit */
#define SPIF_PP_TIMEOUT                 (3)         /* page program time max value in millisecond */
#define SPIF_RST_TIMEREADY              (12)        /* software reset recovery time max 12ms in WRSR opeartion, normal is 30us */
#define SPIF_RX_TIMEREADY               (200UL)     /* wait rx fifo ready time */
#define SPIF_TX_TIMEREADY               (200UL)     /* wait rx fifo ready time */

#define SPIF_SECTOR_SIZE                (4096)

#if (INT_FLASH_TYPE == 0)
#define SPIF_NOUSE_RDADDR               (0x0007FFFC)    /* for enter/exit continue read flash last 4 bytes */
#elif (INT_FLASH_TYPE == 1)
#define SPIF_NOUSE_RDADDR               (0x001FFFFC)    /* for enter/exit continue read flash last 4 bytes */
#elif (INT_FLASH_TYPE == 2)
#define SPIF_NOUSE_RDADDR               (0x0003FFFC)    /* for enter/exit continue read flash last 4 bytes */
#elif (INT_FLASH_TYPE == 3)
#define SPIF_NOUSE_RDADDR               (0x0001FFFC)    /* for enter/exit continue read flash last 4 bytes */
#endif

static const uint32_t eraseCmdTb[5][3] = {
#if (INT_FLASH_TYPE == 0)
    /* cmd, size, timeout(ms) */
    {   SPIF_PAGE_ERASE,        SPIF_PAGE_SIZE,     30, },
    {   SPIF_SECTOR_ERASE,      SPIF_SECTOR_SIZE,   30, },
    {   SPIF_BLOCK_ERASE_32K,   (32 * 1024),        30, },
    {   SPIF_BLOCK_ERASE_64K,   (64 * 1024),        30, },
    {   SPIF_CHIP_ERASE,        (512 * 1024),       30, },
#elif (INT_FLASH_TYPE == 1)
    /* cmd, size, timeout(ms) */
    {   SPIF_PAGE_ERASE,        SPIF_PAGE_SIZE,     30, },
    {   SPIF_SECTOR_ERASE,      SPIF_SECTOR_SIZE,   30, },
    {   SPIF_BLOCK_ERASE_32K,   (32 * 1024),        30, },
    {   SPIF_BLOCK_ERASE_64K,   (64 * 1024),        30, },
    {   SPIF_CHIP_ERASE,        (2048 * 1024),      180,},
#elif (INT_FLASH_TYPE == 2)
    /* cmd, size, timeout(ms) */
    {   SPIF_PAGE_ERASE,        SPIF_PAGE_SIZE,     20, },
    {   SPIF_SECTOR_ERASE,      SPIF_SECTOR_SIZE,   20, },
    {   SPIF_BLOCK_ERASE_32K,   (32 * 1024),        20, },
    {   SPIF_BLOCK_ERASE_64K,   (64 * 1024),        20, },
    {   SPIF_CHIP_ERASE,        (256 * 1024),       20, },
#elif (INT_FLASH_TYPE == 3)
    /* cmd, size, timeout(ms) */
    {   SPIF_PAGE_ERASE,        SPIF_PAGE_SIZE,     20, },
    {   SPIF_SECTOR_ERASE,      SPIF_SECTOR_SIZE,   20, },
    {   SPIF_BLOCK_ERASE_32K,   (32 * 1024),        20, },
    {   SPIF_BLOCK_ERASE_64K,   (64 * 1024),        20, },
    {   SPIF_CHIP_ERASE,        (128 * 1024),       20, },
#endif
};

/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/**
 * @addtogroup SPI_Flash_Exported_Functions SPI Flash Exported Functions
 * @{
 */
/**
 * @brief  wait until cmd complete or timeout
 * @param  waitDelay: delay time in millisecond
 * @retval == 0, cmd complete
 * @retval == -1, cmd not complete and timeout
 */
static int32_t HalSpiFlashCmdCompleteWait(uint32_t waitDelay)
{
    uint32_t startTick = HalSpiFlashTickGet();
    while (HAL_SPIFC_CMD_COMPLETE_CHECK() == 0)
    {
        if ((HalSpiFlashTickGet() - startTick) >= waitDelay)
        {
            return -1;
        }
    }
    HAL_SPIFC_CMD_COMPLETE_CLR();
    return 0;
}

/**
 * @brief  wait until tx ready or timeout
 * @retval == 0, tx ready
 * @retval == -5, tx not ready and timeout
 */
static int32_t HalSpiFlashTxFifoReadyWait(void)
{
    uint32_t startTick = HalSpiFlashTickGet();
    while (HAL_SPIFC_TX_READY_CHECK() == 0)
    {
        if ((HalSpiFlashTickGet() - startTick) >= SPIF_TX_TIMEREADY)
        {
            return -5;
        }
    }
    return 0;
}

static int32_t HalSpiFlashRxFifoReadyWait(void)
{
    uint32_t startTick = HalSpiFlashTickGet();
    while (HAL_SPIFC_RX_READY_CHECK() == 0)
    {
        if ((HalSpiFlashTickGet() - startTick) >= SPIF_RX_TIMEREADY)
        {
            return -5;
        }
    }
    return 0;
}

/**
 * @brief  write data to spi flash controller tx fifo
 * @param  pu8Data, pointer to the data buffer which need to write to the tx fifo
 * @param  len, data number in byte
 * @retval == 0, write data to tx fifo ok
 * @retval != 0, write data to tx fifo error for tx fifo is not ready in 10 milliseconds
 */
static int32_t HalSpiFlashDataSend(uint8_t *pu8Data, uint32_t len)
{
    int32_t ret = 0;
    __packed uint32_t *pu32Data;                            /* input data pointer may not be algin to 4 bytes */
    uint32_t total_byte = len;
    while (total_byte > (SPIFC_FIFO_DEPTH * 4))
    {
        ret = HalSpiFlashTxFifoReadyWait();
        if (0 == ret)
        {
            pu32Data = (__packed uint32_t *)pu8Data;
            for (uint8_t i = 0; i < SPIFC_FIFO_DEPTH; i++)
            {
                HAL_SPIFC_FIFO_U32WR(pu32Data[i]);
            }
            pu8Data += (SPIFC_FIFO_DEPTH * 4);
            total_byte -= (SPIFC_FIFO_DEPTH * 4);
        }
        else
        {
            total_byte = 0;
            break;
        }
    }
    if (total_byte)
    {
        ret = HalSpiFlashTxFifoReadyWait();
        if (0 == ret)
        {
            if (total_byte)
            {
                pu32Data = (__packed uint32_t *)pu8Data;
                uint8_t loopLen = total_byte/4;
                for (uint8_t i = 0; i < loopLen; i++)
                {
                    HAL_SPIFC_FIFO_U32WR(pu32Data[i]);
                }
                total_byte -= (4*loopLen);
                pu8Data += (4*loopLen);
                if (total_byte)
                {
                    for (uint8_t i = 0; i < total_byte; i++)
                    {
                        HAL_SPIFC_FIFO_U8WR(*pu8Data++);
                    }
                }
            }
        }
    }
    return ret;
}

/**
 * @brief  auto polling FLASH WIP status
 * @param  waitDelay: delay time in millisecond
 * @retval == 0, WIP is 0
 * @retval == -1, WIP is 1 and timeout
 */
static int32_t HalSpiFlashWipCheck(uint32_t waitDelay)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .insCode = SPIF_READ_STATUS,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 2,
            .startCs = 0,
            .cmdSeparateDisable = 0,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashCmdCompleteWait(waitDelay);
    return retVal;
}

/**
 * @brief  send cmd to read status in flash
 * @param  readCmd: read cmd, can be RDSR(0x05), RDSR1(0x35)
 * @param  pStatus: pointer to the memery to write the value of status which read from flash
 * @retval == 0, read ok, value of status is in *pStatus
 * @retval == -1, read error and timeout in 30 milliseconds
 */
static int32_t HalSpiFlashCmdSendRDSR(uint8_t readCmd, uint8_t *pStatus)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .dataCounter = 1,
            .insCode = readCmd,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 0,
            .startCs = 0,
            .cmdSeparateDisable = 0,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashCmdCompleteWait(30);
    if (0 == retVal)
    {
        retVal = HalSpiFlashRxFifoReadyWait();
        if (retVal == 0)
        {
            *pStatus = HAL_SPIFC_FIFO_U8RD();
        }
    }
    return retVal;
}

#if 0
/* read config register */
static int32_t HalSpiFlashCmdSendRDCR(uint8_t *pStatus)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .dataCounter = 1,
            .insCode = 0x15,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 0,
            .startCs = 0,
            .cmdSeparateDisable = 0,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashCmdCompleteWait(30);
    if (0 == retVal)
    {
        *pStatus = HAL_SPIFC_FIFO_U8RD();
    }
    return retVal;
}
#endif

/**
 * @brief  send cmd RSTEN and RST to software reset flash
 * @param  cs: internal flash, cs is 0, external flash, cs is 1
 * @retval == 0, cmd send ok
 * @retval == -1, cmd send error and timeout in 30 milliseconds
 */
static int32_t HalSpiFlashCmdSendRESET(void)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .insCode = SPIF_RESET_ENABLE,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 1,
            .startCs = 0,
            .cmdSeparateDisable = 0,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashCmdCompleteWait(30);
    if (0 != retVal)
    {
        return retVal;
    }
    cmdReg.b.insCode = SPIF_RESET;
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    retVal = HalSpiFlashCmdCompleteWait(30);
    if (0 != retVal)
    {
        return retVal;
    }
    HalSpiFlashTickDelay(SPIF_RST_TIMEREADY);
    return retVal;
}

/**
 * @brief  send WREN(0x06) cmd to flash
 * @param  None
 * @retval == 0, cmd send ok
 * @retval == -1, cmd send error and timeout in 30 milliseconds
 */
static int32_t HalSpiFlashCmdSendWREN(void)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .insCode = SPIF_WRITE_ENABLE,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 1,
            .startCs = 0,
            .cmdSeparateDisable = 0,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashCmdCompleteWait(30);
    return retVal;
}

/**
 * @brief  send CE(0x60) cmd to flash
 * @param  None
 * @retval == 0, cmd send ok
 * @retval == -1, cmd send error and timeout in 30 milliseconds
 */
static int32_t HalSpiFlashCmdSendCE(void)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .insCode = SPIF_CHIP_ERASE,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 3,
            .startCs = 0,
            .cmdSeparateDisable = 0,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashCmdCompleteWait(30);
    return retVal;
}

/**
 * @brief  send cmd to erase flash
 * @param  eraseCmd: erase cmd, can be PE(0x81), SE(0x20), BE32(0x52), BE64(0xD8)
 * @param  offset: erase start address in flash
 * @retval == 0, cmd send ok
 * @retval == -1, cmd send error and timeout in 30 milliseconds
 */
static int32_t HalSpiFlashCmdSendERASE(uint8_t eraseCmd, uint32_t offset)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .address = offset,
            .addrLen = 3,
            .insCode = eraseCmd,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 3,
            .startCs = 0,
            .cmdSeparateDisable = 1,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashCmdCompleteWait(30);
    return retVal;
}

/**
 * @brief  send cmd to write status in flash
 * @param  wrtieCmd: write cmd, can be WRSR(0x01), WRSR(0x31)
 * @param  pStatus: pointer to the memery to read the value of status which write from flash
 * @retval == 0, write ok
 * @retval != 0, write error or wait cmd complete timeout in 30 milliseconds
 */
static int32_t HalSpiFlashCmdSendWRSR(uint8_t wrtieCmd, uint8_t *pStatus)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .dataCounter = 1,
            .insCode = wrtieCmd,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 1,
            .startCs = 0,
            .cmdSeparateDisable = 0,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashDataSend(pStatus, 1);
    if (0 == retVal)
    {
        retVal = HalSpiFlashCmdCompleteWait(30);
    }
    return retVal;
}

/**
 * @brief  send QPP(0x32) and data to program the flash in quad page program mode
 * @param  pu8Data: pointer to the memery to read the data which need to program to the flash
 * @param  offset: program start address in flash
 * @param  len: byte number need to program
 * @retval == 0, cmd operation ok
 * @retval != 0, cmd operation error or wait cmd complete timeout in 30 milliseconds
 */
#if (INT_FLASH_TYPE < 2)
static int32_t HalSpiFlashCmdSendQPP(uint8_t *pu8Data, uint32_t offset, uint32_t len)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .address = offset,
            .addrLen = 3,
            .dataCounter = len,
            .insCode = SPIF_QUAD_PAGE_PROGRAM,
            .insLen = 1,
            .spiMode = 2,
            .operationMode = 1,
            .startCs = 0,
            .cmdSeparateDisable = 1,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashDataSend(pu8Data, len);
    if (0 == retVal)
    {
        retVal = HalSpiFlashCmdCompleteWait(30);
    }
    return retVal;
}
#endif

#if ((INT_FLASH_TYPE == 2) || (INT_FLASH_TYPE == 3))
/* enable this function if need page program */
/**
 * @brief  send PP(0x02) and data to program the flash in page program mode
 * @param  pu8Data: pointer to the memery to read the data which need to program to the flash
 * @param  offset: program start address in flash
 * @param  len: byte number need to program
 * @retval == 0, cmd operation ok
 * @retval != 0, cmd operation error or wait cmd complete timeout in 30 milliseconds
 */
static int32_t HalSpiFlashCmdSendPP(uint8_t *pu8Data, uint32_t offset, uint32_t len)
{
    SPIFC_CMDR_TypeDef cmdReg = {
        .b = {
            .address = offset,
            .addrLen = 3,
            .dataCounter = len,
            .insCode = SPIF_PAGE_PROGRAM,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 1,
            .startCs = 0,
            .cmdSeparateDisable = 1,
        },
    };
    SPIFC->CMDR[0] = cmdReg.w[0];
    SPIFC->CMDR[1] = cmdReg.w[1];
    SPIFC->CMDR[2] = cmdReg.w[2];
    SPIFC->CMDR[3] = cmdReg.w[3];
    int32_t retVal = HalSpiFlashDataSend(pu8Data, len);
    if (0 == retVal)
    {
        retVal = HalSpiFlashCmdCompleteWait(30);
    }
    return retVal;
}
#endif

/**
 * @brief  config flash to QUAD mode
 * @param  None
 * @retval == 0, config ok
 * @retval == -1, read status error
 * @retval == -2, clear SRP0 and SRP1 to 0 error
 * @retval == -3, set QE in state register error
 * @retval == -4, read back state register to check QE error
 */
static int32_t HalSpiFlashQuadEnable(void)
{
    uint8_t status[2];
    /* step1: read status1/status2 */
    if (HalSpiFlashCmdSendRDSR(SPIF_READ_STATUS, &status[0]) != 0)
    {
        return -1;
    }
#if (INT_FLASH_TYPE < 2)    
    if (HalSpiFlashCmdSendRDSR(SPIF_READ_STATUS1, &status[1]) != 0)
    {
        return -1;
    }
#endif
    /* step2: check protect bit, ensure the status register is in software protect mode */
    if (status[0] & SPIF_SRP0)
    {
        /* clear SRP0 to 0 */
        if (HalSpiFlashCmdSendWREN() != 0)
        {
            return -2;
        }
        status[0] &= (~SPIF_SRP0);
        if (HalSpiFlashCmdSendWRSR(SPIF_WRITE_STATUS, &status[0]) != 0)
        {
            return -2;
        }
    }
#if (INT_FLASH_TYPE < 2) 
    if (status[1] & SPIF_SRP1)
    {
        /* clear SRP1 to 0 */
        if (HalSpiFlashCmdSendWREN() != 0)
        {
            return -2;
        }
        status[1] &= (~SPIF_SRP1);
        if (HalSpiFlashCmdSendWRSR(SPIF_WRITE_STATUS1, &status[1]) != 0)
        {
            return -2;
        }
    }
#endif
#ifdef INT_FLASH_PROTECT_UNLOCK
    /* disable protect */
    if ((status[0] & 0x7C) != 0)
    {
        if (HalSpiFlashCmdSendWREN() != 0)
        {
            return -2;
        }
        status[0] &= (~0x7C);
        if (HalSpiFlashCmdSendWRSR(SPIF_WRITE_STATUS, &status[0]) != 0)
        {
            return -2;
        }
        if (HalSpiFlashCmdSendRDSR(SPIF_READ_STATUS, &status[0]) != 0)
        {
            return -2;
        }
        if ((status[0] & 0x7C) != 0)
        {
            return -2;
        }
    }
#endif
#if (INT_FLASH_TYPE < 2) 
    /* step3: write quad enable bit in status register, and wait 100ms for timeout */
    if (HalSpiFlashCmdSendWREN() != 0)
    {
        return -3;
    }
    status[1] |= SPIF_QE;
    if (HalSpiFlashCmdSendWRSR(SPIF_WRITE_STATUS1, &status[1]) != 0)
    {
        return -3;
    }
    if (HalSpiFlashWipCheck(100) != 0)
    {
        return -3;
    }
    /* step4: ensure qe bit writen ok */
    if (HalSpiFlashCmdSendRDSR(SPIF_READ_STATUS1, &status[1]) != 0)
    {
        return -4;
    }
    if ((status[1] & SPIF_QE) == 0)
    {
        return -4;
    }
#endif
    return 0;
}

/**
 * @brief  enter flash continue read in XIP mode
 * @return None
 */
static void HalSpiFlashContinueReadEnter(void)
{
    SPIFC->XIPCR = ((1 << 10));                 //cache clear and enable
#ifdef INT_FLASH_CONTINUE_RD_DISABLE
#if ((INT_FLASH_TYPE == 2) || (INT_FLASH_TYPE == 3))
    SPIF_XIPCMD_TypeDef xipCmdR = {
        .b = {
            .xipDumyCycle = 4,              // 4 cycles
            .xipOpMode = 3,                 // 1-2-2
            .xipAddrLen = 0,                // 3 bytes
            .xipInsCode = SPIF_READ_2XIO,   // 2xIO
            .xipModeBitInser = 0,           // no mode bit
            .xipInsLen = 1,                 // 1 byte
        },
    };
#else
    SPIF_XIPCMD_TypeDef xipCmdR = {
        .b = {
            .xipDumyCycle = 4,
            .xipOpMode = 4,
            .xipAddrLen = 0,
            .xipInsCode = SPIF_READ_4XIO,
            .xipModeBitInser = 1,
            .xipInsLen = 1,
        },
    };
#endif
    SPIFC->XIPCMD = xipCmdR.w;
#else
    SPIF_XIPCMD_TypeDef xipCmdR = {
        .b = {
            .xipDumyCycle = 4,
            .xipOpMode = 4,
            .xipAddrLen = 0,
            .xipInsCode = SPIF_READ_4XIO,
            .xipModeBitInser = 1,
            .xipModeBit = 0xA5,
            .xipInsLen = 1,
        },
    };
    SPIFC->XIPCMD = xipCmdR.w;
#ifdef DBG_IN_RAM
    UNUSED(U32REG(SPIF_NOUSE_RDADDR+0x10000000));
#else
    UNUSED(U32REG(SPIF_NOUSE_RDADDR));
#endif
    xipCmdR.b.xipInsLen = 0;
    SPIFC->XIPCMD = xipCmdR.w;
#endif
}

/**
 * @brief  exit flash continue read in XIP mode
 * @return None
 */
static void HalSpiFlashContinueReadExit(void)
{
    SPIFC->XIPCR = ((1 << 10));                 //cache clear and enable
#ifdef INT_FLASH_CONTINUE_RD_DISABLE
#else
    SPIF_XIPCMD_TypeDef xipCmdR = {
        .b = {
            .xipDumyCycle = 4,
            .xipOpMode = 4,
            .xipAddrLen = 0,
            .xipInsCode = SPIF_READ_4XIO,
            .xipModeBitInser = 1,
            .xipModeBit = 0x00,
            .xipInsLen = 0,
        },
    };
    SPIFC->XIPCMD = xipCmdR.w;
#ifdef DBG_IN_RAM
    UNUSED(U32REG(SPIF_NOUSE_RDADDR+0x10000000));
    xipCmdR.b.xipInsLen = 1;
    SPIFC->XIPCMD = xipCmdR.w;
    UNUSED(U32REG(SPIF_NOUSE_RDADDR+0x10000000));
#else
    UNUSED(U32REG(SPIF_NOUSE_RDADDR));
    xipCmdR.b.xipInsLen = 1;
    SPIFC->XIPCMD = xipCmdR.w;
    UNUSED(U32REG(SPIF_NOUSE_RDADDR));
#endif
#endif
}

/**
 * @brief  spi flash write data in cmd mode
 * @param  pu8Data: pointer to the memery of data to write to flash
 * @param  offset: write start address in flash
 * @param  len: byte number need to program
 * @retval == 0, flash write ok
 * @retval != 0, flash write error
 */
static int32_t HalSpiFlashWrite(uint8_t *pu8Data, uint32_t offset, uint32_t len)
{
    uint32_t start_page_offset = offset & SPIF_PAGE_SIZE;
    uint32_t len_each_times;
    if (len + start_page_offset <= SPIF_PAGE_SIZE)
    {
        len_each_times = len;
    }
    else
    {
        len_each_times = SPIF_PAGE_SIZE - start_page_offset;
    }
    if (HalSpiFlashWipCheck(100) != 0)
    {
        return -1;
    }
    if (HalSpiFlashCmdSendWREN() != 0)
    {
        return -2;
    }
    //quad page program first page
#if (INT_FLASH_TYPE < 2)
    if (HalSpiFlashCmdSendQPP(pu8Data, offset, len_each_times) != 0)
#else
    if (HalSpiFlashCmdSendPP(pu8Data, offset, len_each_times) != 0)
#endif
    {
        return -3;
    }
    HalSpiFlashTickDelay(SPIF_PP_TIMEOUT);
    pu8Data += len_each_times;

    do
    {
        len -= len_each_times;
        offset += len_each_times;
        if (len < SPIF_PAGE_SIZE)
        {
            if (len == 0)
            {
                if (HalSpiFlashWipCheck(SPIF_PP_TIMEOUT) != 0)
                {
                    return -7;
                }
                break;
            }
            else
            {
                len_each_times = len;
            }
        }
        else
        {
            len_each_times = SPIF_PAGE_SIZE;
        }
        if (HalSpiFlashWipCheck(SPIF_PP_TIMEOUT) != 0)
        {
            return -4;
        }
        if (HalSpiFlashCmdSendWREN() != 0)
        {
            return -5;
        }
#if (INT_FLASH_TYPE < 2)
        if (HalSpiFlashCmdSendQPP(pu8Data, offset, len_each_times) != 0)
#else
        if (HalSpiFlashCmdSendPP(pu8Data, offset, len_each_times) != 0)
#endif
        {
            return -6;
        }
        HalSpiFlashTickDelay(SPIF_PP_TIMEOUT);
        pu8Data += len_each_times;
    } while (1);

    return 0;
}

/**
 * @brief  spi flash erase data in cmd mode
 * @param  type: erase cmd select, 0 for PE(0x81), 1 for SE(0x20), 2 for BE32(0x52), 3 for BE64(0xD8), 4 for CE(0x60)
 * @param  offset: erase start address in flash
 * @param  len: byte number need to erase
 * @retval == 0, flash erase ok
 * @retval != 0, flash erase error
 */
static int32_t HalSpiFlashErase(uint8_t type, uint32_t offset, uint32_t len)
{
    uint32_t timeOut;
    uint32_t eraseSize;
    uint8_t eraseCmd;

    if (HalSpiFlashWipCheck(100) != 0)
    {
        return -1;
    }
    if (type <= 4)
    {
        eraseCmd = eraseCmdTb[type][0];
        eraseSize = eraseCmdTb[type][1];
        timeOut = eraseCmdTb[type][2];
    }
    else
    {
        return -1;
    }

    if (type == 4)
    {
        if (HalSpiFlashCmdSendWREN() != 0)
        {
            return -1;
        }
        if (HalSpiFlashCmdSendCE() != 0)
        {
            return -1;
        }
        HalSpiFlashTickDelay(timeOut);
        if (HalSpiFlashWipCheck(timeOut) != 0)
        {
            return -1;
        }
    }
    else
    {
        for (uint32_t addr = offset & (~(eraseSize - 1)); addr < (offset + len); addr += eraseSize)
        {
            if (HalSpiFlashCmdSendWREN() != 0)
            {
                return -1;
            }
            if (HalSpiFlashCmdSendERASE(eraseCmd, addr) != 0)
            {
                return -1;
            }
            HalSpiFlashTickDelay(timeOut);
            if (HalSpiFlashWipCheck(timeOut) != 0)
            {
                return -1;
            }
        }
    }
    return 0;
}

/* Exported functions -------------------------------------------------------*/
#ifdef EXT_FLASH_SUPPORT
/**
 * @brief  spi flash erase data in cmd mode
 * @param  type: erase cmd select, 0 for PE(0x81), 1 for SE(0x20), 2 for BE32(0x52), 3 for BE64(0xD8), 4 for CE(0x60)
 * @param  offset: erase start address in flash
 * @param  len: byte number need to erase
 * @retval == 1, this function need to be rewrite in DRV
 * @retval == 0, flash erase ok
 * @retval < 0, flash erase error
 */
__WEAK int32_t HalExtFlashErase(uint8_t type, uint32_t offset, uint32_t len)
{
    return -1;
}
/**
 * @brief  spi flash write data in cmd mode
 * @param  pu8Data: pointer to the memery of data to write to flash
 * @param  offset: write start address in flash
 * @param  len: byte number need to program
 * @retval == 1, this function need to be rewrite in DRV
 * @retval == 0, flash write ok
 * @retval < 0, flash write error
 */
__WEAK int32_t HalExtFlashWrite(uint8_t *pu8Data, uint32_t offset, uint32_t len)
{
    return 1;
}

/**
 * @brief  external spi flash init, software reset flash, quad enable, enter continue read in XIP
 * @param  None
 * @retval == 1, this function need to be rewrite in DRV
 * @retval == 0, init ok
 * @retval == -1, reset error
 * @retval == -2, quad enable error
 * @retval == -3, enter continue read error
 */
__WEAK int32_t HalExtFlashInit(void)
{
    return 1;
}
#endif

/**
 * @brief  spi flash erase operation
 * @param  argv: pointer to 3 input parameter, i.e. <FlashOffset> <DataByteCnt> <type>
 *         <FlashOffset>, erase start address, if the address is not the start of this page or sector, the whole page or sector will be erased
 *         <DataByteCnt>, erase byte size, if the end address is not the end of this page or sector, the whole page or sector will be erased
 *         <type>, erase type select, 0 for page(256Byte), 1 for sector(4KByte), 2 for block 32KByte, 3 for block 64KByte, 4 for chip
 * @retval == 0, flash erase ok
 * @retval != 0, flash erase error
 */
int32_t HalFlashErase(uint32_t *argv)
{
    uint32_t type, addr, size;
    int32_t result;
    addr = argv[0];
    size = argv[1];
    type = argv[2];
#ifdef EXT_FLASH_SUPPORT
    if ((addr & 0xF0000000) == 0)
    {
        addr &= 0x00FFFFFF;
        HalSpiFlashContinueReadExit();
        result = HalSpiFlashErase(type, addr, size);
        HalSpiFlashContinueReadEnter();
    }
    else if ((addr & 0xF0000000) == 0x30000000)
    {
        addr &= 0x0FFFFFFF;
        result = HalExtFlashErase(type, addr, size);
    }
    else
    {
        result = -1;
    }
#else
    addr &= 0x00FFFFFF;
    HalSpiFlashContinueReadExit();
    result = HalSpiFlashErase(type, addr, size);
    HalSpiFlashContinueReadEnter();
#endif
    return result;
}

/**
 * @brief  spi flash program operation
 * @param  argv: pointer to 3 input parameter, i.e. <FlashOffset> <DataByteCnt> <WriteDataPt>
 *         <FlashOffset>, program start address
 *         <DataByteCnt>, program byte size
 *         <WriteDataPt>, pointer to data which need to be programed
 * @retval == 0, flash program ok
 * @retval != 0, flash program error
 */
int32_t HalFlashWrite(uint32_t *argv)
{
    uint32_t addr, len;
    int32_t result;
    uint8_t *dataPt;

    addr = argv[0];
    len = argv[1];
    dataPt = (uint8_t *)argv[2];
#ifdef EXT_FLASH_SUPPORT
    if ((addr & 0xF0000000) == 0)
    {
        addr &= 0x00FFFFFF;
        HalSpiFlashContinueReadExit();
        result = HalSpiFlashWrite(dataPt, addr, len);
        HalSpiFlashContinueReadEnter();
    }
    else if ((addr & 0xF0000000) == 0x30000000)
    {
        addr &= 0x0FFFFFFF;
        result = HalExtFlashWrite(dataPt, addr, len);
    }
    else
    {
        result = -1;
    }
#else
    addr &= 0x00FFFFFF;
    HalSpiFlashContinueReadExit();
    result = HalSpiFlashWrite(dataPt, addr, len);
    HalSpiFlashContinueReadEnter();
#endif
    return result;
}

/**
 * @brief  internal spi flash init, config clk and spi mode of controller, software reset flash, quad enable, enter continue read in XIP
 * @param  None
 * @retval == 0, init ok
 * @retval == -1, reset error
 * @retval == -2, quad enable error
 */
int32_t HalFlashInit(void)
{
    SPIFC->XIPCR = ((1 << 10));                 //cache clear and enable

    SPIF_CR_TypeDef cr = {
        .b = {
            .clkDiv = 0,                        //div2
            .clkMode = 0,                       //mode0
        },
    };
    SPIFC->CR = cr.w;

    if (HalSpiFlashCmdSendRESET() != 0)
    {
        return -1;
    }
    if (HalSpiFlashQuadEnable() != 0)
    {
        return -2;
    }
    HalSpiFlashContinueReadEnter();

#ifdef EXT_FLASH_SUPPORT
    HalExtFlashInit();
#endif

    return 0;
}

#ifdef EXT_FLASH_SUPPORT
int32_t HalFlashCmdSend(SPIFC_CmdTypeDef *pCmdPara)
{
    SPIFC->CMDR[0] = pCmdPara->cmdReg.w[0];
    SPIFC->CMDR[1] = pCmdPara->cmdReg.w[1];
    SPIFC->CMDR[2] = pCmdPara->cmdReg.w[2];
    SPIFC->CMDR[3] = pCmdPara->cmdReg.w[3];
    int32_t retVal;
    uint32_t byteLen = pCmdPara->cmdReg.b.dataCounter;
    if (byteLen != 0)
    {
        if (pCmdPara->pWrite != NULL)
        {
            retVal = HalSpiFlashDataSend(pCmdPara->pWrite, byteLen);
            if (0 == retVal)
            {
                retVal = HalSpiFlashCmdCompleteWait(pCmdPara->timeout);
            }
        }
        else if (pCmdPara->pRead != NULL)
        {
            retVal = HalSpiFlashCmdCompleteWait(30);
            if (0 == retVal)
            {
                uint8_t *pRd = pCmdPara->pRead;
                do {
                    *pRd++ = HAL_SPIFC_FIFO_U8RD();
                    byteLen--;
                } while (0 != byteLen);
            }
        }
        else
        {
            retVal = HalSpiFlashCmdCompleteWait(pCmdPara->timeout);
        }
    }
    else
    {
        retVal = HalSpiFlashCmdCompleteWait(pCmdPara->timeout);
    }
    return retVal;
}

#endif
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
