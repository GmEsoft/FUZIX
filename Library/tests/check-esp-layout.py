#!/usr/bin/env python3
"""Check ESP executable sizes and the loader's contiguous data/BSS layout.

Run with GNU arm-none-eabi binutils on PATH, or set BINUTILS_PREFIX to a
different GNU ELF32 little-endian toolchain prefix (including its path).
The fixtures contain only bytes; no target instructions are executed.
Optional arguments select linker scripts, including older versions.
"""
import os
import pathlib
import struct
import subprocess
import sys
import tempfile

prefix = os.environ.get("BINUTILS_PREFIX", "arm-none-eabi")
library = pathlib.Path(__file__).resolve().parents[1]
scripts = [pathlib.Path(p).resolve() for p in sys.argv[1:]] or [
    library / "elf2esp32.ld", library / "elf2esp8266.ld"]


def link(tmp, script, text=4, data=1, bss=4):
    source = f"""
.text
.global _start, __sighandler
_start:
__sighandler:
.space {text}
.data
.space {data}, 42
.bss
.space {bss}
"""
    (tmp / "probe.s").write_text(source)
    subprocess.run([prefix + "-as", "-o", str(tmp / "probe.o"),
                    str(tmp / "probe.s")], check=True)
    return subprocess.run([prefix + "-ld", "-T", str(script),
                           "-o", str(tmp / "probe"), str(tmp / "probe.o")],
                          capture_output=True, text=True)


def sections(elf):
    assert elf[:6] == b"\x7fELF\x01\x01", "expected little-endian ELF32"
    header = struct.unpack_from("<16sHHIIIIIHHHHHH", elf)
    shoff, size, count, names_index = header[6], header[11], header[12], header[13]
    entries = [struct.unpack_from("<10I", elf, shoff + i * size)
               for i in range(count)]
    names_section = entries[names_index]
    names = elf[names_section[4]:names_section[4] + names_section[5]]
    return {names[s[0]:].split(b"\0", 1)[0].decode(): s for s in entries}


with tempfile.TemporaryDirectory(prefix="fuzix-esp-layout-") as directory:
    tmp = pathlib.Path(directory)
    for script in scripts:
        for data_size in range(1, 5):
            result = link(tmp, script, data=data_size)
            assert result.returncode == 0, result.stderr
            elf = (tmp / "probe").read_bytes()
            named = sections(elf)
            text, data, bss = [named[name] for name in (".text", ".data", ".bss")]
            text_size, header_data, header_bss = struct.unpack_from(
                "<HHH", elf, text[4] + 6)
            assert text_size == text[5], "text size differs from header"
            assert header_data == data[5], "data size differs from header"
            assert header_bss == bss[5], "BSS size differs from header"
            assert bss[3] == data[3] + header_data, "loader clears before BSS"
            assert elf[data[4]:data[4] + data_size] == bytes([42]) * data_size
            assert header_data % 4 == 0, "data must include alignment padding"
        print(f"{script.name}: contiguous data/BSS for data sizes 1 through 4")

        # ESP32's 128 KiB regions exceed the header's 16-bit size fields.
        if script.name == "elf2esp32.ld":
            for field, valid, invalid in [("text", 65512, 65516),
                                           ("data", 65532, 65536),
                                           ("bss", 65535, 65536)]:
                result = link(tmp, script, **{field: valid})
                assert result.returncode == 0, result.stderr
                result = link(tmp, script, **{field: invalid})
                assert result.returncode != 0, f"oversized {field} was accepted"
                assert f"{field} section exceeds" in result.stderr, result.stderr
            print(f"{script.name}: rejects overflow of each 16-bit size field")
