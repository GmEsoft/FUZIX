#!/bin/sh
#
#	Build a flashable ESP8266 kernel + root filesystem that contains the
#	valaddr hardware test (tests/valaddr/check).
#
#	Run this on a tree with the esp8266 fix branches applied.  The
#	esp8266 libc must already be built (make TARGET=esp8266 apps, or at
#	least the Library/libs part).
#
#	Everything is written under Kernel/platform/platform-esp8266/.
#
set -e

here=$(cd "$(dirname "$0")" && pwd)
pdir=$(cd "$here/.." && pwd)
root=${FUZIX_ROOT:-$(cd "$pdir/../../.." && pwd)}

echo "FUZIX_ROOT = $root"

# --- host tools -------------------------------------------------------
if [ ! -x "$root/Standalone/ucp" ] || [ ! -x "$root/Standalone/mkftl" ]; then
	echo "building host tools..."
	make -C "$root" TARGET=esp8266 stand
fi

# --- esp8266 libc -----------------------------------------------------
if [ ! -f "$root/Library/libs/crt0_esp8266.o" ] || \
   [ ! -f "$root/Library/libs/libcesp8266.a" ]; then
	echo "ERROR: esp8266 libc not built." >&2
	echo "       Run: make TARGET=esp8266 apps   (from $root)" >&2
	exit 1
fi

# --- kernel -----------------------------------------------------------
echo "building kernel..."
make -C "$root" TARGET=esp8266 kernel

# --- test program -----------------------------------------------------
echo "building valaddr test..."
make -C "$here/valaddr" FUZIX_ROOT="$root"

# --- root filesystem, with the test added -----------------------------
echo "building root filesystem..."
( cd "$pdir" && ./update-flash.sh )

printf 'cd /bin\nbget %s\nchmod 0755 check\n' "$here/valaddr/check" \
	| "$root/Standalone/ucp" "$pdir/filesystem.img"

# --- FTL + esp8266 flash images ---------------------------------------
echo "creating flash images..."
make -C "$pdir" filesystem.ftl filesystem.elf
esptool elf2image "$pdir/filesystem.elf"
esptool elf2image "$pdir/image.elf"

cat <<EOF

Kernel      : $pdir/image.elf-0x00000.bin
              $pdir/image.elf-0x10000.bin
Filesystem  : $pdir/filesystem.elf-0x20000.bin

Flash both (NodeMCU / ESP-12, /dev/ttyUSB0):

  esptool --port /dev/ttyUSB0 write_flash \\
      0x00000 $pdir/image.elf-0x00000.bin \\
      0x10000 $pdir/image.elf-0x10000.bin \\
      0x20000 $pdir/filesystem.elf-0x20000.bin \\
      -fs 4MB -ff 80m -fm dio

Then open the serial port at 115200 baud and run:  check
EOF
