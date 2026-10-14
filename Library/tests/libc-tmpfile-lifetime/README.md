# libc: use a writable and unlinked tmpfile template

Create a fresh template on every call, unlink the temporary file immediately, and close its descriptor if unlink or fdopen fails.

Run `python3 Library/tests/libc-tmpfile-lifetime/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
