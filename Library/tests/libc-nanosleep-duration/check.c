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
#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME 1
#endif
static struct timespec now;static uint64_t total;static unsigned calls,maximum;
static int test_clock(clockid_t c,struct timespec *t){*t=now;return 0;}
static int test_pause(unsigned t){assert(t>0 && t<=65530);calls++;total+=t;if(t>maximum)maximum=t;return 0;}
#define clock_gettime test_clock
#define _pause test_pause
#define nanosleep fuzix_nanosleep
#define clock_nanosleep fuzix_clock_nanosleep
/* SOURCE libs/nanosleep.c */
static void relative(time_t sec,long ns,uint64_t expected){struct timespec t={sec,ns};calls=0;total=0;maximum=0;assert(fuzix_nanosleep(&t,NULL)==0);assert(total==expected);assert(expected?calls>0:calls==0);}
int main(void){relative(0,0,0);relative(0,1,1);relative(0,100000000,1);relative(0,200000000,2);relative(0,999999999,10);relative(1,1,11);relative(7000,0,70000);relative(65535,100000000,655351);now.tv_sec=100;now.tv_nsec=900000000;struct timespec t={101,100000000};total=0;assert(fuzix_clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&t,NULL)==0&&total==2);t.tv_sec=99;calls=0;assert(fuzix_clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&t,NULL)==0&&calls==0);t.tv_nsec=1000000000;assert(fuzix_nanosleep(&t,NULL)==-1&&errno==EINVAL);return 0;}
