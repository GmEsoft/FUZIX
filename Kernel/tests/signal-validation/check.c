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
/* DEFINE NSIGS */
/* DEFINE SIGKILL */
/* DEFINE SIGSTOP */
/* DEFINE SIG_IGN */
/* DEFINE SIG_DFL */
/* DEFINE sigmask */

#define sig ((int16_t)udata.u_argn)
#define func ((int(*)(int))udata.u_argn1)
int irqstate,invalid,refresh;irqflags_t di(void){int old=irqstate;irqstate=1;return old;}void irqrestore(irqflags_t i){irqstate=i;}unsigned valaddr_r(const void*p,unsigned n){return invalid?0:n;}void recalc_cursig(void){refresh++;}


/* FUNCTION syscall_proc.c sigbits 0 */
/* FUNCTION syscall_proc.c _signal 0 */

static int case_invalid_signals(void)
{
udata.u_ptab=ptab;int vals[]={-1,0,32,33};for(unsigned i=0;i<4;i++){udata.u_argn=vals[i];TEST(_signal()==-1 && udata.u_error==EINVAL && !irqstate && !refresh,"invalid signal is rejected");}
return 0;
}

static int case_bad_handler(void)
{
udata.u_ptab=ptab;udata.u_argn=2;udata.u_argn1=123;invalid=1;TEST(_signal()==-1 && udata.u_error==EFAULT && !irqstate && !refresh,"invalid handler restores interrupts");
return 0;
}

static int case_valid_last(void)
{
udata.u_ptab=ptab;udata.u_argn=31;udata.u_argn1=(intptr_t)SIG_IGN;TEST(_signal()==(arg_t)SIG_DFL && udata.u_sigvec[31]==SIG_IGN && !irqstate && refresh==1,"last signal accepted");
return 0;
}

static int case_disabled_on_entry(void)
{
udata.u_ptab=ptab;udata.u_argn=2;udata.u_argn1=123;invalid=1;irqstate=1;TEST(_signal()==-1 && irqstate==1,"preserve already disabled state");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"invalid-signals"))return case_invalid_signals();
if(!strcmp(argv[1],"bad-handler"))return case_bad_handler();
if(!strcmp(argv[1],"valid-last"))return case_valid_last();
if(!strcmp(argv[1],"disabled-on-entry"))return case_disabled_on_entry();
return 2;
}
