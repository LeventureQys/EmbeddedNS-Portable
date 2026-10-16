
/*
    全局DMA配置，当前应用仅包括I2S的使用
    如需使用DMA的其他通道用作它用，需要谨慎修改代码，避免影响到原I2S
    

*/

#include "app_dma.h"
#include "app_i2s.h"


#include "lib_drv_uac10.h"


#include "a0_hal_otg.h"
#include "a0_hal_gpio.h"
#include "a0_hal_timer.h"
#include "a0_hal_sys_ctrl.h"
#include "app_i2s_resample.h"

/*
    默认按照I2S固定使用DMA进行，不使用I2S中断，DMA传输一包数据完成之后触发DMA中断。
    最大支持8个单声道I2S数据传输，如果需要用到全部7个FIFO的I2S数据进行传输，需要对应修改代码
        a. 使用14个通道DMA进行传输，不需要I2S中断打断。
        b. 使用I2S中断，每次触发中断配置DMA进行8个样本数据的传输，可以占用较少DMA通道。
*/

/* Private define ------------------------------------------------------------*/
#define DMA_IRQ_CLK_RECORD_EN           (1) // 使能时进行系统周期记录，包括每次进DMA中断的间隔时间，DMA中断中处理的时间，一般测试时使能，Release版本时禁用，提升运行效率


#if (I2S_IFX_CLOCK == 0)
//#define DMA_TCISR_CHECK_NUM             (2) // DMA完成中断中需要判定的数量，如果I2S额外使用slave的话，这里数量就需要增加1
//#define DMA_TCISR_CHECK_I2S_IDX         (0) // 对应表格dmaTcCbTb中
//#define DMAS1_TCISR_CHECK_I2S_IDX       (1) // 对应表格dmaTcCbTb中
#else
#define DMA_TCISR_CHECK_NUM             (1) // DMA完成中断中需要判定的数量，如果I2S额外使用slave的话，这里数量就需要增加1
#define DMA_TCISR_CHECK_I2S_IDX         (0) // 对应表格dmaTcCbTb中
#endif

/* Private type --------------------------------------------------------------*/
typedef struct
{
    DMA_CH_CCR_TypeDef  ccr;
    DMA_CH_CCFG_TypeDef ccfg;
}APP_DMA_CH_CFG;

typedef struct
{
    DMA_CH_TypeDef  *pDmaCh;
    uint32_t        addrReg;
}APP_DMA_CH_I2S_INFO;


/* Private function declarations ---------------------------------------------*/
static void DmaRestartI2s(void);
static uint32_t dmaInitForI2s(void);

/* Public variables ----------------------------------------------------------*/
#if DMA_IRQ_CLK_RECORD_EN
uint32_t g_dmaCycleCnt[5];      // 0, last clk count value, 1, dma cycle, 2, dma cycle max, 3, dma irq cycle use, 4, dma irq cycle use max
#endif

/* Private variables ---------------------------------------------------------*/
static uint32_t dmaTcIsr;                                   // 用于DMA传输各通道标志记录，同步所有通道完成之后进行TC中断处理
static APP_DMA_CH_I2S_INFO dmaChInInfo[DMA_CH_I2S_IN];      // 需要初始化配置的时候赋值
static APP_DMA_CH_I2S_INFO dmaChOutInfo[DMA_CH_I2S_OUT];    // 需要初始化配置的时候赋值
static uint32_t dmaTcCheckTb[DMA_TCISR_CHECK_NUM];          // 需要初始化配置的时候赋值


static FUNCP dmaTcCbTb[DMA_TCISR_CHECK_NUM] = {
    DmaRestartI2s,
};

/* Private constants ---------------------------------------------------------*/
//此表格数据不用更改
static const APP_DMA_CH_CFG i2sDmaCfgTb[2] = {
    {//fifo->memery
        .ccr.b = {
            .chEn = 1,
            .dstCtrl = DMA_ADDR_INC,                //dst fixed, i2s data port
            .srcCtrl = DMA_ADDR_FIXED,              //src inc, data in dsp iram
            .mode = DMA_MODE_HARDWARE,              //for i2s, use hardware handshake
            .dstWidth = DMA_WIDTH_32BIT,            //32bit
            .srcWidth = DMA_WIDTH_32BIT,            //32bit
            .srcSize = DMA_BURST_8,                 //32bit*8
            .fifoTh = DMA_FIFO_TH_1,                //1
        },
        .ccfg.b = {
            .srcRequest = 0,                        //src request select, no care, it will be configed in initial function
            .srcHardwareEn = 1,                     //src hardware handshake disable
        },
    },
    {//memery->fifo
        .ccr.b = {
            .chEn = 1,
            .dstCtrl = DMA_ADDR_FIXED,              //dst fixed, i2s data port
            .srcCtrl = DMA_ADDR_INC,                //src inc, data in dsp iram
            .mode = DMA_MODE_HARDWARE,              //for i2s, use hardware handshake
            .dstWidth = DMA_WIDTH_32BIT,            //32bit
            .srcWidth = DMA_WIDTH_32BIT,            //32bit
            .srcSize = DMA_BURST_8,                 //32bit*8
            .fifoTh = DMA_FIFO_TH_1,                //1
        },
        .ccfg.b = {
            .dstRequest = 0,                        //dst request select, no care, it will be configed in initial function
            .dstHardwareEn = 1,                     //dst hardware handshake enable        
        },
    },
};

/* Exported functions --------------------------------------------------------*/
/*
    目前因为只有I2S一个模块使用DMA的TC中断处理，所以在I2S的APP代码中重写中断的回调函数
    后面如果DMA被多个模块使用，那么需要在DMA的应用层进行重写
    进行判断不同的Group，不同的Ch，调用不同的回调函数，进行独立处理
    后面有时间整理，再将这部分挪到DMA的DRV层或者APP层去
*/
__RAM_CODE void HalDmaTcCallback(DMA_CTRL_TypeDef *DMACtrl, uint32_t tcIsr)
{
    if (DMACtrl == DMA1CTRL)
    {
        tcIsr <<= 8;
    }
    dmaTcIsr |= tcIsr;
    for (uint32_t i = 0; i < DMA_TCISR_CHECK_NUM; i++)
    {
        if ((dmaTcIsr & dmaTcCheckTb[i]) == dmaTcCheckTb[i])
        {
            dmaTcIsr &= (~dmaTcCheckTb[i]);
            dmaTcCbTb[i]();
        }
    }
    return;
}    

__RAM_CODE static void DmaRestartI2s(void)
{
#if DMA_IRQ_CLK_RECORD_EN
    HalSysCycleRecordStart(g_dmaCycleCnt);
#endif
    
    uint32_t memAddr, i2sPackNum;
    AppI2sFifoInfoGet(&memAddr, &i2sPackNum);
    for (uint32_t i = 0; i < DMA_CH_I2S_OUT; i++)
    {
        DMA_CH_TypeDef *dmaCh = dmaChOutInfo[i].pDmaCh;
        uint32_t fifoAddr = dmaChOutInfo[i].addrReg;
        //Out, Mem2Fifo
        HAL_DMA_CH_SRC_ADDR_SET(dmaCh, memAddr);
        HAL_DMA_CH_DST_ADDR_SET(dmaCh, fifoAddr);
        memAddr += (I2S_BUFF_MAX*4);
        HAL_DMA_CH_TRANS_SIZE_SET(dmaCh, i2sPackNum);
        HAL_DMA_CH_ENABLE(dmaCh);
    }
    for (uint32_t i = 0; i < DMA_CH_I2S_IN; i++)
    {
        DMA_CH_TypeDef *dmaCh = dmaChInInfo[i].pDmaCh;
        uint32_t fifoAddr = dmaChInInfo[i].addrReg;
        //In, Fifo2Mem
        HAL_DMA_CH_SRC_ADDR_SET(dmaCh, fifoAddr);
        HAL_DMA_CH_DST_ADDR_SET(dmaCh, memAddr);   
        memAddr += (I2S_BUFF_MAX*4);
        HAL_DMA_CH_TRANS_SIZE_SET(dmaCh, i2sPackNum);
        HAL_DMA_CH_ENABLE(dmaCh);
    }

    AppI2sFrameProcess();

#if DMA_IRQ_CLK_RECORD_EN
    HalSysCycleRecordEnd(g_dmaCycleCnt);
#endif
    return;
}


// return:  BIT0 == 1, dma group0 use
//          BIT1 == 1, dma group1 use
static uint32_t dmaInitForI2s(void)
{
    /* <1> I2S使用的DMA通道信息初始化赋值 */
    uint32_t dmaChIdx = DMA_CH_I2S_START;
    for (uint32_t i = 0; i < DMA_CH_I2S_OUT; i++)
    {
        dmaChOutInfo[i].pDmaCh = DMACH(dmaChIdx + i);
        dmaChOutInfo[i].addrReg = DMA_CH_I2S_REG_ADDR_OUT[i];
    }
    dmaChIdx = DMA_CH_I2S_START + DMA_CH_I2S_OUT;
    for (uint32_t i = 0; i < DMA_CH_I2S_IN; i++)
    {
        dmaChInInfo[i].pDmaCh = DMACH(dmaChIdx + i);
        dmaChInInfo[i].addrReg = DMA_CH_I2S_REG_ADDR_IN[i];
    }
    /* <2> I2S使用的DMA通道进行初始化配置 */
    DMA_SEL_TypeDef *dmaSelect;
    dmaChIdx = DMA_CH_I2S_START;
    for (uint32_t i = 0; i < DMA_CH_I2S_OUT; i++)
    {
        DMA_CH_CCFG_TypeDef ccfg = i2sDmaCfgTb[DMA_DIR_MEM2REG].ccfg;
        ccfg.b.srcRequest = dmaChIdx;
        ccfg.b.dstRequest = dmaChIdx;
        HAL_DMA_CH_CFG_SET(dmaChOutInfo[i].pDmaCh, ccfg.w);
        HAL_DMA_CH_CTRL_SET(dmaChOutInfo[i].pDmaCh, i2sDmaCfgTb[DMA_DIR_MEM2REG].ccr.w);
        dmaSelect = DMASELECT(dmaChIdx);
        HAL_DMA_GROUP_REQ_SEL(dmaSelect, dmaChIdx, DMA_REQ_I2S);            //每个通道用独立的REQ，是否可以公用一个REQ，待测试
        dmaChIdx++;
    }  
    for (uint32_t i = 0; i < DMA_CH_I2S_IN; i++)
    {
        DMA_CH_CCFG_TypeDef ccfg = i2sDmaCfgTb[DMA_DIR_REG2MEM].ccfg;
        ccfg.b.srcRequest = dmaChIdx;
        ccfg.b.dstRequest = dmaChIdx;
        HAL_DMA_CH_CFG_SET(dmaChInInfo[i].pDmaCh, ccfg.w);
        HAL_DMA_CH_CTRL_SET(dmaChInInfo[i].pDmaCh, i2sDmaCfgTb[DMA_DIR_REG2MEM].ccr.w);
        dmaSelect = DMASELECT(dmaChIdx);
        HAL_DMA_GROUP_REQ_SEL(dmaSelect, dmaChIdx, DMA_REQ_I2S);    //每个通道用独立的REQ，是否可以公用一个REQ，待测试
        dmaChIdx++;
    }
    /* <3> I2S使用的DMA通道使能硬件同步，这里是对SYNC寄存器进行或操作，不能直接赋值，万一其他代码已经配置过DMA，可能会造成覆盖导致错误 */
    uint32_t hdSync = 0;
    for (uint32_t i = 0; i < DMA_CH_I2S_USE; i++)
    {
        hdSync = (hdSync << 1) | (1 << DMA_CH_I2S_START);
    }
    dmaTcCheckTb[DMA_TCISR_CHECK_I2S_IDX] = hdSync;                 //根据使用的dma通道初始化配置，让dma完成中断需要各通道都完成才可以新的配置
    uint32_t dmaGroupFlag = 0;
    if ((hdSync & 0xFF) != 0)
    {// dma0 need to config
        HAL_DMA_GROUP_HD_SYNC_SET_BIT(DMA0CTRL, (hdSync & 0xFF));
        dmaGroupFlag |= BIT0;
        NVIC_SetPriority(DMA0_IRQn, 0);                             // DMA的IRQ中断优先级配置成0，最高，与USB同级
        NVIC_EnableIRQ(DMA0_IRQn);                                  // 使能DMA的IRQ中断
    }
    hdSync >>= 8;
    if ((hdSync & 0xFF) != 0)
    {// dma1 need to config
        HAL_DMA_GROUP_HD_SYNC_SET_BIT(DMA0CTRL, (hdSync & 0xFF));
        dmaGroupFlag |= BIT1;
        NVIC_SetPriority(DMA1_IRQn, 0);                             // DMA的IRQ中断优先级配置成0，最高，与USB同级
        NVIC_EnableIRQ(DMA1_IRQn);                                  // 使能DMA的IRQ中断
    }
    /* <4> I2S的一帧DMA参数配置，该函数每次DMA完成中断时一样需要调用 */
    DmaRestartI2s();
    /* <5> 为了避免DMA的最高优先级中断占用时间过长，这里开启一个软件IRQ中断，在处理完DMA帧中断中的配置后，触发软件IRQ中断，在退出DMA的
           IRQ之后，如无其他高优先级的中断（如USB），则立刻进入该中断进行信号处理 */
    NVIC_SetPriority(USER0_IRQn, 1);        // 开启用户软件IRQ中断0
    NVIC_EnableIRQ(USER0_IRQn);

    return (dmaGroupFlag);
}


void AppDmaInit(void)
{
    uint32_t dmaGroupFlag = dmaInitForI2s();
    
    if (dmaGroupFlag & BIT0)
    {
        HAL_DMA_GROUP_ENABLE(DMA0CTRL);
    }
    if (dmaGroupFlag & BIT1)
    {
        HAL_DMA_GROUP_ENABLE(DMA1CTRL);
    }
    return;
}

/*****************************************************************************/
