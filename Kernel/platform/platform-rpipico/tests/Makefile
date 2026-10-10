CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined
CASES = break realloc-failure alloc-failure bounds grow-shrink

.PHONY: check clean
check: pagemap-test
	@set -e; for test in $(CASES); do ./pagemap-test $$test; done

pagemap-test: pagemap.c ../swapper.c $(wildcard include/*.h)
	$(CC) $(CFLAGS) -Iinclude pagemap.c ../swapper.c -o $@

clean:
	rm -f pagemap-test
	rm -rf pagemap-test.dSYM
