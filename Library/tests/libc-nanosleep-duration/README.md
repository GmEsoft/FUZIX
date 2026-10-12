# libc: round sleep durations up and split long waits

Validate timespecs, normalize absolute deadlines in nanoseconds, and split long durations into safe 16-bit kernel timeouts so positive requests neither vanish nor wrap.

Run `python3 Library/tests/libc-nanosleep-duration/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
