#!/usr/bin/env python3
"""Compile and run the real Pico utilities with mocked device operations."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

here = Path(__file__).resolve().parent
platform = Path(os.environ.get('PLATFORM_DIR', here / '../..')).resolve()
cc = shlex.split(os.environ.get('CC', 'cc'))
flags = ['-std=c99', '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined',
         '-I' + str(here / 'include')]
count = 0
with tempfile.TemporaryDirectory(prefix='pico-utilities-') as directory:
    build = Path(directory)
    for name in ['picoctl', 'picogpio']:
        subprocess.run(cc + flags + ['-Dopen=fixture_open', '-Dioctl=fixture_ioctl',
                       '-Dclose=fixture_close', '-c', str(platform / 'utils' / (name + '.c')),
                       '-o', str(build / (name + '.o'))], check=True)
        subprocess.run(cc + flags + [str(build / (name + '.o')), str(here / 'mock.c'),
                       '-o', str(build / name)], check=True)

    def run(name, args, success, calls, failure=None, gpio=None):
        global count
        env = dict(os.environ)
        env.pop('FAIL_OPEN', None)
        env.pop('FAIL_IOCTL', None)
        if failure:
            env[failure] = '1'
        result = subprocess.run([str(build / name)] + args, env=env,
                                capture_output=True, text=True)
        assert (result.returncode == 0) == success, (name, args, result.stderr)
        actual = [line for line in result.stdout.splitlines()
                  if line in ['TEST OPEN', 'TEST IOCTL', 'TEST CLOSE']]
        assert actual == ['TEST ' + call for call in calls], (name, args, actual)
        if gpio:
            assert 'TEST GPIO ' + gpio in result.stdout, result.stdout
        count += 1

    for args in [[], ['--help']]:
        run('picoctl', args, True, [])
    for args in [['status'], ['flahs'], [''], ['flash', 'extra'], ['--help', 'flash']]:
        run('picoctl', args, False, [])
    run('picoctl', ['flash'], True, ['OPEN', 'IOCTL', 'CLOSE'])
    run('picoctl', ['flash'], False, ['OPEN'], 'FAIL_OPEN')
    run('picoctl', ['flash'], False, ['OPEN', 'IOCTL', 'CLOSE'], 'FAIL_IOCTL')
    for args in [[], ['--help']]:
        run('picogpio', args, True, [])
    for args in [['', '1'], ['0', ''], ['-1', '1'], ['256', '1'], ['1x', '1'],
                 ['1', '256'], ['1', '-1'], ['999999999999999999999', '1'], ['1']]:
        run('picogpio', args, False, [])
    for args in [['0', '0'], ['28', '1'], ['255', '255']]:
        run('picogpio', args, True, ['OPEN', 'IOCTL', 'CLOSE'], gpio=' '.join(args))
    run('picogpio', ['1', '1'], False, ['OPEN'], 'FAIL_OPEN')
    run('picogpio', ['1', '1'], False, ['OPEN', 'IOCTL', 'CLOSE'], 'FAIL_IOCTL')
print(f'{count} utility cases passed')
