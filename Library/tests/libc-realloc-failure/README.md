# libc: return NULL when realloc growth cannot succeed

Reject overflowing block counts and return NULL on allocation failure while retaining the original allocation.

Run `python3 Library/tests/libc-realloc-failure/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
