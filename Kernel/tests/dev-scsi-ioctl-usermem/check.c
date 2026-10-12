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
struct blkparam blk_op;
#define HDIO_RAWCMD 9
static size_t valid;static unsigned issued;static struct Siioctl user;
static int uget(void*p,void*q,size_t n){(void)p;memcpy(q,&user,n);return 0;}static size_t valaddr(void*p,size_t n,bool w){(void)p;(void)n;(void)w;return valid;}int si_docmd(uint8_t*p){(void)p;issued++;assert(si_user==1);return 0;}
/* FUNCTION dev/devscsi.c si_ioctl */
int main(void){user.si_dcb.direction=SIDIR_READ;user.si_dcb.length=10;valid=5;assert(si_ioctl(0,9,NULL)==-1&&!issued&&udata.u_error==EFAULT);valid=10;assert(si_ioctl(0,9,NULL)==0&&issued==1);return 0;}
