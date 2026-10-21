/* From MUSL */

#ifndef _FLOAT_H
#define _FLOAT_H

#include <stdint.h>

#define FLT_RADIX 2

#define FLT_MIN 1.17549435e-38F
#define FLT_MAX 3.40282347e+38F
#define FLT_EPSILON 1.19209290e-07F

#define FLT_MANT_DIG 24
#define FLT_MIN_EXP (-125)
#define FLT_MAX_EXP 128

#define FLT_DIG 6
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_10_EXP 38

#ifdef NO_64BIT
/* double is the same as float on these targets */
#define DBL_MIN FLT_MIN
#define DBL_MAX FLT_MAX
#define DBL_EPSILON FLT_EPSILON

#define DBL_MANT_DIG FLT_MANT_DIG
#define DBL_MIN_EXP FLT_MIN_EXP
#define DBL_MAX_EXP FLT_MAX_EXP

#define DBL_DIG FLT_DIG
#define DBL_MIN_10_EXP FLT_MIN_10_EXP
#define DBL_MAX_10_EXP FLT_MAX_10_EXP
#else
#define DBL_MIN 2.2250738585072014e-308
#define DBL_MAX 1.7976931348623157e+308
#define DBL_EPSILON 2.2204460492503131e-16

#define DBL_MANT_DIG 53
#define DBL_MIN_EXP (-1021)
#define DBL_MAX_EXP 1024

#define DBL_DIG 15
#define DBL_MIN_10_EXP (-307)
#define DBL_MAX_10_EXP 308
#endif

#define FLT_ROUNDS 1	/* Check this */

#ifndef FLT_EVAL_METHOD
#ifdef __FLT_EVAL_METHOD__
#define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
#else
#define FLT_EVAL_METHOD 0
#endif
#endif

typedef float float_t;
typedef double double_t;

#endif
