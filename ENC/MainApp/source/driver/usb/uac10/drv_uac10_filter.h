
#ifndef __DRV_UAC10_FILTER_H
#define __DRV_UAC10_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif


void DrvUac10MicFilter(float_t *pSrc, float_t *pDst, uint32_t blockSize, uint32_t ch);


#ifdef __cplusplus
}
#endif

#endif

