//test unaligned memery access
//如果是8位指针，转成32位指针取数据，需要加__packed修饰，否则当地址非4的整数倍时会出错。
//如果是16位指针，转成32位指针曲数据，需要加__packed修饰，否则当地址非2的整数倍时会出错。
void testRead(uint8_t *pSrc, uint32_t *pDst, uint32_t dstLen)
{
    __packed uint32_t *pu32Src = (__packed uint32_t *)pSrc;
    for (uint32_t i = 0; i < dstLen; i++)
    {
        *pDst++ = *pu32Src++;
    }
    return;
}

__RAM_CODE void memcpy_user(void *dst, void *src, size_t size)
{
    __packed uint32_t *pu32Src = (__packed uint32_t *)src;
    __packed uint32_t *pu32Dst = (__packed uint32_t *)dst;
    size_t size1 = size >> 4;
    while (size1--)
    {
        *pu32Dst++ = *pu32Src++;
        *pu32Dst++ = *pu32Src++;
        *pu32Dst++ = *pu32Src++;
        *pu32Dst++ = *pu32Src++;
    }
    uint8_t *pu8Src = (uint8_t *)pu32Src;
    uint8_t *pu8Dst = (uint8_t *)pu32Dst;
    size1 = size & 0xF;
    while (size1--)
    {
        *pu8Dst++ = *pu8Src++;
    }
    return;
}