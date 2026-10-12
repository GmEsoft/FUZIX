# tty-canonical-eof regression checks

From the repository root:

```sh
python3 Kernel/tests/tty-canonical-eof/check.py
```

Requires Python 3 and a C compiler with ASan/UBSan. Tests compile extracted
production function bodies, resolve constants from current kernel headers,
and mock the surrounding kernel and device interfaces. Temporary builds are
removed automatically. Each case runs in a fresh process.

Check unmodified sources with `--kernel-dir /path/to/checkout/Kernel`.
These are host functional checks, not full-kernel or hardware tests.
