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
	@set -e; for test in parse invalid remap full disconnect wakeup aliases diagnostic bounds; do ./tty-test $$test; done
main.o: $(PLATFORM_DIR)/main.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude -I$(KERNEL_DIR)/cpu-armm0 -include include/kernel.h -Dmain=unused_platform_main -c $< -o $@
tty-test: check.c main.o $(PLATFORM_DIR)/devtty.c $(PLATFORM_DIR)/core1.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude -include include/kernel.h check.c main.o $(PLATFORM_DIR)/devtty.c $(PLATFORM_DIR)/core1.c $(LDFLAGS) -o $@
clean:
	rm -f tty-test main.o
	rm -rf tty-test.dSYM
