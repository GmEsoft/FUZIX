#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

typedef uintptr_t uarg_t;
typedef struct cinode *inoptr;
typedef uint16_t usize_t;
struct s_queue { unsigned q_count; };
#define NUM_DEV_TTY 1
#include "tty.h"

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    exit(1); \
} } while (0)
struct tty ttydata[2];
struct s_queue ttyinq[2];

static unsigned good, bad, received;
int tty_inproc_full(uint_fast8_t minor, uint_fast8_t c)
{
    good++; received = c; return 1;
}
int tty_inproc_bad(uint_fast8_t minor, uint_fast8_t c)
{
    bad++; received = c; return 1;
}

/* KERNEL_FUNCTIONS */

static unsigned expected_bit(unsigned c, unsigned mode)
{
    unsigned i, bit = 0;
    for (i = 0; i < 7; i++)
        bit ^= (c >> i) & 1;
    if (mode == 1)
        bit ^= 1;
    else if (mode == 2)
        bit = 0;
    else if (mode == 3)
        bit = 1;
    return bit << 7;
}
static void reset(unsigned mode, unsigned iflags)
{
    memset(ttydata, 0, sizeof(ttydata));
    ttydata[1].termios.c_cflag = PARENB | CS7;
    if (mode & 1)
        ttydata[1].termios.c_cflag |= PARODD;
    if (mode & 2)
        ttydata[1].termios.c_cflag |= CMSPAR;
    ttydata[1].termios.c_iflag = iflags;
    good = bad = received = 0;
}
int main(int argc, char **argv)
{
    unsigned mode, c, wire;
    CHECK(argc == 2);
    if (!strcmp(argv[1], "generation")) {
        for (mode = 0; mode < 4; mode++) {
            reset(mode, 0);
            for (c = 0; c < 128; c++) {
                wire = c | expected_bit(c, mode);
                CHECK(tty_add_parity(1, c) == wire);
                CHECK(tty_add_parity(1, c | 0x80) == wire);
            }
        }
    } else if (!strcmp(argv[1], "valid-input")) {
        for (mode = 0; mode < 4; mode++) {
            for (c = 0; c < 128; c++) {
                reset(mode, IGNPAR);
                wire = c | expected_bit(c, mode);
                CHECK(tty_inproc_softparity(1, wire) == 1);
                CHECK(good == 1 && !bad && received == c);
            }
        }
    } else if (!strcmp(argv[1], "invalid-input")) {
        for (mode = 0; mode < 4; mode++) {
            for (c = 0; c < 128; c++) {
                reset(mode, 0);
                wire = (c | expected_bit(c, mode)) ^ 0x80;
                CHECK(tty_inproc_softparity(1, wire) == 1);
                CHECK(!good && bad == 1 && received == c);
            }
        }
    } else if (!strcmp(argv[1], "ignore-errors")) {
        for (mode = 0; mode < 4; mode++) {
            for (c = 0; c < 128; c++) {
                reset(mode, IGNPAR);
                wire = (c | expected_bit(c, mode)) ^ 0x80;
                CHECK(tty_inproc_softparity(1, wire) == 0);
                CHECK(!good && !bad);
            }
        }
    } else if (!strcmp(argv[1], "flag-field")) {
        reset(0, 0);
        ttydata[1].termios.c_cflag |= IGNPAR;
        CHECK(tty_inproc_softparity(1, 0x80) == 1);
        CHECK(!good && bad == 1 && received == 0);
    } else if (!strcmp(argv[1], "disabled")) {
        for (c = 0; c < 256; c++) {
            reset(0, 0);
            ttydata[1].termios.c_cflag = 0;
            CHECK(tty_add_parity(1, c) == c);
            CHECK(tty_inproc_softparity(1, c) == 1);
            CHECK(good == 1 && !bad && received == c);
        }
    } else {
        CHECK(0);
    }
    return 0;
}
