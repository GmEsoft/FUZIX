Run `python3 check.py` with a host C compiler supporting ASan/UBSan. The tests
compile the real utilities and replace only open/ioctl/close with fixtures,
using the production GPIO ABI header. Invalid commands/numeric arguments must
fail without touching a device. Valid calls, device errors and close cleanup
are checked. GPIO values beyond the physical board's range are passed to the
mock driver; these are parser tests rather than GPIO hardware tests.

To compare another checkout, set PLATFORM_DIR to its platform-rpipico directory.
The original e35ad7c sources fail the unknown picoctl command and empty numeric
argument checks (older sources also need unistd.h for strict host compilation).
