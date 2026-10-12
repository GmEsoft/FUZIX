# TTY vmin-vtime regression checks

From the repository root:

```sh
python3 Kernel/tests/tty-vmin-vtime/check.py
```

Requires Python 3 and a host C compiler with ASan/UBSan support. The runner
extracts actual production function bodies and uses the production `tty.h`
and `O_NDELAY` definition. It creates and removes a temporary build directory.

Checks all four VMIN/VTIME combinations, timer insertion after the first byte, expiry returning a short read, a satisfied minimum, a buffer smaller than VMIN, and canonical input. Sleep and timer expiry are deterministic stubs; scheduler races and elapsed time are not tested.

To check an unfixed checkout, pass `--kernel-dir /path/to/checkout/Kernel`.
The runner exits nonzero on compilation failure or any failed assertion.
