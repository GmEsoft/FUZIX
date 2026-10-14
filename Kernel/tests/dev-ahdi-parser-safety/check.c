#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#define used(x) ((void)(x))
#define __packed __attribute__((packed))
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define BLKSHIFT 9
#define BLKSIZE 512
#define MAX_BLKDEV 2
#define kputs(x) ((void)(x))
#define kprintf(...) ((void)0)
static unsigned printed;
static void kputchar(int c) { (void)c; printed++; }
typedef uintptr_t uarg_t;
typedef uint16_t blkno_t;
struct { int u_error; uint16_t u_nblock,u_block,u_page; uint8_t *u_dptr,*u_base; unsigned u_count; } udata;
static int d_blkoff(int n) { (void)n; return 0; }
/* HEADER dev/blkdev.h */

/* HEADER dev/ahdi.h */
struct blkparam blk_op;static blkdev_t disk;static struct atari_bootblock blocks[2];static unsigned allocs,frees,calls;static bool fail;
static void *tmpbuf(void){assert(allocs<2);return &blocks[allocs++];}static void tmpfree(void*p){(void)p;frees++;}
static uint_fast8_t transfer(void){calls++;if(fail)return 0;return 1;}
/* FUNCTION dev/ahdi.c check_sane */
/* FUNCTION dev/ahdi.c ahdi_extended_partition */
/* FUNCTION dev/ahdi.c ahdi_install */
/* FUNCTION dev/ahdi.c ahdi_parse */
int main(void){struct atari_partition p={1,{'G','E','M'},1,99};blocks[0].size=100;assert(check_sane(&blocks[0],&p,1));p.start=90;p.length=0xffffffff;assert(!check_sane(&blocks[0],&p,1));disk.transfer=transfer;blk_op.blkdev=&disk;struct atari_partition x={1,{'X','G','M'},5,10};fail=true;assert(ahdi_extended_partition(&x,0,'a')==0&&frees==1);allocs=frees=0;fail=false;blocks[0].part[0]=p;blocks[0].part[0].start=1;blocks[0].part[0].length=10;memcpy(blocks[0].part[0].id,"GEM",3);assert(ahdi_install(blocks[0].part,4,MAX_PARTITIONS,'a')==MAX_PARTITIONS);allocs=frees=0;ahdi_parse('a');assert(allocs==frees);return 0;}
