# libc: bound newline searches to bytes read

Use memchr on initialized bytes in password and group readers, including long-record skipping, so full or short reads cannot search beyond valid data.

Run `python3 Library/tests/libc-account-line-reads/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
