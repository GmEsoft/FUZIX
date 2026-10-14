# libc: preserve nonzero 8086 longjmp values

Branch around the increment for nonzero values so zero becomes one while all other 16-bit values are preserved.

Run `python3 Library/tests/libc-longjmp-8086/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
