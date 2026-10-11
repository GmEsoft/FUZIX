CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -fsanitize=address,undefined -ffunction-sections
PLATFORM_DIR ?= ../..
KERNEL_DIR = $(PLATFORM_DIR)/../..
ifeq ($(shell uname -s),Darwin)
LDFLAGS = -Wl,-dead_strip
else
LDFLAGS = -Wl,--gc-sections
endif
.PHONY: check clean
check: tty-test
	@set -e; for test in parse invalid remap full irq-echo disconnect wakeup aliases diagnostic bounds; do ./tty-test $$test; done
main.o: generated/flash-layout.h $(PLATFORM_DIR)/main.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude -Igenerated -I$(KERNEL_DIR)/cpu-armm0 -include include/kernel.h -Dmain=unused_platform_main -c $(PLATFORM_DIR)/main.c -o $@
tty-test: generated/tty-echo.c check.c main.o generated/tty-echo.c $(PLATFORM_DIR)/devtty.c $(PLATFORM_DIR)/core1.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude -Igenerated -include include/kernel.h check.c main.o generated/tty-echo.c $(PLATFORM_DIR)/devtty.c $(PLATFORM_DIR)/core1.c $(LDFLAGS) -o $@
generated/tty-echo.c: irq-echo.py $(KERNEL_DIR)/tty.c
	python3 irq-echo.py $(KERNEL_DIR) $@
generated/flash-layout.h: $(PLATFORM_DIR)/flash-layout.py
	python3 $< --flash-bytes 2097152 --firmware-kib 96 --xip-kib 0 --output generated
clean:
	rm -f tty-test main.o
	rm -rf tty-test.dSYM generated
