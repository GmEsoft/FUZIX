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

struct {
    uint8_t *u_base;
    usize_t u_count, u_done;
    unsigned u_error, u_sysio, u_ininterrupt;
} udata;
typedef int irqflags_t;
#define need_reschedule() 0
static uint8_t sent[8];
static unsigned sent_count, capacity, signal_pending, disconnect_after_byte;
static unsigned stop_after_byte, interrupt_on_sleep;
#ifdef CONFIG_LEVEL_2
static unsigned job_error, job_after_bytes;
uint8_t jobcontrol_out(uint8_t minor, struct tty *t)
{
    if (!job_error || udata.u_done < job_after_bytes)
        return 0;
    udata.u_error = job_error;
    if (!udata.u_done)
        udata.u_done = (usize_t)-1;
    return 1;
}
#endif
unsigned valaddr_r(const void *p, unsigned size) { return size; }
uint8_t _ugetc(const void *p) { return *(const uint8_t *)p; }
int chksigs(void) { return signal_pending; }
irqflags_t di(void) { return 0; }
void irqrestore(irqflags_t flags) {}
void psleep(void *p) { CHECK(interrupt_on_sleep); signal_pending = 1; }
void switchout(void) { CHECK(0); }
void tty_sleeping(uint_fast8_t minor) { CHECK(0); }
ttyready_t tty_writeready(uint_fast8_t minor)
{
    return sent_count < capacity ? TTY_READY_NOW : TTY_READY_LATER;
}
void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    CHECK(sent_count < sizeof(sent));
    sent[sent_count++] = c;
    if (disconnect_after_byte)
        ttydata[minor].flag |= TTYF_DEAD;
    if (stop_after_byte)
        ttydata[minor].flag |= TTYF_STOP;
}

/* KERNEL_FUNCTIONS */

int main(int argc, char **argv)
{
    uint8_t data[] = "ab";
    uint_fast8_t flags = O_NDELAY;
    CHECK(argc == 2);
    udata.u_base = data;
    udata.u_count = 2;
    capacity = 1;
    if (!strcmp(argv[1], "partial")) {
        CHECK(tty_write(1, 1, flags) == 1);
        CHECK(udata.u_done == 1 && !udata.u_error);
        CHECK(sent_count == 1 && sent[0] == 'a');
    } else if (!strcmp(argv[1], "empty")) {
        capacity = 0;
        CHECK(tty_write(1, 1, flags) == -1);
        CHECK(udata.u_error == EAGAIN && !sent_count && !udata.u_done);
    } else if (!strcmp(argv[1], "interrupted-partial")) {
        signal_pending = 1;
        CHECK(tty_write(1, 1, 0) == 1);
        CHECK(!udata.u_error && sent_count == 1 && sent[0] == 'a');
    } else if (!strcmp(argv[1], "interrupted-empty")) {
        signal_pending = 1;
        capacity = 0;
        CHECK(tty_write(1, 1, 0) == -1);
        CHECK(udata.u_error == EINTR && !sent_count && !udata.u_done);
    } else if (!strcmp(argv[1], "full")) {
        capacity = 2;
        CHECK(tty_write(1, 1, flags) == 2);
        CHECK(!udata.u_error && sent_count == 2 && !memcmp(sent, "ab", 2));
    } else if (!strcmp(argv[1], "onlcr-partial")) {
        data[1] = '\n';
        ttydata[1].termios.c_oflag = OPOST | ONLCR;
        capacity = 2;
        CHECK(tty_write(1, 1, flags) == 1);
        CHECK(!udata.u_error && sent_count == 2 && !memcmp(sent, "a\r", 2));
    } else if (!strcmp(argv[1], "onlcr-empty")) {
        data[0] = '\n';
        udata.u_count = 1;
        ttydata[1].termios.c_oflag = OPOST | ONLCR;
        CHECK(tty_write(1, 1, flags) == -1);
        CHECK(udata.u_error == EAGAIN && sent_count == 1 && sent[0] == '\r');
        CHECK(!udata.u_done);
    } else if (!strcmp(argv[1], "disconnect")) {
        disconnect_after_byte = 1;
        CHECK(tty_write(1, 1, flags) == 1);
        CHECK(!udata.u_error && sent_count == 1 && udata.u_done == 1);
    } else if (!strcmp(argv[1], "disconnect-empty")) {
        ttydata[1].flag = TTYF_DEAD;
        CHECK(tty_write(1, 1, flags) == -1);
        CHECK(udata.u_error == ENXIO && !sent_count && !udata.u_done);
    } else if (!strncmp(argv[1], "flow-", 5)) {
        unsigned partial = strstr(argv[1], "partial") != NULL;
        unsigned interrupted = strstr(argv[1], "interrupted") != NULL;
        if (partial)
            stop_after_byte = 1;
        else
            ttydata[1].flag = TTYF_STOP;
        interrupt_on_sleep = interrupted;
        CHECK(tty_write(1, 1, interrupted ? 0 : flags) == (partial ? 1 : -1));
        CHECK(sent_count == partial);
        if (partial)
            CHECK(udata.u_done == 1 && !udata.u_error);
        else
            CHECK(udata.u_done == (usize_t)-1 &&
                  udata.u_error == (interrupted ? EINTR : EAGAIN));
#ifdef CONFIG_LEVEL_2
    } else if (!strncmp(argv[1], "job-", 4)) {
        unsigned partial = strstr(argv[1], "partial") != NULL;
        job_error = strstr(argv[1], "eintr") ? EINTR : EIO;
        job_after_bytes = partial;
        capacity = 2;
        CHECK(tty_write(1, 1, flags) == (partial ? 1 : -1));
        CHECK(sent_count == partial);
        if (partial)
            CHECK(udata.u_done == 1 && !udata.u_error);
        else
            CHECK(udata.u_done == (usize_t)-1 && udata.u_error == job_error);
#endif
    } else {
        CHECK(0);
    }
    return 0;
}
