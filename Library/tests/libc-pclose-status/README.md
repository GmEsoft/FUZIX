# libc: wait for the popen child and return its wait status

Associate each pipe stream with its child PID, waitpid that child through EINTR, and return the encoded status without consuming unrelated children.

Run `python3 Library/tests/libc-pclose-status/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
