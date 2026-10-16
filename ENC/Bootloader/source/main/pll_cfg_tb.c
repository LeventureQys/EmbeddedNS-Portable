
#include "Initial.h"
#include "a0_hal_sys_ctrl.h"

typedef struct {
    uint32_t                sysClk;
    SYS_PLL_CFG00_TypeDef   pllCfg00;
    SYS_PLL_CFG01_TypeDef   pllCfg01;
    SYS_PLL_CFG02_TypeDef   pllCfg02;
}pll_cfg_para_t;

pll_cfg_para_t  pllCfgDefault = {
#if (SYS_CLK_PLL0_CFG_SEL == CLK_190M_SSEN) //190M, +/- 2M, 20Kfs, SSEN = 1
    .sysClk = 190000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 1,   .ssen = 0,
        .cpa = 252,
    },
    .pllCfg01.b = {
        .nf = 4150613,
        .od = 3,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 145,
        .rs = 3,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_120M)    //120M, -2M, 20Kfs, SSEN = 0
    .sysClk = 120000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 320,
    },
    .pllCfg01.b = {
        .nf = 5242880,
        .od = 4,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 4,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_115M)    //115M, -2M, 20Kfs, SSEN = 0
    .sysClk = 115000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 306,
    },
    .pllCfg01.b = {
        .nf = 5024426,
        .od = 4,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 4,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_110M)    //110M, -2M, 20Kfs, SSEN = 0
    .sysClk = 110000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 292,
    },
    .pllCfg01.b = {
        .nf = 4805973,
        .od = 4,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 3,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_105M)    //105M, -2M, 20Kfs, SSEN = 0
    .sysClk = 105000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 280,
    },
    .pllCfg01.b = {
        .nf = 4587520,
        .od = 4,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 3,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_100M)    //100M, -2M, 20Kfs, SSEN = 0
    .sysClk = 100000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 266,
    },
    .pllCfg01.b = {
        .nf = 4369066,
        .od = 4,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 3,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_90M)     //90M, -2M, 20Kfs, SSEN = 0
    .sysClk = 90000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 240,
    },
    .pllCfg01.b = {
        .nf = 3932160,
        .od = 4,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 2,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_80M)     //80M, -2M, 20Kfs, SSEN = 0
    .sysClk = 80000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 212,
    },
    .pllCfg01.b = {
        .nf = 3495253,
        .od = 4,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 2,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_70M)     //70M, -2M, 20Kfs, SSEN = 0
    .sysClk = 70000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 372,
    },
    .pllCfg01.b = {
        .nf = 3058346,
        .od = 4,
        .cpax = 3,
    },
    .pllCfg02.b = {
        .nv = 291,
        .rs = 1,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_60M)     //60M, -2M, 20Kfs, SSEN = 0
    .sysClk = 60000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 320,
    },
    .pllCfg01.b = {
        .nf = 5242880,
        .od = 5,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 582,
        .rs = 4,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_55M)     //55M, -2M, 20Kfs, SSEN = 0
    .sysClk = 55000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 292,
    },
    .pllCfg01.b = {
        .nf = 4805973,
        .od = 5,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 582,
        .rs = 3,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_50M)     //50M, -2M, 20Kfs, SSEN = 0
    .sysClk = 50000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 266,
    },
    .pllCfg01.b = {
        .nf = 4369066,
        .od = 5,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 582,
        .rs = 3,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_40M)     //40M, -2M, 20Kfs, SSEN = 0
    .sysClk = 40000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 212,
    },
    .pllCfg01.b = {
        .nf = 3495253,
        .od = 5,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 582,
        .rs = 2,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_30M)     //30M, -2M, 20Kfs, SSEN = 0
    .sysClk = 30000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 320,
    },
    .pllCfg01.b = {
        .nf = 5242880,
        .od = 6,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 1165,
        .rs = 4,
    },
#elif (SYS_CLK_PLL0_CFG_SEL == CLK_20M)     //20M, -2M, 20Kfs, SSEN = 0
    .sysClk = 20000000,
    .pllCfg00.b = {
        .nr = 1,    .ns = 300,  .fir = 0,   .mp = 0,    .pdExt = 1, .testExt = 0,
        .type = 0,   .ssen = 0,
        .cpa = 212,
    },
    .pllCfg01.b = {
        .nf = 3495253,
        .od = 6,
        .cpax = 1,
    },
    .pllCfg02.b = {
        .nv = 1165,
        .rs = 2,
    },
#endif
};


