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
/* Model the target's 32-bit long after loading the host headers. */
#define long int
/* SOURCE libs/__double_bits.c */
#undef long
int main(void){assert(__double_bits(1.0)==UINT64_C(0x3ff0000000000000));assert(__double_bits(-0.0)==UINT64_C(0x8000000000000000));assert(__double_bits(INFINITY)==UINT64_C(0x7ff0000000000000));return 0;}
