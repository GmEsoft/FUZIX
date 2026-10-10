#include <stdint.h>
#define BLKSHIFT 9
#include "../config.h"

/* Check the actual platform configuration, not only the host fixture. */
typedef char process_size[(PROGSIZE + UDATA_SIZE == PROCMEM * 1024) ? 1 : -1];
typedef char swap_size[(SWAP_SIZE * 512 == PROCMEM * 1024) ? 1 : -1];
typedef char pool_size[(USERMEM == TOTALMEM * 1024) ? 1 : -1];
typedef char whole_blocks[((PROGSIZE + UDATA_SIZE) % 4096 == 0) ? 1 : -1];
