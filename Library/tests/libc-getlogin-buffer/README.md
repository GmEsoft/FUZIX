# libc: look up the login using the local tty buffer

Pass the tty buffer and its true capacity to ttyname_r, then copy at most UT_NAMESIZE username bytes and report an undersized result buffer.

Run `python3 Library/tests/libc-getlogin-buffer/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
