#!/usr/bin/env python3
"""Exercise production rawflash.c with a simulated flash/USB safety gate."""
from pathlib import Path
import subprocess
import tempfile

platform = Path(__file__).resolve().parent.parent
kernel = platform.parents[1]
headers = {
    'kernel.h': '''
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef uint32_t irqflags_t;
irqflags_t di(void);
void irqrestore(irqflags_t);
void fpanic(const char *) __attribute__((noreturn));
#define GLOBALS_H
#define FLASH_OFFSET 4096
''',
    'hardware/gpio.h': '',
    'hardware/regs/addressmap.h': '''
extern unsigned char mock_flash[];
#define XIP_NOCACHE_NOALLOC_BASE mock_flash
''',
    'pico/stdlib.h': '#define __not_in_flash_func(name) name\n',
    'hardware/flash.h': '''
void flash_range_program(uint32_t, const uint8_t *, size_t);
void flash_range_erase(uint32_t, size_t);
''',
    'pico/flash.h': '''
#define PICO_OK 0
int flash_safe_execute(void (*)(void *), void *, uint32_t);
''',
}
fixture = r'''
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>
#include "lib/dhara/nand.h"

unsigned char mock_flash[32768];
static unsigned irq_disabled, gate_calls, writes, erases, parked;
static int gate_failure;
static uint8_t input[512], expected[512];
static jmp_buf panic_return;
static unsigned expecting_panic;

irqflags_t di(void) {
    unsigned old = irq_disabled;
    irq_disabled = 1;
    return old;
}
void irqrestore(irqflags_t flags) {
    assert(!parked); /* Core 0 must not resume IRQs before releasing USB. */
    irq_disabled = flags;
}
void fpanic(const char *message) {
    assert(expecting_panic);
    assert(strcmp(message, "flash lockout failed") == 0);
    longjmp(panic_return, 1);
}
int flash_safe_execute(void (*callback)(void *), void *argument, uint32_t timeout) {
    assert(irq_disabled);
    assert(timeout > 0 && timeout <= 1000);
    gate_calls++;
    if (gate_failure == 1) return -1;
    parked = 1;
    /* The original source can no longer be read while XIP is unavailable. */
    memset(input, 0, sizeof(input));
    callback(argument);
    assert(irq_disabled);
    parked = 0;
    return gate_failure == 2 ? -1 : 0;
}
void flash_range_program(uint32_t offset, const uint8_t *data, size_t count) {
    assert(irq_disabled);
#if NUM_DEV_TTY_USB > 0
    assert(parked);
#endif
    assert(data != input);
    assert(offset == FLASH_OFFSET + 7 * 512 && count == 512);
    assert(memcmp(data, expected, count) == 0);
    memcpy(mock_flash + offset, data, count);
    writes++;
}
void flash_range_erase(uint32_t offset, size_t count) {
    assert(irq_disabled);
#if NUM_DEV_TTY_USB > 0
    assert(parked);
#endif
    assert(offset == FLASH_OFFSET + 2 * 4096 && count == 4096);
    memset(mock_flash + offset, 255, count);
    erases++;
}
int main(void) {
    const struct dhara_nand nand = {9, 3, 7};
    uint8_t output[17];
    for (unsigned initial_irq = 0; initial_irq <= 1; initial_irq++) {
        dhara_error_t err = DHARA_E_BAD_BLOCK;
        irq_disabled = initial_irq;
        for (unsigned i = 0; i < sizeof(input); i++) input[i] = i ^ 0xa5;
        memcpy(expected, input, sizeof(input));
        assert(dhara_nand_prog(&nand, 7, input, &err) == 0);
        assert(err == DHARA_E_NONE && irq_disabled == initial_irq);
        assert(dhara_nand_read(&nand, 7, 19, sizeof(output), output, NULL) == 0);
        assert(memcmp(output, expected + 19, sizeof(output)) == 0);
        assert(dhara_nand_erase(&nand, 2, NULL) == 0);
        assert(irq_disabled == initial_irq);
        for (unsigned i = 12288; i < 16384; i++) assert(mock_flash[i] == 255);
    }
    assert(writes == 2 && erases == 2);
#if NUM_DEV_TTY_USB > 0
    assert(gate_calls == 4);
    /* Neither entry nor exit failure may be reported as a successful NAND
     * write (or as a bad block that Dhara would retry). */
    for (gate_failure = 1; gate_failure <= 2; gate_failure++) {
        dhara_error_t err = DHARA_E_BAD_BLOCK;
        unsigned before = writes;
        memcpy(input, expected, sizeof(input));
        expecting_panic = 1;
        if (!setjmp(panic_return)) {
            dhara_nand_prog(&nand, 7, input, &err);
            abort();
        }
        expecting_panic = 0;
        assert(err == DHARA_E_BAD_BLOCK);
        assert(writes == before + (gate_failure == 2));
    }
#else
    assert(gate_calls == 0);
#endif
    puts("PASS: staging, flash offsets, IRQ preservation and safety gate");
}
'''
with tempfile.TemporaryDirectory(prefix='fuzix-kernel-flash-') as directory:
    directory = Path(directory)
    for name, contents in headers.items():
        path = directory / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents)
    src = directory / 'fixture.c'
    src.write_text(fixture)
    for label, definitions in (
        ('kernel XIP with USB', ['-DNUM_DEV_TTY_USB=4']),
        ('kernel XIP without USB', ['-DNUM_DEV_TTY_USB=0']),
    ):
        exe = directory / 'test'
        subprocess.run(['cc', '-std=c99', '-g', '-Wall', '-Wextra',
                        '-Wno-unused-parameter', '-fsanitize=address,undefined',
                        *definitions, '-I' + str(directory), '-I' + str(kernel),
                        str(src), str(platform / 'rawflash.c'), '-o', str(exe)], check=True)
        print(label, flush=True)
        subprocess.run([str(exe)], check=True)
