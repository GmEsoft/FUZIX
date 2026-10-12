# libc: bound parsed group member lists

Keep space for the NULL terminator and reject records exceeding GR_MAX_MEMBERS rather than writing beyond the member array.

Run `python3 Library/tests/libc-group-members/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
