# libc: reject overflowing calloc products

Check the element count against the maximum size_t before multiplication and report ENOMEM on overflow.

Run `python3 Library/tests/libc-calloc-overflow/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
