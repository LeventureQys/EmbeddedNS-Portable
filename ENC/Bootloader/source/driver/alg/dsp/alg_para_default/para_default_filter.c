
#include "alg_filter.h"

#define DACOUT_FILTER_NUM	(5)

#define SPK_FILTER_NUM	(7)



ALG_PARA_FILTER paraDefaultAdcEq[1][5] = //ADC EQ
{
    {
        {
            // default 0
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 1
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 2
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 3
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 4
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
    },

};


ALG_PARA_FILTER paraDefaultDacEq[1][5] = 
{
    {
        {
            // default 0
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 1
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 2
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 3
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 4
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
    },
   
};




ALG_PARA_FILTER paraDefaultSpkEq[1][7] = //SPK EQ //by NIko ¸øADCÊ¹ÓÃ
{
    {
        {
            // default 0
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 1
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 2
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 3
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 4
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
		{
            // default5
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
		{
            // default 6
            // for test usage
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
			
			
    },
   
};



