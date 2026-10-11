CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -Werror -fsanitize=address,undefined
PLATFORM_DIR ?= ../..
.PHONY: check clean
check: gpio-test
	@set -e; for test in masks direction input info errors; do ./gpio-test $$test; done
gpio-test: check.c $(PLATFORM_DIR)/devgpio.c $(shell find include -type f)
	$(CC) $(CFLAGS) -Iinclude check.c $(PLATFORM_DIR)/devgpio.c -o $@
clean:
	rm -f gpio-test
	rm -rf gpio-test.dSYM
