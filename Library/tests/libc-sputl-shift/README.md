# libc: shift out successive bytes in sputl

Advance through the 32-bit value by shifting right so all four bytes survive serialization.

Run `python3 Library/tests/libc-sputl-shift/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
