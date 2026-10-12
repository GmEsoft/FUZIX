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

struct process { uint16_t p_timeout; } process;
struct {
    uint8_t *u_base;
    usize_t u_count, u_done;
    unsigned u_error, u_sysio;
    struct process *u_ptab;
} udata;
static uint8_t queue[8], buffer[8];
static unsigned head, tail, timers, sleeps, mode;
static unsigned expected_initial_timers, expected_later_timers;

static void enqueue(uint8_t c)
{
    CHECK(tail < sizeof(queue));
    queue[tail++] = c;
    ttyinq[1].q_count++;
}
int remq(struct s_queue *q, uint_fast8_t *c)
{
    if (head == tail)
        return 0;
    *c = queue[head++];
    q->q_count--;
    return 1;
}
void tty_data_consumed(uint_fast8_t minor) {}
void wakeup(void *p) {}
void uputc(uint_fast8_t c, void *p) { *(uint8_t *)p = c; }
void ptimer_insert(void) { timers++; }
int psleep_flags_io(void *p, uint_fast8_t flags)
{
    CHECK(++sleeps <= 3);
    if (sleeps == 1) {
        CHECK(timers == expected_initial_timers);
        if (mode == 1) {
            enqueue('a');
            return 0;
        }
    }
    CHECK(timers == expected_later_timers);
    if (mode == 2 || (mode == 1 && sleeps == 2 &&
                      ttydata[1].termios.c_cc[VTIME])) {
        CHECK(process.p_timeout == 2);
        process.p_timeout = 1;
        return 0;
    }
    return -1;
}

/* KERNEL_FUNCTIONS */

static void reset(unsigned minimum, unsigned time)
{
    memset(&udata, 0, sizeof(udata));
    memset(ttydata, 0, sizeof(ttydata));
    memset(ttyinq, 0, sizeof(ttyinq));
    memset(&process, 0, sizeof(process));
    memset(buffer, 0, sizeof(buffer));
    head = tail = timers = sleeps = mode = 0;
    expected_initial_timers = expected_later_timers = 0;
    udata.u_base = buffer;
    udata.u_count = sizeof(buffer);
    udata.u_sysio = 1;
    udata.u_ptab = &process;
    ttydata[1].termios.c_cc[VMIN] = minimum;
    ttydata[1].termios.c_cc[VTIME] = time;
}
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "first-byte")) {
        reset(2, 1);
        mode = 1;
        expected_later_timers = 1;
        CHECK(tty_read(1, 1, 0) == 1);
        CHECK(timers == 1 && sleeps == 2 && buffer[0] == 'a');
    } else if (!strcmp(argv[1], "read-timeout")) {
        reset(0, 1);
        mode = 2;
        expected_initial_timers = expected_later_timers = 1;
        CHECK(tty_read(1, 1, 0) == 0);
        CHECK(timers == 1 && sleeps == 1);
    } else if (!strcmp(argv[1], "poll")) {
        reset(0, 0);
        CHECK(tty_read(1, 1, 0) == 0);
        CHECK(!timers && !sleeps);
    } else if (!strcmp(argv[1], "minimum-only")) {
        reset(2, 0);
        mode = 1;
        CHECK(tty_read(1, 1, 0) == 1);
        CHECK(!timers && sleeps == 2);
    } else if (!strcmp(argv[1], "minimum-satisfied")) {
        reset(2, 1);
        enqueue('a'); enqueue('b');
        CHECK(tty_read(1, 1, 0) == 2);
        CHECK(!timers && !sleeps && !memcmp(buffer, "ab", 2));
    } else if (!strcmp(argv[1], "count-below-minimum")) {
        reset(2, 1);
        udata.u_count = 1;
        enqueue('a');
        CHECK(tty_read(1, 1, 0) == 1);
        CHECK(!timers && !sleeps);
    } else if (!strcmp(argv[1], "canonical")) {
        reset(2, 1);
        ttydata[1].termios.c_lflag = ICANON;
        enqueue('a'); enqueue('\n');
        CHECK(tty_read(1, 1, 0) == 2);
        CHECK(!timers && !sleeps);
    } else {
        CHECK(0);
    }
    return 0;
}
