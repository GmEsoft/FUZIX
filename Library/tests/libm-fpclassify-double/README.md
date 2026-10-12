# libm: extract the binary64 exponent in fpclassify

Take bits 20 through 30 from the high word so normal values, infinities, and NaNs are classified correctly.

Run `python3 Library/tests/libm-fpclassify-double/check.py` from the repository root.
The check uses the actual library source; host syscall adapters supply controlled outcomes.
