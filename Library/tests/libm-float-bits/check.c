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
#include <fcntl.h>
/* Model the target's 16-bit int after loading the host headers. */
#define int short
/* SOURCE libs/__float_bits.c */
#undef int
int main(void){assert(sizeof(__float_bits(1.0f))==4);assert(__float_bits(1.0f)==UINT32_C(0x3f800000));assert(__float_bits(-0.0f)==UINT32_C(0x80000000));assert(__float_bits(INFINITY)==UINT32_C(0x7f800000));return 0;}
