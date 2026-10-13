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

static unsigned disconnects;static uint8_t selected,phase;
static void ppa_c_pulse(uint8_t n){(void)n;}static void ppa_disconnect(void){disconnects++;}static uint8_t ppa_select(uint8_t a,uint8_t b){(void)a;(void)b;return selected;}static void ppa_write_ctrl(uint8_t a){(void)a;}static void ppa_write_data(uint8_t a){(void)a;}static uint8_t ppa_wait_done(void){return phase;}static uint8_t ppa_read_status(void){return 0;}static void ppa_block_write(void){}static void ppa_block_read(void){}
/* FUNCTION dev/devppa.c ppa_connect */
/* FUNCTION dev/devppa.c ppa_command */
int main(void){uint8_t cmd=0;assert(ppa_command(6,&cmd,1,NULL,0)==0xff&&disconnects==1);selected=1;assert(ppa_command(6,&cmd,1,NULL,0)==0xff&&disconnects==2);assert(ppa_command(6,&cmd,0,NULL,0)==0xff&&disconnects==3);phase=0xf0;assert(ppa_command(6,&cmd,0,NULL,0)==0&&disconnects==4);return 0;}
