#!/usr/bin/env python3
"""Check real Makefile/SDK configuration across Pico board changes.

Run in a disposable checkout with PICO_SDK_PATH and the cross-toolchain PATH set.
This changes its normal platform build/ directory. It configures, not builds.
"""
import os
from pathlib import Path
import subprocess

platform = Path(os.environ.get('PLATFORM_DIR', Path(__file__).resolve().parent / '../..')).resolve()

def cache():
    entries = {}
    for line in (platform / 'build/CMakeCache.txt').read_text().splitlines():
        if ':' in line and '=' in line and not line.startswith('//'):
            key, value = line.split('=', 1)
            entries[key.split(':')[0]] = value
    return entries

def configure(args):
    subprocess.run(['make', '-C', str(platform), 'build/Makefile'] + args, check=True)
    return cache()

subprocess.run(['make', '-C', str(platform.parent.parent), '-f', '../version.mk',
                '-f', 'Makefile', 'TARGET=rpipico', 'version.c', 'include/sysinfoblk.h'], check=True)
settings = configure([])
assert settings['PICO_BOARD'] == 'pico'
assert settings['TOTALMEM'] == '160'
for board, processor, memory in [('pico2', 'rp2350-arm-s', '320'), ('pico_w', 'rp2040', '160'),
                                 ('pico2_w', 'rp2350-arm-s', '320'), ('pico', 'rp2040', '160')]:
    settings = configure(['SUBTARGET=' + board])
    assert settings['PICO_BOARD'] == board
    assert settings['PICO_PLATFORM'] == processor
    assert settings['TOTALMEM'] == memory
settings = configure(['TOTALMEM=144', 'PROCMEM=128'])
assert settings['TOTALMEM'] == '144' and settings['PROCMEM'] == '128'
print('Default board, board changes and memory overrides passed')
