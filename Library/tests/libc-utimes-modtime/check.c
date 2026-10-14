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
#include <sys/time.h>
#include <utime.h>
static struct utimbuf got; static int null_call;
static int test_utime(const char *p,const struct utimbuf *u) { if(u)got=*u; else null_call++; return 7; }
#define utime test_utime
#define utimes fuzix_utimes
/* SOURCE libs/utimes.c */
int main(void) { struct timeval t[2]={{123,456},{789,12345}}; assert(fuzix_utimes("x",t)==7); assert(got.actime==123 && got.modtime==789); assert(fuzix_utimes("x",NULL)==7 && null_call==1); return 0; }
