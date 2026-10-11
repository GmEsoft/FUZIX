Set PICO_SDK_PATH to an installed SDK with TinyUSB, then run `python3 check.py`
with a host compiler supporting ASan/UBSan. It compiles the actual descriptor
source against that SDK's TinyUSB macros for 1-4 CDC ports, parses the emitted
bytes and checks the advertised interface count, contiguous unique interface
numbers and total descriptor bounds. No fake descriptor macros are used.

Set PLATFORM_DIR to compare another checkout. The unchanged e35ad7c descriptor
advertises one extra interface and fails. This checks bytes, not enumeration
on a USB host.
