# net: store native bind addresses in the local socket address

Both bind and autobind store the local address in dst_addr. Local port lookup and the network daemon consume src_addr, so bound sockets retain the wrong local endpoint.

Run `python3 Kernel/tests/dev-native-bind-source/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
