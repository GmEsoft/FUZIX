# ESP8266 hardware tests

These are tests to run on a real NodeMCU / ESP-12 class board.  They cover
the fixes made during the `Kernel/platform/platform-esp8266` review and a
couple of the open observations.

**Status: prepared but not yet run — no hardware was available.**  The
expected results below are derived from the code and a host simulation of
`valaddr()`, not from a real board.

## Contents

| Path | What it is |
|------|------------|
| `valaddr/check.c` | User-space test for the `valaddr()` bounds fix (issue F3) |
| `valaddr/Makefile` | Builds `check` as an esp8266 Fuzix binary |
| `make-test-image.sh` | Builds a kernel + root filesystem image containing `check` |

## Prerequisites

- `xtensa-lx106-elf` toolchain and `esptool`.
- The esp8266 libc built once:
  `make TARGET=esp8266 apps` from the top of the tree (this also builds
  the `Standalone/` host tools).
- **All the `fix/esp8266-*` branches applied.**  Without
  `fix/esp8266-misc-valaddr-semicolon` and
  `fix/esp8266-syscall-exec-node-lock` the kernel does not build at all,
  and the tests below assume `fix/esp8266-valaddr-bounds` is present.

### Userspace build gotchas (pre-existing, outside the kernel review)

- `make TARGET=esp8266 apps` finishes with **exit 2** even on success:
  `Applications/util/mktemp` fails to link because
  `Library/libs/Makefile.esp8266` omits `mkdtemps.c` (so the esp8266 libc
  has no `mkdtemp`), and the `size.report` target then fails.  Every
  binary `update-flash.sh` needs is built before that point, so the
  nonzero exit can be ignored — just do not rely on it.  `cursesgames`
  likewise wants a `libcursesesp8266` that is not built.
- If `Library/libs` has objects from another CPU (e.g. a previous
  rpipico build), the esp8266 libraries pick them up and the links fail
  with "skipping incompatible ...".  Remove the stale objects first:
  `rm -f Library/libs/*.o Library/libs/*.lib Library/libs/*.a`.

## Build a test image

```sh
Kernel/platform/platform-esp8266/tests/make-test-image.sh
```

This builds the kernel, builds `check`, builds the root filesystem with
`update-flash.sh`, adds `/bin/check`, and produces:

```
Kernel/platform/platform-esp8266/image.elf-0x00000.bin
Kernel/platform/platform-esp8266/image.elf-0x10000.bin
Kernel/platform/platform-esp8266/filesystem.elf-0x20000.bin
```

## Flash

Standard NodeMCU wiring (see the port README): 1 MB+ flash, `-fs 4MB`
matches the port Makefile.  Use `dio` (or `dout`/`qio` to taste).

```sh
esptool --port /dev/ttyUSB0 write_flash \
    0x00000 image.elf-0x00000.bin \
    0x10000 image.elf-0x10000.bin \
    0x20000 filesystem.elf-0x20000.bin \
    -fs 4MB -ff 80m -fm dio
```

Serial console is UART0 at 115200 baud, e.g.
`screen /dev/ttyUSB0 115200` or `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`.

## Test 1 — `valaddr()` bounds (issue F3)

```
$ check
```

Expected on the fixed kernel:

```
valaddr bounds test
DATABASE=0x3ffe8000 PROGLOAD=0x3ffe8600 DATATOP=0x3fff8000  stack~0x3ffe9xxx
ok    write p[1] buf 4                           ret=4   errno=0
ok    read  p[0] buf 4                           ret=4   errno=0
ok    write src=DATABASE (u_data)                ret=-1  errno=14
ok    write src=PROGLOAD-1 (last u_data byte)    ret=-1  errno=14
ok    write src=DATATOP-4 (above u_top)          ret=-1  errno=14
ok    read  dst=DATABASE (u_data)                ret=-1  errno=14
ok    read  dst=PROGLOAD-1 (last u_data byte)    ret=-1  errno=14
ok    read  dst=DATATOP-4 (above u_top)          ret=-1  errno=14
note  write src=0xfffffffc (wrap addr)           ret=-1  errno=14
note  write src=PROGLOAD size=0xffffff00         ret=-1  errno=14
---
8/8 checks passed (2 informational)
```

errno 14 is `EFAULT`.  Pass = `8/8` and exit status 0.

### Seeing the old behaviour (A/B)

Build and flash a kernel with **only** `fix/esp8266-misc-valaddr-semicolon`
and `fix/esp8266-syscall-exec-node-lock` (i.e. omit
`fix/esp8266-valaddr-bounds`).  On that kernel:

- the `DATABASE` / `PROGLOAD-1` cases *succeed* (the kernel happily reads
  and writes the process's own `u_data`), so they report `FAIL`;
- the `DATATOP-4` case succeeds (reads 4 bytes of another process / free
  RAM) instead of `EFAULT`;
- the `0xfffffffc` informational case takes a physical-address fault, so
  the process is killed (SIGSEGV) or the box panics.

Do not treat the A/B run as a pass; it only demonstrates the defect.

## Test 2 — boot smoke test and FTL capacity (observation O2)

Watch the boot messages.  The flash driver prints its capacity:

```
NAND flash, 1421kB: ...
```

That is `dhara_map_capacity()/2` using the **kernel's** GC ratio of 128.
`Standalone/mkftl -c -s 1984 -e 0x1000 -g 128` agrees (2842 sectors).

The filesystem, however, is built with `mkftl ... -g 10`, whose capacity
is 2598 sectors (1299 kB), and `mkfs` sizes the image to 2590 sectors:

```
$ cd Kernel/platform/platform-esp8266 && make filesystem.ftl
Number of physical erase blocks: 496
Maximum logical size: 1299kB (2598 sectors)
Used space: 1295kB (2590 sectors)
```

So kernel (1421 kB) and image tooling (1299 kB) disagree.  This is the
mismatch to confirm on hardware.  Also run:

```
$ df
$ free
$ ps
```

`df` should show the filesystem size (2590 sectors ≈ 1295 kB), `free`
should show ~96 kB of user memory, and `ps` should list the running
processes.

## Test 3 — flash persistence (FTL/dhara)

Confirms the NAND FTL really survives a reboot:

```
$ echo hello > /tmp/persist
$ sync
$ reboot
... wait for the shell ...
$ cat /tmp/persist
hello
```

Run it a few times.  (The root filesystem is the dhara FTL on the internal
flash, so this exercises `devflash.c`/`rawflash.c` including the
`dhara_map_sync` on write.)

## Test 4 — SD card and swap (optional)

Only if an SD card is wired up (pins in the port README):

```
$ df
$ swapon /dev/hdb1 4096
$ free
```

This exercises `devsdspi.c`.  There is no automated check; just confirm
`/dev/hdb` appears and swap is usable.

## Notes / gotchas

- `PROGLOAD` and friends in `check.c` are hard-coded for this port's fixed
  link layout (`config.h` / `kernel.ld`).  If that layout changes, update
  the constants.
- On a board with only 2 MB flash the filesystem image (FTL at 128 kB for
  1984 kB) runs past the end of the chip; use 4 MB (see observation O6).
- The `0xfffffffc` and `0xffffff00` cases are informational because a
  failing kernel may fault rather than return an errno.
