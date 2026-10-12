# ppa: compile the driver and disconnect failed commands

The duplicate static specifier produces a compiler diagnostic when CONFIG_TD_PPA is enabled. Selection, command, and phase failures also leave the parallel-port adapter connected.

Run `python3 Kernel/tests/dev-ppa-build-cleanup/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
