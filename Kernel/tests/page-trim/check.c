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
#if TEST_USIZE_BITS == 16
typedef uint16_t usize_t;
#else
typedef uint32_t usize_t;
#endif
typedef int irqflags_t;
typedef uint16_t blkno_t;
#define staticfast static
#define min(a,b) ((a)<(b)?(a):(b))
#define TEST(c,msg) do {if(!(c)){puts(msg);return 1;}}while(0)
struct sigbits {uint16_t s_pending,s_held,s_ignored;};
struct p_tab {uint8_t p_status,p_flags,p_tty,p_nice,p_event;uint16_t p_pid,p_uid,p_pgrp,p_session,p_page,p_page2,p_timeout,p_priority,p_exitval; struct p_tab *p_pptr; struct sigbits p_sig[2]; unsigned p_time,p_utime,p_stime,p_cutime,p_cstime;};
typedef struct p_tab *ptptr;
struct cinode {uint16_t c_dev,c_num;struct {uint16_t i_mode,i_nlink,i_uid,i_gid;uint32_t i_size;uint16_t i_addr[20];} c_node;uint8_t c_refs,c_readers,c_writers,c_flags,c_super;};
struct s_queue {uint8_t *q_base,*q_head,*q_tail;int q_size,q_count,q_wakeup;};
typedef struct cinode *inoptr;
struct oft {inoptr o_inode;uint8_t o_access,o_refs;uint32_t o_ptr;};
struct rlimit {uint32_t rlim_cur,rlim_max;};
typedef struct u_data {arg_t u_argn,u_argn1,u_argn2;int u_error;bool u_sysio,u_ininterrupt,u_insys;uaddr_t u_syscall_sp,u_top,u_codebase,u_break;void *u_sp,*u_isp;uint16_t u_page,u_page2,u_euid,u_egid;uint8_t u_flags,u_name[8];int u_cursig;inoptr u_cwd,u_root;uint16_t u_block;unsigned u_nblock;uint8_t *u_dptr;uint8_t *u_base; unsigned u_count,u_done;uint32_t u_offset;uint16_t u_cloexec,u_mask;uint8_t u_files[16];ptptr u_ptab; int (*u_sigvec[32])(int);struct rlimit u_rlimit[9];} u_data;u_data udata;
struct p_tab ptab[16],*ptab_end=ptab+16;
struct oft of_tab[16];
struct cinode node;
#define NULLINODE ((inoptr)0)

#include "blk512.h"
#define swap_map(b) ((uint8_t*)(b))
#define major(d) ((d)>>8)
#define minor(d) ((d)&255)
#define HDIO_TRIM_KERNEL 123
static int result;
static unsigned reads,trims;
static blkno_t trimmed[128];
static uint16_t swappage;
static int device_read(unsigned minor,unsigned raw,unsigned flags){reads++;return result;}
static struct {int(*dev_read)(unsigned,unsigned,unsigned);} dev_tab[1]={{device_read}};
void panic(const char *reason){abort();}
int d_ioctl(unsigned dev,unsigned command,void *arg){trimmed[trims++]=*(blkno_t*)arg;return 0;}

/* FUNCTION page.c pageread 0 */

static int case_failed_read(void)
{
result=-1;TEST(pageread(0,10,1024,0,7)==-1&&!trims,"failed read keeps recovery image");
return 0;
}

static int case_short_read(void)
{
result=512;TEST(pageread(0,10,1024,0,7)==512&&!trims,"short read keeps recovery image");
return 0;
}

static int case_complete_read(void)
{
result=1024;TEST(pageread(0,10,1024,0,7)==1024&&reads==1&&swappage==7&&udata.u_nblock==2,"complete read setup");
#ifdef CONFIG_TRIM
TEST(trims==2&&trimmed[0]==10&&trimmed[1]==11,"trim completed image");
#else
TEST(!trims,"no trim configuration");
#endif

return 0;
}

static int case_zero_read(void)
{
result=0;TEST(pageread(0,10,0,0,7)==0&&!trims,"zero-length read trims nothing");
return 0;
}

/* Model the signed 16-bit byte-count return used by 8-bit targets. */
static int case_large_read(usize_t bytes)
{
#if TEST_USIZE_BITS == 16
result=(int16_t)bytes;
#else
result=bytes;
#endif
TEST(pageread(0,10,bytes,0,7)==result&&reads==1,
     "large completed read returns the driver byte-count bits");
#ifdef CONFIG_TRIM
TEST(trims==(bytes>>BLKSHIFT)&&trimmed[0]==10&&
     trimmed[trims-1]==10+trims-1,
     "large completed read discards the complete image");
#else
TEST(!trims,"TRIM-disabled large read requests no discard");
#endif
return 0;
}

int main(int argc,char **case_argv)
{
if(argc!=2)return 2;
if(!strcmp(case_argv[1],"failed-read"))return case_failed_read();
if(!strcmp(case_argv[1],"short-read"))return case_short_read();
if(!strcmp(case_argv[1],"complete-read"))return case_complete_read();
if(!strcmp(case_argv[1],"zero-read"))return case_zero_read();
if(!strcmp(case_argv[1],"large-read"))return case_large_read(32768);
if(!strcmp(case_argv[1],"max-16bit-read"))return case_large_read(65024);
return 2;
}
