
#ifndef __TEST_DRV_I2C_MASTER_C_
#define __TEST_DRV_I2C_MASTER_C_
/**
  ******************************************************************************
  * @file    drv_i2c_master.c
  * @brief   i2c master drv module.
  * @version V1.0.0
  * @date    2021/09/27
  * @author  lijixuan 
  ******************************************************************************
  */
  
/**
  * 版本日志：
  * V1.0.0, 2021/09/27
  */

/**
  *I2C应用层测试例程说明：
  *1.测试应用场景说明：
  *  A.本测试中1个I2C主机同时挂载3个I2C从设备测试
  *  B.用户可自定义从机设备号(注意该从机设备号为自定义的虚拟设备号，不同于设备真实物理地址，使用时仍需要先把真实物理地址与设备号先关联)
  *  C.本测试例程从机设备号：0：挂载自身做从机  1：挂载从设备codec  2：挂载从设备eeprom，用户也可自定义不同的设备挂载到不同的设备号。
  *2. 硬件连接：改变TestDrvI2CConfigAF(uint8_t afMode)函数参数可实现不同的I2C主从组合情况，根据主从组合连线即可，详细组合参照函数内部说明，
  *   不使用TestDrvI2CConfigAF(uint8_t afMode)中的主从组合，亦可参照TestDrvI2CConfigAF(uint8_t afMode)自定义主从组合，需自己重新配置GPIO。
  *3. 改变测试例程的宏定义可实现相应宏下的功能，详细说明见app_test()函数说明
  *4. 使用流程说明（参考TestDrvI2C（）函数）：
  *   A.定义需要使用到的I2C功能全局变量结构体：
  *     a.I2C_M_CDC_Typedef（使用到codec功能时需定义，且每个codec都需配置新的该类型结构体变量）
  *     b.I2C_M_S_Typedef （使用到自身做从机slave功能时需定义，且每个从机都需配置新的该类型结构体变量）
  *     c.I2C_M_E2R_Typedef （使用到eeprom功能时需定义，且每个eeprom都需配置新的该类型结构体变量）
  *  B.配置I2C的GPIO设置.
  *  C.调用主从机初始化函数
  *  D.配置相关功能结构体参数，并传入初始化函数
  *5.改变testI2cFlag1的宏定义值可测试相关的应用场景
  */ 

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_i2c_master.h"
#include "a0_hal_i2c_slave.h"
#include "a0_hal_gpio.h"
#include "drv_i2c_master.h"
#include "drv_i2c_slave.h"
#include "test_drv_i2c_master.h"

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define TEST_DRV_ENVIROMENT              (1)   /*!< 改变测试场景：1、codec写 2、eepr写 3、eepr读  4、自身做从机读写              
                                        该值1~4                                                                      */
#define TEST_DRV_ENVIROMENT_SLAVE        (0)  /*!< 单独测试做从机情况下改变应用场景：1、写数据  2、读数据  3、检验读写数据 
                                        该值1~3                                                                      */																			
#define IIC_SLAVE_INIT				(0)						
										
uint8_t  codecRegAddr;         /*!< codec访问寄存的地址值                                                        */
uint16_t codecRegData;         /*!< codec寄存器里写数据                                                          */
uint32_t slaveRamAddr;         /*!< i2c做从机时访问的寄存器地址值                                                */
uint32_t slaveRamByteLen;      /*!< i2c做从机时要写/读访问的数据大小                                             */
uint32_t slaveRamBuff[0x100];  /*!< i2c做从机测试时读写测试数据缓冲区                                            */

I2C_M_CDC_Typedef i2cM2cdc;       /*!< 定义codec参数配置结构体                                                      */
I2C_M_S_Typedef i2cM2s;           /*!< 定义自身做从设备参数配置结构体                                               */
I2C_M_E2R_Typedef i2cM2eer;       /*!< 定义eeprom参数配置结构体                                                     */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup USBH_AUDIO_CORE_Private_FunctionPrototypes
  * @{
  */

static void TestDrvI2CConfigAF(uint8_t afMode);

/** @defgroup I2C_App_Test_ConfigAF
  * @{
  */
  
 /**
  * @brief  I2C例程测试GPIO引脚配置
  * @param  afMode  改变该值可以改变A0不同的I2C配置组合
  *         This parameter can be the value of 0~3
  * @retval none                  
  */
static void TestDrvI2CConfigAF(uint8_t afMode)
{      
    /*初始化相应I2C的GPIO时钟*/
    /*第一种I2C主从组合*/
    if (afMode == 1)
    {
        HAL_GPIO_AF_SET(GPIO0, 27, GPIO_AF3_I2CM_SCL);
        HAL_GPIO_AF_SET(GPIO0, 26, GPIO_AF3_I2CM_SDA);
		
		HAL_GPIO_AF_SET(GPIO0, 17, GPIO_AF5_I2CS_SCL);
        HAL_GPIO_AF_SET(GPIO0, 16, GPIO_AF5_I2CS_SDA); 
		
        #if 0 /*改为1配置0.27/0.26为上拉模式*/                                 
        HAL_GPIO_PUPD_SET(GPIO0, 27, GPIO_PULLUP);
        HAL_GPIO_PUPD_SET(GPIO0, 26, GPIO_PULLUP);
        #endif
    }
    /*第二种I2C主从组合*/
    else if (afMode == 2)
    {
        HAL_GPIO_AF_SET(GPIO0, 31, GPIO_AF5_I2CM_SCL);
        HAL_GPIO_AF_SET(GPIO0, 30, GPIO_AF5_I2CM_SDA);
        HAL_GPIO_AF_SET(GPIO0, 17, GPIO_AF5_I2CS_SCL);
        HAL_GPIO_AF_SET(GPIO0, 16, GPIO_AF5_I2CS_SDA); 
        #if 0 /*改为1配置0.30/0.31为上拉模式*/   
        HAL_GPIO_PUPD_SET(GPIO0, 31, GPIO_PULLUP);
        HAL_GPIO_PUPD_SET(GPIO0, 30, GPIO_PULLUP);
        #endif
    }
    /*第三种I2C主从组合*/
    else if (afMode == 3)
    {
        HAL_GPIO_AF_SET(GPIO1, 5, GPIO_AF5_I2CM_SCL);
        HAL_GPIO_AF_SET(GPIO1, 4, GPIO_AF5_I2CM_SDA);
        HAL_GPIO_AF_SET(GPIO1, 1, GPIO_AF5_I2CS_SCL);
        HAL_GPIO_AF_SET(GPIO1, 0, GPIO_AF5_I2CS_SDA); 
        #if 0 /*改为1配置0.4/0.5为上拉模式*/       
        HAL_GPIO_PUPD_SET(GPIO0, 5, GPIO_PULLUP);
        HAL_GPIO_PUPD_SET(GPIO0, 4, GPIO_PULLUP);
        #endif 
    }
}

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup I2C_App_Exported_Types_Functions I2C app exported types functions
  * @{
  */

/**
  * @brief  I2C测试外设配置初始化
  * @retval none                  
  */
void TestDrvI2C(void)
{
    /*配置GPIO引脚*/
    TestDrvI2CConfigAF(1);
    
    /*初始化I2C主机与从机配置*/
    DrvI2cSlaveInit();
	
	#if IIC_SLAVE_INIT
	while(1){;}
	#endif
	
    DrvI2cMasterInit();
    
    /*初始化主机控制codec*/
    i2cM2cdc.devAddr = I2C_DEV_HANDLE_ADDR_CODEC;/*配置codec地址*/
    i2cM2cdc.len = 2;   /*配置一次往codec写多少字节数据*/
    i2cM2cdc.timeOutMs = 20; /*读写超时配置*/
    i2cM2cdc.handleIdx = 1;  /*配置codec作为I2C从设备的ID号*/
    DrvI2cMasterInitCodec(&i2cM2cdc); /*传入codec配置的初始化结构体参数进行codec作为从设备的初始化*/
    
    /*初始化主机控制自身做从设备*/
    i2cM2s.devAddr = I2C_DEV_HANDLE_ADDR_SLAVE;/*配置AO做从机的地址*/ 
    i2cM2s.timeOutMs = 20;/*读写超时配置*/
    i2cM2s.handleIdx  = 0;/*配置A0做从设备时I2C从设备的ID号*/
    DrvI2cMasterInitSlave(&i2cM2s);/*传入A0做从设备时配置的初始化结构体参数进行从设备的初始化*/
    
    /*初始化主机控制eeprom*/
    i2cM2eer.devAddr = I2C_DEV_HANDLE_ADDR_EEPROM;/*配置eeprom地址*/
    i2cM2eer.timeOutMs = 20; /*读写超时配置*/
    i2cM2eer.handleIdx = 2;/*配置eeprom作为I2C从设备的ID号*/
    DrvI2cMasterInitEeprom(&i2cM2eer);/*传入eeprom配置的初始化结构体参数进行eeprom作为从设备的初始化*/

    #if (TEST_DRV_ENVIROMENT_SLAVE != 0) /*该为1时 i2c测试例程单独测试做从机读写*/
    TestDrvI2cModeSelect(TEST_DRV_ENVIROMENT);/*i2c测试例程*/
    #else
    TestDrvI2cModeSletM2S(TEST_DRV_ENVIROMENT_SLAVE);
    #endif
    return;
}

/**
  * @brief  I2C例程测试
  * @param  testI2cFlag  改变该值可以测试不同的应用场景：1、codec写 2、eepr写 3、eepr读  4、自身做从机读写
            该值配置1~4 
  * @retval none                  
  */
void TestDrvI2cModeSelect(uint32_t testI2cFlag)
{
    uint32_t mode = testI2cFlag;
    /*测试控制自身为I2C从设备时读写数据*/
    while(mode == 4)
    {
        slaveRamAddr = 0x20004000;
        slaveRamBuff[0] = 0x01234567;
        slaveRamBuff[1] = 0x89ABCDEF;
        slaveRamBuff[2] = 0x11223344;
        slaveRamBuff[3] = 0x55667788;
        slaveRamBuff[4] = 0x99AABBCC;
        slaveRamBuff[5] = 0xDDEEFF00;        
        slaveRamByteLen = 0x18;
        DrvI2cSlaveCtrlWrite(slaveRamAddr, (uint8_t *)slaveRamBuff, slaveRamByteLen,&i2cM2s);
        memset(slaveRamBuff, 0x00, 0x18);
        DrvI2cSlaveCtrlRead(slaveRamAddr, (uint8_t *)slaveRamBuff, slaveRamByteLen,&i2cM2s);
        TestDrvI2CReadCheck(slaveRamAddr, (uint32_t)&slaveRamBuff[0], (slaveRamByteLen>>2));
        __breakpoint(0);
    }
    while(1)
    {
		/*测试往codec寄存器写数据*/
		if (mode == 1)
		{//codec
			DrvI2cCodecWriteReg(codecRegAddr,codecRegData,2,&i2cM2cdc);
			mode = 0;
		}
		/*测试往eeprom中写数据*/
		else if (mode == 2)
		{
			slaveRamBuff[0] = 0xffff0000;
			slaveRamBuff[1] = 0x88888881;    
			DrvI2cEepromWrite((uint8_t *)&slaveRamBuff[0],8,&i2cM2eer);
			mode = 2;
		}
		/*测试往eeprom中读数据*/
		else if (mode == 3)
		{
			slaveRamBuff[0] = 0;
			slaveRamBuff[1] = 0;
			DrvI2cEepromRead((uint8_t *)&slaveRamBuff[0],8,&i2cM2eer);
			mode = 0;
		}
    }  
}

/**
  * @brief  I2C例程测试单独测试自身作为从设备
  * @param  testI2cFlag  改变该值可以测试不同的应用场景：1、写数据  2、读数据  3、检验读写数据
            该值配置1~4 
  * @retval none                  
  */
void TestDrvI2cModeSletM2S(uint32_t testI2cFlag)
{	
	slaveRamAddr = 0x20004000;
	slaveRamBuff[0] = 0x01234567;
	slaveRamBuff[1] = 0x89ABCDEF;
	slaveRamBuff[2] = 0x11223344;
	slaveRamBuff[3] = 0x55667788;
	slaveRamBuff[4] = 0x99AABBCC;
	slaveRamBuff[5] = 0xDDEEFF00;        
	slaveRamByteLen = 0x18;

    uint32_t mode = testI2cFlag;
    /*单独测试控制自身为I2C从设备时读写数据*/
    while (mode)
    {//slave
        if (mode == 1)
        {
            DrvI2cSlaveCtrlWrite(slaveRamAddr, (uint8_t *)slaveRamBuff,slaveRamByteLen,&i2cM2s);
        }
        else if (mode == 2)
        {
            DrvI2cSlaveCtrlRead(slaveRamAddr, (uint8_t *)slaveRamBuff, slaveRamByteLen,&i2cM2s);
        }
        else if (mode == 3)
        {
            TestDrvI2CReadCheck(slaveRamAddr, (uint32_t)&slaveRamBuff[0],(slaveRamByteLen>>2));
        }
        mode = 0;
    }
    return; 
}

/**
  * @brief  比较数据1与数据2是否相同
  * @param  addr1  数据1的值
  * @param  addr2  数据2的值
  * @param  len  比较的个数
  * @retval none                  
  */
void TestDrvI2CReadCheck(uint32_t addr1, uint32_t addr2, uint32_t len)
{
    uint32_t *pAddr1 = (uint32_t *)addr1;
    uint32_t *pAddr2 = (uint32_t *)addr2;
    while(len--)
    {
        if (*pAddr1++ != *pAddr2++)
        {
            __breakpoint(0);
        }
    }
}

/**
  * @}
  */

/******************************************************************************/

#endif
