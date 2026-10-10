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
The block device is disabled: swap I/O and hardware execution require separate
target tests.
