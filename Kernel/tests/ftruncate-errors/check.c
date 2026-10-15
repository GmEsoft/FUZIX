#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef intptr_t arg_t;
typedef uint16_t blkno_t;
#define off_t kernel_off_t
typedef int32_t off_t;
#define TEST(c,msg) do {if(!(c)){puts(msg);return 1;}}while(0)
/* DEFINE O_RDONLY */
/* DEFINE O_WRONLY */
/* DEFINE O_ACCMODE */
/* DEFINE F_REG */
/* DEFINE MODE_R */
/* DEFINE CRDONLY */
/* DEFINE HIBYTE32 */
/* DEFINE BLKOVERSIZE32 */
/* DEFINE M_TIME */
/* DEFINE C_TIME */
#if TEST_BLOCK == 400
#include "blk400.h"
#else
#include "blk512.h"
#endif

struct cinode {
    struct { uint16_t i_mode; uint32_t i_size; } c_node;
    uint8_t c_flags;
};
typedef struct cinode *inoptr;
struct oft { uint8_t o_access; } of_tab[1];
struct { intptr_t u_argn, u_argn1; int u_error; uint8_t u_files[1]; } udata;
static struct cinode node;
#define NULLINODE ((inoptr)0)
#define file ((uint16_t)udata.u_argn)
#define offset ((uint32_t *)udata.u_argn1)

static unsigned truncations, writes, timestamps, kept;
static int truncate_error, copy_error;
int uget(const void *p, void *out, unsigned n)
{
    if (copy_error) { udata.u_error = EFAULT; return -1; }
    memcpy(out, p, n);
    return 0;
}
inoptr getinode(unsigned fd) { return &node; }
unsigned getmode(inoptr ino) { return MODE_R(ino->c_node.i_mode); }
int f_trunc_blocks(inoptr ino, uint16_t count)
{
    truncations++;
    kept = count;
    if (ino->c_flags & CRDONLY) { udata.u_error = EROFS; return -1; }
    if (truncate_error) { udata.u_error = EIO; return -1; }
    return 0;
}
void setftime(inoptr ino, unsigned flags) { timestamps++; }
void wr_inode(inoptr ino) { writes++; }
static void setup(off_t *size, uint32_t old)
{
    node.c_node.i_mode = F_REG;
    node.c_node.i_size = old;
    of_tab[0].o_access = O_WRONLY;
    udata.u_argn1 = (intptr_t)size;
}

/* FUNCTION syscall_fs3.c _ftruncate 0 */

int main(int argc, char **argv)
{
    off_t size = 10;
    if (argc != 2) return 2;
    setup(&size, BLKSIZE + 20);
    if (!strcmp(argv[1], "shrink")) {
        TEST(_ftruncate() == 0 && node.c_node.i_size == 10 &&
             truncations == 1 && kept == 1 && writes == 1 && timestamps == 1,
             "successful shrink retains the partial block and updates the inode");
    } else if (!strcmp(argv[1], "grow")) {
        size = 2 * BLKSIZE;
        TEST(_ftruncate() == 0 && !truncations && node.c_node.i_size == (uint32_t)size,
             "growth updates size without freeing blocks");
    } else if (!strcmp(argv[1], "truncate-failure")) {
        truncate_error = 1;
        TEST(_ftruncate() == -1 && udata.u_error == EIO &&
             node.c_node.i_size == BLKSIZE + 20 && !writes && !timestamps,
             "truncation failure preserves the inode size and timestamps");
    } else if (!strcmp(argv[1], "readonly-shrink") ||
               !strcmp(argv[1], "readonly-grow")) {
        if (!strcmp(argv[1], "readonly-grow")) size = 2 * BLKSIZE;
        node.c_flags = CRDONLY;
        TEST(_ftruncate() == -1 && udata.u_error == EROFS &&
             node.c_node.i_size == BLKSIZE + 20 && !truncations && !writes && !timestamps,
             "read-only files cannot change size");
    } else if (!strcmp(argv[1], "unchanged")) {
        size = BLKSIZE + 20;
        TEST(_ftruncate() == 0 && !truncations && !writes && !timestamps,
             "unchanged size is a no-op");
    } else if (!strcmp(argv[1], "large-retained-count")) {
        size = 65536UL * BLKSIZE - 1;
        node.c_node.i_size = 65536UL * BLKSIZE;
        TEST(_ftruncate() == 0 && !truncations && node.c_node.i_size == (uint32_t)size,
             "65536 retained blocks must not wrap to zero");
    } else if (!strcmp(argv[1], "invalid-range")) {
        size = 65536UL * BLKSIZE + 1;
        TEST(_ftruncate() == -1 && udata.u_error == EINVAL && !truncations && !writes,
             "reject sizes outside the block-number range");
    } else if (!strcmp(argv[1], "copy-fault")) {
        copy_error = 1;
        TEST(_ftruncate() == -1 && udata.u_error == EFAULT && !truncations && !writes,
             "copy fault preserves the inode");
    } else return 2;
    return 0;
}
