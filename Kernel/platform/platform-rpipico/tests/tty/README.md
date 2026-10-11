Run `make check` with a host C compiler supporting ASan/UBSan. The tests compile
the actual main.c parameter parser, devtty.c and core1.c with the production
termios ABI and SDK/kernel fixtures. Nine cases cover a full five-device map,
invalid numbers/suffixes, remapping and masks, full/disconnected USB queues,
writer wakeups (including aliases), nonblocking diagnostics and API bounds.
Only the parameter parser is retained from main.c; exception/syscall/startup
code is discarded by the linker. The fixture captures the core1 entry point
and runs bounded iterations of the actual USB loop with mocked TinyUSB calls.

Normal process output sleeps when a USB queue is blocked and wakes on the
core0 TTY poll after space becomes available. Interrupt echo and kernel USB
diagnostics are best-effort and drop bytes rather than wait for a host. UART
kernel output keeps its existing behavior. USB enumeration, multicore timing
and sustained throughput still need hardware testing.

Remove main.o/tty-test before changing PLATFORM_DIR to another checkout. The
unchanged e35ad7c sources fail the mapping, backpressure and bounds regressions.

The irq-echo case extracts the actual common tty_echo/tty_putc_wait/output
functions from Kernel/tty.c. It tests a full and disconnected USB queue with
u_ininterrupt set; any wait/schedule attempt aborts. Echo is best effort when
the queue is full, and pending output must remain intact.
