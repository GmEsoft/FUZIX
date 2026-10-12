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
/* DEFINE PFL_BATCH */

static jmp_buf escape;
static ptptr swapproc,swapnext;
static unsigned allocations,loads,released,wakes,daemon_frees,execs,fork_result;
static uint16_t loaded_map,released_map;
irqflags_t di(void){return 0;}void irqrestore(irqflags_t state){}
void kprintf(const char *format,...){}void kputs(const char *text){}
void pagemap_free(ptptr p){daemon_frees++;}
void pagemap_alloc(ptptr p){allocations++;p->p_page=11;p->p_page2=12;}
void swapin(ptptr p,uint16_t map){loads++;loaded_map=map;}
void swapmap_add(uint16_t map){released++;released_map=map;}
void pwake(ptptr p){if(++wakes>1)longjmp(escape,1);}
void switchout(void){longjmp(escape,1);}
void swapper(ptptr p);
ptptr ptab_alloc(void){return ptab+1;}
int16_t dofork(ptptr p){return fork_result;}
void plt_discard(void){}
int _execve(void){execs++;longjmp(escape,1);}
#define PANIC_NOINIT "init"
void panic(const char *message){abort();}

/* FUNCTION swap.c swapper2 0 */
/* FUNCTION swap.c swapload optional */
/* FUNCTION swap.c swapper 1 */
/* FUNCTION swap.c swaptask 0 */
/* FUNCTION process.c exec_or_die 0 */

static int case_daemon_load(void)
{
udata.u_ptab=ptab;ptab[1].p_page2=77;ptab[1].p_flags=PFL_BATCH;swapnext=ptab+1;if(!setjmp(escape))swaptask();TEST(allocations==1&&loads==1&&loaded_map==77&&released==1&&released_map==77&&!swapnext&&!wakes,"daemon loads instead of requeuing");TEST(udata.u_page==11&&udata.u_page2==12&&!(ptab[1].p_flags&PFL_BATCH),"complete load bookkeeping");
return 0;
}

static int case_enqueue(void)
{
swapproc=ptab;swapper(ptab+1);TEST(swapnext==ptab+1&&wakes==1&&!loads,"asynchronous entry queues work");
return 0;
}

static int case_init_parent(void)
{
udata.u_ptab=ptab;fork_result=2;if(!setjmp(escape))exec_or_die();TEST(execs==1&&!daemon_frees,"parent continues to init");
return 0;
}

static int case_daemon_child(void)
{
udata.u_ptab=ptab;fork_result=0;if(!setjmp(escape))exec_or_die();TEST(!execs&&daemon_frees==1&&swapproc==ptab,"only child enters daemon");
return 0;
}

int main(int argc,char **case_argv)
{
if(argc!=2)return 2;
if(!strcmp(case_argv[1],"daemon-load"))return case_daemon_load();
if(!strcmp(case_argv[1],"enqueue"))return case_enqueue();
if(!strcmp(case_argv[1],"init-parent"))return case_init_parent();
if(!strcmp(case_argv[1],"daemon-child"))return case_daemon_child();
return 2;
}
