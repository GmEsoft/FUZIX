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
#include <ctype.h>
#define strtol fuzix_strtol
#define strtoul fuzix_strtoul
/* SOURCE libs/strtol.c */
int main(void) { char b[80], *end; snprintf(b,sizeof b,"%ld",LONG_MIN); errno=0; assert(fuzix_strtol(b,&end,10)==LONG_MIN); assert(errno==0 && *end==0); snprintf(b,sizeof b,"-%lu",(unsigned long)LONG_MAX+2); errno=0; assert(fuzix_strtol(b,&end,10)==LONG_MIN); assert(errno==ERANGE && *end==0); errno=0; assert(fuzix_strtol("-42",&end,10)==-42 && errno==0); assert(fuzix_strtoul("0x10",&end,0)==16); return 0; }
