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

#define SEND_STOP_ACK 0xc3
#define SEND_STOP 0xc2
#define NACK 0x40
#define BUS_LRB 8
static uint8_t i2c_s0,i2c_s1;static int recv_result=-ETIMEDOUT;static unsigned recvs;
static int pcf8584_bus_wait(void){return 0;}static int pcf8584_connect(uint8_t a){(void)a;return 0;}static int pcf8584_recv(void){recvs++;return recv_result;}static int pcf8584_ack_pin(void){return -1;}static int pcf8584_pin_wait(void){return 0;}
/* FUNCTION dev/pcf8584.c pcf8584_xmit */
/* FUNCTION dev/pcf8584.c pcf8584_rx_msg */
int main(void){uint8_t b=99;assert(pcf8584_rx_msg(1,&b,1)==-ETIMEDOUT&&b==99&&i2c_s1==SEND_STOP);assert(pcf8584_rx_msg(1,&b,0)==0&&recvs==1&&b==99);assert(pcf8584_xmit(7)==-1);recv_result=42;assert(pcf8584_rx_msg(1,&b,1)==0&&b==42);return 0;}
