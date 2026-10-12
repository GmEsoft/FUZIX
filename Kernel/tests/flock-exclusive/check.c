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
/* DEFINE CFLOCK */
/* DEFINE CFLEX */
/* DEFINE CFMAX */
/* DEFINE O_FLOCK */
/* DEFINE LOCK_SH */
/* DEFINE LOCK_EX */
/* DEFINE LOCK_UN */
/* DEFINE LOCK_NB */
/* DEFINE O_NDELAY */

#define file ((uint16_t)udata.u_argn)
#define lockop ((uint16_t)udata.u_argn1)
inoptr getinode(int n){return &node;}

void wakeup(void*p){}
int psleep_flags(void*p,unsigned f){udata.u_error=EAGAIN;return -1;}

/* FUNCTION filesys.c deflock 0 */
/* FUNCTION syscall_fs3.c _flock 0 */

static int case_exclusive_shared(void)
{
of_tab[0].o_inode=&node;node.c_flags=1;udata.u_argn1=LOCK_EX|LOCK_NB;TEST(_flock()==-1 && udata.u_error==EAGAIN && node.c_flags==1 && !of_tab[0].o_access,"exclusive lock blocks behind shared holder");
return 0;
}

static int case_exclusive_many(void)
{
of_tab[0].o_inode=&node;node.c_flags=CFMAX;udata.u_argn1=LOCK_EX|LOCK_NB;TEST(_flock()==-1 && udata.u_error==EAGAIN,"exclusive contention is not shared-lock exhaustion");
return 0;
}

static int case_shared_shared(void)
{
of_tab[0].o_inode=&node;node.c_flags=1;udata.u_argn1=LOCK_SH|LOCK_NB;TEST(_flock()==0 && node.c_flags==2 && (of_tab[0].o_access&O_FLOCK),"shared lock coexists with shared holder");
return 0;
}

static int case_exclusive_empty(void)
{
of_tab[0].o_inode=&node;udata.u_argn1=LOCK_EX|LOCK_NB;TEST(_flock()==0 && node.c_flags==CFLEX,"exclusive lock acquires empty inode");
return 0;
}

static int case_upgrade_downgrade(void)
{
of_tab[0].o_inode=&node;of_tab[0].o_access=O_FLOCK;node.c_flags=1;udata.u_argn1=LOCK_EX|LOCK_NB;TEST(_flock()==0 && node.c_flags==CFLEX,"sole shared holder upgrades");udata.u_argn1=LOCK_SH;TEST(_flock()==0 && node.c_flags==1,"exclusive holder downgrades");
return 0;
}

static int case_contended_upgrade(void)
{
of_tab[0].o_inode=&node;of_tab[0].o_access=O_FLOCK;node.c_flags=2;udata.u_argn1=LOCK_EX|LOCK_NB;TEST(_flock()==-1 && node.c_flags==2,"upgrade must not grant exclusive while another holder remains");
return 0;
}

static int case_unlock_nonblocking(void)
{
of_tab[0].o_inode=&node;of_tab[0].o_access=O_FLOCK;node.c_flags=CFLEX;udata.u_argn1=LOCK_UN|LOCK_NB;TEST(_flock()==0 && !node.c_flags && !(of_tab[0].o_access&O_FLOCK),"unlock tolerates nonblocking modifier");
return 0;
}

static int case_invalid_operation(void)
{
of_tab[0].o_inode=&node;udata.u_argn1=LOCK_EX|LOCK_UN;TEST(_flock()==-1 && udata.u_error==EINVAL && !node.c_flags,"mixed lock operations rejected");
return 0;
}

int main(int argc, char **argv)
{
if (argc != 2) return 2;
if (!strcmp(argv[1], "exclusive-shared")) return case_exclusive_shared();
if (!strcmp(argv[1], "exclusive-many")) return case_exclusive_many();
if (!strcmp(argv[1], "shared-shared")) return case_shared_shared();
if (!strcmp(argv[1], "exclusive-empty")) return case_exclusive_empty();
if (!strcmp(argv[1], "upgrade-downgrade")) return case_upgrade_downgrade();
if (!strcmp(argv[1], "contended-upgrade")) return case_contended_upgrade();
if (!strcmp(argv[1], "unlock-nonblocking")) return case_unlock_nonblocking();
if (!strcmp(argv[1], "invalid-operation")) return case_invalid_operation();
return 2;
}
