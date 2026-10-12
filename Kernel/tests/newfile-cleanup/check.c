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
/* DEFINE CDIRTY */
/* DEFINE CRDONLY */
/* DEFINE OTH_WR */
/* DEFINE F_REG */
/* DEFINE F_DIR */
/* DEFINE F_PIPE */
/* DEFINE MODE_R */
/* DEFINE FILESYS_TABSIZE */

struct super {
    uint16_t s_isize, s_tinode, s_ninode, s_inode[FILESYS_TABSIZE];
} filesystem;
typedef struct super *fsptr;
static struct cinode parent;
static unsigned allocations, links, writes, truncations;
static uint16_t disk_mode, disk_links;
static int insert_error, allocation_error;

#define magic(i) ((void)0)
#define baddev(p) ((p)==NULL)
#define PANIC_IFREE_BADI "invalid inode"
#define PANIC_INODE_FREED "unbalanced reference"
void panic(const char *reason) { fprintf(stderr,"%s\n",reason); abort(); }
void wakeup(void *address) {}
unsigned getmode(inoptr ino) { return MODE_R(ino->c_node.i_mode); }
int getperm(inoptr ino) { return OTH_WR; }
fsptr getdev(uint16_t dev) { return &filesystem; }
inoptr i_open(uint16_t dev, uint16_t num)
{
    if (allocation_error) return NULL;
    allocations++;
    filesystem.s_tinode--;
    if (filesystem.s_ninode) {
        if (filesystem.s_inode[--filesystem.s_ninode] != 42) abort();
    }
    memset(&node,0,sizeof(node));
    node.c_dev=dev;
    node.c_num=42;
    node.c_refs=1;
    return &node;
}
bool ch_link(inoptr dir,uint8_t *oldname,uint8_t *newname,inoptr ino)
{
    links++;
    if (insert_error) { udata.u_error=insert_error; return false; }
    return true;
}
int f_trunc(inoptr ino) { truncations++; return 0; }
int bwritei(inoptr ino)
{
    if (ino==&node) {
        writes++;
        disk_mode=ino->c_node.i_mode;
        disk_links=ino->c_node.i_nlink;
    }
    return 0;
}
void corrupt_fs(uint16_t dev) { abort(); }
void i_deref(inoptr);
void wr_inode(inoptr);
void i_free(uint16_t,uint16_t);

static void setup(void)
{
    filesystem.s_isize=10;
    filesystem.s_tinode=100;
    parent.c_node.i_mode=F_DIR;
    parent.c_node.i_nlink=1;
    parent.c_refs=2; /* One retained parent reference and the lookup reference. */
}

/* FUNCTION filesys.c i_free 0 */
/* FUNCTION filesys.c wr_inode 0 */
/* FUNCTION filesys.c i_deref 0 */
/* FUNCTION filesys.c newfile 0 */

static int case_insertion_enospc(void)
{

setup();insert_error=ENOSPC;
inoptr result=newfile(&parent,(uint8_t*)"new");
printf("free=%u cached=%u inode_refs=%u links=%u mode=%u writes=%u\n",filesystem.s_tinode,filesystem.s_ninode,node.c_refs,node.c_node.i_nlink,node.c_node.i_mode,writes);
TEST(result==NULL,"failed insertion returns NULL");
TEST(udata.u_error==insert_error,"retain insertion error after successful cleanup");
TEST(parent.c_refs==1 && !node.c_refs,"consume only acquired references");
TEST(!node.c_node.i_nlink && !node.c_node.i_mode,"clear orphaned inode");
TEST(filesystem.s_tinode==100 && filesystem.s_ninode==1 && filesystem.s_inode[0]==42,"return inode to free pool");
TEST(truncations==1 && writes==2 && !disk_mode && !disk_links,"run normal inode disposal and write the cleared inode");

return 0;
}

static int case_insertion_eio(void)
{

setup();insert_error=EIO;
inoptr result=newfile(&parent,(uint8_t*)"new");
printf("free=%u cached=%u inode_refs=%u links=%u mode=%u writes=%u\n",filesystem.s_tinode,filesystem.s_ninode,node.c_refs,node.c_node.i_nlink,node.c_node.i_mode,writes);
TEST(result==NULL,"failed insertion returns NULL");
TEST(udata.u_error==insert_error,"retain insertion error after successful cleanup");
TEST(parent.c_refs==1 && !node.c_refs,"consume only acquired references");
TEST(!node.c_node.i_nlink && !node.c_node.i_mode,"clear orphaned inode");
TEST(filesystem.s_tinode==100 && filesystem.s_ninode==1 && filesystem.s_inode[0]==42,"return inode to free pool");
TEST(truncations==1 && writes==2 && !disk_mode && !disk_links,"run normal inode disposal and write the cleared inode");

return 0;
}

static int case_repeated_failure(void)
{

setup();insert_error=ENOSPC;
for(unsigned i=0;i<64;i++) {
    parent.c_refs=2;udata.u_error=0;
    TEST(newfile(&parent,(uint8_t*)"new")==NULL,"failed create");
    TEST(filesystem.s_tinode==100 && filesystem.s_ninode==1,"repeated failures do not exhaust inodes");
    TEST(parent.c_refs==1 && !node.c_refs,"balanced repeated references");
}
TEST(allocations==64 && truncations==64 && writes==128,"dispose each failed allocation exactly once");

return 0;
}

static int case_success(void)
{

setup();
TEST(newfile(&parent,(uint8_t*)"new")==&node,"successful insertion returns inode");
TEST(parent.c_refs==1 && node.c_refs==1 && node.c_node.i_nlink==1,"preserve successful link and caller reference");
TEST(filesystem.s_tinode==99 && !filesystem.s_ninode && !truncations && writes==1,"successful inode remains allocated");
i_deref(&node);
TEST(!node.c_refs && node.c_node.i_nlink==1 && filesystem.s_tinode==99,"closing a linked inode does not free it");

return 0;
}

static int case_allocation_failure(void)
{

setup();allocation_error=1;
TEST(newfile(&parent,(uint8_t*)"new")==NULL && udata.u_error==ENFILE,"allocation failure");
TEST(parent.c_refs==1 && !allocations && !links && !writes && !truncations && filesystem.s_tinode==100,"allocation failure consumes only parent lookup reference");

return 0;
}

static int case_readonly_parent(void)
{

setup();parent.c_flags=CRDONLY;
TEST(newfile(&parent,(uint8_t*)"new")==NULL && udata.u_error==EROFS,"read-only directory");
TEST(parent.c_refs==1 && !allocations && !links && !writes && !truncations && filesystem.s_tinode==100,"reject before inode allocation");

return 0;
}

int main(int argc,char **argv)
{
if(argc!=2)return 2;
if(!strcmp(argv[1],"insertion-enospc"))return case_insertion_enospc();
if(!strcmp(argv[1],"insertion-eio"))return case_insertion_eio();
if(!strcmp(argv[1],"repeated-failure"))return case_repeated_failure();
if(!strcmp(argv[1],"success"))return case_success();
if(!strcmp(argv[1],"allocation-failure"))return case_allocation_failure();
if(!strcmp(argv[1],"readonly-parent"))return case_readonly_parent();
return 2;
}
