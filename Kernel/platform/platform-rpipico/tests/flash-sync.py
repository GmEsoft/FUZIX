#!/usr/bin/env python3
"""Check real Dhara restart behavior through the production transfer callback."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--platform', choices=('rpipico', 'esp8266'), default='rpipico')
args = parser.parse_args()
root = Path(__file__).resolve().parents[4]
platform = root / 'Kernel/platform' / ('platform-' + args.platform)
text = (platform / 'devflash.c').read_text()
nand_definition = re.search(r'static const struct dhara_nand nand\s*=\s*\{.*?\};', text, re.S).group(0)
gc_ratio = int(re.search(r'dhara_map_init\(&dhara, &nand, journal_buf, (\d+)\)', text).group(1))
source = (root / 'Standalone/mkftl.c').read_text().split('static void panic(')[0]
source = source.replace('"../Kernel/', '"' + str(root / 'Kernel') + '/')
# Dhara resume also calls the NAND backend without an error-output pointer.
source = source.replace('*err = DHARA_E_NONE;', 'if (err) *err = DHARA_E_NONE;')
source = source.replace('*err = DHARA_E_BAD_BLOCK;', 'if (err) *err = DHARA_E_BAD_BLOCK;')
if args.platform == 'rpipico':
    source += '#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)\n'
    source += re.search(r'^#define FLASH_OFFSET .+$', (platform / 'globals.h').read_text(), re.M).group(0) + '\n'
source += '#define TEST_GC_RATIO %d\n' % gc_ratio
source += nand_definition + '\n'
source += '''
#include <assert.h>
static struct dhara_map dhara;
static uint8_t journal_buf[512];
static struct { unsigned lba; uint8_t *addr; bool is_read; } blk_op;
static struct { int u_error; } udata;
static unsigned sync_calls;
static bool fail_sync;
static int test_map_sync(struct dhara_map *map, dhara_error_t *err) {
    sync_calls++;
    if (fail_sync) {
        if (err) *err = DHARA_E_BAD_BLOCK;
        return -1;
    }
    return dhara_map_sync(map, err);
}
#define dhara_map_sync test_map_sync
'''
start = text.index('static uint_fast8_t transfer_cb(')
end = text.index('static int trim_cb(', start)
assert 'blk->flush = flush_cb;' in text
source += text[start:end]
source += '''
#undef dhara_map_sync
static void restart(void) {
    dhara_error_t err;
    dhara_map_init(&dhara, &nand, journal_buf, TEST_GC_RATIO);
    assert(dhara_map_resume(&dhara, &err) == 0);
}
int main(void) {
    dhara_error_t err;
    uint8_t input[512], output[512];
    flashsize = nand.num_blocks * erasesize;
    flashdata = malloc(flashsize);
    assert(flashdata);
    memset(flashdata, 255, flashsize);
    memset(input, 1, sizeof(input));
    dhara_map_init(&dhara, &nand, journal_buf, TEST_GC_RATIO);
    assert(dhara_map_write(&dhara, 1, input, &err) == 0);
    assert(dhara_map_sync(&dhara, &err) == 0);
    restart();
    memset(input, 2, sizeof(input));
    assert(dhara_map_write(&dhara, 1, input, &err) == 0);
    restart();
    assert(dhara_map_read(&dhara, 1, output, &err) == 0);
    assert(output[0] == 1); /* reproduces loss without checkpoint */
    blk_op.lba = 1; blk_op.addr = input; blk_op.is_read = false;
    for (unsigned i = 3; i < 35; i++) {
        memset(input, i, sizeof(input));
        assert(transfer_cb() == 1);
        restart();
        assert(dhara_map_read(&dhara, 1, output, &err) == 0);
        assert(memcmp(input, output, sizeof(input)) == 0);
    }
    blk_op.is_read = true; blk_op.addr = output;
    assert(transfer_cb() == 1);
    assert(sync_calls == 32); /* reads do not checkpoint */
    fail_sync = true;
    blk_op.is_read = false; blk_op.addr = input;
    assert(transfer_cb() == 0); /* a failed checkpoint is not a completed write */
    assert(flush_cb() == -1 && udata.u_error == EIO);
    fail_sync = false;
    assert(flush_cb() == 0);
    restart();
    assert(dhara_map_read(&dhara, 1, output, &err) == 0);
    assert(memcmp(input, output, sizeof(input)) == 0);
    free(flashdata);
}
'''
with tempfile.TemporaryDirectory(prefix='fuzix-flash-sync-') as directory:
    src = Path(directory) / 'flash.c'
    exe = Path(directory) / 'flash'
    src.write_text(source)
    subprocess.run(['cc', '-std=c99', '-fsanitize=address,undefined', str(src),
                    str(root / 'Kernel/lib/dhara/map.c'), str(root / 'Kernel/lib/dhara/journal.c'),
                    str(root / 'Kernel/lib/dhara/error.c'), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print('PASS %s: unsynced sector loss, 32 durable writes, read path, flush and checkpoint errors' % args.platform)
