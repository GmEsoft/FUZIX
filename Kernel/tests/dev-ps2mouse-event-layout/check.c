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

#define MOUSE_REL 0
static uint8_t packet[4],event_copy[4];static bool fivebutton;
static void plt_ps2mouse_event(uint8_t*e){memcpy(event_copy,e,4);}
/* FUNCTION dev/ps2mouse.c ps2mouse_event */
int main(void){packet[0]=5;packet[1]=20;packet[2]=40;ps2mouse_event();assert(event_copy[1]==5&&event_copy[2]==10&&event_copy[3]==20);packet[0]=0x31;packet[1]=0xfe;packet[2]=0xfc;packet[3]=0x30;fivebutton=true;ps2mouse_event();assert(event_copy[1]==25&&event_copy[2]==0xff&&event_copy[3]==0xfe);return 0;}
