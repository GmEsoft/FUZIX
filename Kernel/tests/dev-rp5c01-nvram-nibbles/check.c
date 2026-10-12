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

/* HEADER dev/rp5c01.h */
static uint8_t regs[4][13],mode;uint8_t rp5c01_read_reg(uint8_t r){return regs[mode][r];}void rp5c01_write_reg(uint8_t r,uint8_t v){if(r==MODE_SEL)mode=v&3;else regs[mode][r]=v;}
/* FUNCTION dev/rp5c01.c rp5c01_nvram_write */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wreturn-mismatch"
#endif
/* FUNCTION dev/rp5c01.c rp5c01_nvram_read */
int main(void){for(unsigned v=0;v<256;v++){rp5c01_nvram_write(0,v);assert(rp5c01_nvram_read(0)==v);}assert(rp5c01_nvram_read(13)==0);return 0;}
