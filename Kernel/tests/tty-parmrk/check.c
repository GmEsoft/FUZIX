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

static uint8_t queue[4];
static unsigned count;
int insq(struct s_queue *q, uint_fast8_t c)
{
    CHECK(count < sizeof(queue));
    queue[count++] = c;
    return 1;
}
uint_fast8_t tty_inproc(uint_fast8_t minor, uint_fast8_t c)
{
    if (ttydata[minor].termios.c_iflag & ISTRIP)
        c &= 0x7F;
    return insq(ttyinq + minor, c);
}

/* KERNEL_FUNCTIONS */

int main(int argc, char **argv)
{
    unsigned c, flags = 0;
    CHECK(argc == 2);
    if (!strcmp(argv[1], "marked"))
        flags = PARMRK;
    else if (!strcmp(argv[1], "stripped"))
        flags = ISTRIP;
    else if (!strcmp(argv[1], "marked-stripped"))
        flags = PARMRK | ISTRIP;
    else
        CHECK(!strcmp(argv[1], "unmarked"));
    ttydata[1].termios.c_iflag = flags;
    for (c = 0; c < 256; c++) {
        unsigned expected = (flags & ISTRIP) ? (c & 0x7F) : c;
        count = 0;
        CHECK(tty_inproc_full(1, c) == 1);
        if (c == 255 && flags == PARMRK) {
            CHECK(count == 2 && queue[0] == 255 && queue[1] == 255);
        } else {
            CHECK(count == 1 && queue[0] == expected);
        }
    }
    return 0;
}
