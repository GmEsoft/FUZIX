#!/usr/bin/env python3
"""Image construction must not hide an earlier ucp command failure."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]


class ImageToolTests(unittest.TestCase):
    def test_ucp_stops_before_a_later_success(self):
        subprocess.run(["make", "-C", str(ROOT / "Standalone"), "mkfs", "ucp"],
                       check=True, stdout=subprocess.DEVNULL)
        with tempfile.TemporaryDirectory() as tmp:
            image = Path(tmp) / "filesystem.img"
            subprocess.run([str(ROOT / "Standalone/mkfs"), str(image), "8", "128"],
                           check=True, stdout=subprocess.DEVNULL)
            result = subprocess.run([str(ROOT / "Standalone/ucp"), "-e", str(image)],
                                    input="cd /missing\nmkdir marker\n", text=True, capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            listing = subprocess.check_output([str(ROOT / "Standalone/ucp"), str(image), "ls /"], text=True)
            self.assertNotIn("marker", listing)
            result = subprocess.run([str(ROOT / "Standalone/ucp"), "-e", str(image)],
                                    input="mkdir marker\nchmod 0755 marker\n", text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()
