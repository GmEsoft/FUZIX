# libm: retain all 32 float representation bits

Use uint32_t in the conversion union and public helper declaration, including targets with 16-bit unsigned int.

Run `python3 Library/tests/libm-float-bits/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
