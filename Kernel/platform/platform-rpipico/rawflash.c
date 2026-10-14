#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include "lib/dhara/nand.h"
#include "picosdk.h"
#include <hardware/flash.h>
#include "globals.h"
#include <pico/flash.h>

struct flash_operation {
    uint32_t offset;
    const uint8_t *data;
    size_t count;
};

/* Everything accessed after leaving XIP must be in RAM/ROM. The argument
 * lives on the kernel stack; program data is staged in program_buffer. */
static void __not_in_flash_func(perform_flash_operation)(void *argument)
{
    const struct flash_operation *op = argument;
    if (op->data)
        flash_range_program(op->offset, op->data, op->count);
    else
        flash_range_erase(op->offset, op->count);
}

static void execute_flash_operation(struct flash_operation *op)
{
    irqflags_t flags = di();
#if NUM_DEV_TTY_USB > 0
    /* Keep core-0 interrupts disabled until the SDK releases core 1: a USB
     * buffer spinlock may be held by that core while it is parked. The SDK
     * otherwise restores interrupts before releasing the victim. TinyUSB
     * resumes when XIP is restored. Never mark a flash block bad merely
     * because inter-core coordination failed. */
    if (flash_safe_execute(perform_flash_operation, op, 1000) != PICO_OK)
        panic("flash lockout failed");
#else
    perform_flash_operation(op);
#endif
    irqrestore(flags);
}

int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b,
                     dhara_error_t *err)
{
    struct flash_operation op = { FLASH_OFFSET + b * 4096, NULL, 4096 };
    execute_flash_operation(&op);
	if (err)
		*err = DHARA_E_NONE;
	return 0;
}

/* Sources may live in flash (kernel constants, or a user process's XIP
 * text). Stage them into RAM before XIP is disabled. Cooperative I/O
 * serializes use of this buffer. */
static uint8_t program_buffer[512];

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
                    const uint8_t *data,
                    dhara_error_t *err)
{
    memcpy(program_buffer, data, sizeof(program_buffer));
    struct flash_operation op = { FLASH_OFFSET + p * 512, program_buffer, 512 };
    execute_flash_operation(&op);
	if (err)
		*err = DHARA_E_NONE;
	return 0;
}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p,
					size_t offset, size_t length,
                    uint8_t *data,
                    dhara_error_t *err)
{
    memcpy(data,
        (uint8_t*)XIP_NOCACHE_NOALLOC_BASE + FLASH_OFFSET + (p*512) + offset,
        length);
	if (err)
		*err = DHARA_E_NONE;
	return 0;
}

/* Raw program and erase for the automatic XIP extent store. Offsets are
 * absolute flash addresses; lengths are multiples of 512 (program) or 4096
 * (erase), and data must already live in SRAM. Batching a whole run into
 * one call parks the cores once, which is what keeps USB alive during an
 * install. */
int rpipico_flash_program(uint32_t offset, const uint8_t *data, uint32_t len)
{
    struct flash_operation op = { offset, data, len };
    execute_flash_operation(&op);
    return 0;
}

int rpipico_flash_erase(uint32_t offset, uint32_t len)
{
    struct flash_operation op = { offset, NULL, len };
    execute_flash_operation(&op);
    return 0;
}

/* vim: sw=4 ts=4 et: */

