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
/* HEADER dev/gpt.h */
struct blkparam blk_op; static blkdev_t disk;

static void kputs_utf16le(uint8_t*p){(void)p;}
/* FUNCTION dev/gpt.c parse_gpt_part_entry */
int main(void){gpt_partition_entry_t e={0};e.type_guid[0]=1;e.first_lba_lsb=e.last_lba_lsb=10;disk.drive_lba_count=100;blk_op.blkdev=&disk;assert(parse_gpt_part_entry(&e,0)==1&&disk.lba_count[0]==1);e.last_lba_lsb=99;assert(parse_gpt_part_entry(&e,0)==1&&disk.lba_count[0]==90);e.last_lba_lsb=100;assert(!parse_gpt_part_entry(&e,1));e.last_lba_lsb=9;assert(!parse_gpt_part_entry(&e,1));e.first_lba_msb=1;e.last_lba_lsb=20;assert(!parse_gpt_part_entry(&e,1));return 0;}
