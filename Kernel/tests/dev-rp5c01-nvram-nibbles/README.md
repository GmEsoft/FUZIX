# rtc: preserve both RP5C01 NVRAM nibbles

Operator precedence stores the low nibble in both banks, turning 0xA5 into 0x55. Invalid-address reads also return without a value.

Run `python3 Kernel/tests/dev-rp5c01-nvram-nibbles/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
