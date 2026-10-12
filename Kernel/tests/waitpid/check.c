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
/* DEFINE WNOHANG */
/* DEFINE WUNTRACED */
/* DEFINE P_ZOMBIE */
/* DEFINE P_RUNNING */
/* DEFINE P_EMPTY */
/* DEFINE _WSTOPPED */

#define pid udata.u_argn
#define statloc ((int*)udata.u_argn1)
#define options ((int)udata.u_argn2)
unsigned valaddr_w(void*a,unsigned n){return n;} int chksigs(void){return 0;} void psleep(void*p){abort();} void uputi(int n,int*p){*p=n;}

/* FUNCTION syscall_proc.c _waitpid 0 */

static int case_nohang(void)
{
udata.u_ptab=ptab;ptab[1].p_status=P_RUNNING;ptab[1].p_pptr=ptab;ptab[1].p_pid=42;udata.u_argn=-1;udata.u_argn2=WNOHANG;TEST(_waitpid()==0 && !udata.u_error,"running child returns zero");
return 0;
}

static int case_stopped(void)
{
int status=123;udata.u_ptab=ptab;ptab[1].p_status=P_RUNNING;ptab[1].p_pptr=ptab;ptab[1].p_pid=42;ptab[1].p_event=4;udata.u_argn=-1;udata.u_argn1=(intptr_t)&status;udata.u_argn2=WUNTRACED;TEST(_waitpid()==42 && status==((4<<8)|_WSTOPPED),"return PID and copy stopped status");TEST(!ptab[1].p_event && ptab[1].p_status==P_RUNNING,"consume event without reaping child");
return 0;
}

static int case_unmatched(void)
{
udata.u_ptab=ptab;ptab[1].p_status=P_RUNNING;ptab[1].p_pptr=ptab;ptab[1].p_pid=42;udata.u_argn=99;udata.u_argn2=WNOHANG;TEST(_waitpid()==-1 && udata.u_error==ECHILD,"absent requested PID yields ECHILD");
return 0;
}

static int case_zombie(void)
{
int status=0;udata.u_ptab=ptab;ptab[1].p_status=P_ZOMBIE;ptab[1].p_pptr=ptab;ptab[1].p_pid=42;ptab[1].p_exitval=7<<8;ptab[1].p_utime=2;ptab[1].p_stime=3;udata.u_argn=42;udata.u_argn1=(intptr_t)&status;TEST(_waitpid()==42 && status==(7<<8),"reap selected zombie");TEST(ptab[1].p_status==P_EMPTY && ptab[0].p_cutime==2 && ptab[0].p_cstime==3,"reap and accumulate times");
return 0;
}

static int case_process_group(void)
{
udata.u_ptab=ptab;ptab[0].p_pgrp=7;ptab[1].p_pptr=ptab;ptab[1].p_status=P_RUNNING;ptab[1].p_pid=42;ptab[1].p_pgrp=8;udata.u_argn=0;udata.u_argn2=WNOHANG;TEST(_waitpid()==-1 && udata.u_error==ECHILD,"no child in own process group");
return 0;
}

static int case_group_nohang(void)
{
udata.u_ptab=ptab;ptab[1].p_pptr=ptab;ptab[1].p_status=P_RUNNING;ptab[1].p_pid=42;ptab[1].p_pgrp=7;udata.u_argn=-7;udata.u_argn2=WNOHANG;TEST(_waitpid()==0 && !udata.u_error,"matching running process group returns zero");
return 0;
}

static int case_null_status(void)
{
udata.u_ptab=ptab;ptab[1].p_status=P_RUNNING;ptab[1].p_pptr=ptab;ptab[1].p_pid=42;ptab[1].p_event=4;udata.u_argn=42;udata.u_argn2=WUNTRACED;TEST(_waitpid()==42,"null status pointer allowed");
return 0;
}

static int case_minimum_pid(void)
{
udata.u_ptab=ptab;ptab[1].p_pptr=ptab;ptab[1].p_status=P_RUNNING;ptab[1].p_pid=42;udata.u_argn=INT16_MIN;udata.u_argn2=WNOHANG;TEST(_waitpid()==-1 && udata.u_error==ECHILD,"minimum 16-bit PID has no matching group");
return 0;
}

#define npid ((uint16_t)udata.u_argn)
#define npgid ((uint16_t)udata.u_argn1)
/* FUNCTION syscall_level2.c _setpgid 0 */

static void setup_groups(void)
{
    udata.u_ptab = ptab;
    ptab[0].p_pid = 41;
    ptab[0].p_pgrp = 7;
    ptab[0].p_session = 7;
    ptab[1].p_pid = 42;
    ptab[1].p_pgrp = 7;
    ptab[1].p_session = 7;
    ptab[1].p_pptr = ptab;
    ptab[1].p_status = P_RUNNING;
}

static int case_setpgid_zero(int target)
{
    setup_groups();
    udata.u_argn = target ? 42 : 0;
    udata.u_argn1 = 0;
    TEST(_setpgid() == 0 && !udata.u_error &&
         ptab[target].p_pgrp == ptab[target].p_pid,
         "zero group argument selects the target PID");
    TEST(ptab[!target].p_pgrp == 7,"other process group remains unchanged");
    return 0;
}

static int case_setpgid_valid(uint16_t group)
{
    setup_groups();
    udata.u_argn1 = group;
    TEST(_setpgid() == 0 && !udata.u_error && ptab[0].p_pgrp == group,
         "valid group ID is accepted");
    return 0;
}

static int case_setpgid_invalid(arg_t group)
{
    setup_groups();
    udata.u_argn = 42;
    udata.u_argn1 = group;
    TEST(_setpgid() == -1 && udata.u_error == EINVAL &&
         ptab[0].p_pgrp == 7 && ptab[1].p_pgrp == 7,
         "invalid group ID is rejected without changing either group");
    return 0;
}

static int case_setpgid_session(int foreign_group)
{
    setup_groups();
    udata.u_argn = 42;
    udata.u_argn1 = 10;
    if (foreign_group) {
        ptab[2].p_pid = 43;
        ptab[2].p_pgrp = 10;
        ptab[2].p_session = 8;
    } else
        ptab[1].p_session = 8;
    TEST(_setpgid() == -1 && udata.u_error == EPERM &&
         ptab[1].p_pgrp == 7,"existing session checks are preserved");
    return 0;
}

static int case_maximum_group_wait(void)
{
    setup_groups();
    udata.u_argn = 42;
    udata.u_argn1 = 32767;
    TEST(_setpgid() == 0,"maximum group ID is accepted");
    udata.u_argn = -32767;
    udata.u_argn1 = 0;
    udata.u_argn2 = WNOHANG;
    TEST(_waitpid() == 0 && !udata.u_error,
         "maximum group matches a negative waitpid selector");
    return 0;
}

int main(int argc, char **argv)
{
if (argc != 2) return 2;
if (!strcmp(argv[1], "minimum-pid")) return case_minimum_pid();
if (!strcmp(argv[1], "setpgid-self-zero")) return case_setpgid_zero(0);
if (!strcmp(argv[1], "setpgid-target-zero")) return case_setpgid_zero(1);
if (!strcmp(argv[1], "setpgid-minimum")) return case_setpgid_valid(1);
if (!strcmp(argv[1], "setpgid-maximum")) return case_setpgid_valid(32767);
if (!strcmp(argv[1], "setpgid-32768")) return case_setpgid_invalid(32768);
if (!strcmp(argv[1], "setpgid-65535")) return case_setpgid_invalid(65535);
if (!strcmp(argv[1], "setpgid-negative")) return case_setpgid_invalid(-1);
if (!strcmp(argv[1], "setpgid-foreign-target")) return case_setpgid_session(0);
if (!strcmp(argv[1], "setpgid-foreign-group")) return case_setpgid_session(1);
if (!strcmp(argv[1], "maximum-group-wait")) return case_maximum_group_wait();
if (!strcmp(argv[1], "nohang")) return case_nohang();
if (!strcmp(argv[1], "stopped")) return case_stopped();
if (!strcmp(argv[1], "unmatched")) return case_unmatched();
if (!strcmp(argv[1], "zombie")) return case_zombie();
if (!strcmp(argv[1], "process-group")) return case_process_group();
if (!strcmp(argv[1], "group-nohang")) return case_group_nohang();
if (!strcmp(argv[1], "null-status")) return case_null_status();
return 2;
}
