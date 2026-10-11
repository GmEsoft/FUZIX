#!/usr/bin/env python3
"""Compile the production generic echo/output path into the IRQ queue fixture."""
from pathlib import Path
import re
import sys
kernel = Path(sys.argv[1]).resolve()
output = Path(sys.argv[2])
text = (kernel / 'tty.c').read_text()
def function(name):
    start = re.search(r'^(?:void|uint_fast8_t)\s+' + name + r'\s*\([^;]*\)\n\{', text, re.M).start()
    end = text.index('\n}', start) + 2
    return text[start:end]
ndelay = re.search(r'^#define O_NDELAY[^\n]*',
                  (kernel.parent / 'Library/include/fcntl.h').read_text(), re.M).group(0)
source = r"""
#include <kernel.h>
#include <tty.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
typedef int irqflags_t;
/* Any attempt to wait or schedule during IRQ echo is a test failure. */
int chksigs(void) { abort(); }
int need_reschedule(void) { abort(); }
void switchout(void) { abort(); }
int di(void) { abort(); }
void irqrestore(int irq) { (void)irq; abort(); }
void psleep(void *channel) { (void)channel; abort(); }
""" + ndelay + '\n'
source += '\n'.join(function(name) for name in ('tty_putc_maywait', 'tty_putc_wait', 'tty_echo')) + '\n'
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(source)
