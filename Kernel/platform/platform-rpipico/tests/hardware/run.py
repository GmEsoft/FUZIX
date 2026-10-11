#!/usr/bin/env python3
"""Run the board ABI test or request BOOTSEL. Uses POSIX serial APIs only."""
import argparse
import os
import select
import termios
import time
import tty

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('port', help='Pico serial port, e.g. /dev/cu.usbmodem...')
parser.add_argument('--bootsel', action='store_true', help='request the test firmware bootloader')
args = parser.parse_args()
fd = os.open(args.port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
try:
    tty.setraw(fd)
    attrs = termios.tcgetattr(fd)
    attrs[4] = attrs[5] = termios.B115200
    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    time.sleep(0.3)
    os.write(fd, b'b' if args.bootsel else b't')
    if args.bootsel:
        termios.tcdrain(fd)
        time.sleep(0.2)
        raise SystemExit(0)
    result = b''
    deadline = time.monotonic() + 15
    while time.monotonic() < deadline:
        if select.select([fd], [], [], 0.3)[0]:
            chunk = os.read(fd, 4096)
            if not chunk:
                break
            result += chunk
            if b'ABI RESULT:' in result and b'\n' in result[result.index(b'ABI RESULT:'):]:
                break
    print(result.decode(errors='replace'), end='')
    if b'ABI RESULT: PASS (0 failures)' not in result:
        raise SystemExit('Board ABI test failed or timed out')
finally:
    os.close(fd)
