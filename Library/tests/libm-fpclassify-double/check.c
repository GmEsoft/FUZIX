#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <unistd.h>
#define EXTRACT_WORDS(hi,lo,x) do { union {double d; uint64_t u;} b; b.d=(x); (hi)=b.u>>32; (lo)=(uint32_t)b.u; } while(0)
/* SOURCE libs/__fpclassify.c */
int main(void) { assert(__fpclassify(1.0)==FP_NORMAL); assert(__fpclassify(INFINITY)==FP_INFINITE); assert(__fpclassify(NAN)==FP_NAN); assert(__fpclassify(0.0)==FP_ZERO); assert(__fpclassify(-0.0)==FP_ZERO); union {uint64_t u; double d;} v={1}; assert(__fpclassify(v.d)==FP_SUBNORMAL); return 0; }
