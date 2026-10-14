# libc: discard cached capacity after environ changes

Spare slots belong to the array allocated by putenv; reset the spare-slot count when setenv or the caller replaces environ.

Run `python3 Library/tests/libc-putenv-capacity/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
