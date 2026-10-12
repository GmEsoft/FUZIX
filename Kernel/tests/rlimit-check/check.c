#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <setjmp.h>
#define rlimit kernel_rlimit
#define time_t kernel_time_t
typedef intptr_t arg_t;
typedef uintptr_t uarg_t, uaddr_t, uptr_t;
#define clock_t kernel_clock_t
typedef uint32_t clock_t;
typedef unsigned usize_t;
typedef int irqflags_t;
typedef uint16_t blkno_t;
#define staticfast static
#define min(a,b) ((a)<(b)?(a):(b))
#define TEST(c,msg) do {if(!(c)){puts(msg);return 1;}}while(0)

#define rlim_t kernel_rlim_t
#undef RLIMIT_AS
#undef RLIMIT_CORE
#undef RLIMIT_CPU
#undef RLIMIT_DATA
#undef RLIMIT_FSIZE
#undef RLIMIT_NOFILE
#undef RLIMIT_NPROC
#undef RLIMIT_RSS
#undef RLIMIT_STACK
#undef RLIM_INFINITY
#include "level2.h"
struct {struct rlimit u_rlimit[NRLIMIT];} udata;


static int case_soft_limit(void)
{
udata.u_rlimit[RLIMIT_AS]=(struct rlimit){100,200};TEST(!limit_exceeded(RLIMIT_AS,100) && limit_exceeded(RLIMIT_AS,101),"compare current soft limit");
return 0;
}

static int case_infinity(void)
{
udata.u_rlimit[RLIMIT_FSIZE]=(struct rlimit){RLIM_INFINITY,RLIM_INFINITY};TEST(!limit_exceeded(RLIMIT_FSIZE,0xffffffffUL),"unlimited soft limit");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"soft-limit"))return case_soft_limit();
if(!strcmp(argv[1],"infinity"))return case_infinity();
return 2;
}
