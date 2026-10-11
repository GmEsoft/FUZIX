Pico process allocator regression tests
======================================

Run `make check` here with a host C compiler supporting AddressSanitizer and
UndefinedBehaviorSanitizer. The tests compile the actual platform `swapper.c`
against a small kernel fixture. No Pico SDK is required.

The fixture models a 160 KiB RAM pool, 1,536 bytes of process metadata and a
64, 128 and 160 KiB process limits. It exercises exact heap boundaries, failed allocation
and growth with only one block free, size limits/overflow, and content
preservation during growth and shrink. Failed requests are repeated to check
that capacity remains available. Before the fixes, the break, realloc-failure,
alloc-failure and bounds cases each fail an assertion.

The `check-config` target separately compiles assertions against the production
platform configuration, including its swap-slot geometry. Invalid or oversized
process limits must fail compilation.

Host addresses use uintptr_t so the real memory movement can run on 64-bit
hosts. This does not verify the ARM ABI or exception/context-switch assembly.
Swap tests use a mock disk, with the real udata alias at the beginning of RAM.
They check repeated slot reuse, first/partial read and write errors, flush
failure, exhausted swap space, cleanup after a failed swap-in, and alternating
128/160 KiB processes. The mock read modifies udata scratch state on each call,
so full-pool tests also detect premature reuse of physical block zero. Failed
swap-in must panic rather than resume a partial image; its disk slot is retained.

These tests verify allocator and I/O control flow, not physical SD behavior or
power-loss durability. To compare swap regressions with another checkout:

    make swap-test-128 PLATFORM_DIR=/path/to/checkout/Kernel/platform/platform-rpipico
    ./swap-test-128 reuse

Build in a clean test directory or remove swap-test-* first when changing the
source under test. The unmodified e35ad7c code fails slot reuse, I/O failure and
large-process checks. ARM ABI tests are in hardware/ and require a board.

Dhara flash checkpoint tests
---------------------------

`make check` also tests the production Pico and ESP8266 flash transfer/flush
callbacks against real Dhara under ASan/UBSan, using each platform's block count
and GC ratio. It reproduces loss of an uncheckpointed write, checks 32 durable
writes across restart, confirms reads do not checkpoint, and verifies flush
and checkpoint error handling. No platform SDK is required. This tests completed
flash operations; hardware power cuts and torn flash operations are not covered.

