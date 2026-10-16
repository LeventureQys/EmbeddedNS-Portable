
#include "a0_hal_conf.h"
#include "UserFunction.h"
#include "drv_uac10.h"
#include "lib_drv_uac10.h"
#include "a0_hal_otg.h"
#include "a0_hal_spi_flash.h"
#include "a0_hal_def.h"
#include "drv_alg.h"
#include "para_default_filter.h"

#pragma anon_unions

#define USER_INFO_ADDR_OFFSET       (0x0001F000)
#define USER_INFO_WORD_SIZE         (64)            // x4 byte
#define USER_DEV_VER                (0x0001)

extern void AppCodecMicBoostCfg(uint8_t boostEn);
extern void DrvAlgUserParaInit(uint32_t idx, uint16_t *pPara);



typedef struct
{
    union {
        struct{
            uint16_t biSerialNumberEn:1;
            uint16_t biProductEn:1;
            uint16_t biManufacturerEn:1;
            uint16_t bVIDPIDEn:1;
            uint16_t bVolumeRangeEn:1;
            uint16_t bConfigEn:1;
            uint16_t bMicEqEn:1;
            uint16_t bSpkEqEn:1;
            uint16_t bID:8;
        };
        uint16_t ctrlState;
    };
    uint16_t spkVolMin;
    uint16_t spkVolMax;
    uint16_t spkVolDef;
    uint16_t micVolMin;
    uint16_t micVolMax;
    uint16_t micVolDef;
    uint16_t sidetoneVolMin;
    uint16_t sidetoneVolMax;
    uint16_t sidetoneVolDef;
    union{
        struct{
            uint16_t bReserved:4;
            uint16_t bSelSidetone:1;        // 1, sidetone enable, 0, sidetone disable
            uint16_t bSelFSType:1;          // 1, 44.1/48k, 0, 44.1/48/96/192k
            uint16_t bSelANS:1;             // 1, ans, 0, reverb      
            uint16_t bSelPotentiometer:1;   // 0, potentiometer, 1, encoder
            uint16_t bOutputTerminalType:2;
        };
        uint16_t cfgInfo;
    };
    uint16_t reserved1[3];
    uint16_t userVID;
    uint16_t userPID;
    uint16_t adcEqPara[16];
    uint16_t dacEqPara[16];
    uint16_t iSerialNumber[16];
    uint16_t iProduct[32];
    uint16_t iManufaturer[32];    
}user_info_t;


user_info_t userInfo;
uint32_t userInfoAll[USER_INFO_WORD_SIZE];
// if for 5seg eq 5*4+1 = 21
//uint32_t TempEqPara[21];
// if for 7 seg 7*4 + 1 = 29
uint32_t TempEqPara[29];

uint32_t SpkTempEqPara[29];
uint8_t dspAlgAdcEqRefreshEn = 0;
uint8_t dspAlgDacEqRefreshEn;
void AppAlgUserParaSet(uint16_t idx, uint16_t *pPara)
{  
    return;//Niko Bootloader
    if (idx == 0)
    { 
        // adc eq para
        uint8_t *src = (uint8_t *)&paraDefaultAdcEq[0][0];
        uint8_t *dst = (uint8_t *)&TempEqPara[1];
        // Calculate the total size of data to copy
        size_t size = sizeof(ALG_PARA_FILTER) * 5;  
//		size_t size = sizeof(ALG_PARA_FILTER) * 7;
        for (uint32_t i = 0; i < 7; i++)
        {
            uint16_t info = *pPara++;
            paraDefaultAdcEq[0][i].type = (uint32_t)((uint8_t)info);
            paraDefaultAdcEq[0][i].gain = (float_t)((int32_t)((int8_t)(info >> 8)));
            info = *pPara++;
            paraDefaultAdcEq[0][i].fc = (float_t)((int32_t)((int16_t)info));
            info = *pPara++;
            paraDefaultAdcEq[0][i].q = (float_t)((int32_t)((int16_t)info)) / 10.0f;
        }
        TempEqPara[0] = idx;
        // Copy the data byte by byte
        for (size_t i = 0; i < size; i++)
        {
            *dst++ = *src++;
        }      
        LibDrvMemcpy(dst, src, size);        
        DrvAlgParaSet(DRV_ALG_IDX_EQ,(uint32_t *)&TempEqPara);
    }
	else if (idx == 1)
	{
//		// dac out eq para
//        uint8_t *src = (uint8_t *)&paraDefaultDacEq[0][0];
//        uint8_t *dst = (uint8_t *)&TempEqPara[1];
//        // Calculate the total size of data to copy
//        size_t size = sizeof(ALG_PARA_FILTER) * 5;  
//        // dac eq para
//        for (uint32_t i = 0; i < 5; i++)
//        {
//            uint16_t info = *pPara++;
//            paraDefaultDacEq[0][i].type = (uint32_t)((uint8_t)info);
//            paraDefaultDacEq[0][i].gain = (float_t)((int32_t)((int8_t)(info >> 8)));
//            info = *pPara++;
//            paraDefaultDacEq[0][i].fc = (float_t)((int32_t)((int16_t)info));
//            info = *pPara++;
//            paraDefaultDacEq[0][i].q = (float_t)((int32_t)((int16_t)info)) / 10.0f;
//        }  
//        
//        TempEqPara[0] = idx;
//        // Copy the data byte by byte
//        for (size_t i = 0; i < size; i++)
//        {
//            *dst++ = *src++;
//        }      
//        LibDrvMemcpy(dst, src, size);        
//        DrvAlgParaSet(DRV_ALG_IDX_EQ,(uint32_t *)&TempEqPara);
//        uint8_t *src1 = (uint8_t *)&paraDefaultSpkEq[0][0];
//        uint8_t *dst1 = (uint8_t *)&SpkTempEqPara[1];
//        // Calculate the total size of data to copy
//        size = sizeof(ALG_PARA_FILTER) * 7;  
//        // dac eq para
//        for (uint32_t i = 0; i < 7; i++)
//        {
//            uint16_t info = *pPara++;
//            paraDefaultSpkEq[0][i].type = (uint32_t)((uint8_t)info);
//            paraDefaultSpkEq[0][i].gain = (float_t)((int32_t)((int8_t)(info >> 8)));
//            info = *pPara++;
//            paraDefaultSpkEq[0][i].fc = (float_t)((int32_t)((int16_t)info));
//            info = *pPara++;
//            paraDefaultSpkEq[0][i].q = (float_t)((int32_t)((int16_t)info)) / 10.0f;
//        }  
//        
//        TempEqPara[0] = idx;
//        // Copy the data byte by byte
//        for (size_t i = 0; i < size; i++)
//        {
//            *dst1++ = *src1++;
//        }      
//        LibDrvMemcpy(dst1, src1, size);        
//        DrvAlgParaSet(DRV_ALG_IDX_EQ,(uint32_t *)&SpkTempEqPara);
	}


}


// 接收处理PC配置工具下发的数据，并烧写到Flash
uint32_t UsbUserInfoHandler(uint32_t wLength, uint32_t *pData)
{
    uint32_t idx = *pData++;
    uint32_t *pDst = (uint32_t *)&userInfoAll;
    switch (idx)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        {
            if (wLength != 15)
            {
                return 1;
            }
            pDst += (14 * idx);
            LibDrvMemcpy(pDst, pData, (wLength - 1) * 4);
            break;
        }
        case 4:
        {
            if (wLength != 9)
            {
                return 1;
            }
            pDst += (14 * idx);
            LibDrvMemcpy(pDst, pData, (wLength - 1) * 4);
            break;
        }
        case 0xFFFF0000:
        {
            if (wLength != 2)
            {
                return 1;
            }
            uint32_t infoDataChecksum = *pData;
            uint32_t userInfoAllChecksum = 0;
            for (uint32_t i = 0; i < USER_INFO_WORD_SIZE; i++)
            {
                userInfoAllChecksum += userInfoAll[i];
            }
            if (infoDataChecksum != userInfoAllChecksum)
            {
                *pData = userInfoAllChecksum;
                return 2;
            }
            break;
        }
        case 0xFFFF0001:
        {
            uint32_t argv[3] = {
                USER_INFO_ADDR_OFFSET,
                0x100,
                0,
            };
            OTG_IRQ_DISABLE();
            NVIC_DisableIRQ(DMA1_IRQn);
            HalFlashErase(argv);
            HAL_TickDelay(10);
            argv[2] = (uint32_t)&userInfoAll[0];
            HalFlashWrite(argv);
            HAL_TickDelay(10);
            uint32_t *pSrc = (uint32_t *)argv[2];
            uint32_t *pDst = (uint32_t *)argv[0];
            for (uint32_t i = 0; i < USER_INFO_WORD_SIZE; i++)
            {
                uint32_t dataSrc = *pSrc++;
                uint32_t dataDst = *pDst++;
                if (dataSrc != dataDst)
                {
                    NVIC_EnableIRQ(DMA1_IRQn);
                    OTG_IRQ_ENABLE();
                    return 2;
                }
            }
            NVIC_EnableIRQ(DMA1_IRQn);
            OTG_IRQ_ENABLE();
            break;
        }
        case 0xFFFF0002:
        {
            user_info_t *pInfoSrc = (user_info_t *)userInfoAll;
            user_info_t *pInfoDst = (user_info_t *)&userInfo;
            LibDrvMemcpy(pInfoDst, pInfoSrc, sizeof(user_info_t));
            if (pInfoDst->bMicEqEn != 0)
            {
                AppAlgUserParaSet(0, &pInfoDst->adcEqPara[0]);
            }
            if (pInfoDst->bSpkEqEn != 0)
            {
                AppAlgUserParaSet(1, &pInfoDst->dacEqPara[0]);
            }
            break;
        }
    }
    return 0;
}

// 该初始化需要在codec/i2s初始化之后，usb以及算法初始化之前执行
void UsbUserInfoInit(void)
{
    user_info_t *pInfoRom = (user_info_t *)USER_INFO_ADDR_OFFSET;
    user_info_t *pInfoRam = (user_info_t *)&userInfo;
    memcpy(pInfoRam, pInfoRom, sizeof(user_info_t));
    if (pInfoRam->bID == 0x32)
    {
        if (pInfoRam->biSerialNumberEn != 0)
        {
            LibDrvUac10DscStringConfig((uint32_t)&pInfoRam->iSerialNumber, USB_STR_SERIALNUMBER_INDEX);
        }
        if (pInfoRam->biProductEn != 0)
        {
            LibDrvUac10DscStringConfig((uint32_t)&pInfoRam->iProduct, USB_STR_PRODUCT_INDEX);
        }
        if (pInfoRam->biManufacturerEn != 0)
        {
            LibDrvUac10DscStringConfig((uint32_t)&pInfoRam->iManufaturer, USB_STR_MANUFACTURER_INDEX);
        }
        if (pInfoRam->bVIDPIDEn != 0)
        {
            LibDrvUac10PidVidConfig(pInfoRam->userPID, pInfoRam->userVID, USER_DEV_VER);     //pid, vid
        }
        if (pInfoRam->bVolumeRangeEn != 0)
        {
            DrvUac10VolRangeConfig(USB_PARA_IDX_SPK, pInfoRam->spkVolMin, pInfoRam->spkVolMax, pInfoRam->spkVolDef);
            DrvUac10VolRangeConfig(USB_PARA_IDX_MIC, pInfoRam->micVolMin, pInfoRam->micVolMax, pInfoRam->micVolDef);
            DrvUac10VolRangeConfig(USB_PARA_IDX_SIDETONE, pInfoRam->sidetoneVolMin, pInfoRam->sidetoneVolMax, pInfoRam->sidetoneVolDef);
        }
        if (pInfoRam->bMicEqEn != 0)
        {
//            DrvAlgUserParaInit(0, &pInfoRam->adcEqPara[0]);
        }
        if (pInfoRam->bSpkEqEn != 0)
        {
//            DrvAlgUserParaInit(1, &pInfoRam->dacEqPara[0]);
        }
        if (pInfoRam->bConfigEn == 0)
        {
            // sidetone 默认打开
            pInfoRam->bSelSidetone = 1;
            // 采样率配置，默认1，44.1/48
            pInfoRam->bSelFSType = 1;
            // 电位器、编码器选择按照默认值，选择电位器
            pInfoRam->bSelPotentiometer = 0;
            // 降噪、混响选择按照默认值，选择降噪
            pInfoRam->bSelANS = 1;
            // 终端类型按照默认值
            pInfoRam->bOutputTerminalType = 0;
        }
    }
    else
    {
        // sidetone 默认打开
        pInfoRam->bSelSidetone = 1;
        // 采样率配置，默认1，44.1/48
        pInfoRam->bSelFSType = 1;
        // 电位器、编码器选择按照默认值，选择电位器
        pInfoRam->bSelPotentiometer = 0;
        // 降噪、混响选择按照默认值，选择降噪
        pInfoRam->bSelANS = 1;
        // 终端类型按照默认值
        pInfoRam->bOutputTerminalType = 0;
    }
    return;
}

__RAM_CODE uint32_t UsbUserInfoTerminalTypeGet(void)
{
    user_info_t *pInfoRam = (user_info_t *)&userInfo;
    if (pInfoRam->bOutputTerminalType == 2)
    {
        return UAC_TT_OUTPUT_HEADPHONES;
    }
    else if (pInfoRam->bOutputTerminalType == 1)
    {
        return UAC_TT_OUTPUT_SPEAKER;
    }
    else //if (pInfoRam->bOutputTerminalType == 0)
    {
        return UAC_TT_HEADSET;
    }
}

// return: 1, ans, 0, reverb
__RAM_CODE uint32_t UsbUserInfoSelectAlgGet(void)
{
    user_info_t *pInfoRam = (user_info_t *)&userInfo;
    return pInfoRam->bSelANS;
}

// return: 1, potentiometer, 0, encoder
__RAM_CODE uint32_t UsbUserInfoSelectCtrlGet(void)
{
    user_info_t *pInfoRam = (user_info_t *)&userInfo;
    return pInfoRam->bSelPotentiometer;
}

// return: 1, 44.1/48,  0, 44.1/48/96/192
__RAM_CODE uint32_t UsbUserInfoSelectFsTypeGet(void)
{
    user_info_t *pInfoRam = (user_info_t *)&userInfo;
    return pInfoRam->bSelFSType;
}

// return: 1, sidetone enable, 0, sidetone disable
__RAM_CODE uint32_t UsbUserInfoSelectSidetoneGet(void)
{
    user_info_t *pInfoRam = (user_info_t *)&userInfo;
    return pInfoRam->bSelSidetone;
}

