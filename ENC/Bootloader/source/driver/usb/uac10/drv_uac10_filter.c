
#include "drv_uac10_user_config.h"
#if USB_MIC_LPF_EN
#include "arm_math.h"                        // 使能滤波器时，需要调用math库

/* Private define ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static arm_biquad_casd_df1_inst_f32 usbMicLpf[USB_MIC_DATA_BUFF_CH];
static float_t usbMicLpfXYMem[USB_MIC_DATA_BUFF_CH][4*4];
static float_t usbMicLpfCoeff[5*4];
static float_t usbMicLpfTemp[192];

/* Private constants ---------------------------------------------------------*/
static const float_t usbMicLpfFs96Fc20[] = {
    0.00400177789703110f, 0.00800600229672686f, 0.00400297264076446f, 0.198874935277404f, -0.0193992875821027f,
    1.0f,   2.02501937393162f,  1.02534004997240f,  0.214917165928335f, -0.101629049085111f,
    1.0f,   1.99938921206706f,  0.999702296645625f, 0.252561253072099f, -0.294586273998873f,
    1.0f,   1.97498006006623f,  0.975285456788754f, 0.327513624959366f, -0.678779457508352f,
};
static const float_t usbMicLpfFs192Fc20[] = {
    0.0000440581824f, 0.00008943747214f, 0.0000453909316f, 0.953037529569016f, -0.232890868968996f,
    1.0f,   2.01207535978489f,  1.01233662291722f,  1.01213906827787f,  -0.309347194303757f,
    1.0f,   1.98743293671177f,  0.987689919485824f, 1.14312608856478f,  -0.478797710422124f,
    1.0f,   1.97050618586563f,  0.970760083019421f, 1.37575231130035f,  -0.779733126913791f,
};

/* Weak functions rewrite ----------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void DrvUac10MicFilterInit(uint32_t type)
{
    return;//Niko Bootloader
    LibDrvMemset(usbMicLpfXYMem, 0, sizeof(usbMicLpfXYMem));
    if (type == 0)
    {
        //fs=96k, fc=200, 8 order lpf
        LibDrvMemcpy((uint8_t *)&usbMicLpfCoeff[0], (uint8_t *)&usbMicLpfFs96Fc20[0], sizeof(usbMicLpfCoeff));
        arm_biquad_cascade_df1_init_f32 (&usbMicLpf[0], 4, &usbMicLpfCoeff[0], usbMicLpfXYMem[0]);
        arm_biquad_cascade_df1_init_f32 (&usbMicLpf[1], 4, &usbMicLpfCoeff[0], usbMicLpfXYMem[1]);
    }
    else if (type == 1)
    {
        //fs=192k, fc=20k, 8 order lpf
        LibDrvMemcpy((uint8_t *)&usbMicLpfCoeff[0], (uint8_t *)&usbMicLpfFs192Fc20[0], sizeof(usbMicLpfCoeff));
        arm_biquad_cascade_df1_init_f32 (&usbMicLpf[0], 4, &usbMicLpfCoeff[0], usbMicLpfXYMem[0]);
        arm_biquad_cascade_df1_init_f32 (&usbMicLpf[1], 4, &usbMicLpfCoeff[0], usbMicLpfXYMem[1]);
    }
}

__RAM_CODE static void DrvUac10MicFilter(float_t *pSrc, float_t *pDst, uint32_t blockSize, uint32_t ch)
{
//    arm_biquad_cascade_df1_f32(&usbMicLpf[ch], pSrc, pDst, blockSize);
    return;
}

#else

__RAM_CODE static void DrvUac10MicFilter(float_t *pSrc, float_t *pDst, uint32_t blockSize, uint32_t ch)
{
    return;
}

#endif

