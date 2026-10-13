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

static uint8_t td_raw,scsi_status[2];static uint8_t cmd_rw[6]={0,0,0,0,1,0},cmd_rw10[10]={0,0,0,0,0,0,0,0,1,0},cmd_sense[6]={3,0,0,0,16,0};
static unsigned calls;static uint8_t opcode;static unsigned wanted_len;static int result;
static int scsi_cmd(unsigned d,uint8_t*c,uint8_t*b,unsigned l){(void)d;(void)b;calls++;opcode=c[0];wanted_len=l;return result;}

/* FUNCTION dev/tinyscsi.c scsi_sense */
/* FUNCTION dev/tinyscsi.c scsi_xfer */
int main(void){uint8_t b[16];td_raw=2;assert(scsi_sense(0,b)==0&&wanted_len==16&&td_raw==2);uint8_t data[512];scsi_status[0]=2;result=1;assert(scsi_xfer(0,true,1,data)==0);return 0;}
