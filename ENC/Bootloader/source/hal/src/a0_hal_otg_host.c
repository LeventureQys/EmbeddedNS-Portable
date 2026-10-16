/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_otg_host.c
 * @brief   OTG Host HAL module.
 * @version V1.0.3
 * @date    2022/05/27
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.3, 2022/05/27
 *      1. Doxygen更新，部分需要对齐的结构体，改为头文件中定义，这样在使用的时候避免出现遗忘对齐的情况。
 *      2. 枚举过程中，描述符支持的长度，使用a0_hal_conf.h中的定义
 * V1.0.2, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.1, 2021/10/11,
 *     1. 优化调整，头文件中结构体定义为了让Doxygen正常识别，将__ALIGNED从typedef struct中去掉，加到变量声明中去进行修饰。
 * V1.0.0, 2021/07/14,
 *     1. 在例程基础上整理的Host部分代码
 */
/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup OTG_HOST
 * @{
 */

#ifdef HAL_OTG_HOST_MODULE_ENABLED
#include "a0_hal_otg_host.h"
#include "a0_hal_otg.h"
#include "a0_hal_sys_ctrl.h"
#include "a0_hal_def.h"
#ifdef USB_MALLOC_EN
#include <stdlib.h>
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#ifndef HAL_OTG_PRINT_EN
#define USB_PRINT(...)
#define USB_SCAN(...)
#else
#define USB_PRINT(...)  DBG_PRINT(__VA_ARGS__)
#define USB_SCAN(...)   DBG_SCAN(__VA_ARGS__)
#endif

/* Private variables ---------------------------------------------------------*/
#ifndef USB_MALLOC_EN
static uint8_t otgDscCfgTemp[0x400];
#endif

/* Private constants ---------------------------------------------------------*/
static const uint8_t otgHCmdGetDscDev8[]    = {0x80,0x06,0x00,0x01,0x00,0x00,0x08,0x00};
static const uint8_t otgHCmdGetDscDev[]     = {0x80,0x06,0x00,0x01,0x00,0x00,0x12,0x00};
static const uint8_t otgHCmdGetDscCfg[]     = {0x80,0x06,0x00,0x02,0x00,0x00,0x08,0x00};
static const uint8_t otgHCmdSetAddr[]       = {0x00,0x05,0x03,0x00,0x00,0x00,0x00,0x00};
static const uint8_t otgHSetCfg[]           = {0x00,0x09,0x01,0x00,0x00,0x00,0x00,0x00};
static const uint8_t otgHGetDscStr70[]      = {0x80,0x06,0x70,0x03,0x00,0x00,0x06,0x00};

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief  host asynchronous schedule enable or disable control
 * @param  option      'OPTION_DISABLE' for disable, 'OPTION_ENABLE' for enable
 * @return NONE
 */
static void HalOtgHostAsynchronousScheduleSetting(uint8_t option)
{
    if (option != 0)
    {
        if (OTGHC_ASCH_CHECK() == 0)
        {
            OTGHC_ASCH_EN();
            while(OTGHC_ASCH_CHECK() == 0);
        }
    }
    else
    {
        if (OTGHC_ASCH_CHECK() != 0)
        {
            OTGHC_ASCH_DIS();
            while(OTGHC_ASCH_CHECK() != 0);
        }
    }
    return;
}
/**
 * @brief  host stop or run enable or disable control
 * @param  option      'OPTION_DISABLE' for stop, 'OPTION_ENABLE' for run
 * @return NONE
 */
static void HalOtgHostStopRunSetting(uint8_t option)
{
    if (option != 0)
    {
        if (OTGHC_RUN_CHECK() == 0)
        {
            OTGHC_RUN();
            while(OTGHC_RUN_CHECK() == 0);
        }
    }
    else
    {
        if (OTGHC_RUN_CHECK() != 0)
        {
            if (OTGHC_ASCH_CHECK() != 0)
            {
                OTGHC_ASCH_DIS();
                while(OTGHC_ASCH_CHECK() != 0);
            }
            if (OTGHC_PSCH_CHECK() != 0)
            {
                OTGHC_PSCH_DIS();
                while(OTGHC_PSCH_CHECK() != 0);
            }
            OTGHC_STOP();
            while(OTGHC_RUN_CHECK() != 0);
        }
    }
    return;
}
/**
 * @brief  host suspend, stop and disable ASCH and PSCH
 * @return NONE
 */
static void HalOtgHostSuspend(void)
{
    if (OTGHC_RUN_CHECK() != 0)
    {
        if (OTGHC_PORT_EN_CHECK() != 0)
        {
            if (OTGHC_ASCH_EN_CHECK() != 0)
            {
                OTGHC_ASCH_DIS();
                while(OTGHC_ASCH_CHECK() != 0);
            }
            if (OTGHC_PSCH_EN_CHECK() != 0)
            {
                OTGHC_PSCH_DIS();
                while(OTGHC_PSCH_CHECK() != 0);
            }
            HalOtgHostStopRunSetting(OPTION_DISABLE);
            OTGHC_PORT_SUSPEND_SET();
            while (OTGHC_PORT_SUSPEND_CHECK() == 0);
        }
    }
}
/**
 * @brief  host close, control to suspend host, disable int and clear int
 * @return NONE
 */
static int32_t HalOtgHostClose(void)
{
    if (OTGHC_INT_GET() != 0)
    {
        //flib_Host20_Suspend();
        HalOtgHostSuspend();
        //disable int
        OTGHC_INT_SET(0);
        //clear int
        OTGHC_USBSTS_SET(OTGHC_USBSTS_RD() & 0x3F);
    }
    return 0;
}

/**
 * @brief  host release structure of qTD, iTD, siTD, or page 4K memory
 * @param  pPara       pointer to structure @ref otg_host_para_t
 * @param  pQTDHead    pointer to structure @ref otg_host_qTD_t
 * @param  pQHDTemp    pointer to structure @ref otg_host_qTD_t
 * @param  timeOutMs   time out value in milliseconds
 * @retval == 0        OK
 * @retval == 1        time out error
 */
static int32_t HalOtgHostSendQTD(otg_host_para_t *pPara, otg_host_qTD_t *pQTDHead, otg_host_qHD_t *pQHDTemp, uint32_t timeOutMs)
{
    otg_host_qTD_t *pQTDOldDumy;
    otg_host_qTD_t *pQTDNewDumy;
    otg_host_qTD_t *pQTDLast;
    otg_host_qTD_t *pQTDRelease;
    otg_host_qTD_t *pQTDReleaseNext;

    int32_t result = 0;
    /* <1>. copy head-qTD to olddumy-qTD */
    pQTDOldDumy = (otg_host_qTD_t *)(OTG_S_ADDR_RETURN((uint32_t)(pQHDTemp->bOverlay_NextqTD)));
    //pQTDOldDumy = (otg_host_qTD_t *)((((uint32_t)(pQHDTemp->bOverlay_NextqTD)) << 5) | 0x10000000);
    memcpy(pQTDOldDumy, pQTDHead, sizeof(otg_host_qTD_t));
    /* <2>. prepare new dumy qTD */
    pQTDNewDumy = pQTDHead;
    pQTDNewDumy->bTerminate = 1;
    /* <3>. find last-qTD, link to newdumy-qTD, and set newdumy-qTD->T=1 */
    pQTDLast = pQTDOldDumy;
    while (pQTDLast->bTerminate == 0)
    {
        pQTDLast = (otg_host_qTD_t *)(OTG_S_ADDR_RETURN((uint32_t)(pQTDLast->bNextQTDPointer)));
        //pQTDLast = (otg_host_qTD_t *)((((uint32_t)(pQTDLast->bNextQTDPointer)) << 5) | 0x10000000);
    }
    pQTDLast->bNextQTDPointer = OTG_S_ADDR_ADJ((uint32_t)pQTDNewDumy);
    //pQTDLast->bNextQTDPointer = ((((uint32_t)pQTDNewDumy) & 0x2FFFFFFF) >> 5);
    pQTDLast->bTerminate = 0;
    /* link alternate qTD pointer */
    pQTDLast->bAlternateQTDPointer = OTG_S_ADDR_ADJ((uint32_t)pQTDNewDumy);
    //pQTDLast->bAlternateQTDPointer = ((((uint32_t)pQTDNewDumy) & 0x2FFFFFFF) >> 5) ;
    pQTDLast->bAlternateTerminate = 0;
    /* <4>. set olddumy-qTD->active=1 */
    pPara->pAttachDevice->pQTDSendLast = pQTDLast;
    pPara->pAttachDevice->bSendStatusError = 0;
    pQTDOldDumy->bStatus_Active = 1;
    /* <5>. wait for result after IRQ handler */
    uint32_t startTick = HalOtgHostTickGet();
    uint32_t waitCnt = 0;
    do {
        waitCnt++;
        waitCnt &= 0x7;
        if (waitCnt == 0)
        {
            if ((((volatile otg_host_qTD_t *)pQTDLast)->bStatus_Active) == 0)
            {
                break;
            }
            if ((HalOtgHostTickGet() - startTick) > timeOutMs)
            {
                result = 1;
//                USB_PRINT("Send qTD timeout\n");
                break;
            }
        }
    } while(1);
    /* <6>. check the result */
    if (pPara->pAttachDevice->bSendStatusError != 0)
    {
        pPara->pAttachDevice->bSendStatusError = 0;
        pQHDTemp->bOverlay_Status = 0;
        /* if user want to get info, add code here to judge the error status */
        /*
            #define HOST20_qTD_STATUS_Active            0x80
            #define HOST20_qTD_STATUS_Halted            0x40
            #define HOST20_qTD_STATUS_BufferError       0x20
            #define HOST20_qTD_STATUS_Babble            0x10
            #define HOST20_qTD_STATUS_TransactionError  0x08
            #define HOST20_qTD_STATUS_MissMicroFrame    0x04
            #define HOST20_qTD_STATUS_Split             0x02
            #define HOST20_qTD_STATUS_Ping              0x01
        */
    }
    /* <7>. release all the qTD except newdumy-qTD */
    pQTDRelease = pQTDOldDumy;
    do {
        pQTDReleaseNext = (otg_host_qTD_t *)(OTG_S_ADDR_RETURN((uint32_t)(pQTDRelease->bNextQTDPointer)));
        //pQTDReleaseNext = (otg_host_qTD_t *)((((uint32_t)(pQTDRelease->bNextQTDPointer)) << 5) | 0x10000000);
        HalOtgHostStructureRelease(pPara, OTG_HOST_MEM_TYPE_QTD, (uint32_t)pQTDRelease);
        pQTDRelease = pQTDReleaseNext;
    } while (((uint32_t)pQTDRelease) != ((uint32_t)pQTDNewDumy));
    return result;
}

/**
 * @brief  host issue control, standard cmd
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  pCtrlPara       pointer to structure, @ref otg_host_ctrl_para_t
 * @retval == 0            OK
 * @retval == 1            time out error
 * @retval == 2            cmd is not supported
 */
static int32_t HalOtgHostIssueControl(otg_host_para_t *pPara, otg_host_ctrl_para_t *pCtrlPara)
{
    int32_t result = 2;
    /* analysis the controller cmomand */
    switch (*(pCtrlPara->pbCmd + 1))
    {
        /* Case-1: 'Setup/In/Out' */
        case 0:         // get status
        case 6:         // get descriptor
        case 8:         // get configuration
        case 10:        // get interface
        {
            result = HalOtgHostIssueControlBase(pPara, pCtrlPara, 1);
            break;
        }
        /* Case-2:'Setup/In' */
        case 1:         // clear feature
        case 3:         // set feature
        case 5:         // set address
        case 9:         // set configuration
        case 11:        // set interface
        {
            result = HalOtgHostIssueControlBase(pPara, pCtrlPara, 2);
            break;
        }
        /* Case-3:'Setup/Out/In' */
        case 7:         // set descriptor
        {
            result = HalOtgHostIssueControlBase(pPara, pCtrlPara, 0);
            break;
        }
        default:
            break;
    }
    return result;
}

/* Exported functions --------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief  host structure initial
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval != 0            Error
 */
__WEAK int32_t HalOtgHostStructureInit(otg_host_para_t *pPara)
{
    return 1;
}

/**
 * @brief  host configuration descriptor analysis
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  pbData
 * @retval == 0            OK
 * @retval != 0            Error
 */
__WEAK int32_t HalOtgHostAnalysisConfiguration(otg_host_para_t *pPara, uint8_t *pbData)
{
    return 1;
}

/**
 * @brief  host enumerate information interface of class respond
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval != 0            Error
 */
__WEAK int32_t HalOtgHostEnumerateInterfaceClass(otg_host_para_t *pPara)
{
    return 1;
}

/**
 * @brief  host interrupt isr process, update periodic schedule
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval != 0            Error
 */
__WEAK int32_t HalOtgHostInterruptIsr(otg_host_para_t *pPara)
{
    return 1;
}

/**
 * @brief  host iso isr process, update periodic schedule
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @return NONE
 */
__RAM_CODE __WEAK void HalOtgHostIsoIsr(otg_host_para_t *pPara)
{
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief  host irq process, called in irq handler, the code of this function is assigned in sram to run
 * @param  pPara       pointer to structure @ref otg_host_para_t
 * @return NONE
 */
__RAM_CODE void HalOtgHostIrq(otg_host_para_t *pPara)
{
    uint32_t intStatus = OTGC_INT_STATE_RD();
    if (intStatus != 0)
    {
        /* clear otg ctrl int */
        OTGC_INT_STATE_CLR(intStatus);
        #if 0
        intStatus &= OTGC_INT_EN_RD();
        if (intStatus != 0)
        {
            /* otg ctrl int isr */
            if (intStatus & OTGC_A_VBUSERR)
            {
                USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
                __breakpoint(0);
            }
            if (intStatus & OTGC_A_OVC)
            {
                USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
                __breakpoint(0);
            }
        }
        #endif
    }

    /* read interrupt status */
    intStatus = OTGHC_USBSTS_RD();
    /* check for error type */
    if (intStatus & OTGHC_INT_SYS_ERR)
    {
        /* system error, halt the system */
        HalOtgHostClose();
        HalOtgHostInit();
        USB_PRINT("OTG Host system error, halt and reinit");
    }
    if (intStatus & OTGHC_INT_USB_ERR)
    {
        OTGHC_INT_USBERR_CLR();
        pPara->pAttachDevice->bSendStatusError = 1;
    }
    /* check port change status */
    if (intStatus & OTGHC_INT_PORT_CHG)
    {
        /* Process event resume, and clear status */
        if (OTGHC_FORCE_PORT_RESUME_CHECK() != 0)
        {
            pPara->pAttachDevice->bRemoteWakeUpDetected = 1;
        }
        /* process event connect status change, and clear status */
        if (OTGHC_CONNECT_CHANGE_CHECK() != 0)
        {
            OTGHC_CONNECT_CHANGE_CLR();
            pPara->pAttachDevice->bConnectStatus = 1;
            if (OTGHC_CONNECT_CHECK() == 0)
            {
                USB_PRINT("Device Remove\n");
                HalOtgHostStopRunSetting(OPTION_DISABLE);
                pPara->pAttachDevice->bEnumerated = 0;
            }
            else
            {
                USB_PRINT("Device Insert\n");
            }
        }
        /* process event port enable/disable change, and clear status */
        if (OTGHC_PORT_EN_DIS_CHANGE_CHECK() != 0)
        {
            OTGHC_PORT_EN_DIS_CHANGE_CLR();
        }
        /* process event port reset */
        if (pPara->pAttachDevice->bPortReset != 0)
        {
            if (OTGHC_PORT_RESET_CHECK() == 0)
            {
                pPara->pAttachDevice->bPortReset = 0;
            }
        }
        if (pPara->pAttachDevice->bSuspend == 0)
        {
            if (OTGHC_PORT_SUSPEND_CHECK() != 0)
            {
                pPara->pAttachDevice->bSuspend = 1;
            }
        }
        /* clear status of port status */
        OTGHC_INT_PORT_CHG_CLR();
    }
    /* check complete interrupt */
    if (intStatus & OTGHC_INT)
    {
        OTGHC_INT_CLR();
        if (pPara->pAttachDevice->bISOTransferEnable & (BIT1|BIT2))
        {
            HalOtgHostIsoIsr(pPara);
        }
    }

    /* check for framelist = 0 interrupt */
    if (intStatus & OTGHC_INT_FRL_ROL)
    {
        OTGHC_INT_FRL_ROL_CLR();
        pPara->pAttachDevice->bFrameList0Int++;
    }
    return;
}

/**
 * @brief  host module registers initial
 * @return NONE
 */
void HalOtgHostInit(void)
{
    HAL_SYS_USB_ID_CLR();
    HAL_SYS_USB_VBUS_EN();

    /* disable system irq first */
    OTG_IRQ_DISABLE();

    /* write 1 to clear interrupt flag */
    OTGC_INT_STATE_CLR(OTGC_INT_STATE_RD());
    /* disable all interrupt */
    OTGC_INT_EN_CLR(0xFFFFFFFF);
    /* enable the interrupt of usb as device mode */
    OTGC_INT_EN_SET(OTGC_INT_A_TYPE);

    OTG_DEV_CHIP_EN();

    OTGHC_RESET_SET();
    while(OTGHC_RESET_CHECK() != 0);

    HalOtgHostTickDelay(10);

    OTGC_A_BUS_DROP_CLR();
    OTGC_A_BUS_REQ_SET();

    OTGHC_INT_SET(OTGHC_INT_SYS_ERR|OTGHC_INT_PORT_CHG|OTGHC_INT_USB_ERR|OTGHC_INT);

    /* enable system irq at last */
    OTG_IRQ_ENABLE();

    return;
}

/**
 * @brief  host issue control, standard cmd
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval == 1            time out error
 * @retval == 2            cmd is not supported
 */
int32_t HalOtgHostConnectedInit(otg_host_para_t *pPara)
{
    /* disable asynchronous and periodic schedule */
    HalOtgHostAsynchronousScheduleSetting(OPTION_DISABLE);
    HalOtgHostPeriodicScheduleSetting(OPTION_DISABLE);

    /* get speed */
    pPara->pAttachDevice->bSpeed = OTGC_SPEED_RD();
    /* init configuration */
    OTGHC_PK_CNT_SET(1);
    OTGHC_EOF1_TIME_SET(3);
    OTGHC_EOF1_TIMEING_UNUSE();
    /* write base address register */
    OTGHC_ASYNC_ADDR_SET((uint32_t)pPara->pQHD);
    /* enable interrupts */
    OTGHC_INT_SET(OTGHC_INT_SYS_ERR|OTGHC_INT_PORT_CHG|OTGHC_INT_USB_ERR|OTGHC_INT);

    /* port bus reset */
    /* stop */
    HalOtgHostStopRunSetting(OPTION_DISABLE);
    /* port disable */
    OTGHC_PORT_DIS();
    /* port reset = 1 */
    OTGHC_PORT_RESET_SET();
    /* wait time 55ms */
    HalOtgHostTickDelay(55);
    /* port reset = 0 */
    pPara->pAttachDevice->bPortReset = 1;
    OTGHC_PORT_RESET_CLR();
    /* run */
    HalOtgHostStopRunSetting(OPTION_ENABLE);

    /* check 3 times for the device connection */
    uint8_t checkTime = 3;
    while (1)
    {
        if (OTGHC_CONNECT_CHECK() != 0)
        {
            pPara->pAttachDevice->bSpeed = OTGC_SPEED_RD();
            break;
        }
        else
        {
            checkTime--;
        }
        if (checkTime == 0)
        {
            return 1;
        }
        HalOtgHostTickDelay(1);
    }

    /* init all the data structure */
    if (HalOtgHostStructureInit(pPara) != 0)
    {
        return 1;
    }

    /* init qHD control */
    /* init control 0/1 */
    pPara->pQHDListControl[0]->bEdSpeed = pPara->pAttachDevice->bSpeed;
    pPara->pQHDListControl[0]->bInactiveOnNextTransaction = 0;
    pPara->pQHDListControl[0]->bDataToggleControl = 1;
    pPara->pQHDListControl[1]->bEdSpeed = pPara->pAttachDevice->bSpeed;
    pPara->pQHDListControl[1]->bInactiveOnNextTransaction = 0;
    pPara->pQHDListControl[1]->bDataToggleControl = 1;
    /* init bulk 0/1 */
    pPara->pQHDListBulk[0]->bEdSpeed = pPara->pAttachDevice->bSpeed;
    pPara->pQHDListBulk[0]->bInactiveOnNextTransaction = 0;
    pPara->pQHDListBulk[0]->bDataToggleControl = 0;
    pPara->pQHDListBulk[1]->bEdSpeed = pPara->pAttachDevice->bSpeed;
    pPara->pQHDListBulk[1]->bInactiveOnNextTransaction = 0;
    pPara->pQHDListBulk[1]->bDataToggleControl = 0;
    /* enable asynchronous */
    OTGHC_ASCH_EN();

    return 0;
}

/**
 * @brief  host enumerate process
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  bAddress        device address
 * @retval == 0            OK
 * @retval != 0            Error
 */
int32_t HalOtgHostEnumerate(otg_host_para_t *pPara, uint8_t bAddress)
{
    uint8_t bCMD[8];
    pPara->pAttachDevice->bISOTransferEnable = 0;

    /* <1>. GetDescriptor Device (8Bytes) */
    memcpy(bCMD, otgHCmdGetDscDev8, 8);
    otg_host_ctrl_para_t otgCtrlPara = {
        .bEdNum = 0,
        .hwDataSize = 8,
        .pbCmd = bCMD,
        .pbData = (uint8_t *)&pPara->pAttachDevice->deviceDescriptor,
    };
    /* otgCtrlPara = {0, 0x08, bCMD, pPara->pAttachDevice->deviceDescriptor} */
    int32_t result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
    if (result != 0)
    {
        return result;
    }
    HalOtgHostTickDelay(100);

    /* set the ep0 max packet size */
    pPara->pQHDListControl[0]->bMaxPacketSize = pPara->pAttachDevice->deviceDescriptor.bMaxPacketSize0;
    pPara->pQHDListControl[1]->bMaxPacketSize = pPara->pAttachDevice->deviceDescriptor.bMaxPacketSize0;

    /* <3>. SetAddress to i */
    memcpy(bCMD, otgHCmdSetAddr, 8);
    pPara->pAttachDevice->bAddress = bAddress;
    pPara->pQHDListControl[1]->bDeviceAddress = bAddress;       //update ctrl[1] address
    pPara->pQHDListBulk[0]->bDeviceAddress = bAddress;          //update bulk[0]/bulk[1] address
    pPara->pQHDListBulk[1]->bDeviceAddress = bAddress;
    pPara->pQHDListInt[0]->bDeviceAddress = bAddress;           //update int[0]/int[1] address
    pPara->pQHDListInt[1]->bDeviceAddress = bAddress;
    bCMD[2] = bAddress;
    /* otgCtrlPara = {0, 0, bCMD, NULL} */
    otgCtrlPara.hwDataSize = 0;
    otgCtrlPara.pbData = NULL;
    result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
    if (result != 0)
    {
        return result;
    }
    HalOtgHostTickDelay(10);

    /* <2>. GetDescriptor Device (18Bytes) */
    memcpy(bCMD, otgHCmdGetDscDev, 8);
    /* otgCtrlPara = {1, 0x12, bCMD, pPara->pAttachDevice->deviceDescriptor} */
    otgCtrlPara.bEdNum = 1;
    otgCtrlPara.hwDataSize = 0x12;
    otgCtrlPara.pbData = (uint8_t *)&pPara->pAttachDevice->deviceDescriptor;
    result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
    if (result != 0)
    {
        return result;
    }

    /* <4>. GetDescriptor Configuration (8Bytes) */
    memcpy(bCMD, otgHCmdGetDscCfg, 8);
    uint8_t *pu8Data = NULL;

#ifndef USB_MALLOC_EN
    pu8Data = otgDscCfgTemp;
#else
    pu8Data = malloc(0x400);
    if (pu8Data == NULL)
    {
        /* malloc failed */
        USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
        __breakpoint(0);
    }
#endif

    memset(pu8Data, 0x00, 0x400);
    /* otgCtrlPara = {1, 8, bCMD, pu8Data} */
    otgCtrlPara.bEdNum = 1;
    otgCtrlPara.hwDataSize = 8;
    otgCtrlPara.pbData = pu8Data;
    result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
    if (result != 0)
    {
#if (MEM_MODE == 0)
#else
        free(pu8Data);
#endif
        return result;
    }

    /* <5>. GetDescriptor Configuration (N bytes) */
    /* otgCtrlPara = {1, wLength, bCMD, pu8Data} */
    otgCtrlPara.hwDataSize = (uint16_t)pu8Data[2] + ((uint16_t)pu8Data[3] << 8);
    bCMD[6] = pu8Data[2];
    bCMD[7] = pu8Data[3];
    result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
    if (result != 0)
    {
#ifndef USB_MALLOC_EN
#else
        free(pu8Data);
#endif
        return result;
    }
    /* <6>. Analysis Configuration */
    if (HalOtgHostAnalysisConfiguration(pPara, pu8Data) != 0)
    {
#ifndef USB_MALLOC_EN
#else
        free(pu8Data);
#endif
        return result;
    }

#ifndef USB_MALLOC_EN
#else
    free(pu8Data);
#endif

    /* <7>. Get String */
    /* <7.1> Get String Language ID */
    if ((pPara->pAttachDevice->deviceDescriptor.iManufacturer != 0) || \
        (pPara->pAttachDevice->deviceDescriptor.iProduct != 0) ||      \
        (pPara->pAttachDevice->deviceDescriptor.iSerialNumber != 0))
    {
        memcpy(bCMD, otgHGetDscStr70, 8);
        /* otgCtrlPara = {1, 10, bCMD, pAttachDevice->pStrLanguage} */
        otgCtrlPara.hwDataSize = HAL_USBH_DSC_STR_LANGUAGE_MAX_LEN;
        otgCtrlPara.pbData = pPara->pAttachDevice->pStrLanguage;
        bCMD[2] = 0;
        bCMD[6] = HAL_USBH_DSC_STR_LANGUAGE_MAX_LEN;
        result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
        if (result != 0)
        {
            return result;
        }
    }
    /* <7.2> Get String Manufature */
    if (pPara->pAttachDevice->deviceDescriptor.iManufacturer != 0)
    {
        memcpy(bCMD, otgHGetDscStr70, 8);
        /* otgCtrlPara = {1, 0xFF, bCMD, pAttachDevice->pStrManufacture} */
        otgCtrlPara.hwDataSize = HAL_USBH_DSC_STR_MANUFACTURE_MAX_LEN;
        otgCtrlPara.pbData = pPara->pAttachDevice->pStrManufacture;
        bCMD[2] = pPara->pAttachDevice->deviceDescriptor.iManufacturer;
        bCMD[4] = 0x09;
        bCMD[5] = 0x04;
        bCMD[6] = HAL_USBH_DSC_STR_MANUFACTURE_MAX_LEN;
        result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
        if (result != 0)
        {
            return result;
        }
    }

    /* <7.3> Get String Product */
    if (pPara->pAttachDevice->deviceDescriptor.iProduct != 0)
    {
        memcpy(bCMD, otgHGetDscStr70, 8);
        /* otgCtrlPara = {1, 0xFF, bCMD, pAttachDevice->pStrManufacture} */
        otgCtrlPara.hwDataSize = HAL_USBH_DSC_STR_PRODUCT_MAX_LEN;
        otgCtrlPara.pbData = pPara->pAttachDevice->pStrProduct;
        bCMD[2] = pPara->pAttachDevice->deviceDescriptor.iProduct;
        bCMD[4] = 0x09;
        bCMD[5] = 0x04;
        bCMD[6] = HAL_USBH_DSC_STR_PRODUCT_MAX_LEN;
        result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
        if (result != 0)
        {
            return result;
        }
    }

    /* <7.4> Get String Serial Number */
    if (pPara->pAttachDevice->deviceDescriptor.iSerialNumber != 0)
    {
        memcpy(bCMD, otgHGetDscStr70, 8);
        /* otgCtrlPara = {1, 0xFF, bCMD, pAttachDevice->pStrManufacture} */
        otgCtrlPara.hwDataSize = HAL_USBH_DSC_STR_SERIAL_MAX_LEN;
        otgCtrlPara.pbData = pPara->pAttachDevice->pStrSerialNum;
        bCMD[2] = pPara->pAttachDevice->deviceDescriptor.iSerialNumber;
        bCMD[4] = 0x09;
        bCMD[5] = 0x04;
        bCMD[6] = HAL_USBH_DSC_STR_SERIAL_MAX_LEN;
        result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
        if (result != 0)
        {
            return result;
        }
    }

    /* <8> Set Configuration to 0 */
    memcpy(bCMD, otgHSetCfg, 8);
    /* otgCtrlPara = {1, 0, bCMD, NULL} */
    otgCtrlPara.hwDataSize = 0;
    otgCtrlPara.pbData = NULL;
    result = HalOtgHostIssueControl(pPara, &otgCtrlPara);
    if (result != 0)
    {
        return result;
    }

    result = HalOtgHostEnumerateInterfaceClass(pPara);

    if (result == 0)
    {
        USB_PRINT("Device Attached!\n");
    }
    return result;
}

/**
 * @brief  host periodic schedule enable or disable control
 * @param  option      'OPTION_DISABLE' for disable, 'OPTION_ENABLE' for enable
 * @return NONE
 */
void HalOtgHostPeriodicScheduleSetting(uint8_t option)
{
    if (option != 0)
    {
        if (OTGHC_PSCH_CHECK() == 0)
        {
            OTGHC_PSCH_EN();
            while(OTGHC_PSCH_CHECK() == 0);
        }
    }
    else
    {
        if (OTGHC_PSCH_CHECK() != 0)
        {
            OTGHC_PSCH_DIS();
            while(OTGHC_PSCH_CHECK() != 0);
        }
    }
    return;
}

/**
 * @brief  host get structure of qTD, iTD, siTD, or page 4K memory
 * @param  pPara       pointer to structure @ref otg_host_para_t
 * @param  type        memory type, @ref OTG_HOST_MEM_TYPE
 * @retval == 0        error with not found
 * @retval != 0        address
 */
uint32_t HalOtgHostStructureGet(otg_host_para_t *pPara, uint8_t type)
{
    switch (type)
    {
        case OTG_HOST_MEM_TYPE_QTD:
        {
            for (uint8_t i = 0; i < pPara->numQTD; i++)
            {
                if (*(pPara->pQTDManage + i) == OTG_HOST_MEM_FREE)
                {
                    *(pPara->pQTDManage + i) = OTG_HOST_MEM_USED;
                    otg_host_qTD_t *pQTD = (pPara->pQTD + i);
                    pQTD->bTerminate = 1;
                    pQTD->bStatus_Active = 0;
                    pQTD->bInterruptOnComplete = 1;
                    pQTD->bAlternateTerminate = 1;
                    pQTD->bErrorCounter = 3;
                    return ((uint32_t)pQTD);
                }
            }
            break;
        }
        case OTG_HOST_MEM_TYPE_ITD:
        {
            for (size_t i = 0; i < pPara->numITD; i++)
            {
                if (*(pPara->pITDManage + i) == OTG_HOST_MEM_FREE)
                {
                    *(pPara->pITDManage + i) = OTG_HOST_MEM_USED;
                    otg_host_iTD_t *pITD = (pPara->pITD + i);
                    pITD->iTD.bTerminate = 1;
                    pITD->iTD.bType = OTG_HOST_HD_Type_ITD;
                    return ((uint32_t)pITD);
                }
            }
            break;
        }
        case OTG_HOST_MEM_TYPE_SITD:
        {
            for (size_t i = 0; i < pPara->numITD; i++)
            {
                if (*(pPara->pITDManage + i) == OTG_HOST_MEM_FREE)
                {
                    *(pPara->pITDManage + i) = OTG_HOST_MEM_USED;
                    otg_host_iTD_t *pITD = (pPara->pITD + i);
                    pITD->siTD.bTerminate = 1;
                    pITD->siTD.bType = OTG_HOST_HD_Type_SITD;
                    return ((uint32_t)pITD);
                }
            }
            break;
        }
        case OTG_HOST_MEM_TYPE_PAGE4K:
        {
            for (uint8_t i = 0; i < pPara->numDataPage; i++)
            {
                if (*(pPara->pDataPageManage +i) == OTG_HOST_MEM_FREE)
                {
                    *(pPara->pDataPageManage +i) = OTG_HOST_MEM_USED;
                    uint32_t page4KAddr = (uint32_t)i * 0x1000 + pPara->addrDataPage;
                    return (page4KAddr);
                }
            }
            break;
        }
        default:
            break;
    }
    /* if got here means memery is not enough */
    USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
    __breakpoint(0);
    return 0;
}

/**
 * @brief  host release structure of qTD, iTD, siTD, or page 4K memory
 * @param  pPara       pointer to structure @ref otg_host_para_t
 * @param  type        memory type, @ref OTG_HOST_MEM_TYPE
 * @param  address     start address of structure to release
 * @retval == 0        OK
 * @retval == 1        type error
 * @retval == 2        address error
 */
int32_t HalOtgHostStructureRelease(otg_host_para_t *pPara, uint8_t type, uint32_t address)
{
    int32_t result = 2;
    switch (type)
    {
        case OTG_HOST_MEM_TYPE_QTD:
        {
            if (address >= (uint32_t)pPara->pQTD)
            {
                uint32_t idxNum = (address - (uint32_t)pPara->pQTD) / sizeof(otg_host_qTD_t);
                if (idxNum < pPara->numQTD)
                {
                    *(pPara->pQTDManage + idxNum) = OTG_HOST_MEM_FREE;
                    memset((void *)address, 0x00, sizeof(otg_host_qTD_t));
                    result = 0;
                }
            }
            break;
        }
        case OTG_HOST_MEM_TYPE_ITD:
        case OTG_HOST_MEM_TYPE_SITD:
        {
            if (address >= (uint32_t)pPara->pITD)
            {
                uint32_t idxNum = (address - (uint32_t)pPara->pITD) / sizeof(otg_host_iTD_t);
                if (idxNum < pPara->numITD)
                {
                    *(pPara->pITDManage + idxNum) = OTG_HOST_MEM_FREE;
                    memset((void *)address, 0x00, sizeof(otg_host_iTD_t));
                    result = 0;
                }
            }
            break;
        }
        case OTG_HOST_MEM_TYPE_PAGE4K:
        {
            if (address >= pPara->addrDataPage)
            {
                uint32_t idxNum = (address - pPara->addrDataPage) / 0x1000;
                if (idxNum < pPara->numDataPage)
                {
                    *(pPara->pDataPageManage + idxNum) = OTG_HOST_MEM_FREE;
                    result = 0;
                }
            }
            break;
        }
        default:
            result = 1;
            break;
    }
    return result;
}

/**
 * @brief  host allocate new qTD for qHD
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  pQHD            pointer to structure, @ref otg_host_qHD_t
 * @param  pQHDPara        pointer to structure, @ref otg_host_qhd_para_t
 * @return None
 */
void HalOtgHostAllocateQHD(otg_host_para_t *pPara, otg_host_qHD_t *pQHD, otg_host_qhd_para_t *pQHDPara)
{
    /* <1>. set the QHead */
    pQHD->bTerminate = 0;
    pQHD->bType = pQHDPara->bNextType;
    pQHD->bDeviceAddress = pQHDPara->bAddress;
    pQHD->bEdNumber = pQHDPara->bEndPt;
    pQHD->bHeadOfReclamationListFlag = pQHDPara->bHead;
    pQHD->bMaxPacketSize = pQHDPara->wMaxPacketSize;
    pQHD->bNakCounter = 0;
    pQHD->bOverlay_NextTerminate = 1;
    pQHD->bOverlay_AlternateNextTerminate = 1;
    /* <2>. allocate dumy qTD */
    /* allocate qTD and put qTD to QHD */
    pQHD->bOverlay_NextqTD = OTG_S_ADDR_ADJ(HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD));
    //pQHD->bOverlay_NextqTD = ((HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD) & 0x2FFFFFFF) >> 5);
    /* active the qTD */
    pQHD->bOverlay_NextTerminate = 0;
    return;
}

/**
 * @brief  host issue control base function
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  pCtrlPara       pointer to structure, @ref otg_host_ctrl_para_t
 * @param  mode            0, setup/out/in, 1, setup/in/out, 2, setup/in
 * @retval == 0            OK
 * @retval == 1            time out error
 */
int32_t HalOtgHostIssueControlBase(otg_host_para_t *pPara, otg_host_ctrl_para_t *pCtrlPara, uint8_t mode)
{
    int32_t result = 0;
    HalOtgHostAsynchronousScheduleSetting(OPTION_ENABLE);

    otg_host_qTD_t *pQTD;
    otg_host_qHD_t *pQHDListCtrl = (otg_host_qHD_t *)pPara->pQHDListControl[pCtrlPara->bEdNum];
    /* allocate qTD and data buffer */
    uint8_t *pDataPage = pPara->pAttachDevice->pDataBuffer;
    pQTD = (otg_host_qTD_t *)HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD);
    /* Setup packet */
    /* Fill qTD */
    pQTD->bPID = OTG_HOST_QTD_PID_SETUP;
    pQTD->bTotalBytes = 8;
    pQTD->bDataToggle = 0;
    pQTD->ArrayBufferPointer_Word[0] = (uint32_t)pDataPage;
    memcpy(pDataPage, pCtrlPara->pbCmd, 8);
    /* Send qTD */
    result = HalOtgHostSendQTD(pPara, pQTD, pQHDListCtrl, 3000);
    if (result == 0)
    {
        /* analysis the controller cmomand */
        switch (mode)
        {
            case 0:     /* Case: 'Setup/Out/In' */
            {
                /* Out packet */
                pQTD = (otg_host_qTD_t *)HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD);
                /* Fill qTD */
                pQTD->bPID = OTG_HOST_QTD_PID_OUT;
                pQTD->bTotalBytes = pCtrlPara->hwDataSize;
                pQTD->bDataToggle = 1;
                pQTD->ArrayBufferPointer_Word[0] = (uint32_t)pDataPage;
                memcpy(pDataPage, pCtrlPara->pbData, pCtrlPara->hwDataSize);
                /* Send qTD */
                result = HalOtgHostSendQTD(pPara, pQTD, pQHDListCtrl, 3000);
                if (result != 0)
                {
                    break;
                }
                /* In packet */
                pQTD = (otg_host_qTD_t *)HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD);
                /* Fill qTD */
                pQTD->bPID = OTG_HOST_QTD_PID_IN;
                pQTD->bTotalBytes = 0;
                pQTD->bDataToggle = 1;
                /* Send qTD */
                result = HalOtgHostSendQTD(pPara, pQTD, pQHDListCtrl, 3000);
                break;
            }
            case 1:     /* Case: 'Setup/In/Out' */
            {
                /* In packet */
                pQTD = (otg_host_qTD_t *)HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD);
                /* Fill qTD */
                pQTD->bPID = OTG_HOST_QTD_PID_IN;
                pQTD->bTotalBytes = pCtrlPara->hwDataSize;
                pQTD->bDataToggle = 1;
                pQTD->ArrayBufferPointer_Word[0] = (uint32_t)pDataPage;
                /* Send qTD */
                result = HalOtgHostSendQTD(pPara, pQTD, pQHDListCtrl, 3000);
                if (result != 0)
                {
                    break;
                }
                /* copy result data */
                memcpy(pCtrlPara->pbData, pDataPage, pCtrlPara->hwDataSize);
                /* Out packet */
                pQTD = (otg_host_qTD_t *)HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD);
                /* Fill qTD */
                pQTD->bPID = OTG_HOST_QTD_PID_OUT;
                pQTD->bTotalBytes = 0;
                pQTD->bDataToggle = 1;
                /* Send qTD */
                result = HalOtgHostSendQTD(pPara, pQTD, pQHDListCtrl, 3000);
                break;
            }
            case 2:     /* Case: 'Setup/In' */
            {
                /* In packet */
                pQTD = (otg_host_qTD_t *)HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD);
                /* Fill qTD */
                pQTD->bPID = OTG_HOST_QTD_PID_IN;
                pQTD->bTotalBytes = pCtrlPara->hwDataSize;
                pQTD->bDataToggle = 1;
                pQTD->ArrayBufferPointer_Word[0] = (uint32_t)pDataPage;
                /* Send qTD */
                result = HalOtgHostSendQTD(pPara, pQTD, pQHDListCtrl, 3000);
                if (result != 0)
                {
                    break;
                }
                /* copy result data */
                memcpy(pCtrlPara->pbData, pDataPage, pCtrlPara->hwDataSize);
                break;
            }
            default:
                break;
        }
    }
    HalOtgHostAsynchronousScheduleSetting(OPTION_DISABLE);
    return result;
}

/**
 * @brief  host issue control base function
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  dir             1, in(OTG_HOST_QTD_PID_IN), 0, out(OTG_HOST_QTD_PID_OUT)
 * @param  pBufferArray    pointer to the buffer array
 * @param  size            byte number
 * @retval == 0            OK
 * @retval == 1            time out error
 */
int32_t HalOtgHostIssueInterrupt(otg_host_para_t *pPara, uint8_t dir, uint32_t *pBufferArray, size_t size)
{
    int32_t result = 0;

    otg_host_qTD_t *pQTD;
    otg_host_qHD_t *pQHDListInt = (otg_host_qHD_t *)pPara->pQHDListInt[dir];
    /* allocate qTD and data buffer */
    pQTD = (otg_host_qTD_t *)HalOtgHostStructureGet(pPara, OTG_HOST_MEM_TYPE_QTD);
    /* Fill qTD */
    pQTD->bTotalBytes = size;
    for (uint8_t i = 0; i < 5; i++)
    {
        pQTD->ArrayBufferPointer_Word[i] = *pBufferArray++;
    }
    pQTD->bPID = dir;

    /* Send qTD */
    result = HalOtgHostSendQTD(pPara, pQTD, pQHDListInt, 1000);
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
/**
 * @}
 */
/**
 * @}
 */
