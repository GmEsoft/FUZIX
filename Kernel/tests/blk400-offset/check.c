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
#include "blk400.h"


static int case_large_offsets(void)
{
uint32_t offsets[]={0,399,400,65535,65536,65537,131072,0xffffffffUL};for(unsigned i=0;i<sizeof(offsets)/sizeof(*offsets);i++)TEST(BLKOFF(offsets[i])==offsets[i]%400,"full-width block remainder");
return 0;
}

static int case_block_consistency(void)
{
for(uint32_t x=65000;x<67000;x++)TEST((uint32_t)BLOCK(x)*BLKSIZE+BLKOFF(x)==x,"block and offset reconstruct file position");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"large-offsets"))return case_large_offsets();
if(!strcmp(argv[1],"block-consistency"))return case_block_consistency();
return 2;
}
