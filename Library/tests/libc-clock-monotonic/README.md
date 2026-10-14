# libc: convert monotonic deciseconds accurately

Correct the division-by-ten correction and calculate the remainder from the original counter so tv_nsec remains normalized.

Run `python3 Library/tests/libc-clock-monotonic/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
