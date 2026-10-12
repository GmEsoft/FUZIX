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
#include <time.h>
#define NO_64BIT
#define clock_gettime fuzix_clock_gettime
typedef struct {uint32_t low,high;} __ktime_t;
static uint32_t ticks;static int _time(__ktime_t *p,int kind){p->low=ticks;p->high=0;return 0;}
/* SOURCE libs/clock_gettime.c */
static void check(uint32_t n){struct timespec t;ticks=n;assert(fuzix_clock_gettime(CLOCK_MONOTONIC,&t)==0);assert(t.tv_sec==n/10 && t.tv_nsec==(long)(n%10)*100000000L);}
int main(void){for(uint32_t i=0;i<100000;i++)check(i);for(uint32_t i=UINT32_MAX-99;i!=UINT32_MAX;i++)check(i);check(UINT32_MAX);uint32_t s=1;for(int i=0;i<10000;i++){s=s*1664525U+1013904223U;check(s);}return 0;}
