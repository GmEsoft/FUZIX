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
/* HEADER dev/devscsi.h */
struct Sidcmd si_dcb;uint8_t si_user;
/* HEADER dev/blkdev.h */

struct blkparam blk_op;static blkdev_t disk;static unsigned observed;
int si_docmd(uint8_t*p){(void)p;observed=si_dcb.length;assert(si_dcb.length==512U*si_dcb.cb.cb0.noblocks);return 0;}static int si_err(const char*s,int e){(void)s;(void)e;return 0;}void si_reset(void){}
/* FUNCTION dev/devscsi.c si_cmd */
int main(void){blk_op.blkdev=&disk;blk_op.nblock=128;assert(si_cmd()==127&&observed==65024);blk_op.nblock=256;assert(si_cmd()==127);blk_op.nblock=1;assert(si_cmd()==1&&observed==512);return 0;}
