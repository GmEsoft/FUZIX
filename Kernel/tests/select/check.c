#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#c); exit(1); } } while(0)
typedef uint16_t blkno_t;
typedef uint32_t uoff_t;
typedef intptr_t arg_t;
typedef int irqflags_t;
#define BLKSIZE 512
#define ITABSIZE 4
/* DEFINE SELMAPSIZE */
/* TYPES */
/* DEFINE SELECT_BEGIN */
/* DEFINE SELECT_TEST */
/* DEFINE SELECT_END */
/* DEFINE SELECT_IN */
/* DEFINE SELECT_OUT */
/* DEFINE SELECT_EX */
/* DEFINE F_REG */
/* DEFINE F_DIR */
/* DEFINE F_CDEV */
/* DEFINE F_BDEV */
/* DEFINE F_PIPE */
/* DEFINE F_MASK */
/* DEFINE MODE_R */
#define UFTSIZE 16 /* Normal configuration; CONFIG_SMALL is not enabled. */
/* DEFINE CMAGIC */
#define ROOTINODE 2
#define INO_PER_BLOCK 8
#define MS_RDONLY 1
#define PANIC_IOPEN 1
struct p_tab { uint16_t p_timeout; } ptab[PTABSIZE];
typedef struct p_tab *ptptr;
uint16_t maxproc=PTABSIZE;
struct { arg_t u_argn,u_argn1; int u_error; ptptr u_ptab; } udata;
struct cinode i_tab[ITABSIZE];
struct mount { struct { uint16_t s_isize; } m_fs; uint16_t m_flags; } fs_tab[1];
uint16_t pipesel;
unsigned awakened[PTABSIZE];
unsigned begin_calls,end_calls,test_calls,sleeps,ready_bits,seen_wait,sleep_mode;
int irqstate,bad_fd=-1;
uint16_t sets[8];
#define nfd ((uint16_t)udata.u_argn)
#define base ((uint16_t*)udata.u_argn1)
#define NULLINODE ((inoptr)0)
static struct selmap *maps(inoptr i)
{
#ifdef HAS_SELECT_STORAGE
    return i->c_select;
#else
    return (struct selmap*)&i->c_node.i_addr[17];
#endif
}
void ptimer_insert(void) {}
uint16_t ugetw(const void *p) { return *(const uint16_t*)p; }
int uputw(uint16_t v, void *p) { *(uint16_t*)p=v; return 0; }
irqflags_t di(void) { int old=irqstate;irqstate=1;return old; }
void irqrestore(irqflags_t i) { irqstate=i; }
void pwake(ptptr p) { CHECK(p>=ptab && p<ptab+PTABSIZE);awakened[p-ptab]++; }
inoptr getinode(unsigned fd)
{
    if(fd>=16 || (int)fd==bad_fd) {udata.u_error=EBADF;return NULL;}
    return &i_tab[0];
}
unsigned getmode(inoptr i) { return MODE_R(i->c_node.i_mode); }
int d_ioctl(unsigned d, unsigned op, void *p)
{
    if(op==SELECT_BEGIN) begin_calls++;
    if(op==SELECT_TEST) test_calls++;
    if(op==SELECT_END) end_calls++;
    if(p) *(char*)p=ready_bits;
    return 0;
}
int psleep_flags(void *p, unsigned flags)
{
    CHECK(++sleeps<=3);
    seen_wait = maps(&i_tab[0])[0].map[0]&1;
    if(sleep_mode==1) {
        ready_bits=SELECT_IN;
        i_tab[0].c_node.i_size=1;
        irqstate=0; /* Scheduling may return with interrupts enabled. */
        return 0;
    }
    if(sleep_mode==2) {udata.u_ptab->p_timeout=1;return 0;}
    udata.u_error=EINTR;return -1;
}
void validchk(uint16_t d, unsigned p) {}
uint16_t i_alloc(uint16_t d) { return 2; }
struct mount *fs_tab_get(uint16_t d) { return fs_tab; }
void kputs(const char *s) { CHECK(0); }
int breadi(uint16_t d, uint16_t ino, void *p)
{
    dinode *n=p; memset(n,0,sizeof(*n)); n->i_nlink=1;n->i_mode=F_CDEV;return 0;
}
/* FUNCTION select.c selwait_inode 0 */
/* FUNCTION select.c selwake 0 */
/* FUNCTION select.c selwake_inode 0 */
/* FUNCTION select.c pipesel_test optional */
/* FUNCTION select.c pipesel_begin 0 */
/* FUNCTION select.c pipesel_end 0 */
/* FUNCTION select.c _select 0 */
/* FUNCTION filesys.c i_open 0 */
static void setup(void)
{
    udata.u_ptab=ptab;udata.u_argn=1;udata.u_argn1=(intptr_t)sets;
    i_tab[0].c_node.i_mode=F_REG;i_tab[0].c_refs=1;
    fs_tab[0].m_fs.s_isize=3;
}
int main(int argc, char **argv)
{
    CHECK(argc==2);setup();
    if(!strcmp(argv[1],"fd-boundaries")) {
        for(unsigned fd=0;fd<16;fd++) {
            sets[0]=1U<<fd;udata.u_argn=fd+1;
            CHECK(_select()==0 && sets[0]==(1U<<fd) && !irqstate);
        }
    } else if(!strcmp(argv[1],"bounds")) {
        udata.u_argn=17;CHECK(_select()==-1 && udata.u_error==EINVAL && !irqstate);
        udata.u_argn=-1;CHECK(_select()==-1 && udata.u_error==EINVAL);
    } else if(!strcmp(argv[1],"invalid-fd")) {
        i_tab[0].c_node.i_mode=F_CDEV;sets[0]=3;udata.u_argn=2;bad_fd=1;
        CHECK(_select()==-1 && udata.u_error==EBADF && !irqstate);
        CHECK(begin_calls==end_calls && !pipesel);
    } else if(!strcmp(argv[1],"requested-wait")) {
        i_tab[0].c_node.i_mode=F_CDEV;sets[0]=1;sets[6]=2;sleep_mode=1;
        CHECK(_select()==0 && sets[0]==1 && !udata.u_error && !irqstate);
        CHECK(seen_wait && begin_calls==1 && end_calls==1 && test_calls==1);
        CHECK(!maps(&i_tab[0])[0].map[0]);
    } else if(!strcmp(argv[1],"interrupt-cleanup")) {
        i_tab[0].c_node.i_mode=F_CDEV;sets[0]=1;sets[6]=2;
        CHECK(_select()==-1 && udata.u_error==EINTR && !irqstate);
        CHECK(begin_calls==1 && end_calls==1 && !maps(&i_tab[0])[0].map[0]);
    } else if(!strcmp(argv[1],"pipe-repoll")) {
        i_tab[0].c_node.i_mode=F_PIPE;i_tab[0].c_writers=1;sets[0]=1;sets[6]=2;sleep_mode=1;
        CHECK(_select()==0 && sets[0]==1 && !irqstate && !pipesel);
    } else if(!strcmp(argv[1],"pipe-eof")) {
        i_tab[0].c_node.i_mode=F_PIPE;i_tab[0].c_writers=0;
        int mask=pipesel_begin(&i_tab[0],SELECT_IN);
        CHECK(mask==SELECT_IN);pipesel_end(&i_tab[0]);CHECK(!pipesel);
    } else if(!strcmp(argv[1],"timeout-cleanup")) {
        i_tab[0].c_node.i_mode=F_CDEV;sets[0]=1;sets[6]=2;sleep_mode=2;
        CHECK(_select()==0 && !sets[0] && !irqstate && !udata.u_error);
        CHECK(begin_calls==1 && end_calls==1 && !udata.u_ptab->p_timeout);
    } else if(!strcmp(argv[1],"map-bounds")) {
        inoptr i=calloc(1,sizeof(*i));CHECK(i);
        i->c_refs=0x7f;i->c_flags=0x80;
        for(unsigned b=0;b<20;b++) i->c_node.i_addr[b]=0x100+b;
        for(unsigned p=0;p<PTABSIZE;p++) {
            udata.u_ptab=ptab+p;
            selwait_inode(i,SELECT_IN|SELECT_OUT|SELECT_EX,1);
            for(unsigned m=0;m<3;m++) CHECK(maps(i)[m].map[p>>3]&(1U<<(p&7)));
        }
        CHECK(i->c_refs==0x7f && i->c_flags==0x80);
        for(unsigned b=0;b<20;b++) CHECK(i->c_node.i_addr[b]==0x100+b);
        for(unsigned p=0;p<PTABSIZE;p++) {udata.u_ptab=ptab+p;selwait_inode(i,7,0);}
        for(unsigned m=0;m<3;m++) for(unsigned b=0;b<SELMAPSIZE;b++) CHECK(!maps(i)[m].map[b]);
        free(i);
    } else if(!strcmp(argv[1],"wake-map")) {
        selwait_inode(&i_tab[0],SELECT_IN,1);
        udata.u_ptab=ptab+PTABSIZE-1;selwait_inode(&i_tab[0],SELECT_IN,1);
        selwake_inode(&i_tab[0],SELECT_IN);
        CHECK(awakened[0]==1 && awakened[PTABSIZE-1]==1 && !irqstate);
    } else if(!strcmp(argv[1],"cached-inode-reuse")) {
        i_tab[0].c_refs=0;i_tab[0].c_dev=0;i_tab[0].c_num=2;
        i_tab[0].c_node.i_mode=F_CDEV;i_tab[0].c_node.i_nlink=1;
#ifdef HAS_SELECT_STORAGE
        memset(maps(&i_tab[0]),0xff,3*sizeof(struct selmap));
#else
        memset(&i_tab[0].c_node.i_addr[17],0xff,3*sizeof(blkno_t));
#endif
        CHECK(i_open(0,2)==&i_tab[0]);
        for(unsigned m=0;m<3;m++) for(unsigned b=0;b<SELMAPSIZE;b++) CHECK(!maps(&i_tab[0])[m].map[b]);
    } else CHECK(0);
    return 0;
}
