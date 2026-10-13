# mbr: preserve minor numbers for empty primary slots

An empty primary slot compresses subsequent partitions into earlier minors, contradicting the documented primary partition numbering.

Run `python3 Kernel/tests/dev-mbr-primary-slots/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
