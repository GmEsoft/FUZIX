CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -Werror -Wno-unused-parameter -fsanitize=address,undefined
PLATFORM_DIR ?= ../..
.PHONY: check clean
check: timer-network-test
	python3 $(PLATFORM_DIR)/../../dev/net/tests/w5x00-counters.py
	@set -e; for test in phase missed race pending spi normal; do ./timer-network-test $$test; done
timer-network-test: generated/flash-layout.h check.c $(PLATFORM_DIR)/devices.c $(PLATFORM_DIR)/wiznet.c $(PLATFORM_DIR)/wiznet_spi.c $(PLATFORM_DIR)/devsdspi.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude -Igenerated -include include/kernel.h check.c $(PLATFORM_DIR)/devices.c $(PLATFORM_DIR)/wiznet.c $(PLATFORM_DIR)/wiznet_spi.c $(PLATFORM_DIR)/devsdspi.c -o $@
generated/flash-layout.h: $(PLATFORM_DIR)/flash-layout.py
	python3 $< --flash-bytes 2097152 --firmware-kib 96 --xip-kib 0 --output generated
clean:
	rm -f timer-network-test
	rm -rf timer-network-test.dSYM generated
