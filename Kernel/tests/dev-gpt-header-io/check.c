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

static uint8_t header[512],buf[512];static unsigned calls,entries;static unsigned fail;
static uint_fast8_t transfer(void){calls++;if(fail==calls)return 0;if(calls==1)memcpy(buf,header,512);else memset(buf,0,512);return 1;}
static int parse_gpt_part_entry(gpt_partition_entry_t*e,uint8_t n){(void)e;(void)n;entries++;return 1;}
/* FUNCTION dev/gpt.c parse_gpt */
static void reset(void){memset(header,0,512);memcpy(header,"EFI PART",8);gpt_header_t*h=(void*)header;h->revision=0x00010000;h->header_size=92;h->table_st_lba_lsb=2;h->size_part_entry=128;h->num_part_entries=1;calls=entries=0;disk.transfer=transfer;blk_op.blkdev=&disk;}
int main(void){reset();parse_gpt(buf,0);assert(calls==2 && entries==1);reset();fail=1;parse_gpt(buf,0);assert(calls==1&&!entries);reset();fail=2;parse_gpt(buf,0);assert(calls==2&&!entries);return 0;}
