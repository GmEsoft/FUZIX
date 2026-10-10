#include <kernel.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct p_tab ptab[PTABSIZE];
uint16_t swap_dev = 0xffff;

void panic(const char *message)
{
    fprintf(stderr, "panic: %s\n", message);
    abort();
}
void kprintf(const char *fmt, ...) { (void)fmt; }
ptptr swapneeded(ptptr p, int notself)
{
    (void)p;
    (void)notself;
    return NULL;
}
int16_t swapmap_alloc(void) { return -1; }
void swapread(uint16_t dev, uint16_t block, usize_t len, uaddr_t buf, uint16_t page)
{
    (void)dev; (void)block; (void)len; (void)buf; (void)page;
    abort();
}
void swapwrite(uint16_t dev, uint16_t block, usize_t len, uaddr_t buf, uint16_t page)
{
    (void)dev; (void)block; (void)len; (void)buf; (void)page;
    abort();
}
void swap_blocks(void *a, void *b, size_t len)
{
    uint8_t *x = a, *y = b;
    while (len--) {
        uint8_t tmp = *x;
        *x++ = *y;
        *y++ = tmp;
    }
}

static void reset(void)
{
    memset(ptab, 0, sizeof(ptab));
    memset(progbase, 0, USERMEM);
    pagemap_init();
    ptab[0].p_top = PROGBASE + 4096;
    assert(pagemap_alloc(&ptab[0]) == 0);
    udata.u_ptab = &ptab[0];
    udata.u_top = ptab[0].p_top;
    udata.u_break = PROGLOAD + 512;
}

static void alloc_process(unsigned slot, unsigned bytes)
{
    ptab[slot].p_top = PROGBASE + bytes;
    assert(pagemap_alloc(&ptab[slot]) == 0);
}

static void fill_except_last_block(void)
{
    /* 4 KiB current + 64 + 64 + 24 = 156 KiB: only one block remains. */
    alloc_process(1, 65536);
    alloc_process(2, 65536);
    alloc_process(3, 24576);
    assert(pagemap_mem_used() == 156);
}

static void break_boundary(void)
{
    uaddr_t top = ptab[0].p_top;
    assert(brk_extend(top) == 0);
    assert(ptab[0].p_top == top);
    assert(brk_extend(top + 1) == 0);
    assert(ptab[0].p_top == top + 4096);
    assert(brk_extend(top + 4096) == 0);
    assert(ptab[0].p_top == top + 4096);
}

static void realloc_failure(void)
{
    fill_except_last_block();
    uaddr_t top = ptab[0].p_top;
    memset((void *)PROGLOAD, 0x5a, top - PROGLOAD);
    for (unsigned i = 0; i < 3; i++) {
        assert(pagemap_realloc(NULL, 12288 - UDATA_SIZE) == ENOMEM);
        assert(pagemap_mem_used() == 156);
        assert(ptab[0].p_top == top);
        assert(udata.u_top == top);
    }
    for (uaddr_t p = PROGLOAD; p < top; p++)
        assert(*(uint8_t *)p == 0x5a);
    assert(pagemap_realloc(NULL, 8192 - UDATA_SIZE) == 0);
    assert(pagemap_mem_used() == 160);
}

static void alloc_failure(void)
{
    fill_except_last_block();
    ptab[4].p_top = PROGBASE + 8192;
    for (unsigned i = 0; i < 3; i++) {
        assert(pagemap_alloc(&ptab[4]) == ENOMEM);
        assert(pagemap_mem_used() == 156);
        assert(ptab[4].p_page == 0);
    }
    alloc_process(4, 4096);
    assert(pagemap_mem_used() == 160);
}

static void bounds(void)
{
    assert(pagemap_realloc(NULL, PROGSIZE + 1) == ENOMEM);
    assert(pagemap_realloc(NULL, UINT32_MAX) == ENOMEM);
    assert(pagemap_realloc(NULL, UINTPTR_MAX) == ENOMEM);
    assert(pagemap_mem_used() == 4);
    assert(brk_extend(PROGBASE) == EINVAL);
    assert(brk_extend(PROGTOP + 1) == ENOMEM);
    ptab[1].p_top = PROGTOP + 1;
    assert(pagemap_alloc(&ptab[1]) == ENOMEM);
    assert(pagemap_mem_used() == 4);
    assert(pagemap_realloc(NULL, PROGSIZE) == 0);
    assert(ptab[0].p_top == PROGTOP);
    assert(brk_extend(PROGTOP) == 0);
    assert(pagemap_mem_used() == TEST_PROCMEM / 1024);
}

static void grow_shrink(void)
{
    for (usize_t bytes = 4096; bytes <= TEST_PROCMEM; bytes += 4096) {
        assert(pagemap_realloc(NULL, bytes - UDATA_SIZE) == 0);
        assert(pagemap_mem_used() == bytes / 1024);
        assert(ptab[0].p_top == PROGBASE + bytes);
        assert(udata.u_top == ptab[0].p_top);
        for (uaddr_t p = PROGLOAD; p < PROGBASE + bytes - 4096; p++)
            assert(*(uint8_t *)p == 0xa5);
        memset((void *)PROGLOAD, 0xa5, bytes - UDATA_SIZE);
    }
    assert(pagemap_realloc(NULL, 4096 - UDATA_SIZE) == 0);
    assert(pagemap_mem_used() == 4);
    assert(ptab[0].p_top == PROGBASE + 4096);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    reset();
    if (!strcmp(argv[1], "break")) break_boundary();
    else if (!strcmp(argv[1], "realloc-failure")) realloc_failure();
    else if (!strcmp(argv[1], "alloc-failure")) alloc_failure();
    else if (!strcmp(argv[1], "bounds")) bounds();
    else if (!strcmp(argv[1], "grow-shrink")) grow_shrink();
    else abort();
    puts(argv[1]);
    return 0;
}
