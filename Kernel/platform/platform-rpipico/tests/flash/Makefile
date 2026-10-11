CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined
PLATFORM_DIR ?= ../..
KERNEL_DIR = $(PLATFORM_DIR)/../..
DHARA = $(KERNEL_DIR)/lib/dhara
.PHONY: check clean
check: flash-test
	@set -e; for test in restart trim failure; do ./flash-test $$test; done
flash-test: generated/flash-layout.h check.c $(PLATFORM_DIR)/devflash.c $(DHARA)/map.c $(DHARA)/journal.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude -Igenerated -I$(KERNEL_DIR) -include include/kernel.h check.c $(PLATFORM_DIR)/devflash.c $(DHARA)/map.c $(DHARA)/journal.c -o $@
generated/flash-layout.h: $(PLATFORM_DIR)/flash-layout.py
	python3 $< --flash-bytes 2097152 --firmware-kib 96 --xip-kib 0 --output generated
clean:
	rm -f flash-test
	rm -rf flash-test.dSYM generated
