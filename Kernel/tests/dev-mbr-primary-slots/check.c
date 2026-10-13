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

#define BOOTDEVICE 1
/* HEADER dev/mbr.h */
struct blkparam blk_op;static blkdev_t disk;static boot_record_t record;
static void *tmpbuf(void){return &record;}static void tmpfree(void*p){assert(p==&record);}
static uint_fast8_t transfer(void){return 1;}
/* FUNCTION dev/mbr.c mbr_parse */
int main(void){record.signature=MBR_SIGNATURE;record.partition[1].type_chs_last[0]=0x83;record.partition[1].lba_first=100;record.partition[1].lba_count=50;record.partition[3].type_chs_last[0]=0x83;record.partition[3].lba_first=200;record.partition[3].lba_count=60;disk.transfer=transfer;blk_op.blkdev=&disk;mbr_parse('a');assert(!disk.lba_count[0] && disk.lba_first[1]==100 && !disk.lba_count[2] && disk.lba_first[3]==200);return 0;}
