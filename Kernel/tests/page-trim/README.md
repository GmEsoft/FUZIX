# page-trim regression checks

Run `python3 Kernel/tests/page-trim/check.py` from the repository root.
Requires Python 3 and a C compiler with ASan/UBSan. The runner extracts the actual
`pageread` body and mocks device reads and discard requests. Six cases run with
16- and 32-bit unsigned byte counts, with TRIM enabled and disabled (24 runs).
They cover failed, short, complete, zero-length, 32-KiB and 65024-byte reads.

The 16-bit fixture models the signed driver return for large successful reads;
it does not compile or emulate an entire 8-bit target. Use `--kernel-dir /path/Kernel`
to check another checkout. These are host functional checks, not hardware tests.
