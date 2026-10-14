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
#define calloc fuzix_calloc
/* SOURCE libs/calloc.c */
int main(void) { errno=0; assert(fuzix_calloc(SIZE_MAX/2+1,2)==NULL && errno==ENOMEM); unsigned char *p=fuzix_calloc(16,4); assert(p); for(int i=0;i<64;i++)assert(p[i]==0); free(p); p=fuzix_calloc(0,2); free(p); return 0; }
