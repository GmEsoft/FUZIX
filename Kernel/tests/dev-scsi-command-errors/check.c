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

static unsigned deselects,clears;static uint8_t cmd_error,data_error,end_error;
uint8_t si_select(void){return 0;}uint8_t si_writecmd(uint8_t l){(void)l;return cmd_error;}uint8_t si_read(uint8_t*p){(void)p;return data_error;}uint8_t si_write(uint8_t*p){(void)p;return data_error;}static uint8_t si_cmdend(void){return end_error;}void si_clear(void){clears++;}void si_deselect(void){deselects++;}
/* FUNCTION dev/devscsi.c si_docmd */
int main(void){si_dcb.direction=SIDIR_READ;si_dcb.length=1;cmd_error=4;assert(si_docmd(NULL)==4&&deselects==1&&clears==1);cmd_error=0;data_error=5;assert(si_docmd(NULL)==5&&deselects==2);data_error=0;end_error=2;assert(si_docmd(NULL)==2&&deselects==3);end_error=0;assert(!si_docmd(NULL)&&deselects==4);return 0;}
