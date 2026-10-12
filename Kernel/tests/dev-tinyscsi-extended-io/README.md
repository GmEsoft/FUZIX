# scsi: issue READ10 and WRITE10 for extended block transfers

The high-LBA path returns success without sending any command. Its read opcode is WRITE10, so simply adding the missing command call would also corrupt data.

Run `python3 Kernel/tests/dev-tinyscsi-extended-io/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
