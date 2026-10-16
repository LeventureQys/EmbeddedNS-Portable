
#ifndef __MAIN_H
#define __MAIN_H

#include "a0.h"
#include "UserRetype.h"

#include "OsTick.h"
#include "Gpio.h"
#include "Crc.h"
#include "Iap.h"
#include "Spi.h"
#include "Hid.h"

#include "Ui.h"
#include "Dsp.h"
#include "Memory.h"
#include "Com.h"
#include "Fw.Upgrade.h"

//#include "main_sdk.h"
#include "Initial.h"
#include "UserFunction.h"


#include "a0_hal_conf.h"
#include "a0_hal_def.h"

/*     
ARM_MATH_CM4
__FPU_USED
__FPU_PRESENT
__CC_ARM
ARM_MATH_MATRIX_CHECK
ARM_MATH_ROUNDING
#include "arm_math.h"
*/

#if (defined (__VTOR_PRESENT)) && (__VTOR_PRESENT == 1U)
  extern uint32_t __Vectors;
  extern uint32_t __Vectors_End;
#endif

#endif
