# libm: provide the force-evaluation helper

Define __force_eval independently of the macro that calls it and retain a volatile result so optimizers preserve evaluation.

Run `python3 Library/tests/libm-force-eval/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
