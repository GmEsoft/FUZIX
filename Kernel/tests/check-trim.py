#!/usr/bin/env python3
"""Exercise the real block ioctl handler's internal/user TRIM distinction."""
from pathlib import Path
import subprocess
import tempfile
kernel = Path(__file__).resolve().parent.parent
text = (kernel / 'dev/blkdev.c').read_text()
start = text.index('int blkdev_ioctl(')
end = text.index('blkdev_t *blkdev_alloc(', start)
fixture = r'''
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#define CONFIG_TRIM
#define used(x) ((void)(x))
#define BLKFLSBUF 0x4103
#define HDIO_TRIM 0x0106
#define HDIO_TRIM_KERNEL 0x8106
#define BLKGETSIZE 0x0100
typedef uint16_t blkno_t;
typedef uint16_t uarg_t;
struct { int u_error; } udata;
struct device { int (*flush)(void); int (*trim)(void); uint32_t drive_lba_count, lba_count[15]; } blkdev_table[1];
struct { struct device *blkdev; uint32_t lba; } blk_op;
static unsigned trims, user_reads;
static uint16_t user_sector = 9;
static unsigned valaddr_r(const uint8_t *p, unsigned size) {
    return p == (uint8_t *)&user_sector ? size : 0;
}
static uint16_t ugetw(void *p) { user_reads++; return *(uint16_t *)p; }
static int uput(void *p, void *d, unsigned size) { (void)p; (void)d; (void)size; return 0; }
static int translate_lba(unsigned minor) { (void)minor; return blk_op.lba >= 100; }
static int trim(void) { trims++; return 0; }
'''
fixture += text[start:end]
fixture += r'''
int main(void) {
    uint16_t kernel_sector = 7;
    blkdev_table[0].trim = trim;
    assert(blkdev_ioctl(0, HDIO_TRIM_KERNEL, (char *)&kernel_sector) == 0);
    assert(blk_op.lba == 7 && trims == 1 && user_reads == 0);
    assert(blkdev_ioctl(0, HDIO_TRIM, (char *)&user_sector) == 0);
    assert(blk_op.lba == 9 && trims == 2 && user_reads == 1);
    assert(blkdev_ioctl(0, HDIO_TRIM, (char *)&kernel_sector) == -1);
    assert(udata.u_error == EFAULT && trims == 2 && user_reads == 1);
    kernel_sector = 100;
    assert(blkdev_ioctl(0, HDIO_TRIM_KERNEL, (char *)&kernel_sector) == -1);
    assert(trims == 2);
}
'''
# Public syscall rejects internal request classes before d_ioctl is reached.
header = (kernel / 'include/kernel.h').read_text()
assert '#define HDIO_TRIM_KERNEL 0x8106' in header
assert '#define IOCTL_CLASS_KERNEL\t0x80' in header
assert ((0x8106 >> 8) & 0xC0) == 0x80
assert 'rclass == IOCTL_CLASS_KERNEL' in (kernel / 'syscall_fs.c').read_text()
with tempfile.TemporaryDirectory(prefix='fuzix-trim-') as directory:
    src = Path(directory) / 'trim.c'; exe = Path(directory) / 'trim'
    src.write_text(fixture)
    subprocess.run(['cc', '-std=c99', '-Wall', '-Wextra', '-fsanitize=address,undefined', str(src), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print('PASS: internal TRIM avoids user reads; user TRIM validates pointers; invalid sectors rejected')
