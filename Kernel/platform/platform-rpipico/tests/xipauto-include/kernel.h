#ifndef XIP_TEST_KERNEL_H
#define XIP_TEST_KERNEL_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uintptr_t uaddr_t;
typedef uint32_t usize_t;
typedef uint32_t uoff_t;

typedef struct {
	uint32_t i_mtime;
	uint32_t i_size;
} dinode;
typedef struct cinode {
	uint16_t c_dev;
	uint16_t c_num;
	dinode c_node;
} cinode, *inoptr;

struct p_tab;
struct u_data {
	uint32_t u_offset;
	uint32_t u_count;
	uint8_t *u_base;
	int u_sysio;
	uint32_t u_done;
	uint32_t u_error;
	struct p_tab *u_ptab;
	uint32_t u_codebase;
};
extern struct u_data udata;

extern void readi(inoptr ino, int flag);
extern void *tmpbuf(void);
extern void tmpfree(void *p);
extern void ptimer_insert(void);
extern void psleep(void *event);

typedef int irqflags_t;
static inline int di(void) { return 0; }
static inline void irqrestore(int f) { (void)f; }

#define PTABSIZE 16
#define P_EMPTY 0
#define P_RUNNING 1
#define P_READY 2
#define P_FORKING 6
#define P_NOSLEEP 8
struct p_tab {
	uint8_t p_status;
	uint16_t p_timeout;
};
extern struct p_tab ptab[PTABSIZE];

#define USERSTACK (4 * 1024)
#define ALIGNUP(v) ((uaddr_t)(((uintptr_t)(v) + 3) & ~(uintptr_t)3))
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#endif
