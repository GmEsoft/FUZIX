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

static int scsi_sense(unsigned d,uint8_t*b){(void)d;(void)b;return 1;}
/* FUNCTION dev/tinyscsi.c scsi_xfer */
int main(void){uint8_t b[512];assert(scsi_xfer(0,true,0x12345678,b)==1&&calls==1&&opcode==0x28&&wanted_len==512);assert(cmd_rw10[2]==0x12&&cmd_rw10[5]==0x78);assert(scsi_xfer(0,false,0x12345678,b)==1&&calls==2&&opcode==0x2a);result=1;assert(!scsi_xfer(0,true,0x12345678,b));result=0;assert(scsi_xfer(0,true,9,b)==1&&opcode==8);return 0;}
