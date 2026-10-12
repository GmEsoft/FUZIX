# gpt: bound UTF-16 partition names to their field

A valid full-length GPT name need not have a NUL terminator. Printing scans beyond its 72-byte field and potentially beyond the temporary sector buffer.

Run `python3 Kernel/tests/dev-gpt-name-bounds/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
