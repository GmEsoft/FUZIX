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
/* DEFINE SIGBUS */

#define MISALIGNED(p,a) ((uintptr_t)(p)&((a)-1))
unsigned valaddr_r(const void*p,unsigned n){return n;}unsigned valaddr_w(const void*p,unsigned n){return n;}
unsigned signals,accesses;ptptr signalled;void ssig(ptptr p,int s){signals++;signalled=p;}uint16_t _ugetw(const void*p){accesses++;return 0x1234;}int _uputw(uint16_t v,void*p){accesses++;return 0;}
uint32_t _ugetl(const void*p){accesses++;return 0x12345678;}int _uputl(uint32_t v,void*p){accesses++;return 0;}


/* FUNCTION usermem.c ugetw 0 */
/* FUNCTION usermem.c uputw 0 */
/* FUNCTION usermem.c ugetl 0 */
/* FUNCTION usermem.c uputl 0 */

static int case_words(void)
{
udata.u_ptab=ptab;TEST(ugetw((void*)1)==UINT16_MAX && uputw(1,(void*)1)==-1 && signals==2 && !accesses && signalled==ptab,"misaligned words signal current process without access");
return 0;
}

static int case_longs(void)
{
udata.u_ptab=ptab;TEST(ugetl((void*)2)==UINT32_MAX && uputl(1,(void*)2)==-1 && signals==2 && !accesses && signalled==ptab,"longs require four-byte alignment");
return 0;
}

static int case_aligned(void)
{
TEST(ugetw((void*)4)==0x1234 && uputw(1,(void*)4)==0 && ugetl((void*)4)==0x12345678 && uputl(1,(void*)4)==0 && accesses==4 && !signals,"aligned helpers delegate");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"words"))return case_words();
if(!strcmp(argv[1],"longs"))return case_longs();
if(!strcmp(argv[1],"aligned"))return case_aligned();
return 2;
}
