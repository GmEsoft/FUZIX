#!/usr/bin/env python3
"""Package matching RP2040 firmware and a fresh flash filesystem in one UF2.

UF2 format: https://github.com/microsoft/uf2
Family ID: Pico SDK boot/uf2.h (RP2040_FAMILY_ID).
This offline provisioning image replaces the entire configured flash.
"""
import argparse
import hashlib
import json
from pathlib import Path
import runpy
import struct

layout = runpy.run_path(str(Path(__file__).with_name("flash-layout.py")))["layout"]
RP2040_FAMILY = 0xE48BFF56


def bundle(geometry, firmware, filesystem):
    expected = layout(geometry["flash_bytes"], geometry["firmware_bytes"] // 1024,
                      geometry["xip_bytes"] // 1024)
    if geometry != expected:
        raise ValueError("manifest is not a valid generated flash layout")
    if not firmware or len(firmware) > geometry["firmware_bytes"]:
        raise ValueError("firmware is empty or exceeds its flash reservation")
    if len(filesystem) != geometry["ftl_bytes"]:
        raise ValueError("filesystem FTL image size does not match the layout")
    image = bytearray(b"\xff" * geometry["flash_bytes"])
    image[:len(firmware)] = firmware
    image[geometry["ftl_offset"]:] = filesystem
    blocks = len(image) // 256
    uf2 = bytearray()
    for index in range(blocks):
        uf2.extend(struct.pack("<8I", 0x0A324655, 0x9E5D5157, 0x2000,
                               geometry["flash_base"] + index * 256,
                               256, index, blocks, RP2040_FAMILY))
        uf2.extend(image[index * 256:(index + 1) * 256])
        uf2.extend(bytes(220))
        uf2.extend(struct.pack("<I", 0x0AB16F30))
    return bytes(uf2)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--layout", type=Path, required=True)
    parser.add_argument("--firmware", type=Path, required=True)
    parser.add_argument("--filesystem", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    geometry = json.loads(args.layout.read_text())
    firmware, filesystem = args.firmware.read_bytes(), args.filesystem.read_bytes()
    try:
        uf2 = bundle(geometry, firmware, filesystem)
    except (ValueError, KeyError) as error:
        parser.error(str(error))
    args.output.write_bytes(uf2)
    report = dict(layout=geometry, family="RP2040", replaces_entire_flash=True,
                  firmware_bytes=len(firmware), filesystem_bytes=len(filesystem),
                  firmware_sha256=hashlib.sha256(firmware).hexdigest(),
                  filesystem_sha256=hashlib.sha256(filesystem).hexdigest(),
                  uf2_sha256=hashlib.sha256(uf2).hexdigest())
    args.output.with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
