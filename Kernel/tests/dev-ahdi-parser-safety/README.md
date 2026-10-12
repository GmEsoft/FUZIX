# ahdi: bound partition scans and release temporary buffers

The primary scan examines eight entries in a four-entry array; XGM chains can write past MAX_PARTITIONS and leak a temporary buffer on failed reads. Overflowing sizes evade the sanity check, while a partition ending exactly at disk size is rejected.

Run `python3 Kernel/tests/dev-ahdi-parser-safety/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
