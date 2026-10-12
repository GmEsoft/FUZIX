# libc: unlink the correct FILE from the open-stream list

Track the predecessor while searching __IO_list so closing an older stream preserves newer streams for exit-time flushing.

Run `python3 Library/tests/libc-fclose-list/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
