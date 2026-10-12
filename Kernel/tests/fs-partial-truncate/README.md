# Partial truncation regression checks

Run `python3 Kernel/tests/fs-partial-truncate/check.py` from the repository root.
Requires Python 3 and a host C compiler with ASan/UBSan. The fixture extracts
production freeblk and f_trunc_blocks, mocks disk buffers and the free list,
and tests inline/external buffers with 400/512-byte block geometry and TRIM.

Eight cases per variant cover partial single/double trees, both indirect
boundaries, full/repeated truncation, large retained counts, and read-only
inodes. Every freed block must be trimmed exactly once; retained blocks and
pointer tables must survive, and freed child pointers must be cleared.

Use `--kernel-dir /path/to/unmodified/Kernel` for negative controls. Builds
use temporary directories; these do not test power-loss or writeback ordering.
