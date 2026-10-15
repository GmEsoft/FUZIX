# i2c: clock data and acknowledgements correctly in bitbang transfers

Transmit shifts an uninitialized variable rather than the byte, ACK sampling keeps SDA driven, and receive writes the destination buffer back to the device. Receive NACK leaks the bus claim; successful helpers return positive counts that devi2c treats as errors.

Run `python3 Kernel/tests/dev-i2c-bitbang-transactions/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
