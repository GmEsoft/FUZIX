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

#define SEND 0x100
#define FAIL 0x200
#define STOP 0xffff
/* ARRAY dev/ps2mouse.c mouse_init */
/* ARRAY dev/ps2mouse.c mouse_fivetest */
int main(void){assert(mouse_init[4]==STOP);unsigned values[3],n=0;for(unsigned i=0;mouse_fivetest[i]!=STOP;i++)if(mouse_fivetest[i]==(SEND|0xf3)){assert(n<3);values[n++]=mouse_fivetest[i+2]&255;}assert(n==3&&values[0]==200&&values[1]==200&&values[2]==80);return 0;}
