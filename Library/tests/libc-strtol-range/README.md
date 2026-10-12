# libc: accept LONG_MIN and clamp negative strtol overflow

Compare against the signed magnitude limit selected by the sign; LONG_MIN is representable without an ERANGE error.

Run `python3 Library/tests/libc-strtol-range/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
