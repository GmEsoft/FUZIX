# i2c: propagate PCF8584 ACK and final-byte failures

Final-byte timeouts are copied into the receive buffer and reported as success because only -1 is tested; zero-length receive still writes one byte. Transmit ignores slave NACKs.

Run `python3 Kernel/tests/dev-pcf8584-errors/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
