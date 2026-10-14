# libc: return the unslept duration after interruption

Subtract normalized elapsed time from the original request, clamp late wakeups to zero, and leave the remainder untouched for absolute sleeps.

Run `python3 Library/tests/libc-nanosleep-remainder/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
