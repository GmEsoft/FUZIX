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
/* DEFINE F_SETFL */
/* DEFINE F_GETFL */
/* DEFINE F_GETFD */
/* DEFINE F_SETFD */
/* DEFINE F_DUPFD */
/* DEFINE O_APPEND */
/* DEFINE O_NDELAY */
/* DEFINE O_CLOEXEC */

#define fd ((int)udata.u_argn)
#define request ((int)udata.u_argn1)
#define data ((int)udata.u_argn2)
inoptr getinode(int n){return &node;} int _dup(void){abort();}
#define UFTSIZE 16
#define NO_FILE 255



/* FUNCTION filesys.c uf_alloc_n 0 */
/* FUNCTION syscall_fs3.c _fcntl 0 */

static int case_minimum(void)
{
memset(udata.u_files,NO_FILE,16);udata.u_files[2]=0;of_tab[0].o_refs=1;udata.u_argn=2;udata.u_argn1=F_DUPFD;udata.u_argn2=7;udata.u_cloexec=0xffff;TEST(_fcntl()==7 && udata.u_files[7]==0 && of_tab[0].o_refs==2,"honor minimum and duplicate reference");TEST(udata.u_cloexec==(uint16_t)(0xffff & ~(1U<<7)),"clear only new close-on-exec bit");
return 0;
}

static int case_occupied_minimum(void)
{
memset(udata.u_files,NO_FILE,16);udata.u_files[7]=0;TEST(uf_alloc_n(7)==8,"scan past occupied minimum");
return 0;
}

static int case_invalid_minimum(void)
{
udata.u_argn1=F_DUPFD;int vals[]={-1,16,256,32767};for(unsigned i=0;i<4;i++){udata.u_argn2=vals[i];udata.u_error=0;TEST(_fcntl()==-1 && udata.u_error==EINVAL,"reject full-width invalid minimum");}TEST(uf_alloc_n(16)==-1 && udata.u_error==EINVAL,"allocator bounds");
return 0;
}

static int case_exhausted(void)
{
memset(udata.u_files,0,16);TEST(uf_alloc_n(7)==-1 && udata.u_error==EMFILE,"no available descriptor");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"minimum"))return case_minimum();
if(!strcmp(argv[1],"occupied-minimum"))return case_occupied_minimum();
if(!strcmp(argv[1],"invalid-minimum"))return case_invalid_minimum();
if(!strcmp(argv[1],"exhausted"))return case_exhausted();
return 2;
}
