Run `make check` with a host C compiler supporting ASan/UBSan. The tests compile
the actual rawuart.c and devtty.c, use the actual kernel termios definitions,
and mock the SDK UART/GPIO/time calls. Five cases check UART1's CTS/RTS pins,
draining the final shift-register byte before changing settings, avoiding UART
reset during normal reconfiguration, parity/stop-bit settings, B0 hangup and
restart, and the advertised CSTOPB mask. Each case runs in a fresh process.

To compare another checkout, remove uart-test and set PLATFORM_DIR to its
platform-rpipico directory. The unchanged e35ad7c sources fail all five cases.
These are driver sequencing tests; electrical UART timing/flow control still
requires a board and serial connection.
