# gpt: compare the fixed signature and stop on failed reads

The eight-byte GPT signature has no terminating NUL; strcmp compares bytes from the revision field as part of the signature. Failed header/table reads also parse stale data.

Run `python3 Kernel/tests/dev-gpt-header-io/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
