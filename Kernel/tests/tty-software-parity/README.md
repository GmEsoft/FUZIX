# TTY software-parity regression checks

From the repository root:

```sh
python3 Kernel/tests/tty-software-parity/check.py
```

Requires Python 3 and a host C compiler with ASan/UBSan support. The runner
extracts actual production function bodies and uses the production `tty.h`
and `O_NDELAY` definition. It creates and removes a temporary build directory.

Checks all 128 seven-bit payloads in even, odd, space and mark modes, independently calculates expected parity, and flips the wire parity bit for invalid-input tests. Covers input payload stripping, IGNPAR in the input flag field, control-flag bit aliases, and all 256 bytes with parity disabled. The downstream good/bad-input helpers are stubs; marker encoding and hardware are outside this test.

To check an unfixed checkout, pass `--kernel-dir /path/to/checkout/Kernel`.
The runner exits nonzero on compilation failure or any failed assertion.
