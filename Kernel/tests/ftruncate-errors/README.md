# ftruncate error-handling regression checks

Run `python3 Kernel/tests/ftruncate-errors/check.py` from the repository root.
Requires Python 3 and a C compiler with ASan/UBSan. The runner executes the actual
`_ftruncate` body with inode and truncation interfaces mocked. Nine cases run for
both 512- and 400-byte geometry, covering error propagation, read-only shrink and
growth, successful changes, unchanged size, copy faults, and block-count limits.

Use `--kernel-dir /path/Kernel` to check another checkout. These are host checks,
not full-kernel or hardware tests. Partial-tail zeroing is outside this change.
