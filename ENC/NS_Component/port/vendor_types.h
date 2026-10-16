#ifndef NS_VENDOR_TYPES_H
#define NS_VENDOR_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

typedef uint8_t uint8;

#ifndef MIN
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(x)      (((x) >= 0) ? (x) : (-(x)))
#endif

#endif /* NS_VENDOR_TYPES_H */
