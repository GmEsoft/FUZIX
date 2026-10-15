#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#define TEST(c,msg) do {if(!(c)){puts(msg);return 1;}}while(0)
#define BLKSIZE 512
/* DEFINE BF_FREE */
/* DEFINE BF_BUSY */
#include "panic.h"
struct { int u_error; } udata;
struct blkbuf {
    uint8_t data[BLKSIZE];
    uint16_t bf_dev, bf_blk, bf_time;
    uint8_t bf_dirty, bf_busy;
};
typedef struct blkbuf *bufptr;
static uint16_t bufclock;
static struct blkbuf bufpool[2];
static bufptr bufpool_end = bufpool + 2;
#define bisbusy(b) ((b)->bf_busy == BF_BUSY)
#define block(b) ((b)->bf_busy = BF_BUSY)
#define bunlock(b) ((b)->bf_busy = BF_FREE)
#define block_s(b) ((b)->bf_busy = BF_BUSY)
#define bunlock_s(b) ((b)->bf_busy = BF_FREE)
static int write_result;
static unsigned writes, flushes;
static bool flush_clears_error;
/* Deliberately return a count without setting errno: the caller checks it. */
int bdwrite(bufptr bp) { writes++; return write_result; }
int d_flush(unsigned dev)
{
    flushes++;
    if (flush_clears_error) udata.u_error = 0;
    return 0;
}
void panic(const char *reason) { abort(); }

/* FUNCTION devio.c bfree 0 */
/* FUNCTION devio.c bdput 0 */
/* FUNCTION devio.c freebuf 0 */

static int immediate(int result)
{
    struct blkbuf b = { .bf_dirty = 1, .bf_busy = BF_BUSY };
    write_result = result;
    TEST(bfree(&b, 2) == (result == BLKSIZE ? 0 : -1),
         "immediate write reports incomplete writes");
    TEST(udata.u_error == (result == BLKSIZE ? 0 : EIO) && writes == 1 &&
         !b.bf_dirty && b.bf_busy == BF_FREE,
         "immediate write reports errors and releases the buffer");
    return 0;
}

static int sync_write(int result)
{
    struct blkbuf b = { .bf_dirty = 1 };
    write_result = result;
    bdput(&b);
    TEST(udata.u_error == (result == BLKSIZE ? 0 : EIO) && writes == 1 &&
         flushes == 1 && !b.bf_dirty && b.bf_busy == BF_FREE,
         "sync reports incomplete writes and releases the buffer");
    return 0;
}

static int no_repeat_sync(void)
{
    struct blkbuf b = { .bf_dirty = 1 };
    write_result = -1;
    bdput(&b);
    TEST(udata.u_error == EIO && !b.bf_dirty && b.bf_busy == BF_FREE,
         "failed sync reports EIO and releases its buffer");
    udata.u_error = 0;
    write_result = BLKSIZE;
    bdput(&b);
    TEST(writes == 1 && flushes == 1 && !b.bf_dirty && b.bf_busy == BF_FREE,
         "a subsequent sync does not retry the discarded data");
    return 0;
}

static int eviction(int result)
{
    bufclock = 100;
    bufpool[0].bf_dirty = 1;
    bufpool[0].bf_time = 1;
    bufpool[1].bf_busy = BF_BUSY;
    write_result = result;
    TEST(freebuf() == bufpool && writes == 1 && !bufpool[0].bf_dirty &&
         bufpool[0].bf_busy == BF_BUSY &&
         udata.u_error == (result == BLKSIZE ? 0 : EIO),
         "eviction reports errors but still returns a recyclable buffer");
    return 0;
}

int main(int argc, char **argv)
{
    struct blkbuf b = { .bf_busy = BF_BUSY };
    if (argc != 2) return 2;
    if (!strcmp(argv[1], "failed-immediate")) return immediate(-1);
    if (!strcmp(argv[1], "short-immediate")) return immediate(BLKSIZE-1);
    if (!strcmp(argv[1], "complete-immediate")) return immediate(BLKSIZE);
    if (!strcmp(argv[1], "failed-sync")) return sync_write(-1);
    if (!strcmp(argv[1], "short-sync")) return sync_write(BLKSIZE-1);
    if (!strcmp(argv[1], "complete-sync")) return sync_write(BLKSIZE);
    if (!strcmp(argv[1], "no-repeat-sync")) return no_repeat_sync();
    if (!strcmp(argv[1], "failed-eviction")) return eviction(-1);
    if (!strcmp(argv[1], "short-eviction")) return eviction(BLKSIZE-1);
    if (!strcmp(argv[1], "complete-eviction")) return eviction(BLKSIZE);
    if (!strcmp(argv[1], "sync-flush-clears-error")) {
        flush_clears_error = true;
        return sync_write(-1);
    }
    if (!strcmp(argv[1], "delayed")) {
        TEST(bfree(&b, 1) == 0 && b.bf_dirty && b.bf_busy == BF_FREE && !writes,
             "delayed write keeps data dirty without doing IO");
    } else if (!strcmp(argv[1], "clean-sync")) {
        bdput(&b);
        TEST(!writes && !flushes && b.bf_busy == BF_FREE,
             "clean sync releases the buffer without IO");
    } else return 2;
    return 0;
}
