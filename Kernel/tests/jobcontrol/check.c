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

#define NUM_DEV_TTY 1
#include "tty.h"

struct tty ttydata[2];
struct s_queue ttyinq[2];
uint8_t queued[16];unsigned qpos,qlen,putpos,timer_calls,ready_calls;uint8_t output[16];
int remq(struct s_queue*q,uint_fast8_t*c){if(qpos>=qlen)return 0;*c=queued[qpos++];q->q_count--;return 1;}
void tty_data_consumed(uint_fast8_t m){}void wakeup(void*p){}
int psleep_flags_io(void*p,int f){return -1;}void ptimer_insert(void){timer_calls++;}
unsigned valaddr_r(const void*p,unsigned n){return n;}int uputc(uint16_t c,void*p){*(uint8_t*)p=c;return 0;}
uint8_t _ugetc(const void*p){return *(const uint8_t*)p;}
int chksigs(void){return 0;}irqflags_t di(void){return 0;}void irqrestore(irqflags_t i){}void psleep(void*p){abort();}void switchout(void){abort();}
#define need_reschedule() 0
void tty_sleeping(uint_fast8_t m){}void tty_putc(uint_fast8_t m,uint_fast8_t c){output[putpos++]=c;}
ttyready_t tty_writeready(uint_fast8_t m){return ready_calls++==0?TTY_READY_NOW:TTY_READY_LATER;}
/* DEFINE SIGTTIN */
/* DEFINE SIGTTOU */
/* DEFINE SIG_IGN */
/* DEFINE sigmask */

void ssig(ptptr p,unsigned s){}


/* FUNCTION level2.c orphan_pgrp 0 */
/* FUNCTION level2.c jobop 0 */
/* FUNCTION level2.c jobcontrol_in 0 */
/* FUNCTION level2.c jobcontrol_out 0 */

static int case_blocked_input(void)
{
udata.u_ptab=ptab;ptab[0].p_pgrp=123;ptab[0].p_session=7;ptab[0].p_tty=1;ptab[0].p_pptr=ptab+2;ptab[2].p_pgrp=456;ptab[2].p_session=7;ttydata[1].pgrp=999;ptab[0].p_sig[1].s_held=sigmask(SIGTTIN);TEST(jobcontrol_in(1,ttydata+1) && udata.u_error==EIO && udata.u_done==(usize_t)-1,"blocked input returns EIO");
return 0;
}

static int case_blocked_output(void)
{
udata.u_ptab=ptab;ptab[0].p_pgrp=123;ptab[0].p_session=7;ptab[0].p_tty=1;ptab[0].p_pptr=ptab+2;ptab[2].p_pgrp=456;ptab[2].p_session=7;ttydata[1].pgrp=999;ptab[0].p_sig[1].s_held=sigmask(SIGTTOU);ttydata[1].termios.c_lflag=TOSTOP;TEST(!jobcontrol_out(1,ttydata+1) && !udata.u_error,"blocked output proceeds");
return 0;
}

static int case_ignored_input(void)
{
udata.u_ptab=ptab;ptab[0].p_pgrp=123;ptab[0].p_session=7;ptab[0].p_tty=1;ptab[0].p_pptr=ptab+2;ptab[2].p_pgrp=456;ptab[2].p_session=7;ttydata[1].pgrp=999;udata.u_sigvec[SIGTTIN]=SIG_IGN;TEST(jobcontrol_in(1,ttydata+1) && udata.u_error==EIO,"ignored input fails EIO");
return 0;
}

static int case_ignored_orphan_output(void)
{
udata.u_ptab=ptab;ptab[0].p_pgrp=123;ptab[0].p_session=7;ptab[0].p_tty=1;ptab[0].p_pptr=ptab+2;ptab[2].p_pgrp=456;ptab[2].p_session=7;ttydata[1].pgrp=999;ptab[2].p_session=0;udata.u_sigvec[SIGTTOU]=SIG_IGN;ttydata[1].termios.c_lflag=TOSTOP;TEST(!jobcontrol_out(1,ttydata+1) && !udata.u_error,"ignored output proceeds even in orphan group");
return 0;
}

static int case_foreground(void)
{
udata.u_ptab=ptab;ptab[0].p_pgrp=123;ptab[0].p_session=7;ptab[0].p_tty=1;ptab[0].p_pptr=ptab+2;ptab[2].p_pgrp=456;ptab[2].p_session=7;ttydata[1].pgrp=999;ttydata[1].pgrp=123;TEST(!jobcontrol_in(1,ttydata+1) && !udata.u_error,"foreground read proceeds");
return 0;
}

static int case_partial_input(void)
{
udata.u_ptab=ptab;ptab[0].p_pgrp=123;ptab[0].p_session=7;ptab[0].p_tty=1;ptab[0].p_pptr=ptab+2;ptab[2].p_pgrp=456;ptab[2].p_session=7;ttydata[1].pgrp=999;ptab[0].p_sig[1].s_held=sigmask(SIGTTIN);udata.u_done=2;TEST(jobcontrol_in(1,ttydata+1) && udata.u_done==2 && udata.u_error==EIO,"preserve completed count");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"blocked-input"))return case_blocked_input();
if(!strcmp(argv[1],"blocked-output"))return case_blocked_output();
if(!strcmp(argv[1],"ignored-input"))return case_ignored_input();
if(!strcmp(argv[1],"ignored-orphan-output"))return case_ignored_orphan_output();
if(!strcmp(argv[1],"foreground"))return case_foreground();
if(!strcmp(argv[1],"partial-input"))return case_partial_input();
return 2;
}
