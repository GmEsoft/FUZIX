#!/usr/bin/env python3
"""Run focused host checks against actual kernel function bodies and constants."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

CASES = ['shrink', 'grow', 'truncate-failure', 'readonly-shrink', 'readonly-grow', 'unchanged', 'large-retained-count', 'invalid-range', 'copy-fault']
VARIANTS = [('512', ['-DTEST_BLOCK=512']), ('400', ['-DTEST_BLOCK=400'])]


def function(text, name):
    matches = list(re.finditer(r'(?m)^[\w *\t]+\b' + re.escape(name) +
                              r'\s*\([^;{}]*\)\s*\{', text))
    match = matches[0]
    masked = re.sub(r'/\*[\s\S]*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                    lambda m: ' ' * len(m.group()), text)
    depth = 0
    for position in range(text.index('{', match.start()), len(text)):
        if masked[position] == '{':
            depth += 1
        elif masked[position] == '}':
            depth -= 1
            if not depth:
                return text[match.start():position + 1] + '\n'
    raise ValueError('unterminated function: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kernel-dir', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--cc', default=os.environ.get('CC', 'cc'))
    args = parser.parse_args()
    kernel = args.kernel_dir.resolve()
    headers = '\n'.join((kernel/'include'/name).read_text() for name in ['kernel.h'])
    fixture = Path(__file__).with_name('check.c').read_text()
    def define(match):
        name = match.group(1)
        value = re.search(r'(?m)^#define\s+' + re.escape(name) + r'(?:\s|\()[^\n]*', headers)
        if value is None:
            raise ValueError('missing kernel constant: ' + name)
        return '#undef ' + name + '\n' + value.group()
    fixture = re.sub(r'/\* DEFINE (\w+) \*/', define, fixture)
    failed = False
    with tempfile.TemporaryDirectory(prefix='fuzix-core-test-') as directory:
        directory = Path(directory)
        for label, flags in VARIANTS:
            def body(match):
                file, name, variant = match.groups()
                return function((kernel/file).read_text(), name)
            source = re.sub(r'/\* FUNCTION ([\w./-]+) (\w+) (\w+) \*/', body, fixture)
            cfile = directory/(label+'.c')
            binary = directory/label
            cfile.write_text(source)
            build = subprocess.run([args.cc,'-std=c99','-g','-Wall','-Wextra',
                                    '-Wno-unused-parameter','-Wno-unused-variable','-Wno-sign-compare',
                                    '-fsanitize=address,undefined',*flags,
                                    '-I'+str(kernel/'include'),str(cfile),'-o',str(binary)])
            if build.returncode:
                failed = True
                print('COMPILE FAIL ' + label, flush=True)
                continue
            for case in CASES:
                result = subprocess.run([str(binary),case],timeout=5,
                                        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
                passed = result.returncode == 0
                print(('PASS ' if passed else 'FAIL ') + label + ' ' + case,flush=True)
                failed |= not passed
    return int(failed)


if __name__ == '__main__':
    raise SystemExit(main())
