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
/* DEFINE SIGCHLD */
/* DEFINE P_EMPTY */
/* DEFINE P_ZOMBIE */
/* DEFINE P_RUNNING */

unsigned signals,wakes;ptptr target;void ssig(ptptr p,unsigned s){signals++;target=p;}void wakeup(void*p){wakes++;}


/* FUNCTION process.c signal_parent 0 */

static int case_ignored(void)
{
udata.u_ptab=ptab;ptab[0].p_status=P_RUNNING;ptab[1].p_status=P_ZOMBIE;ptab[1].p_pptr=ptab+2;ptab[2].p_sig[1].s_ignored=1<<(SIGCHLD-16);TEST(signal_parent(ptab+1)==0 && ptab[1].p_status==P_EMPTY && ptab[0].p_status==P_RUNNING && !signals,"reap specified child");
return 0;
}

static int case_zombie(void)
{
udata.u_ptab=ptab;ptab[0].p_status=P_RUNNING;ptab[1].p_status=P_RUNNING;ptab[1].p_pptr=ptab+2;TEST(signal_parent(ptab+1)==1 && ptab[1].p_status==P_ZOMBIE && ptab[0].p_status==P_RUNNING && signals==1 && target==ptab+2 && wakes==1,"mark specified child zombie and wake its parent");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"ignored"))return case_ignored();
if(!strcmp(argv[1],"zombie"))return case_zombie();
return 2;
}
