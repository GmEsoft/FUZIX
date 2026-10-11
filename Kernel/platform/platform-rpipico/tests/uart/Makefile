CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined
PLATFORM_DIR ?= ../..
.PHONY: check clean
check: uart-test
	@set -e; for test in pins drain settings hangup mask prompt; do ./uart-test $$test; done
uart-test: check.c $(PLATFORM_DIR)/rawuart.c $(PLATFORM_DIR)/devtty.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude -include include/kernel.h check.c $(PLATFORM_DIR)/rawuart.c $(PLATFORM_DIR)/devtty.c -o $@
clean:
	rm -f uart-test
	rm -rf uart-test.dSYM
