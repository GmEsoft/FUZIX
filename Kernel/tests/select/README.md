# select regression checks

Run `python3 Kernel/tests/select/check.py` from the repository root. Requires
Python 3 and a host C compiler with ASan/UBSan. Tests use actual production
function bodies and inode/select-map declarations from current kernel headers.

Eleven cases run with 16, 32, and 64 process slots: every descriptor bit,
bounds, failed-descriptor cleanup, requested wait masks, interruption, pipe
repoll/EOF, timeouts, all process-map bits with adjacent data protected, wakeups,
and cached-inode reuse. Queue/device/scheduler interfaces are deterministic
stubs. Extra select storage costs three maps per in-core inode when LEVEL_2
is enabled; on-disk inode layout is unchanged.

Use `--kernel-dir /path/to/unmodified/Kernel` for negative controls. The new
optional pipesel_test helper is absent on old sources and is omitted there.
These are functional host checks, not scheduler-race or hardware tests.
