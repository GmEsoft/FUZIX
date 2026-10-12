# TTY partial-write regression checks

From the repository root:

```sh
python3 Kernel/tests/tty-partial-write/check.py
```

Requires Python 3 and a host C compiler with ASan/UBSan support. The runner
extracts actual production function bodies and uses the production `tty.h`
and `O_NDELAY` definition. It creates and removes a temporary build directory.

Checks EAGAIN and EINTR with and without completed bytes, full writes, ONLCR expansion and carrier loss. No-progress errors must return -1 and retain their error; partial EAGAIN/EINTR must return the completed count without an error. A partially emitted CR/LF sequence is not counted as a completed input byte. This does not test hardware or platform syscall glue.

To check an unfixed checkout, pass `--kernel-dir /path/to/checkout/Kernel`.
The runner exits nonzero on compilation failure or any failed assertion.
