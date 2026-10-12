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

#define BLKSIZE 512
#define CDIRTY 128
#define F_BDEV 060000
#define MODE_R(x) ((x)>>8)
#define NULLBLK 65535
struct blkbuf {uint16_t words[256];};typedef struct blkbuf *bufptr;struct blkbuf first,second,third;
unsigned getmode(inoptr i){return 0;} blkno_t blk_alloc(unsigned d){abort();} unsigned corrupt_count;void corrupt_fs(unsigned d){corrupt_count++;} unsigned fail_block=65535;bufptr bread(unsigned d,unsigned b,int r){if(b==fail_block)return NULL;if(b==100)return &first;if(b==200)return &second;if(b==201)return &third;abort();} void brelse(bufptr b){} void bawrite(bufptr b){}
#define blkptr(b,o,l) ((uint8_t*)(b)->words+(o))
#define blkfromk(p,b,o,l) memcpy((uint8_t*)(b)->words+(o),p,l)

/* FUNCTION blk512.c ifetch 0 */
/* FUNCTION blk512.c bfetch 0 */
/* FUNCTION blk512.c bmap 0 */

static int case_first_double(void)
{
node.c_node.i_addr[19]=100;first.words[0]=200;first.words[1]=201;second.words[0]=900;third.words[0]=901;TEST(bmap(&node,274,1)==900,"first double-indirect entry is zero");
return 0;
}

static int case_last_first_double(void)
{
node.c_node.i_addr[19]=100;first.words[0]=200;first.words[1]=201;second.words[255]=902;TEST(bmap(&node,529,1)==902,"last block of first double-indirect child");
return 0;
}

static int case_next_double(void)
{
node.c_node.i_addr[19]=100;first.words[1]=201;third.words[0]=903;TEST(bmap(&node,530,1)==903,"first block of next double-indirect child");
return 0;
}

static int case_last_logical(void)
{
node.c_node.i_addr[19]=100;first.words[254]=200;first.words[255]=201;second.words[237]=904;TEST(bmap(&node,65535,1)==904,"last representable logical block");
return 0;
}

static int case_single_range(void)
{
node.c_node.i_addr[18]=100;first.words[0]=900;first.words[255]=901;TEST(bmap(&node,18,1)==900 && bmap(&node,273,1)==901,"single-indirect boundaries unchanged");
return 0;
}

static int case_failed_single(void)
{
node.c_node.i_addr[18]=100;fail_block=100;TEST(bmap(&node,18,1)==NULLBLK && corrupt_count==1,"failed table read returns NULLBLK");
return 0;
}

static int case_failed_double(void)
{
node.c_node.i_addr[19]=100;for(int i=0;i<256;i++)first.words[i]=200;fail_block=200;TEST(bmap(&node,274,1)==NULLBLK && corrupt_count==1,"failed child table cannot become block zero");
return 0;
}

static int case_direct_hole(void)
{
node.c_node.i_addr[0]=900;TEST(bmap(&node,0,1)==900,"direct block unchanged");TEST(bmap(&node,1,1)==NULLBLK,"read hole does not allocate");
return 0;
}

int main(int argc, char **argv)
{
if (argc != 2) return 2;
if (!strcmp(argv[1], "first-double")) return case_first_double();
if (!strcmp(argv[1], "last-first-double")) return case_last_first_double();
if (!strcmp(argv[1], "next-double")) return case_next_double();
if (!strcmp(argv[1], "last-logical")) return case_last_logical();
if (!strcmp(argv[1], "single-range")) return case_single_range();
if (!strcmp(argv[1], "failed-single")) return case_failed_single();
if (!strcmp(argv[1], "failed-double")) return case_failed_double();
if (!strcmp(argv[1], "direct-hole")) return case_direct_hole();
return 2;
}
