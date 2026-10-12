# libc: count the primary group inside the group limit

Start with one populated entry, append before incrementing, and stop scanning each group after finding the requested user.

Run `python3 Library/tests/libc-initgroups-bound/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
