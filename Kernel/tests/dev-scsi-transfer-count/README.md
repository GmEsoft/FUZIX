# scsi: bound READ6 transfers to the command and transport widths

The block count is copied into an eight-bit CDB and multiplied into a 16-bit byte length. Requests of 128+ sectors wrap the transport length, and 256 wraps both command and return count.

Run `python3 Kernel/tests/dev-scsi-transfer-count/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
