#!/usr/bin/env python3
"""Link an ARM libgcc unwind section and check the FUZIX ELF load layout.

Run with arm-none-eabi-gcc/ld on PATH. No Pico SDK or built libc is needed.
An optional argument selects a linker script, e.g. to test an older version.
"""
import pathlib
import struct
import subprocess
import sys
import tempfile

script = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else (
    pathlib.Path(__file__).resolve().parents[1] / "elfexe32.ld")

source = """
volatile long long numerator = 100, denominator = 3;
volatile unsigned char scratch[8192];
void __sighandler(void) {}
int raise(int sig) { return sig; }
void _start(void) { scratch[0] = numerator / denominator; }
"""

with tempfile.TemporaryDirectory(prefix="fuzix-elf32-") as tmp:
    tmp = pathlib.Path(tmp)
    (tmp / "probe.c").write_text(source)
    cc = ["arm-none-eabi-gcc", "-mcpu=cortex-m0plus", "-mthumb"]
    libgcc = subprocess.check_output(cc + ["-print-libgcc-file-name"], text=True).strip()
    subprocess.run(cc + ["-Os", "-ffunction-sections", "-fdata-sections", "-c",
                        str(tmp / "probe.c"), "-o", str(tmp / "probe.o")], check=True)
    subprocess.run(["arm-none-eabi-ld", "-pie", "-static", "--no-dynamic-linker",
                    "-z", "max-page-size=4", "-T", str(script),
                    str(tmp / "probe.o"), libgcc, "-o", str(tmp / "probe")], check=True)
    elf = (tmp / "probe").read_bytes()

assert elf[:6] == b"\x7fELF\x01\x01", "expected little-endian ELF32"
header = struct.unpack_from("<16sHHIIIIIHHHHHH", elf)
phoff, shoff = header[5:7]
phentsize, phnum, shentsize, shnum, shstrndx = header[9:14]
programs = [struct.unpack_from("<8I", elf, phoff + i * phentsize) for i in range(phnum)]
loads = sorted((p for p in programs if p[0] == 1), key=lambda p: p[2])
assert len(loads) == 3, "expected text, data and relocation-info loads"
for left, right in zip(loads, loads[1:]):
    assert left[2] + left[5] <= right[2], "PT_LOAD memory ranges overlap"

sections = [struct.unpack_from("<10I", elf, shoff + i * shentsize) for i in range(shnum)]
names_section = sections[shstrndx]
names = elf[names_section[4]:names_section[4] + names_section[5]]
named = {names[s[0]:].split(b"\0", 1)[0].decode(): s for s in sections}
unwind = named[".ARM.exidx"]
assert unwind[5] > 0, "fixture must exercise a libgcc unwind section"
assert loads[0][2] <= unwind[3] < loads[0][2] + loads[0][5]
assert unwind[3] + unwind[5] <= named[".data"][3]
bss = named[".bss"]
dynamic = next(p for p in programs if p[0] == 2)
assert dynamic[2] == bss[3] + bss[5], "loader expects dynamic area immediately after BSS"
assert loads[0][5] == loads[0][4], "text must not incorporate the BSS gap"
assert named[".dynstr"][3] >= loads[2][2]
print("ARM ELF: disjoint loads, unwind in text, dynamic area immediately after BSS")
