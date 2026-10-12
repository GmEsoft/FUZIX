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
/* DEFINE O_RDONLY */
/* DEFINE O_WRONLY */
/* DEFINE O_ACCMODE */
/* DEFINE F_REG */
/* DEFINE MODE_R */
/* DEFINE CRDONLY */
/* DEFINE HIBYTE32 */
/* DEFINE BLKOVERSIZE32 */
/* DEFINE NULLBLK */
/* DEFINE M_TIME */
/* DEFINE C_TIME */

#define off_t kernel_off_t
typedef int32_t off_t;
#if TEST_BLOCK == 400
#include "blk400.h"
#else
#include "blk512.h"
#endif
#define file ((unsigned)udata.u_argn)
#define offset ((uint32_t*)udata.u_argn1)
struct blkbuf {uint8_t bytes[BLKSIZE];};typedef struct blkbuf *bufptr;
static struct blkbuf buffer;
static unsigned maps,reads,dirty,truncations,writes,timestamps,kept;
static int hole,map_error,read_error,truncate_error,copy_error;
int uget(const void *p,void *out,unsigned n){if(copy_error){udata.u_error=EFAULT;return -1;}memcpy(out,p,n);return 0;}
inoptr getinode(unsigned fd){return &node;}
unsigned getmode(inoptr ino){return MODE_R(ino->c_node.i_mode);}
blkno_t bmap(inoptr ino,blkno_t bn,unsigned read){maps++;if(read!=1)abort();if(map_error){udata.u_error=EIO;return NULLBLK;}return hole?NULLBLK:77;}
bufptr bread(unsigned dev,blkno_t bn,unsigned rewrite){reads++;if(read_error){udata.u_error=EIO;return NULL;}return &buffer;}
void blkfromk(const void *src,bufptr bp,unsigned start,unsigned size){memcpy(bp->bytes+start,src,size);}
int bfree(bufptr bp,unsigned mode){dirty+=mode;return 0;}
int f_trunc_blocks(inoptr ino,uint16_t count){truncations++;kept=count;if(ino->c_flags&CRDONLY){udata.u_error=EROFS;return -1;}if(truncate_error){udata.u_error=EIO;return -1;}return 0;}
void setftime(inoptr ino,unsigned flags){timestamps++;}
void wr_inode(inoptr ino){writes++;}
static void setup(off_t *size,uint32_t old)
{
    node.c_node.i_mode=F_REG;node.c_node.i_size=old;of_tab[0].o_access=O_WRONLY;
    udata.u_argn1=(intptr_t)size;memset(buffer.bytes,0xa5,BLKSIZE);
}

/* FUNCTION syscall_fs3.c truncate_tail optional */
/* FUNCTION syscall_fs3.c _ftruncate 0 */

static int case_shrink_regrow(void)
{
off_t size=10;setup(&size,BLKSIZE+20);TEST(_ftruncate()==0&&node.c_node.i_size==10&&kept==1,"shrink size and block count");for(unsigned i=0;i<BLKSIZE;i++)TEST(buffer.bytes[i]==(i<10?0xa5:0),"zero removed tail");size=20;TEST(_ftruncate()==0&&node.c_node.i_size==20&&truncations==1,"regrow without freeing retained block");for(unsigned i=10;i<20;i++)TEST(!buffer.bytes[i],"regrowth exposes zeros");
return 0;
}

static int case_grow_partial(void)
{
off_t size=BLKSIZE+20;setup(&size,10);TEST(_ftruncate()==0&&!truncations,"grow without freeing blocks");for(unsigned i=0;i<BLKSIZE;i++)TEST(buffer.bytes[i]==(i<10?0xa5:0),"clear old partial-block suffix");
return 0;
}

static int case_truncate_failure(void)
{
off_t size=BLKSIZE;setup(&size,BLKSIZE*2);truncate_error=1;TEST(_ftruncate()==-1&&udata.u_error==EIO&&node.c_node.i_size==BLKSIZE*2&&!writes&&!timestamps,"propagate truncation failure before size update");
return 0;
}

static int case_read_failure(void)
{
off_t size=10;setup(&size,100);read_error=1;TEST(_ftruncate()==-1&&udata.u_error==EIO&&node.c_node.i_size==100&&!writes&&!truncations,"failed tail read preserves size");
return 0;
}

static int case_sparse_hole(void)
{
off_t size=10;setup(&size,100);hole=1;TEST(_ftruncate()==0&&!reads&&!dirty&&kept==1,"do not allocate sparse tail");
return 0;
}

static int case_readonly(void)
{
off_t size=10;setup(&size,100);node.c_flags=CRDONLY;TEST(_ftruncate()==-1&&udata.u_error==EROFS&&node.c_node.i_size==100&&!maps&&!writes,"reject read-only inode before mutation");
return 0;
}

static int case_unchanged(void)
{
off_t size=10;setup(&size,10);TEST(_ftruncate()==0&&!maps&&!writes&&!truncations,"same size is a no-op");
return 0;
}

static int case_large_retained_count(void)
{
off_t size=65536UL*BLKSIZE-1;setup(&size,65536UL*BLKSIZE);TEST(_ftruncate()==0&&!truncations&&node.c_node.i_size==(uint32_t)size,"65536 retained blocks must not wrap to zero");for(unsigned i=0;i<BLKSIZE;i++)TEST(buffer.bytes[i]==(i==BLKSIZE-1?0:0xa5),"clear full-width last-byte tail");
return 0;
}

static int case_map_error(void)
{
off_t size=10;setup(&size,100);map_error=1;TEST(_ftruncate()==-1&&udata.u_error==EIO&&!reads&&!writes&&node.c_node.i_size==100,"map error preserves size");
return 0;
}

static int case_invalid_range(void)
{
off_t size=65536UL*BLKSIZE+1;setup(&size,100);TEST(_ftruncate()==-1&&udata.u_error==EINVAL&&!maps&&!writes,"reject beyond block-number range");
return 0;
}

static int case_copy_fault(void)
{
off_t size=10;setup(&size,100);copy_error=1;TEST(_ftruncate()==-1&&udata.u_error==EFAULT&&!maps&&!writes,"copy fault");
return 0;
}

int main(int argc,char **case_argv)
{
if(argc!=2)return 2;
if(!strcmp(case_argv[1],"shrink-regrow"))return case_shrink_regrow();
if(!strcmp(case_argv[1],"grow-partial"))return case_grow_partial();
if(!strcmp(case_argv[1],"truncate-failure"))return case_truncate_failure();
if(!strcmp(case_argv[1],"read-failure"))return case_read_failure();
if(!strcmp(case_argv[1],"sparse-hole"))return case_sparse_hole();
if(!strcmp(case_argv[1],"readonly"))return case_readonly();
if(!strcmp(case_argv[1],"unchanged"))return case_unchanged();
if(!strcmp(case_argv[1],"large-retained-count"))return case_large_retained_count();
if(!strcmp(case_argv[1],"map-error"))return case_map_error();
if(!strcmp(case_argv[1],"invalid-range"))return case_invalid_range();
if(!strcmp(case_argv[1],"copy-fault"))return case_copy_fault();
return 2;
}
