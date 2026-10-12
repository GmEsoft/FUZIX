#!/usr/bin/env python3
"""Compile production TTY function bodies against bounded host fixtures."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

FUNCTIONS = ['parity', 'tty_add_parity', 'tty_inproc_softparity']
CASES = ['generation', 'valid-input', 'invalid-input', 'ignore-errors', 'flag-field', 'disabled']
CHAR_MODES = ['signed-char']


def function(source, name):
    pattern = r'(?m)^[\w *\t]+\b' + re.escape(name) + r'\s*\([^;{}]*\)\s*\{'
    match = re.search(pattern, source)
    if match is None:
        raise ValueError('missing function: ' + name)
    masked = re.sub(r'/\*[\s\S]*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                    lambda m: ' ' * len(m.group()), source)
    depth = 0
    for position in range(source.index('{', match.start()), len(source)):
        if masked[position] == '{':
            depth += 1
        elif masked[position] == '}':
            depth -= 1
            if depth == 0:
                return source[match.start():position + 1] + '\n'
    raise ValueError('unterminated function: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kernel-dir', type=Path,
                        default=Path(__file__).resolve().parents[2])
    parser.add_argument('--cc', default=os.environ.get('CC', 'cc'))
    args = parser.parse_args()
    kernel = args.kernel_dir.resolve()
    source = (kernel / 'tty.c').read_text()
    bodies = ''.join(function(source, name) for name in FUNCTIONS)
    if 'parity' in FUNCTIONS:
        table = re.search(r'static uint8_t paritytab\[16\] = \{[\s\S]*?\};', source)
        if table is None:
            raise ValueError('missing parity table')
        bodies = table.group() + '\n' + bodies
    # Use the real file flag definition without pulling in a target config.
    flag = re.search(r'(?m)^#define\s+O_NDELAY\s+[^\n]+',
                     (kernel / 'include/kernel.h').read_text())
    if flag is None:
        raise ValueError('missing O_NDELAY')
    bodies = flag.group() + '\n' + bodies
    fixture = Path(__file__).with_name('check.c').read_text()
    assert fixture.count('/* KERNEL_FUNCTIONS */') == 1
    fixture = fixture.replace('/* KERNEL_FUNCTIONS */', bodies)
    failed = False
    with tempfile.TemporaryDirectory(prefix='fuzix-tty-test-') as directory:
        directory = Path(directory)
        generated = directory / 'check.c'
        generated.write_text(fixture)
        for char_mode in CHAR_MODES:
            binary = directory / char_mode
            subprocess.run([args.cc, '-std=c99', '-g', '-Wall', '-Wextra',
                            '-Werror', '-Wno-unused-parameter',
                            '-fsanitize=address,undefined', '-f' + char_mode,
                            '-I' + str(kernel / 'include'), str(generated),
                            '-o', str(binary)], check=True)
            for case in CASES:
                result = subprocess.run([str(binary), case], timeout=5,
                                        env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
                passed = result.returncode == 0
                print(('PASS' if passed else 'FAIL') + ' ' + char_mode + ' ' + case,
                      flush=True)
                failed |= not passed
    return int(failed)


if __name__ == '__main__':
    raise SystemExit(main())
