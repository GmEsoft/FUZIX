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
#define CONFIG_NET
#include "netdev.h"
typedef struct u_data {arg_t u_argn,u_argn1,u_argn2;int u_error;bool u_sysio,u_ininterrupt,u_insys;uaddr_t u_syscall_sp,u_top,u_codebase,u_break;void *u_sp,*u_isp;uint16_t u_page,u_page2,u_euid,u_egid;uint8_t u_flags,u_name[8];int u_cursig;inoptr u_cwd,u_root;uint16_t u_block;unsigned u_nblock;uint8_t *u_dptr;uint8_t *u_base; unsigned u_count,u_done;uint32_t u_offset;uint16_t u_cloexec,u_mask;uint8_t u_files[16];ptptr u_ptab; int (*u_sigvec[32])(int);struct udata_net u_net;arg_t u_retval;struct rlimit u_rlimit[9];} u_data;u_data udata;
struct p_tab ptab[16],*ptab_end=ptab+16;
struct oft of_tab[16];
struct cinode node;
#define NULLINODE ((inoptr)0)

#define is_datagram(s) ((s)->s_class!=SOCK_STREAM && (s)->s_class!=SOCK_SEQPACKET)
struct socket sockets[NSOCKET];uint8_t sock_wake[NSOCKET];struct ksockaddr *write_address;unsigned shutdown_calls;
int netproto_socket(void){return 0;}int netproto_autobind(struct socket*s){return 0;}int netproto_listen(struct socket*s){return 0;}
int netproto_find_local(struct ksockaddr*a){return -1;}int netproto_bind(struct socket*s){return 0;}int netproto_begin_connect(struct socket*s){return 0;}
struct socket *netproto_sockpending(struct socket*s){return NULL;}int netproto_accept_complete(struct socket*s){return 0;}
arg_t netproto_write(struct socket*s,struct ksockaddr*a){write_address=a;udata.u_done=5;return 0;}
int netproto_read(struct socket*s){return 0;}arg_t netproto_shutdown(struct socket*s,uint8_t h){shutdown_calls++;return 0;}

#define N_MAKE		0x80
#define N_SOCKFD	0x40
#define N_ADDR_IN	0x20
#define N_ADDR_OUT	0x10
#define N_DATAI		0x08
#define N_DATAO		0x04
#define N_DATAIO	0x0C
#define NUM_NETCALL	10
static uint8_t ncall_tab[NUM_NETCALL] = {
	N_MAKE,			/* Socket */
	N_SOCKFD,		/* Listen fd, num */
	N_SOCKFD | N_ADDR_IN,	/* bind */
	N_SOCKFD | N_ADDR_IN,	/* connect */
	N_MAKE | N_SOCKFD | N_ADDR_OUT,	/* accept */
	N_SOCKFD | N_ADDR_OUT,	/* getsockname */
	N_SOCKFD | N_DATAO | N_ADDR_IN,	/* sendto */
	N_SOCKFD | N_DATAI | N_ADDR_OUT,	/* recvfrom */
	N_SOCKFD,		/* shutdown */
	N_SOCKFD | N_ADDR_OUT	/* getpeername */
};
#define argptr ((void*)udata.u_argn)
unsigned valaddr_r(const void*p,unsigned n){return n;}unsigned valaddr(const void*p,unsigned n,int w){return n;}
int copy_fault;int uget(const void*a,void*b,unsigned n){if(copy_fault){udata.u_error=EFAULT;return -1;}memcpy(b,a,n);return 0;}int uput(const void*a,void*b,unsigned n){memcpy(b,a,n);return 0;}
inoptr sock_get(int fd,uint_fast8_t*f){return &node;}int run_sockfunc(int(*f)(void),unsigned n,unsigned flag){return f();}
unsigned protocol_calls;int net_syscall(void){protocol_calls++;return 0;}int make_socket(unsigned n){return 0;}void net_free(void){}void net_inode(void){}
unsigned ugeti(const void*p){return *(const uint16_t*)p;}void uputi(unsigned v,void*p){*(uint16_t*)p=v;}


/* FUNCTION syscall_net.c _netcall 0 */

static int case_invalid_operation(void)
{
arg_t vals[]={-1,10,256,257,65536};for(unsigned i=0;i<5;i++){arg_t a[7]={vals[i]};udata.u_argn=(intptr_t)a;TEST(_netcall()==-1 && udata.u_error==ENOSYS && !protocol_calls,"validate operation before indexing or narrowing");}
return 0;
}

static int case_invalid_length(void)
{
uint8_t addr[16]={0};arg_t vals[]={-1,0,1,17,272,65552};for(unsigned i=0;i<6;i++){arg_t a[7]={2,0,(intptr_t)addr,vals[i]};udata.u_argn=(intptr_t)a;TEST(_netcall()==-1 && udata.u_error==EINVAL && !protocol_calls,"validate full address length");}
return 0;
}

static int case_valid_length(void)
{
uint8_t addr[16]={0};arg_t a[7]={2,0,(intptr_t)addr,16};udata.u_argn=(intptr_t)a;_netcall();TEST(!udata.u_error && protocol_calls==1 && udata.u_net.addrlen==16,"valid address reaches protocol");
return 0;
}

static int case_argument_copy_fault(void)
{
arg_t a[7]={0};udata.u_argn=(intptr_t)a;copy_fault=1;TEST(_netcall()==-1 && udata.u_error==EFAULT && !protocol_calls,"argument copy failure stops dispatch");
return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"invalid-operation"))return case_invalid_operation();
if(!strcmp(argv[1],"invalid-length"))return case_invalid_length();
if(!strcmp(argv[1],"valid-length"))return case_valid_length();
if(!strcmp(argv[1],"argument-copy-fault"))return case_argument_copy_fault();
return 2;
}
