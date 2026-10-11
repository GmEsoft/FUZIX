Pico CPU regression tests
========================

This firmware tests the production tricks.S, syscall_handler, deliver_signals
and doexec on a real RP2040. It does not require an SD card or filesystem. It
seeds r4-r12, exercises a context switch and fork, and checks register/return
values; then exercises SVC and a user signal callback, checks their entry stack
alignment and argument frame, and checks u_insys on syscall return and exec.
Only scheduler, process-copy and syscall dispatch operations are stubbed. The
assembly under test gets a separate swapper stack so it cannot overwrite the
SDK main frame. The test wrappers preserve the SDK caller's registers too.

Requirements: Pico SDK with TinyUSB, arm-none-eabi toolchain, CMake, a host C
compiler, picotool and a Pico. Loading the UF2 replaces the firmware in flash;
use a spare board or save any firmware you need before loading this test.

From the FUZIX repository root, generate the usual kernel header:

    make -C Kernel -f ../version.mk -f Makefile TARGET=rpipico include/sysinfoblk.h

Configure and build, with PICO_SDK_PATH and the cross-toolchain PATH set:

    cmake -S Kernel/platform/platform-rpipico/tests/hardware -B /tmp/pico-abi -DPICO_BOARD=pico
    cmake --build /tmp/pico-abi

Put the board in BOOTSEL and load /tmp/pico-abi/rpipico_abi_test.uf2 with
picotool, or copy the UF2 to its bootloader drive. For picotool:

    picotool load /tmp/pico-abi/rpipico_abi_test.uf2 -x

Open its USB serial port and send `t`, or use the POSIX Python runner:

    python3 Kernel/platform/platform-rpipico/tests/hardware/run.py /dev/cu.usbmodem...

Success prints `ABI RESULT: PASS (0 failures)`; the runner returns nonzero for
failures/timeouts. Sending `b` returns to BOOTSEL for the next firmware load:

    python3 Kernel/platform/platform-rpipico/tests/hardware/run.py /dev/cu.usbmodem... --bootsel

For a regression comparison, pass -DPLATFORM_DIR=/path/to/other/checkout/Kernel/platform/platform-rpipico
when configuring a separate build directory. The tests compile that checkout's
actual platform/CPU sources against these same fixtures. Generate that
checkout's sysinfoblk.h too. The unmodified e35ad7c sources fail r4 preservation,
SVC/signal stack alignment and the two u_insys checks.
