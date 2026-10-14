# input: complete PS/2 reset before negotiating mouse modes

Reset leaves SET SAMPLE RATE waiting for its parameter, so the next command is consumed as a rate value. The five-button negotiation uses 200/100/100 instead of its documented 200/200/80 sequence.

Run `python3 Kernel/tests/dev-ps2mouse-init-sequence/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
