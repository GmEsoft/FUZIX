#ifndef XIP_TEST_PICOSDK_H
#define XIP_TEST_PICOSDK_H
#include <stdint.h>

extern uint8_t xip_flash[];
#define XIP_NOCACHE_NOALLOC_BASE ((uintptr_t)xip_flash)
#endif
