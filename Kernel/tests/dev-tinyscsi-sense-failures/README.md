# scsi: bound sense transfers and preserve failure status

REQUEST SENSE advertises 16 bytes but supplies a 36-byte transport length to a 16-byte stack buffer. A failed sense command is treated as successful data I/O.

Run `python3 Kernel/tests/dev-tinyscsi-sense-failures/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
