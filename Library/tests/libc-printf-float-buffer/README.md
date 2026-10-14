# libc: size the floating printf scratch buffer for its range

Keep the integer-only buffer small, but reserve enough libm scratch space for fixed-format finite values across the supported binary32 and binary64 exponent ranges.

Run `python3 Library/tests/libc-printf-float-buffer/check.py` from the repository root.
Add `--binary32` to check the single-precision target profile with host varargs adapted to its floating-point width.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
