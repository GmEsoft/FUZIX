# gpt: count the inclusive final sector and reject invalid ranges

GPT ending LBAs are inclusive. Single-sector partitions become empty and every other partition loses its last sector. Reversed or high-word start LBAs also produce invalid ranges.

Run `python3 Kernel/tests/dev-gpt-partition-range/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
