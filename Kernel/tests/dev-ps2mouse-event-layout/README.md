# input: preserve PS/2 buttons and both movement axes

The X delta overwrites buttons, Y occupies the X field, and wheel data overwrites the Y field in the four-byte input ABI. Extra button bits are also taken from the wrong PS/2 bits.

Run `python3 Kernel/tests/dev-ps2mouse-event-layout/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
