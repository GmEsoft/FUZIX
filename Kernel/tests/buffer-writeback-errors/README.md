# buffer writeback error-reporting checks

Run `python3 Kernel/tests/buffer-writeback-errors/check.py` from the repository
root. Requires Python 3 and a C compiler with ASan/UBSan. The runner compiles
actual `bfree`, `bdput` and `freebuf` bodies with mocked device operations.

The 13 cases cover failed, short and complete immediate/sync/eviction writes,
delayed and clean buffers, release after failed writes, and the absence of an
automatic sync retry. A flush stub that clears errno checks that the write
failure is still reported. The write stub deliberately returns a count without
setting errno so the caller's result check is exercised.

Use `--kernel-dir /path/Kernel` to check another checkout. These are host checks,
not hardware tests. Buffer recycling is preserved; failed data is not retained
for recovery. Driver retries/remapping and taking a failed filesystem offline
remain outside this change.
