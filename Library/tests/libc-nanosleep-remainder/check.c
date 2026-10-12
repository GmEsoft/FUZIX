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
static struct timespec start,end;static int queries;
static int test_clock(clockid_t c,struct timespec *t){*t=queries++?end:start;return 0;}
static int test_pause(unsigned t){errno=EINTR;return -1;}
#define clock_gettime test_clock
#define _pause test_pause
#define nanosleep fuzix_nanosleep
#define clock_nanosleep fuzix_clock_nanosleep
/* SOURCE libs/nanosleep.c */
static void check(time_t endsec,long endns,time_t sec,long ns){struct timespec req={10,200000000},rem={99,99};start.tv_sec=100;start.tv_nsec=500000000;end.tv_sec=endsec;end.tv_nsec=endns;queries=0;assert(fuzix_nanosleep(&req,&rem)==-1&&errno==EINTR);assert(rem.tv_sec==sec&&rem.tv_nsec==ns);}
int main(void){check(103,500000000,7,200000000);check(103,900000000,6,800000000);check(111,0,0,0);check(99,0,10,200000000);struct timespec req={110,200000000},rem={55,66};start.tv_sec=100;start.tv_nsec=0;end.tv_sec=103;end.tv_nsec=0;queries=0;assert(fuzix_clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,&req,&rem)==-1&&rem.tv_sec==55&&rem.tv_nsec==66);return 0;}
