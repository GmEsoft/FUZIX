CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined
CASES = break realloc-failure alloc-failure bounds grow-shrink
LIMITS = 64 128 160
TESTS = $(addprefix pagemap-test-,$(LIMITS))

.PHONY: check check-config check-flash-sync check-esp8266-flash-sync check-kernel-flash clean
check: $(TESTS) check-config check-flash-sync check-esp8266-flash-sync check-kernel-flash
	@set -e; for exe in $(TESTS); do echo $$exe; for test in $(CASES); do ./$$exe $$test; done; done

pagemap-test-%: pagemap.c ../swapper.c $(wildcard include/*.h)
	$(CC) $(CFLAGS) -DTEST_PROCMEM='($* * 1024)' -Iinclude pagemap.c ../swapper.c -o $@

check-config:
	@set -e; for limit in $(LIMITS); do \
		$(CC) -std=c99 -fsyntax-only -DTOTALMEM=160 -DPROCMEM=$$limit config.c; \
	done; \
	for limit in 0 3 65 164; do \
		if $(CC) -std=c99 -fsyntax-only -DTOTALMEM=160 -DPROCMEM=$$limit config.c >/dev/null 2>&1; then \
			echo "invalid limit $$limit accepted"; exit 1; \
		fi; \
	done

clean:
	rm -f pagemap-test $(TESTS)
	rm -rf pagemap-test.dSYM $(addsuffix .dSYM,$(TESTS))

check-flash-sync:
	python3 flash-sync.py

check-esp8266-flash-sync:
	python3 flash-sync.py --platform esp8266

check-kernel-flash:
	python3 kernel-flash.py
