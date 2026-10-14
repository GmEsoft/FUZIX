# libc: avoid consuming arguments for suppressed strings

Only fetch a destination for stored string, character, or scanset conversions so suppressed fields leave the next conversion's argument intact.

Run `python3 Library/tests/libc-scanf-suppression/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
