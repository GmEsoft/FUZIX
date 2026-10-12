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

/* FUNCTION tty.c tty_read 0 */

static int case_mid_line(void)
{
uint8_t buf[8]={0};queued[0]='a';queued[1]='b';queued[2]=4;queued[3]=10;qlen=ttyinq[1].q_count=4;ttydata[1].termios.c_lflag=ICANON;ttydata[1].termios.c_cc[VEOF]=4;udata.u_ptab=ptab;udata.u_sysio=1;udata.u_base=buf;udata.u_count=8;TEST(tty_read(1,1,0)==2 && !buf[2],"EOF after data terminates without being copied");TEST(qpos==3 && ttyinq[1].q_count==1,"following newline remains queued");
return 0;
}

static int case_empty_line(void)
{
uint8_t buf[8];memset(buf,0xaa,sizeof(buf));queued[0]=4;qlen=ttyinq[1].q_count=1;ttydata[1].termios.c_lflag=ICANON;ttydata[1].termios.c_cc[VEOF]=4;udata.u_ptab=ptab;udata.u_sysio=1;udata.u_base=buf;udata.u_count=8;TEST(tty_read(1,1,0)==0 && buf[0]==0xaa && qpos==1,"empty EOF leaves user buffer untouched");
return 0;
}

static int case_noncanonical(void)
{
uint8_t buf[8]={0};queued[0]=4;qlen=ttyinq[1].q_count=1;ttydata[1].termios.c_cc[VEOF]=4;udata.u_ptab=ptab;udata.u_sysio=1;udata.u_base=buf;udata.u_count=1;TEST(tty_read(1,1,0)==1 && buf[0]==4,"raw EOF byte remains data");
return 0;
}

static int case_newline(void)
{
uint8_t buf[8]={0};queued[0]='a';queued[1]=10;qlen=ttyinq[1].q_count=2;ttydata[1].termios.c_lflag=ICANON;ttydata[1].termios.c_cc[VEOF]=4;udata.u_ptab=ptab;udata.u_base=buf;udata.u_count=8;TEST(tty_read(1,1,0)==2 && !memcmp(buf,"a\n",2),"ordinary canonical newline preserved");
return 0;
}

int main(int argc, char **argv)
{
if (argc != 2) return 2;
if (!strcmp(argv[1], "mid-line")) return case_mid_line();
if (!strcmp(argv[1], "empty-line")) return case_empty_line();
if (!strcmp(argv[1], "noncanonical")) return case_noncanonical();
if (!strcmp(argv[1], "newline")) return case_newline();
return 2;
}
