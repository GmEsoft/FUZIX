Run `make check` with a host compiler supporting ASan/UBSan. The five cases compile the actual Pico GPIO driver and shared kernel ioctl ABI with GPIO SDK primitives and user-memory copies mocked. They cover byte-mask SET/CLR/SETBYTE semantics, direction changes, input sampling, complete group metadata (including GP28/GP29), error propagation, and preservation of unselected peripheral pins/output latches.

GPIOC_SETRW selects SIO for requested outputs and releases existing SIO outputs to input. Peripheral pins whose bits are not requested as outputs retain their alternate function. SET/CLR change output latches without changing directions. The SDK's bank-zero pin count is used, including onboard GPIOs; board wiring still determines which pins are accessible.

`../utilities/check.py` separately tests the matching picogpio command. These are functional API tests, without electrical measurements on a board.
