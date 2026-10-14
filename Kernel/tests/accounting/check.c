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
/* DEFINE MODE_R */
/* DEFINE F_REG */
/* DEFINE CRDONLY */

#define CONFIG_ACCT
#define fd ((int16_t)udata.u_argn)
int16_t acct_fh=3;int privileged=1;int esuper(void){if(privileged)return 0;udata.u_error=EPERM;return -1;} void oft_deref(unsigned n){if(!--of_tab[n].o_refs)of_tab[n].o_inode=NULL;} int badfd;inoptr getinode(int n){if(badfd){udata.u_error=EBADF;return NULL;}return &node;} unsigned getmode(inoptr i){return MODE_R(i->c_node.i_mode);}


unsigned writes;uint32_t observed_offset;void writei(inoptr i,unsigned f){writes++;observed_offset=udata.u_offset;}

/* FUNCTION syscall_fs2.c _acct 0 */
/* FUNCTION process.c acctexit 0 */

static int case_disable(void)
{
of_tab[3].o_refs=1;of_tab[3].o_inode=&node;udata.u_argn=-1;TEST(_acct()==0 && acct_fh==-1 && !of_tab[3].o_refs,"release and disable");acctexit(ptab);TEST(!writes,"disabled accounting avoids freed entry");
return 0;
}

static int case_invalid_replacement(void)
{
of_tab[3].o_refs=1;of_tab[3].o_inode=&node;udata.u_argn=1;badfd=1;TEST(_acct()==-1 && acct_fh==3 && of_tab[3].o_refs==1,"keep old handle on failure");
return 0;
}

static int case_replace_same(void)
{
node.c_node.i_mode=F_REG;of_tab[3].o_refs=2;of_tab[3].o_inode=&node;udata.u_files[1]=3;udata.u_argn=1;TEST(_acct()==0 && acct_fh==3 && of_tab[3].o_refs==2 && of_tab[3].o_inode==&node,"replacement with same entry stays live");
return 0;
}

static int case_append(void)
{
of_tab[3].o_inode=&node;of_tab[3].o_ptr=9;node.c_node.i_size=200;udata.u_offset=17;acctexit(ptab);TEST(writes==1 && observed_offset==200 && of_tab[3].o_ptr==9,"append uses kernel IO offset and preserves shared file offset");
return 0;
}

static int case_unprivileged(void)
{
privileged=0;of_tab[3].o_refs=1;udata.u_argn=-1;TEST(_acct()==-1 && acct_fh==3 && of_tab[3].o_refs==1,"authorization failure preserves handle");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"disable"))return case_disable();
if(!strcmp(argv[1],"invalid-replacement"))return case_invalid_replacement();
if(!strcmp(argv[1],"replace-same"))return case_replace_same();
if(!strcmp(argv[1],"append"))return case_append();
if(!strcmp(argv[1],"unprivileged"))return case_unprivileged();
return 2;
}
