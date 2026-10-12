# waitpid regression checks

From the repository root:

```sh
python3 Kernel/tests/waitpid/check.py
```

Requires Python 3 and a C compiler with ASan/UBSan. Tests compile extracted
production function bodies, resolve constants from current kernel headers,
and mock the surrounding kernel and device interfaces. Temporary builds are
removed automatically. Each case runs in a fresh process.

Check unmodified sources with `--kernel-dir /path/to/checkout/Kernel`.
These are host functional checks, not full-kernel or hardware tests.

The 18 cases cover waitpid return/status behavior, PID and group selection,
and setpgid range validation. Group IDs 1 and 32767 are accepted; 32768, 65535
and -1 are rejected with EINVAL without changing process groups. A zero group
argument still selects the target PID. Existing cross-session checks and
waiting for group 32767 are also covered. Pointer-sized fixture syscall slots
hold host addresses; selector tests use values from the 16-bit pid_t ABI.
