#!/usr/bin/env python3
"""Exercise generated geometry, the real Dhara capacity and linker guard."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

PLATFORM = Path(__file__).resolve().parents[1]
ROOT = PLATFORM.parents[2]
spec = importlib.util.spec_from_file_location("flash_layout", PLATFORM / "flash-layout.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class FlashLayoutTests(unittest.TestCase):
    def test_default_and_reserved_regions(self):
        for xip in (0, 256):
            values = module.layout(2 * 1024 * 1024, 96, xip)
            with tempfile.TemporaryDirectory() as tmp:
                out = Path(tmp)
                module.generate(values, out)
                self.assertEqual(json.loads((out / "flash-layout.json").read_text()), values)
                header = (out / "flash-layout.h").read_text()
                self.assertIn("#define FUZIX_FLASH_FTL_OFFSET " + str((96 + xip) * 1024) + "u", header)
                env = subprocess.check_output(["sh", "-c", '. "$1"; printf "%s %s" "$FTL_SIZE_KIB" "$FTL_ADDRESS"',
                                               "sh", str(out / "flash-layout.env")], text=True)
                self.assertEqual(env, str(2048 - 96 - xip) + " " + hex(0x10000000 + (96 + xip) * 1024))

    def test_layout_identity_changes_with_geometry(self):
        self.assertNotEqual(module.layout(2097152, 96, 0)["layout_id"],
                            module.layout(2097152, 96, 256)["layout_id"])

    def test_invalid_geometry(self):
        for args in ((0, 96, 0), (2097152, 0, 0), (2097152, 97, 0),
                     (2097152, 96, -4), (2097152, 96, 3),
                     (2097152, 96, 1952), (2097152, 2048, 4), (33554432, 96, 0)):
            with self.subTest(args=args), self.assertRaises(ValueError):
                module.layout(*args)

    def test_actual_dhara_capacity_and_image_bounds(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            dhara = ROOT / "Kernel/lib/dhara"
            executable = tmp / "mkftl"
            subprocess.run([os.environ.get("HOSTCC", "cc"), "-O2",
                            str(ROOT / "Standalone/mkftl.c"), str(dhara / "journal.c"),
                            str(dhara / "map.c"), str(dhara / "error.c"), "-o", str(executable)], check=True)
            for xip in (0, 256):
                values = module.layout(2097152, 96, xip)
                geometry = ["-s", str(values["ftl_bytes"] // 1024), "-e", "4096", "-g", "10"]
                capacity = int(subprocess.check_output([str(executable), "-c"] + geometry))
                self.assertEqual(capacity, 2555 if xip == 0 else 2211)
                image = tmp / "filesystem.img"
                image.write_bytes(bytes((capacity * 9 // 10) * 512))
                result = subprocess.run([str(executable)] + geometry + ["-o", str(tmp / "filesystem.ftl"), str(image)], capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual((tmp / "filesystem.ftl").stat().st_size, values["ftl_bytes"])
                image.write_bytes(bytes((capacity + 1) * 512))
                result = subprocess.run([str(executable)] + geometry + ["-o", str(tmp / "oversize.ftl"), str(image)], capture_output=True)
                self.assertNotEqual(result.returncode, 0)
                self.assertFalse((tmp / "oversize.ftl").exists())

    @unittest.skipUnless(shutil.which("arm-none-eabi-ld"), "ARM linker is not on PATH")
    def test_firmware_overlap_is_a_link_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            module.generate(module.layout(2097152, 96, 256), tmp)
            subprocess.run(["arm-none-eabi-as", "-o", str(tmp / "empty.o"), "-"], input=b"", check=True)
            for extent, expected in ((0x10018000, 0), (0x10018001, 1)):
                result = subprocess.run(["arm-none-eabi-ld", "-T", str(tmp / "flash-layout.ld"),
                                         "--defsym=__flash_binary_end=" + hex(extent),
                                         str(tmp / "empty.o"), "-o", str(tmp / "probe")], capture_output=True)
                self.assertEqual(bool(result.returncode), bool(expected), result.stderr)


if __name__ == "__main__":
    unittest.main()
