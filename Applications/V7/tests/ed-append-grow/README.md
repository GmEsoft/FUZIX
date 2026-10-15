# ed: grow the line pointer table without freeing it first

`append()` grows the buffer's line table with `realloc()` once it fills.
A `free()` before the `realloc()` turns every grow into a use-after-free,
so reading any file big enough to cross the initial 128-line table
aborts under AddressSanitizer ("attempting double-free").

Run `python3 Applications/V7/tests/ed-append-grow/check.py` from the
repository root. The check uses the actual `append()` source; host
stand-ins supply lines and line numbers.
