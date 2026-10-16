
/** @brief  the RO of this file must be assigned in sram
  */

#include "a0_hal_conf.h"
#include "a0_hal_spi_flash.h"

#ifdef EXT_FLASH_SUPPORT
#include "a0_hal_def.h"

/* current driver is for MX25L256 */
#define SPIF_EXT_READ_STATUS                    (0x05)          /* RDSR     */
#define SPIF_EXT_RESET_ENABLE                   (0x66)          /* RSTEN    */
#define SPIF_EXT_RESET                          (0x99)          /* RST      */
#define SPIF_EXT_WRITE_ENABLE                   (0x06)          /* WREN     */
#define SPIF_EXT_WRITE_STATUS                   (0x01)          /* WRSR     */

#define SPIF_EXT_4READ                          (0xEB)          /* 4READ    */
#define SPIF_EXT_SECTOR_ERASE                   (0x20)          /* SE       */
#define SPIF_EXT_BLOCK_ERASE_32K                (0x52)          /* BE32K    */
#define SPIF_EXT_BLOCK_ERASE_64K                (0xD8)          /* BE64K    */
#define SPIF_EXT_CHIP_ERASE                     (0x60)          /* CE(0xC7) */
#define SPIF_EXT_QUAD_PAGE_PROGRAM              (0x38)          /* QPP      */

#define SPIF_EXT_4READ4B                        (0xEC)          /* 4READ4B  */
#define SPIF_EXT_SECTOR_ERASE_4B                (0x21)          /* SE4B     */
#define SPIF_EXT_BLOCK_ERASE_32K_4B             (0x5C)          /* BE32K4B  */
#define SPIF_EXT_BLOCK_ERASE_64K_4B             (0xDC)          /* BE64K4B  */
#define SPIF_EXT_QUAD_PAGE_PROGRAM4B            (0x3E)          /* QPP4B    */

#define SPIF_EXT_BP                             (0xF<<2)        /* status register block protect bit, bp3~bp0 */
#define SPIF_EXT_SRWD                           (1<<7)          /* status register protect bit 0 */
#define SPIF_EXT_QE                             (1<<6)          /* status register quad enable bit */
#define SPIF_EXT_RST_TIMEREADY                  (150)           /* max 100ms for Chip Erase operation */
#define SPIF_EXT_SECTOR_SIZE                    (4096)
#define SPIF_EXT_NOUSE_RDADDR                   (0x0007FFFC)    /* for enter/exit continue read flash last 4 bytes */

#define SPIF_EXT_CMD_WIP_IDX                    (0)
#define SPIF_EXT_CMD_WREN_IDX                   (1)
#define SPIF_EXT_CMD_RDSR_IDX                   (2)
#define SPIF_EXT_CMD_WRSR_IDX                   (3)
#define SPIF_EXT_CMD_CE_IDX                     (4)

static const SPIFC_CMDR_TypeDef DRV_CMD_REG_TB[] = {
    {/* WIP */
        .b = {
            .insCode = SPIF_EXT_READ_STATUS,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 2,
            .startCs = 1,
            .cmdSeparateDisable = 0,
        },
    },
    {/* WREN */
        .b = {
            .insCode = SPIF_EXT_WRITE_ENABLE,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 1,
            .startCs = 1,
            .cmdSeparateDisable = 0,
        },
    },
    {/* RDSR */
        .b = {
            .dataCounter = 1,
            .insCode = SPIF_EXT_READ_STATUS,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 0,
            .startCs = 1,
            .cmdSeparateDisable = 0,
        },
    },
    {/* WRSR */
        .b = {
            .dataCounter = 1,
            .insCode = SPIF_EXT_WRITE_STATUS,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 1,
            .startCs = 1,
            .cmdSeparateDisable = 0,
        },
    },
    {/* CE */
        .b = {
            .insCode = SPIF_EXT_CHIP_ERASE,
            .insLen = 1,
            .spiMode = 0,
            .operationMode = 3,
            .startCs = 1,
            .cmdSeparateDisable = 0,
        },
    }
};

/* MX25L256 has no page erase, the first group are all zeros */
static const uint32_t HAL_ERASE_CMD_TB[5][3] = {
    /* cmd, size, timeout(ms) */
#if (EXT_FLASH_TYPE == 0)    
    {
        0,
        0,
        0,
    },
    {
        SPIF_EXT_SECTOR_ERASE,
        SPIF_EXT_SECTOR_SIZE,
        400,
    },
    {
        SPIF_EXT_BLOCK_ERASE_32K,
        (32 * 1024),
        1000,
    },
    {
        SPIF_EXT_BLOCK_ERASE_64K,
        (64 * 1024),
        2000,
    },
    {
        SPIF_EXT_CHIP_ERASE,
        (32 * 1024 * 1024),
        150000,
    },
#else
    {
        0,
        0,
        0,
    },
    {
        SPIF_EXT_SECTOR_ERASE_4B,
        SPIF_EXT_SECTOR_SIZE,
        400,
    },
    {
        SPIF_EXT_BLOCK_ERASE_32K_4B,
        (32 * 1024),
        1000,
    },
    {
        SPIF_EXT_BLOCK_ERASE_64K_4B,
        (64 * 1024),
        2000,
    },
    {
        SPIF_EXT_CHIP_ERASE,
        (32 * 1024 * 1024),
        150000,
    },
#endif
};
  

/** @brief  enter continue read in 4XIO mode for external flash
  * @param  None
  * @retval == 0, enter ok
  * @retval != 0, enter failed
  */
static int32_t drv_ext_flash_continue_read_enter(void)
{
    /* enter continue read mode by cmd */
    uint32_t firstRead4Byte;                        //to read 4 byte in cmd mode
    SPIFC_CmdTypeDef cmdPara = {
        .cmdReg = {
            .b = {
                .dataCounter = 4,
                .address = SPIF_EXT_NOUSE_RDADDR,
#if (EXT_FLASH_TYPE == 0)
                .addrLen = 3,
                .insCode = SPIF_EXT_4READ,
#else
                .addrLen = 4,
                .insCode = SPIF_EXT_4READ4B,
#endif
                .insLen = 1,
                .dumCycle = 4,
                .operationMode = 0,
                .spiMode = 4,
                .modeBitsInsert = 1,
                .modeBitValue = 0xA5,
                .startCs = 1,
                .cmdSeparateDisable = 0,
            },
        },
        .timeout = 40,
        .pRead = (uint8_t *)&firstRead4Byte,
        .pWrite = NULL,
    };
    int32_t retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -1;
    }
    /* change the controller to access the ext flash */
    SYS_SPI_TypeDef regSysSpi = {
        .b = {
#if (EXT_FLASH_TYPE == 0)
            .addrWidth = SPI_EXT_ADDR_3BYTES,
#else
            .addrWidth = SPI_EXT_ADDR_4BYTES,
#endif
            .dummyCycle = 3,
            .modeValue = 0xA5,
            .clkDiv = 0,
            .ctrlSrc = 0,
        }
    };
    HAL_SYS_EXT_SPI_CFG_SET(regSysSpi.w);
    return 0;
}

/** @brief  exit continue read in 4XIO mode for external flash
  * @param  None
  * @return None
  */
static void drv_ext_flash_continue_read_exit(void)
{
    SYS_SPI_TypeDef regSysSpi = {
        .b = {
#if (EXT_FLASH_TYPE == 0)
            .addrWidth = SPI_EXT_ADDR_3BYTES,
#else
            .addrWidth = SPI_EXT_ADDR_4BYTES,
#endif
            .dummyCycle = 3,
            .modeValue = 0x00,
            .clkDiv = 0,
            .ctrlSrc = 0,
        }
    };
    HAL_SYS_EXT_SPI_CFG_SET(regSysSpi.w);           //[M7:M0]=0，读一次，退出Flash的连续读
    UNUSED(U32REG(0x30000000));
    regSysSpi.b.ctrlSrc = 1;                        //控制器切换成SPIFC
    HAL_SYS_EXT_SPI_CFG_SET(regSysSpi.w);
    return;
}

static int32_t drv_ext_flash_protect_disable(void)
{
    uint8_t status;
    /* step1: read status1/status2 */
    /* RDSR */
    SPIFC_CmdTypeDef cmdPara = {
        .cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_RDSR_IDX],
        .timeout = 40,
        .pRead = (uint8_t *)&status,
        .pWrite = NULL,
    };
    int32_t retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -1;
    }
    /* step2: check protect bit, ensure the status register is in software protect mode */
    if (status & SPIF_EXT_SRWD)
    {
        /* clear SRWD to 0 */
        /* WREN */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
        cmdPara.pRead = NULL;
        //cmrPara.pWrite = NULL;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -2;
        }
        /* WRSR */
        status &= (~SPIF_EXT_SRWD);
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WRSR_IDX];
        //cmdPara.pRead = NULL;
        cmdPara.pWrite = (uint8_t *)&status;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -2;
        }
    }
    if ((status & SPIF_EXT_BP) != 0)
    {
        /* step3: write quad enable bit in status register, and wait 100ms for timeout */
        /* WREN */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
        cmdPara.pRead = NULL;
        cmdPara.pWrite = NULL;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -3;
        }
        /* WRSR */
        status &= (~SPIF_EXT_BP);
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WRSR_IDX];
        //cmdPara.pRead = NULL;
        cmdPara.pWrite = (uint8_t *)&status;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -3;
        }
        /* WIP check */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX];
        //cmdPara.pRead = NULL;
        cmdPara.pWrite = NULL;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -3;
        }
        /* step4: ensure qe bit writen ok */
        /* RDSR */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_RDSR_IDX];
        cmdPara.pRead = (uint8_t *)&status;
        //cmrPara.pWrite = NULL;   
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -4;
        }
        if ((status & SPIF_EXT_BP) != 0)
        {
            return -4;
        }
    }
    return 0;
}

/** @brief  config flash to QUAD mode
  * @param  None
  * @retval == 0, config ok
  * @retval == -1, read status error
  * @retval == -2, clear SRWD to 0 error
  * @retval == -3, set QE in state register error
  * @retval == -4, read back state register to check QE error
  */
static int32_t drv_ext_flash_quad_enable(void)
{
    uint8_t status;
    /* step1: read status1/status2 */
    /* RDSR */
    SPIFC_CmdTypeDef cmdPara = {
        .cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_RDSR_IDX],
        .timeout = 40,
        .pRead = (uint8_t *)&status,
        .pWrite = NULL,
    };
    int32_t retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -1;
    }
    /* step2: check protect bit, ensure the status register is in software protect mode */
    if (status & SPIF_EXT_SRWD)
    {
        /* clear SRWD to 0 */
        /* WREN */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
        cmdPara.pRead = NULL;
        //cmrPara.pWrite = NULL;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -2;
        }
        /* WRSR */
        status &= (~SPIF_EXT_SRWD);
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WRSR_IDX];
        //cmdPara.pRead = NULL;
        cmdPara.pWrite = (uint8_t *)&status;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -2;
        }
    }
    if ((status & SPIF_EXT_QE) == 0)
    {
        /* step3: write quad enable bit in status register, and wait 100ms for timeout */
        /* WREN */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
        cmdPara.pRead = NULL;
        cmdPara.pWrite = NULL;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -3;
        }
        /* WRSR */
        status |= SPIF_EXT_QE;
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WRSR_IDX];
        //cmdPara.pRead = NULL;
        cmdPara.pWrite = (uint8_t *)&status;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -3;
        }
        /* WIP check */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX];
        //cmdPara.pRead = NULL;
        cmdPara.pWrite = NULL;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -3;
        }
        /* step4: ensure qe bit writen ok */
        /* RDSR */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_RDSR_IDX];
        cmdPara.pRead = (uint8_t *)&status;
        //cmrPara.pWrite = NULL;   
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -4;
        }
        if ((status & SPIF_EXT_QE) == 0)
        {
            return -4;
        }
    }
    return 0;    
}

int32_t hal_ext_flash_init(void)
{
    /* config gpio for EXT Flash Access */
    HAL_GPIO_AF_SET(GPIO1, 0, GPIO_AF1_SPI_CS);
    HAL_GPIO_AF_SET(GPIO1, 1, GPIO_AF1_SPI_CLK);
    HAL_GPIO_AF_SET(GPIO1, 2, GPIO_AF1_SPI_TX);
    HAL_GPIO_AF_SET(GPIO1, 3, GPIO_AF1_SPI_RX);
    HAL_GPIO_AF_SET(GPIO1, 4, GPIO_AF1_SPI_WP);
    HAL_GPIO_AF_SET(GPIO1, 5, GPIO_AF1_SPI_HOLD);
    
    /* SPIFC access ext flash */
    HAL_SYS_EXT_SPI_CTL_SEL1();
    /* cache clear and enable */
//    SPIFC->XIPCR = ((1 << 10));                 
    /* reset enable and reset */
    SPIFC_CmdTypeDef cmdPara = {
        .cmdReg = {
            .b = {
                .insCode = SPIF_EXT_RESET_ENABLE,
                .insLen = 1,
                .spiMode = 0,
                .operationMode = 1,
                .startCs = 1,
                .cmdSeparateDisable = 0,
            },
        },
        .timeout = 40,
        .pRead = NULL,
        .pWrite = NULL,
    };
    int32_t retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -1;
    }
    cmdPara.cmdReg.b.insCode = SPIF_EXT_RESET;
    retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -1;
    }
    HAL_TickDelay(SPIF_EXT_RST_TIMEREADY);
    /* quad enable */
    if (drv_ext_flash_quad_enable() != 0)
    {
        return -2;
    }
    /* enter continue read mode by SYS_SPI controller */
    if (drv_ext_flash_continue_read_enter() != 0)
    {
        return -3;
    }
    return 0;
}

/** @brief  spi flash erase data in cmd mode
  * @param  type: erase cmd select, 0 for PE(0x81), 1 for SE(0x20), 2 for BE32(0x52), 3 for BE64(0xD8), 4 for CE(0x60)
  * @param  offset: erase start address in flash
  * @param  len: byte number need to erase
  * @retval == 0, flash erase ok
  * @retval != 0, flash erase error
  */
int32_t hal_ext_flash_erase(uint8_t type, uint32_t offset, uint32_t len)
{
    uint32_t timeOut, eraseSize;
    uint8_t eraseCmd;
    
    drv_ext_flash_continue_read_exit();
    
    int32_t retVal = drv_ext_flash_protect_disable();
    if (0 != retVal)
    {
        return -1;
    }
    
    /* WIP check */
    SPIFC_CmdTypeDef cmdPara = {
        .cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX],
        .timeout = 100,
        .pRead = NULL,
        .pWrite = NULL,
    };
    retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -1;
    }
    if (type <= 4)
    {
        eraseCmd = HAL_ERASE_CMD_TB[type][0];
        eraseSize = HAL_ERASE_CMD_TB[type][1];
        timeOut = HAL_ERASE_CMD_TB[type][2];
        if (0 == eraseCmd)
        {
            /* not support page erase */
            return -1;
        }
    }
    else
    {
        /* type out of range */
        return -1;
    }

    if (type == 4)
    {
        /* WREN */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -2;
        }
        /* CE */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_CE_IDX];
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -2;
        }
        /* WIP */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX];
        cmdPara.timeout = timeOut;
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -2;
        }
    }
    else
    {
        cmdPara.timeout = timeOut;
        SPIFC_CmdTypeDef erasePara = {
            .cmdReg = {
                .b = {
#if (EXT_FLASH_TYPE == 0)
                    .addrLen = 3,
#else
                    .addrLen = 4,
#endif                    
                    .insCode = eraseCmd,
                    .insLen = 1,
                    .spiMode = 0,
                    .operationMode = 3,
                    .startCs = 1,
                    .cmdSeparateDisable = 0,
                },
            },
            .timeout = 40,
            .pRead = NULL,
            .pWrite = NULL,
        };
        for (uint32_t addr = offset & (~(eraseSize - 1)); addr < (offset + len); addr += eraseSize)
        {
            /* WREN */
            cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
            retVal = hal_flash_cmd_send(&cmdPara);
            if (0 != retVal)
            {
                return -2;
            }
            /* ERASE */
            erasePara.cmdReg.b.address = addr;
            retVal = hal_flash_cmd_send(&erasePara);
            if (0 != retVal)
            {
                return -2;
            }      
            /* WIP */
            cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX];
            cmdPara.timeout = timeOut;
            retVal = hal_flash_cmd_send(&cmdPara);
            if (0 != retVal)
            {
                return -2;
            }
        }
    } 

    HAL_TickDelay(10);

    if (drv_ext_flash_continue_read_enter() != 0)
    {
        return -3;
    }
    return 0;
}
/** @brief  spi flash write data in cmd mode
  * @param  pu8Data: pointer to the memery of data to write to flash
  * @param  offset: write start address in flash
  * @param  len: byte number need to program
  * @retval == 0, flash write ok
  * @retval != 0, flash write error
  */
int32_t hal_ext_flash_write(uint8_t *pu8Data, uint32_t offset, uint32_t len)
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

    drv_ext_flash_continue_read_exit();

    int32_t retVal = drv_ext_flash_protect_disable();
    if (0 != retVal)
    {
        return -1;
    }
    
    /* WIP check */
    SPIFC_CmdTypeDef cmdPara = {
        .cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX],
        .timeout = 100,
        .pRead = NULL,
        .pWrite = NULL,
    };
    retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -1;
    }
    /* WREN */
    cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
    retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -2;
    }
    /* QPP first page */
    SPIFC_CmdTypeDef programPara = {
        .cmdReg = {
            .b = {
                .address = offset,
                .dataCounter = len_each_times,
#if (EXT_FLASH_TYPE == 0)                
                .addrLen = 3,
                .insCode = SPIF_EXT_QUAD_PAGE_PROGRAM,
#else
                .addrLen = 4,
                .insCode = SPIF_EXT_QUAD_PAGE_PROGRAM4B,
#endif
                .insLen = 1,
                .spiMode = 4,
                .operationMode = 1,
                .startCs = 1,
                .cmdSeparateDisable = 0,
            },
        },
        .timeout = 50,
        .pRead = NULL,
        .pWrite = pu8Data,
    };
    retVal = hal_flash_cmd_send(&programPara);
    if (0 != retVal)
    {
        return -3;
    }
    /* WIP */
    cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX];
    retVal = hal_flash_cmd_send(&cmdPara);
    if (0 != retVal)
    {
        return -4;
    }
    
	pu8Data += len_each_times;

    do
    {
		len -= len_each_times;
		offset += len_each_times;
		if (len < SPIF_PAGE_SIZE) 
		{
			if (len == 0) 
            {
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
        /* WREN */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WREN_IDX];
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -5;
        }
        /* QPP */
        programPara.cmdReg.b.address = offset;
        programPara.cmdReg.b.dataCounter = len_each_times;
        programPara.pWrite = pu8Data;
        retVal = hal_flash_cmd_send(&programPara);
        if (0 != retVal)
        {
            return -6;
        }
        /* WIP */
        cmdPara.cmdReg = DRV_CMD_REG_TB[SPIF_EXT_CMD_WIP_IDX];
        retVal = hal_flash_cmd_send(&cmdPara);
        if (0 != retVal)
        {
            return -7;
        }
        
        pu8Data += len_each_times;
	} while (1);

    HAL_TickDelay(10);
    
    if (drv_ext_flash_continue_read_enter() != 0)
    {
        return -8;
    }
	return 0;
}


#endif
