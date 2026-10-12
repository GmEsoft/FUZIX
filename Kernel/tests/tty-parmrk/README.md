# TTY parmrk regression checks

From the repository root:

```sh
python3 Kernel/tests/tty-parmrk/check.py
```

Requires Python 3 and a host C compiler with ASan/UBSan support. The runner
extracts actual production function bodies and uses the production `tty.h`
and `O_NDELAY` definition. It creates and removes a temporary build directory.

Checks all 256 input bytes with every PARMRK/ISTRIP combination under both signed and unsigned plain char. The input stub applies the production ISTRIP transformation and records queued bytes. Only literal-0xff escaping is covered; parity-error/break marker handling and hardware are outside this test.

To check an unfixed checkout, pass `--kernel-dir /path/to/checkout/Kernel`.
Add `--allow-warnings` when checking the original character-literal comparison;
otherwise its signed-char compiler warning stops the test before execution.
The runner exits nonzero on compilation failure or any failed assertion.
