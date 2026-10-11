#!/usr/bin/env python3
"""Decode each UF2 block independently and verify offline provisioning."""
from pathlib import Path
import runpy
import struct
import unittest

platform = Path(__file__).resolve().parents[1]
layout = runpy.run_path(str(platform / "flash-layout.py"))["layout"]
bundle = runpy.run_path(str(platform / "boot-bundle.py"))["bundle"]


class BootBundleTests(unittest.TestCase):
    def test_decode_complete_flash(self):
        geometry = layout(2097152, 96, 256)
        firmware = bytes(range(256)) * 300 + b"end"
        filesystem = b"\xaa" * geometry["ftl_bytes"]
        uf2 = bundle(geometry, firmware, filesystem)
        recovered = bytearray()
        blocks = len(uf2) // 512
        self.assertEqual(blocks, geometry["flash_bytes"] // 256)
        for i in range(blocks):
            block = uf2[i * 512:(i + 1) * 512]
            magic0, magic1, flags, address, size, index, count, family = struct.unpack_from("<8I", block)
            self.assertEqual((magic0, magic1, flags, size, index, count, family),
                             (0x0A324655, 0x9E5D5157, 0x2000, 256, i, blocks, 0xE48BFF56))
            self.assertEqual(address, 0x10000000 + i * 256)
            self.assertEqual(struct.unpack_from("<I", block, 508)[0], 0x0AB16F30)
            recovered.extend(block[32:32 + size])
        self.assertEqual(recovered[:len(firmware)], firmware)
        self.assertEqual(recovered[len(firmware):geometry["ftl_offset"]],
                         b"\xff" * (geometry["ftl_offset"] - len(firmware)))
        self.assertEqual(recovered[geometry["ftl_offset"]:], filesystem)

    def test_reject_mismatched_images(self):
        geometry = layout(2097152, 96, 256)
        ftl = bytes(geometry["ftl_bytes"])
        for firmware, filesystem in ((b"", ftl), (bytes(98305), ftl),
                                     (b"ok", ftl[:-1]), (b"ok", bytes(1952 * 1024))):
            with self.assertRaises(ValueError):
                bundle(geometry, firmware, filesystem)
        geometry["ftl_offset"] -= 4096
        with self.assertRaises(ValueError):
            bundle(geometry, b"ok", ftl)


if __name__ == "__main__":
    unittest.main()
