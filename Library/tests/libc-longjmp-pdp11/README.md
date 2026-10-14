# libc: preserve nonzero PDP11 longjmp values

Branch around the increment for nonzero values so zero becomes one while all other 16-bit values are preserved.

Run `python3 Library/tests/libc-longjmp-pdp11/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
