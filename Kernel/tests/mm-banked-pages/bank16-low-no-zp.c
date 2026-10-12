#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
typedef uint16_t usize_t;
typedef unsigned irqflags_t;
struct p_tab { uint16_t p_page,p_page2,p_top,p_size,p_pid; unsigned p_status; };
typedef struct p_tab *ptptr;
struct u_block { char bytes[512]; };
struct exec { uint16_t a_base,a_size; };
static struct p_tab process;
static struct { uint16_t u_page,u_page2,u_top; int u_error; ptptr u_ptab; } udata;
static uint16_t ramtop=0xF000, procmem=256;
#define MAX_MAPS 16
#define MAPBASE 0
#define PROGBASE 0
#define PROGLOAD 0x100
#define PROGTOP 0xF000
#define SWAPDEV 1
#define SWAPBASE 0x200
#define SWAP_SIZE 127
#define MAP_SIZE 0xF000
#define ENOMEM 12
#define ENOEXEC 8
#define P_ZOMBIE 6
#define P_EMPTY 0
#define PANIC_MAPOVER "mapover"
#define PANIC_ALREADYSWAP "alreadyswap"
#define PANIC_FREE0 "free0"
static jmp_buf trap;
static int expect_panic, irq_depth, swapped, fail_swap, vectors;
static void panic(const char *s) { if(expect_panic)longjmp(trap,1); abort(); }
static void kprintf(const char *s, ...) {}
static irqflags_t __hard_di(void) { irq_depth++; return 0; }
static void __hard_irqrestore(irqflags_t f) { irq_depth--; }
static void program_vectors(void *p) { vectors++; }
static ptptr swapneeded(ptptr p,int notself);
int swapout(ptptr p);
static int swapmap_alloc(void) { return 2; }
static void copy_common(uint8_t p) {}
static void swapper2(ptptr p,uint16_t m) {}
static struct { unsigned block,size,base,page; } io[16];
static int n_io;
static int transfer(uint16_t dev,uint16_t blk,usize_t bytes,uint16_t base,uint16_t page) {
 assert(n_io<16);io[n_io].block=blk;io[n_io].size=bytes;io[n_io].base=base;io[n_io++].page=page;return 0;
}
#define swapwrite transfer
#define swapread transfer

#define CONFIG_BANK16_LOW

/* SOURCE mm/bank16k_low.c */
static ptptr swapneeded(ptptr p,int f) { return NULL; }
int main(void) {
 udata.u_ptab=&process;process.p_page=0x0403;process.p_page2=0x0605;
 assert(swapout(&process)==0 && n_io==4);
 unsigned total=0;for(int i=0;i<4;i++) {
 total+=io[i].size/512;if(i)assert(io[i].block==io[i-1].block+io[i-1].size/512);
 }
 assert(total==SWAP_SIZE && io[0].base==SWAPBASE && io[1].base==0x4000);
 n_io=0;process.p_page=0x0403;process.p_page2=0x0605;swapin(&process,2);
 assert(n_io==4 && io[3].base==0xC000);return 0;
}
