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

#define BLKSIZE 512
struct blkbuf {uint8_t data[512];uint16_t bf_dev,bf_blk,bf_time;uint8_t bf_dirty,bf_busy;};typedef struct blkbuf *bufptr;
static uint16_t bufclock;
#define bunlock(b) ((b)->bf_busy=0)
#define block_s(b) ((b)->bf_busy=1)
#define bunlock_s(b) ((b)->bf_busy=0)
int write_result=-1;unsigned writes;int bdwrite(bufptr p){writes++;if(write_result!=BLKSIZE)udata.u_error=EIO;return write_result;}
int d_flush(unsigned d){return 0;}

/* FUNCTION devio.c bfree 0 */
/* FUNCTION devio.c bdput 0 */

static int case_failed_immediate(void)
{
struct blkbuf b={.bf_dirty=1,.bf_busy=1};TEST(bfree(&b,2)==-1 && b.bf_dirty && !b.bf_busy && udata.u_error==EIO,"failed immediate write remains dirty and unlocks");
return 0;
}

static int case_retry_immediate(void)
{
struct blkbuf b={.bf_dirty=1,.bf_busy=1};TEST(bfree(&b,2)==-1,"first write fails");write_result=BLKSIZE;udata.u_error=0;b.bf_busy=1;TEST(bfree(&b,2)==0 && !b.bf_dirty && !b.bf_busy && writes==2,"retry writes data and then cleans buffer");
return 0;
}

static int case_failed_sync(void)
{
struct blkbuf b={.bf_dirty=1};bdput(&b);TEST(b.bf_dirty && !b.bf_busy && udata.u_error==EIO,"failed sync remains dirty and unlocks");
return 0;
}

static int case_retry_sync(void)
{
struct blkbuf b={.bf_dirty=1};bdput(&b);write_result=BLKSIZE;udata.u_error=0;bdput(&b);TEST(!b.bf_dirty && !b.bf_busy && writes==2,"successful sync retry cleans buffer");
return 0;
}

static int case_delayed(void)
{
struct blkbuf b={.bf_busy=1};TEST(bfree(&b,1)==0 && b.bf_dirty && !b.bf_busy && !writes,"delayed write does not perform IO");
return 0;
}

static int case_clean_sync(void)
{
struct blkbuf b={0};bdput(&b);TEST(!writes && !b.bf_busy,"clean sync does not perform IO");
return 0;
}

static int case_short_write(void)
{
struct blkbuf b={.bf_dirty=1,.bf_busy=1};write_result=BLKSIZE-1;TEST(bfree(&b,2)==-1 && b.bf_dirty && !b.bf_busy,"short device write retains dirty data");
return 0;
}

int main(int argc, char **argv)
{
if (argc != 2) return 2;
if (!strcmp(argv[1], "short-write")) return case_short_write();
if (!strcmp(argv[1], "failed-immediate")) return case_failed_immediate();
if (!strcmp(argv[1], "retry-immediate")) return case_retry_immediate();
if (!strcmp(argv[1], "failed-sync")) return case_failed_sync();
if (!strcmp(argv[1], "retry-sync")) return case_retry_sync();
if (!strcmp(argv[1], "delayed")) return case_delayed();
if (!strcmp(argv[1], "clean-sync")) return case_clean_sync();
return 2;
}
