#ifndef XIP_TEST_EXEC_H
#define XIP_TEST_EXEC_H
#include <kernel.h>

struct exec_xip {
	uaddr_t entry;
	usize_t data_size;
	usize_t bss_size;
};

int xip_auto_try(struct cinode *ino, const void *ehdr, uaddr_t himem,
		 struct exec_xip *image, int *index);
void xip_auto_load(int index);
usize_t xip_auto_readable(uaddr_t base, usize_t size);
void xip_auto_yield(void);
#endif
