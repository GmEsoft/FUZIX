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

/* FUNCTION network.c net_write 0 */

static int case_connected_datagram(void)
{
sockets[0].s_state=SS_CONNECTED;sockets[0].s_class=SOCK_DGRAM;TEST(net_write()==0 && write_address==&sockets[0].dst_addr && udata.u_retval==5,"connected write uses peer address and count");
return 0;
}

static int case_peer_half_close(void)
{
sockets[0].s_state=SS_CLOSEWAIT;sockets[0].s_class=SOCK_STREAM;TEST(net_write()==0 && udata.u_done==5 && !udata.u_error && !udata.u_net.sig,"peer read EOF permits outbound data");
return 0;
}

static int case_closed(void)
{
sockets[0].s_state=SS_CLOSED;sockets[0].s_class=SOCK_STREAM;TEST(net_write()==0 && udata.u_error==EPIPE && !write_address,"closed socket still rejects write");
return 0;
}

static int case_unconnected(void)
{
sockets[0].s_state=SS_UNCONNECTED;TEST(net_write()==0 && udata.u_error==EINVAL && !write_address,"unconnected socket remains rejected");
return 0;
}

static int case_connected_stream(void)
{
sockets[0].s_state=SS_CONNECTED;sockets[0].s_class=SOCK_STREAM;TEST(net_write()==0 && udata.u_retval==5 && !udata.u_error,"ordinary stream write remains valid");
return 0;
}

int main(int argc, char **argv)
{
if (argc != 2) return 2;
if (!strcmp(argv[1], "connected-datagram")) return case_connected_datagram();
if (!strcmp(argv[1], "peer-half-close")) return case_peer_half_close();
if (!strcmp(argv[1], "closed")) return case_closed();
if (!strcmp(argv[1], "unconnected")) return case_unconnected();
if (!strcmp(argv[1], "connected-stream")) return case_connected_stream();
return 2;
}
