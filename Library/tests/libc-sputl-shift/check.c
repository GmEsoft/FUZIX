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
/* SOURCE libs/sputl.c */
/* SOURCE libs/sgetl.c */
int main(void) { char b[4]; sputl(0x12345678L,b); assert((unsigned char)b[0]==0x78 && (unsigned char)b[1]==0x56 && (unsigned char)b[2]==0x34 && (unsigned char)b[3]==0x12); assert(sgetl(b)==0x12345678L); sputl(0,b); assert(sgetl(b)==0); return 0; }
