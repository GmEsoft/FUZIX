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

static int bank,process_switches,kernel_switches;static uint8_t bytes[8];
#define BANK_KERNEL (bank=0,kernel_switches++)
#define BANK_PROCESS (bank=1,process_switches++)

/* FUNCTION usermem.c _uzero 0 */
/* FUNCTION usermem.c _ugetl 0 */
/* FUNCTION usermem.c _uputl 0 */

static int case_clear_range(void)
{
memset(bytes,9,8);TEST(_uzero(bytes+1,6)==0 && !bank && process_switches==1 && kernel_switches==1,"restore kernel mapping");TEST(bytes[0]==9 && bytes[7]==9,"preserve surrounding bytes");for(unsigned i=1;i<7;i++)TEST(!bytes[i],"zero selected bytes");
return 0;
}

static int case_long_transfer(void)
{
uint32_t value=0x12345678;TEST(_ugetl(&value)==value && !bank,"read long and restore mapping");TEST(_uputl(0xabcdef01,&value)==0 && value==0xabcdef01 && !bank,"write long and restore mapping");
return 0;
}

static int case_empty_range(void)
{
memset(bytes,9,8);TEST(_uzero(bytes,0)==0 && !bank && bytes[0]==9,"zero-length call restores mapping");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"clear-range"))return case_clear_range();
if(!strcmp(argv[1],"long-transfer"))return case_long_transfer();
if(!strcmp(argv[1],"empty-range"))return case_empty_range();
return 2;
}
