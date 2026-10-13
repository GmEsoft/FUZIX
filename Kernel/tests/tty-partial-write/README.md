# TTY partial-write regression checks

From the repository root:

```sh
python3 Kernel/tests/tty-partial-write/check.py
```

Requires Python 3 and a host C compiler with ASan/UBSan. The runner extracts the
actual `tty_write`, `tty_putc_maywait` and `psleep_flags_io` bodies and uses the
production `tty.h` and `O_NDELAY` definition. Temporary builds are removed.

Thirty runs cover TTY levels 1 and 2: EAGAIN/EINTR with and without progress,
full writes, ONLCR expansion, carrier loss, flow-control waits, and simulated
job-control EIO/EINTR with and without progress. The level-2 job-control fixture
models the helper's return/error interface; it does not test session policy.

Completed input bytes must survive any subsequent error. No-progress errors
return -1 and retain their error, including when a helper marks `u_done` with
`(usize_t)-1`. A partially emitted CR/LF sequence is not a completed input byte.
These are host checks, not hardware or platform syscall-glue tests.

Use `--kernel-dir /path/to/checkout/Kernel` to check another checkout. The runner
exits nonzero on compilation failure or any failed assertion.
