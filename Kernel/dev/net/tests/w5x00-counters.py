#!/usr/bin/env python3
"""Exercise the shared WIZnet counter reader with changing transport samples."""
from pathlib import Path
import re
import subprocess
import tempfile

text = Path(__file__).resolve().parents[1].joinpath('net_w5x00.c').read_text()
start = text.index('static uint16_t w5x00_read_counter(')
end = text.index('\n}', start) + 2
helper = text[start:end]
# No counter consumer may bypass the shared retry rule.
assert not re.search(r'w5x00_readsw\([^;]*Sn_(?:TX_FSR|RX_RSR)', text)
source = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
static uint16_t samples[8], expected_reg;
static unsigned index, count;
static uint16_t w5x00_readsw(uint8_t slot, uint16_t reg) {
    assert(slot == 3 && reg == expected_reg && index < count);
    return samples[index++];
}
""" + helper + r"""
static void check(uint16_t reg, const uint16_t *values, unsigned length, uint16_t expected) {
    expected_reg = reg; index = 0; count = length;
    for (unsigned n=0; n<length; n++) samples[n] = values[n];
    assert(w5x00_read_counter(3, reg) == expected && index == count);
}
int main(void) {
    const uint16_t torn[] = {0x01ff, 0x0200, 0x0200};
    const uint16_t changing[] = {0x100, 0x101, 0x102, 0x103, 0x103};
    const uint16_t empty[] = {0, 0};
    const uint16_t stable[] = {0x1234, 0x1234};
    const uint16_t rollover[] = {0xffff, 0, 0};
    const uint16_t *vectors[] = {torn, changing, empty, stable, rollover};
    const unsigned lengths[] = {3, 5, 2, 2, 3};
    const uint16_t expected[] = {0x200, 0x103, 0, 0x1234, 0};
    const uint16_t regs[] = {Sn_TX_FSR, Sn_RX_RSR};
    for (unsigned r=0; r<2; r++)
        for (unsigned n=0; n<5; n++) check(regs[r], vectors[n], lengths[n], expected[n]);
}
"""
with tempfile.TemporaryDirectory(prefix='wiznet-counters-') as directory:
    for chip in ('W5100', 'W5200', 'W5500'):
        marker = '#ifdef CONFIG_NET_' + chip if chip == 'W5100' else '#elif defined(CONFIG_NET_' + chip + ')'
        section = text[text.index(marker):]
        regs = '\n'.join(re.search(r'^#define Sn_' + reg + r'\s+[^\n]+', section, re.M).group(0)
                         for reg in ('TX_FSR', 'RX_RSR'))
        src = Path(directory) / (chip + '.c')
        exe = Path(directory) / chip
        src.write_text(regs + '\n' + source)
        subprocess.run(['cc', '-std=c99', '-Wall', '-Wextra', '-Werror',
                        '-fsanitize=address,undefined', str(src), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
        print('PASS', chip + ': TX/RX torn, changing, empty, stable and rollover counters')
