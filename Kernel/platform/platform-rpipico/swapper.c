#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>
#include "config.h"
#include "globals.h"

#undef DEBUG

/* A special swap and pagemap implementation for the Raspberry Pi Pico. Available
 * memory is divided into 4kB chunks; processes can occupy any number of these (up
 * to PROCMEM KiB). When a process is switched in, memory is rearranged by swapping chunks
 * until the running process is at the bottom of memory in the right order.
 *
 * p->p_page is 0 if swapped out, 1 is swapped in.
 */

#define BLOCKSIZE 4096
#define NUM_ALLOCATION_BLOCKS (USERMEM / BLOCKSIZE)

uint8_t progbase[USERMEM];

struct mapentry
{
    uint8_t slot;
    uint8_t block;
};
static struct mapentry allocation_map[NUM_ALLOCATION_BLOCKS];

static uint8_t get_slot(ptptr p)
{
	uint8_t slot = p - ptab;
    if (slot >= PTABSIZE)
        panic("bad ptab");
    return slot;
}

static uaddr_t get_proc_size(ptptr p)
{
    if (!p)
        return 0;
    /* init is initially created with a p_top of 0, but it actually needs 512 bytes. */
    if (!p->p_top)
        udata.u_top = p->p_top = PROGLOAD + 512;
    return p->p_top - PROGBASE;
}

static int get_proc_size_blocks(ptptr p)
{
    return (uaddr_t)alignup(get_proc_size(p), BLOCKSIZE) / BLOCKSIZE;
}

static struct mapentry* find_block(uint8_t slot, uint8_t block)
{
    for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
    {
        struct mapentry* b = &allocation_map[i];
        if ((b->slot == slot) && (b->block == block))
            return b;
    }

    return NULL;
}

static struct mapentry* find_free_block(ptptr p)
{
    for (;;)
    {
        struct mapentry* b = find_block(0xff, 0xff); /* find a free block */
        if (b)
            return b;

        #ifdef DEBUG
            kprintf("alloc failed, finding a process to swap out");
        #endif
        if (!swapneeded(p, true))
        {
            kprintf("warning: out of memory\n");
            return NULL;
        }
    }
}

#ifdef DEBUG
    static void debug_blocks(void)
    {
        kprintf("current process size %p bytes %d blocks; isp %d rel\n",
            get_proc_size(udata.u_ptab), get_proc_size_blocks(udata.u_ptab),
            udata.u_isp - PROGBASE);
        for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
        {
            const struct mapentry* b = &allocation_map[i];
            void* p = (void*)PROGBASE + i*BLOCKSIZE;
            if (b->block != 0xff)
                kprintf("#%d: slot %d block %d/%d %p\n",
                        i, b->slot,
                        b->block,
                        get_proc_size_blocks(&ptab[b->slot]),
                        p);
        }
    }
#endif

/* Release an allocation suffix, including blocks claimed by a failed grow. */
static void free_blocks(uint8_t slot, unsigned first)
{
    for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
    {
        struct mapentry* b = &allocation_map[i];
        if (b->slot == slot && b->block >= first)
            b->slot = b->block = 0xff;
    }
}

void pagemap_free(ptptr p)
{
    #ifdef DEBUG
        kprintf("free %d\n", get_slot(p));
    #endif
    int slot = get_slot(p);
    for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
    {
        struct mapentry* b = &allocation_map[i];
        if (b->slot == slot)
        {
            #ifdef DEBUG
                kprintf("free slot #%d\n", i);
            #endif
            b->slot = b->block = 0xff;
        }
    }

	p->p_page = 0;
}

int pagemap_alloc(ptptr p)
{
	if (p == udata.u_ptab)
		return 0;

    uaddr_t size = get_proc_size(p);
    if (size > PROGSIZE + UDATA_SIZE || size > USERMEM)
        return ENOMEM;
    int blocks = get_proc_size_blocks(p);
    int slot = get_slot(p);
    #ifdef DEBUG
        kprintf("alloc %d, %d blocks\n", get_slot(p), blocks);
        debug_blocks();
    #endif

    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_free_block(p);
        if (!b)
        {
            pagemap_free(p);
            return ENOMEM;
        }
        b->slot = slot;
        b->block = i;
    }

	p->p_page = 1;
    #ifdef DEBUG
        kprintf("done alloc\n");
        debug_blocks();
    #endif
	return 0;
}

/* size does *not* include udata */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
    struct p_tab* p = udata.u_ptab;

    /* Check before addition/rounding, including the physical pool bound. */
    if (size > PROGSIZE || size > USERMEM - UDATA_SIZE)
        return ENOMEM;
    int oldblocks = get_proc_size_blocks(p);
    int blocks = (int)alignup(size + UDATA_SIZE, BLOCKSIZE) / BLOCKSIZE;
    int slot = get_slot(p);
    #ifdef DEBUG
        kprintf("realloc %d from %d to %d blocks\n", get_slot(udata.u_ptab), oldblocks, blocks);
    #endif
    if (blocks < oldblocks)
    {
        #ifdef DEBUG
            kprintf("shrinking process\n");
        #endif
        for (int i=blocks; i<oldblocks; i++)
        {
            struct mapentry* b = find_block(slot, i);
            b->slot = b->block = 0xff;
        }
    }
    else if (blocks > oldblocks)
    {
        #ifdef DEBUG
            kprintf("growing process\n");
        #endif
        for (int i=oldblocks; i<blocks; i++)
        {
            struct mapentry* b = find_free_block(p);
            if (!b)
            {
                free_blocks(slot, oldblocks);
                return ENOMEM;
            }
            b->slot = slot;
            b->block = i;
        }
    }

    udata.u_top = p->p_top = PROGBASE + blocks*BLOCKSIZE;
    p->p_size = blocks*BLOCKSIZE / 1024;
    #ifdef DEBUG
        debug_blocks();
    #endif
    contextswitch(p);
	return 0;
}

usize_t pagemap_mem_used(void)
{
    usize_t count = 0;
    for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
    {
        struct mapentry* b = &allocation_map[i];
        if (b->slot != 0xff)
            count++;
    }
    return count * (BLOCKSIZE/1024);
}

void pagemap_init(void)
{
    #ifdef DEBUG
        kprintf("%d blocks of memory\n", NUM_ALLOCATION_BLOCKS);
    #endif
    memset(allocation_map, 0xff, sizeof(allocation_map));
	udata.u_ptab = NULL;
}

void contextswitch(ptptr p)
{
    #ifdef DEBUG
        kprintf("context switch from %d to %d\n", get_slot(udata.u_ptab), get_slot(p));
    #endif

    int map = -1;
    if (!p->p_page)
    {
        map = p->p_page2;
        swapin(p, map);
    }

    int slot = get_slot(p);
    int blocks = get_proc_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b1 = &allocation_map[i];
        int i1 = b1 - allocation_map;
        void* p1 = (void*)PROGBASE + i1*BLOCKSIZE;
        if ((b1->slot != slot) || (b1->block != i))
        {
            struct mapentry* b2 = find_block(slot, i);
            if (!b2)
                panic("missing block");
            int i2 = b2 - allocation_map;
            void* p2 = (void*)PROGBASE + i2*BLOCKSIZE;
            if (b1->slot == 0xff)
            {
                #ifdef DEBUG
                    kprintf("copy #%d to #%d\n", i2, i1);
                #endif
                memcpy(p1, p2, BLOCKSIZE);
            }
            else
            {
                #ifdef DEBUG
                    kprintf("swap #%d and #%d\n", i1, i2);
                #endif
                swap_blocks(p1, p2, BLOCKSIZE);
            }

            struct mapentry t = *b1;
            *b1 = *b2;
            *b2 = t;
        }
    }

    /* The incoming udata is now at PROGBASE. Do not issue I/O before this:
     * swapread uses the old udata as scratch, even if block zero was reused. */
    if (map >= 0)
    {
#ifdef CONFIG_TRIM
        uint8_t error = udata.u_error;
        blkno_t block = map * SWAP_SIZE;
        for (unsigned i = 0; i < SWAP_SIZE; i++, block++)
            d_ioctl(SWAPDEV, HDIO_TRIM_KERNEL, (void *)&block);
        udata.u_error = error;
#endif
        swapmap_add(map);
    }

    #ifdef DEBUG
        debug_blocks();
    #endif
}

/* Copy the current process into a new child slot, and context switch so it's live. */
void clonecurrentprocess(ptptr p)
{
    #ifdef DEBUG
        kprintf("clone %d to slot %d\n", get_slot(udata.u_ptab), get_slot(p));
        if (p->p_top != udata.u_ptab->p_top)
            panic("mismatched sizes");
    #endif
    int srcslot = get_slot(udata.u_ptab);
    int destslot = get_slot(p);
    int blocks = get_proc_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b1 = find_block(srcslot, i);
        struct mapentry* b2 = find_block(destslot, i);
        if (!b1 || !b2)
            panic("missing block");
        int i1 = b1 - allocation_map;
        int i2 = b2 - allocation_map;
        void* p1 = (void*)PROGBASE + i1*BLOCKSIZE;
        void* p2 = (void*)PROGBASE + i2*BLOCKSIZE;
        #ifdef DEBUG
            kprintf("copy #%d to #%d (%p to %p)\n", i1, i2, p1, p2);
        #endif
        memcpy(p2, p1, BLOCKSIZE);

        struct mapentry t = *b1;
        *b1 = *b2;
        *b2 = t;
    }
    #ifdef DEBUG
        kprintf("end clone\n");
    #endif
}

uint_fast8_t plt_canswapon(uint16_t devno)
{
    /* Only allow swapping to hd devices. */
    return (devno >> 8) == 0;
}

int swapout(ptptr p)
{
#ifdef DEBUG
	kprintf("swapping out %d (%d)\n", get_slot(p), p->p_pid);
#endif

	uint16_t page = p->p_page;
	if (!page)
		panic(PANIC_ALREADYSWAP);
    if (SWAPDEV == 0xffff)
        return ENOMEM;

	/* Are we out of swap ? */
	int16_t map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;

	uint16_t swaparea = map * SWAP_SIZE;

    int slot = get_slot(p);
    int blocks = get_proc_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_block(slot, i);
        if (!b)
            panic("missing block");
        int blockindex = b - allocation_map;
        uaddr_t addr = PROGBASE + blockindex*BLOCKSIZE;

        if (swapwrite(SWAPDEV, swaparea + (i*(BLOCKSIZE>>BLKSHIFT)),
                BLOCKSIZE, addr, 1) != BLOCKSIZE)
            goto failed;
    }
    if (d_flush(SWAPDEV))
        goto failed;

    /* Keep every RAM block until the complete disk image is committed. */
    free_blocks(slot, 0);
    p->p_page = 0;
    p->p_page2 = map;
    return 0;
failed:
    swapmap_add(map);
    return EIO;
}

/*
 * Swap ourself in: must be on the swap stack when we do this.
 */
void swapin(ptptr p, uint16_t map)
{
    uint16_t swaparea = map * SWAP_SIZE;
    int slot = get_slot(p);
    int blocks = get_proc_size_blocks(p);
    ptptr current = udata.u_ptab;
    bool reserved_zero = false;

    /* Allocate the whole image before reading. The normal victim selector
     * excludes the outgoing process; here its stack is no longer in use. */
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_free_block(p);
        if (!b && current && current != p && current->p_page)
        {
            if (swapout(current))
                goto failed;
            /* udata remains I/O scratch until the very last read. */
            allocation_map[0].block = 0;
            reserved_zero = true;
            b = find_free_block(p);
        }
        if (!b && reserved_zero && i == blocks - 1)
        {
            b = &allocation_map[0];
            reserved_zero = false;
        }
        if (!b)
            goto failed;
        b->slot = slot;
        b->block = i;
    }

    /* Read physical block zero last: subsequent swapread calls would modify
     * the data just loaded there through the udata alias. */
    for (int pass=0; pass<2; pass++)
        for (int i=0; i<blocks; i++)
        {
            struct mapentry* b = find_block(slot, i);
            int blockindex = b - allocation_map;
            if ((blockindex == 0) != (pass == 1))
                continue;
            if (swapread(SWAPDEV, swaparea + (i*(BLOCKSIZE>>BLKSHIFT)),
                    BLOCKSIZE, PROGBASE + blockindex*BLOCKSIZE, 1) != BLOCKSIZE)
                goto failed;
        }
    if (reserved_zero)
        allocation_map[0].block = 0xff;
    p->p_page = 1;
    p->p_page2 = 0;
    return;
failed:
    free_blocks(slot, 0);
    if (reserved_zero)
        allocation_map[0].block = 0xff;
    /* The disk slot remains owned by p. Never resume a partial image. The
     * void swapin interface has no way to return a scheduling failure. */
    panic("swapin failed");
}

arg_t brk_extend(uaddr_t addr)
{
    if (addr < PROGLOAD)
        return EINVAL;
    if (addr > PROGTOP)
        return ENOMEM;
    if (addr > brk_limit()) {
        /* Claim more memory for this process. */
        if (pagemap_realloc(NULL, addr - PROGLOAD))
		{
			kprintf("%d: out of memory by %d\n", udata.u_ptab->p_pid,
				addr - brk_limit());
            return ENOMEM;
		}
        return 0;
    }
    return 0;
}

// vim: ts=4 sw=4 et
