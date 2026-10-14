# libc: reserve the terminating byte in fgets

Read at most count minus one bytes, and handle zero and one-byte buffers without consuming input or overrunning the destination.

Run `python3 Library/tests/libc-fgets-bound/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
