# libm: retain all 64 double representation bits

Use uint64_t rather than unsigned long for the double representation so 32-bit-long targets preserve exponent and sign bits.

Run `python3 Library/tests/libm-double-bits/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
