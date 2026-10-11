#ifndef TEST_KERNEL_H
#define TEST_KERNEL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

/* Host fixture for swapper.c; no SDK, interrupts or physical block device. */
#define CONFIG_H
/* Allocator tests do not use generated flash geometry. */
#define GLOBALS_H
#define PTABSIZE 16
#define BLKSHIFT 9
#define USERMEM (160 * 1024)
#define UDATA_SIZE (3 * 512)
#ifndef TEST_PROCMEM
#define TEST_PROCMEM 65536
#endif
#define PROGSIZE (TEST_PROCMEM - UDATA_SIZE)
#define SWAP_SIZE (TEST_PROCMEM >> BLKSHIFT)
#define SWAPDEV swap_dev
#define PANIC_ALREADYSWAP "already swapped"

typedef uintptr_t uaddr_t;
typedef uintptr_t usize_t;
typedef int32_t arg_t;
struct exec;
struct p_tab {
    uaddr_t p_top;
    usize_t p_size;
    uint16_t p_page, p_page2, p_pid;
};
typedef struct p_tab *ptptr;
struct u_data {
    ptptr u_ptab;
    uaddr_t u_top, u_break, u_isp;
};
extern struct p_tab ptab[PTABSIZE];
extern uint8_t progbase[USERMEM];
extern uint16_t swap_dev;
#define udata (*(struct u_data *)progbase)
#define PROGBASE ((uaddr_t)progbase)
#define PROGLOAD (PROGBASE + UDATA_SIZE)
#define PROGTOP (PROGLOAD + PROGSIZE)
#define brk_limit() (udata.u_ptab->p_top)
#define alignup(v, a) (((uaddr_t)(v) + (a) - 1) & ~((uaddr_t)(a) - 1))

void panic(const char *) __attribute__((noreturn));
void kprintf(const char *, ...);
ptptr swapneeded(ptptr, int);
int16_t swapmap_alloc(void);
void swapread(uint16_t, uint16_t, usize_t, uaddr_t, uint16_t);
void swapwrite(uint16_t, uint16_t, usize_t, uaddr_t, uint16_t);
void swap_blocks(void *, void *, size_t);
void swapin(ptptr, uint16_t);
int pagemap_alloc(ptptr);
int pagemap_realloc(struct exec *, usize_t);
void pagemap_free(ptptr);
void pagemap_init(void);
void contextswitch(ptptr);
usize_t pagemap_mem_used(void);
arg_t brk_extend(uaddr_t);

#endif
