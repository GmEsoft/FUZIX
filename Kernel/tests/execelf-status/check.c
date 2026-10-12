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
struct p_tab {uint8_t p_status,p_flags,p_tty,p_nice,p_event;uint16_t p_pid,p_uid,p_pgrp,p_session,p_page,p_page2,p_timeout,p_priority,p_exitval; struct p_tab *p_pptr; struct sigbits p_sig[2]; uint8_t p_name[8];unsigned p_time,p_utime,p_stime,p_cutime,p_cstime;};
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
/* DEFINE F_REG */
/* DEFINE OWN_EX */
/* DEFINE OTH_EX */
/* DEFINE GRP_EX */
/* DEFINE OTH_RD */
/* DEFINE MS_NOEXEC */
/* DEFINE MS_NOSUID */
/* DEFINE A_TIME */
/* DEFINE P_NOSLEEP */
/* DEFINE P_RUNNING */
/* DEFINE SIGKILL */
/* DEFINE SET_UID */
/* DEFINE SET_GID */
/* DEFINE U_FLAG_NOCORE */

#include "elf.h"
#include "blk512.h"
#define name ((uint8_t*)udata.u_argn)
#define argv ((uint8_t**)udata.u_argn1)
#define envp ((uint8_t**)udata.u_argn2)
#define PROGLOAD 0
#define USERSTACK 128
#define ALIGNUP(x) (x)
struct s_argblk {int a_argc;uint8_t bytes[500];};
static uaddr_t ramtop=65536;
static struct {unsigned m_flags;} fs_tab[1];
static Elf32_Ehdr header;
static Elf32_Phdr program[2];
static unsigned buffers,freed,derefs,reallocs,closed;
static int fail_args,fail_header;
inoptr n_open_argn(void){node.c_refs++;return &node;}
int getperm(inoptr ino){return OTH_EX|OTH_RD;}
void setftime(inoptr ino,unsigned flags){}
void readi(inoptr ino,unsigned flags){if(udata.u_offset==0)memcpy(udata.u_base,&header,sizeof(header));else memcpy(udata.u_base,program,udata.u_count);udata.u_done=udata.u_count;}
void *tmpbuf(void){buffers++;return calloc(1,512);}
void tmpfree(void *p){freed++;free(p);}
bool rargs(uint8_t **args,struct s_argblk *buf){return fail_args;}
int pagemap_realloc(void *header,uaddr_t end){reallocs++;return 1;}
void close_on_exec(void){closed++;}
unsigned valaddr_r(void *p,unsigned n){return n;}
int uzero(void *p,unsigned n){return 0;}
int plt_relocate_rel(Elf32_Rel *rel,uaddr_t base){return 0;}
uint8_t **wargs(uint8_t *p,struct s_argblk *buf,int *argc){return NULL;}
uaddr_t ugetp(void *p){return 0;}
int uget(void *p,void *out,unsigned n){return 0;}
int uputp(uaddr_t value,void *p){return 0;}
void i_deref(inoptr ino){derefs++;ino->c_refs--;}
void ssig(ptptr p,unsigned sig){}
void doexec(uaddr_t address){abort();}
static void setup(void)
{
    memcpy(header.e_ident,ELFMAG,SELFMAG);header.e_phnum=2;header.e_phentsize=sizeof(Elf32_Phdr);header.e_phoff=64;
    program[0].p_type=PT_LOAD;program[0].p_memsz=512;
    program[1].p_type=PT_DYNAMIC;program[1].p_vaddr=1024;
    node.c_node.i_mode=F_REG|OWN_EX;node.c_refs=1;udata.u_ptab=ptab;ptab[0].p_status=P_RUNNING;
}

/* FUNCTION syscall_execelf32.c _execve 0 */

static int case_realloc_failure(void)
{
setup();TEST(_execve()==-1&&udata.u_error==ENOMEM&&ptab[0].p_status==P_RUNNING,"restore runnable status on allocation failure");TEST(reallocs==1&&buffers==3&&freed==3&&derefs==1&&node.c_refs==1&&!closed,"release staging resources before exec commitment");
return 0;
}

static int case_argument_failure(void)
{
setup();fail_args=1;TEST(_execve()==-1&&ptab[0].p_status==P_RUNNING&&!reallocs&&buffers==3&&freed==3&&derefs==1,"early argument failure preserves runnable status");
return 0;
}

static int case_header_failure(void)
{
setup();header.e_ident[0]=0;TEST(_execve()==-1&&udata.u_error==ENOEXEC&&ptab[0].p_status==P_RUNNING&&!buffers&&derefs==1,"early header failure");
return 0;
}

int main(int argc,char **case_argv)
{
if(argc!=2)return 2;
if(!strcmp(case_argv[1],"realloc-failure"))return case_realloc_failure();
if(!strcmp(case_argv[1],"argument-failure"))return case_argument_failure();
if(!strcmp(case_argv[1],"header-failure"))return case_header_failure();
return 2;
}
