
#include "alg_filter.h"

#define DACOUT_FILTER_NUM	(5)

#define SPK_FILTER_NUM	(7)


extern void setAdcEqValue(int row, int col, float gain, float fc, float q, uint32_t type);

ALG_PARA_FILTER paraDefaultAdcEq[1][14] =
{
    {
        {
            // default 0
            .gain =	0.0f,
            .fc = 4000.0f,
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
            // default 5
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 10.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 6
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 7
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 8
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 9
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },

        {
            // default 10
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 11
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 12
            .gain = 0.0f,
            .fc = 1000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
        {
            // default 13
            .gain = 0.0f,
            .fc = 4000.0f,
            .q = 0.7f,
            .type = (uint32_t)FILTER_PEAK,
        },
    },

};



void setAdcEqValue(int row, int col, float gain, float fc, float q, uint32_t type) 
{
    paraDefaultAdcEq[row][col].gain = gain;
    paraDefaultAdcEq[row][col].fc = fc;
    paraDefaultAdcEq[row][col].q = q;
    paraDefaultAdcEq[row][col].type = type;

}







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




ALG_PARA_FILTER paraDefaultSpkEq[2][7] = //SPK EQ //by NIko 给ADC使用
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



