Run `python3 check.py` with a host C compiler supporting AddressSanitizer and UndefinedBehaviorSanitizer. It compiles the real picoctl and picogpio programs with mocked open/ioctl/close calls and the real userspace GPIO ABI header. The 31 cases cover command/argument validation, GPIO direction-mask preservation, SET/CLR bit masks, pin-group selection, and failure handling at each ioctl stage.

The GPIO driver itself is exercised by `../gpio` with SDK primitives mocked. Physical pin voltages and the complete flash image are not tested here.
