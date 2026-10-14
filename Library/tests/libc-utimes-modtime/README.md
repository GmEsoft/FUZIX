# libc: use the modification timeval in utimes

Pass times[1].tv_sec to utime instead of the access timestamp's microsecond field.

Run `python3 Library/tests/libc-utimes-modtime/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
