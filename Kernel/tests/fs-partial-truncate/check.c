#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#c); exit(1); } } while (0)
#define min(a,b) ((a)<(b)?(a):(b))
typedef uint16_t blkno_t;
struct cinode { uint16_t c_dev; struct { blkno_t i_addr[20]; } c_node; uint8_t c_flags; } node;
typedef struct cinode *inoptr;
struct { int u_error; } udata;
/* DEFINE CDIRTY */
/* DEFINE CRDONLY */
/* DEFINE HDIO_TRIM_KERNEL */
#define NINDIRECT (BLKSIZE/sizeof(blkno_t))
struct blkbuf { blkno_t words[NINDIRECT]; } blocks[4];
typedef struct blkbuf *bufptr;
static unsigned freed[2048], trimmed[2048], dirty_tables;
bufptr bread(uint16_t dev, blkno_t blk, unsigned flag)
{
    CHECK(blk>=100 && blk<=103);
    return &blocks[blk-100];
}
void brelse(bufptr b) {}
void bawrite(bufptr b) { dirty_tables++; }
void corrupt_fs(uint16_t dev) { CHECK(0); }
void blk_free(uint16_t dev, blkno_t blk) { CHECK(blk<2048); freed[blk]++; }
void blktok(void *p, bufptr b, unsigned off, unsigned n)
{
    CHECK(off+n<=BLKSIZE); memcpy(p,(uint8_t*)b->words+off,n);
}
void blkfromk(const void *p, bufptr b, unsigned off, unsigned n)
{
    CHECK(off+n<=BLKSIZE); memcpy((uint8_t*)b->words+off,p,n);
}
#define blkptr(b,o,n) ((void*)((uint8_t*)(b)->words+(o)))
int d_ioctl(uint16_t dev, unsigned op, void *p)
{
    unsigned blk=*(blkno_t*)p; CHECK(op==HDIO_TRIM_KERNEL && blk<2048); trimmed[blk]++; return 0;
}
/* FUNCTION filesys.c freeblk configured */
/* FUNCTION filesys.c f_trunc_blocks 0 */
static void setup(void)
{
    for(unsigned i=0;i<18;i++) node.c_node.i_addr[i]=i+1;
    node.c_node.i_addr[18]=100;
    node.c_node.i_addr[19]=102;
    blocks[2].words[0]=101;
    blocks[2].words[1]=103;
    for(unsigned i=0;i<NINDIRECT;i++) {
        blocks[0].words[i]=500+i;
        blocks[1].words[i]=800+i;
        blocks[3].words[i]=1100+i;
    }
}
static void verify_trim(void)
{
    for(unsigned i=0;i<2048;i++) CHECK(freed[i]==trimmed[i]);
}
int main(int argc, char **argv)
{
    CHECK(argc==2); setup();
    if(!strcmp(argv[1],"partial-single")) {
        CHECK(!f_trunc_blocks(&node,20));
        CHECK(!freed[100] && !freed[500] && !freed[501] && freed[502]==1);
        CHECK(blocks[0].words[0]==500 && blocks[0].words[1]==501 && !blocks[0].words[2]);
        CHECK(node.c_node.i_addr[18]==100 && !node.c_node.i_addr[19] && dirty_tables);
    } else if(!strcmp(argv[1],"partial-double")) {
        CHECK(!f_trunc_blocks(&node,18+NINDIRECT+1));
        CHECK(!freed[100] && !freed[500] && !freed[101] && !freed[102] && !freed[800]);
        CHECK(freed[801]==1 && freed[103]==1 && freed[1100]==1);
        CHECK(blocks[2].words[0]==101 && !blocks[2].words[1] && !blocks[1].words[1]);
    } else if(!strcmp(argv[1],"double-child-boundary")) {
        CHECK(!f_trunc_blocks(&node,18+2*NINDIRECT));
        CHECK(!freed[100] && !freed[101] && !freed[102] && !freed[800]);
        CHECK(freed[103]==1 && freed[1100]==1 && !blocks[2].words[1]);
    } else if(!strcmp(argv[1],"single-boundary")) {
        CHECK(!f_trunc_blocks(&node,18+NINDIRECT));
        CHECK(!freed[100] && !freed[500] && node.c_node.i_addr[18]==100);
        CHECK(freed[101]==1 && freed[102]==1 && freed[103]==1 && !node.c_node.i_addr[19]);
    } else if(!strcmp(argv[1],"full")) {
        CHECK(!f_trunc_blocks(&node,0));
        for(unsigned i=0;i<20;i++) CHECK(!node.c_node.i_addr[i]);
        for(unsigned i=1;i<=18;i++) CHECK(freed[i]==1);
        for(unsigned i=100;i<=103;i++) CHECK(freed[i]==1);
        for(unsigned i=0;i<NINDIRECT;i++) CHECK(freed[500+i]==1 && freed[800+i]==1 && freed[1100+i]==1);
    } else if(!strcmp(argv[1],"repeat")) {
        CHECK(!f_trunc_blocks(&node,20));
        CHECK(!f_trunc_blocks(&node,20));
        CHECK(!freed[500] && !freed[501] && !freed[100] && freed[502]==1 && freed[102]==1);
    } else if(!strcmp(argv[1],"large-count")) {
        CHECK(!f_trunc_blocks(&node,65535));
        for(unsigned i=1;i<=18;i++) CHECK(!freed[i]);
        CHECK(!freed[100] && !freed[101] && !freed[102] && !freed[103]);
    } else if(!strcmp(argv[1],"readonly")) {
        node.c_flags=CRDONLY;
        CHECK(f_trunc_blocks(&node,0)==-1 && udata.u_error==EROFS);
        for(unsigned i=0;i<2048;i++) CHECK(!freed[i]);
    } else CHECK(0);
    verify_trim();
    return 0;
}
