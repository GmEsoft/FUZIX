# libc: copy only realloc payload bytes

The stored block count includes the allocator header; subtract that header before copying from the user pointer.

Run `python3 Library/tests/libc-realloc-copy/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
