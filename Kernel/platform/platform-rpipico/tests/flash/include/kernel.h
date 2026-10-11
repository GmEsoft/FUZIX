#ifndef FLASH_TEST_KERNEL_H
#define FLASH_TEST_KERNEL_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#define CONFIG_H
#define CONFIG_TRIM
#define PICO_FLASH_SIZE_BYTES (2*1024*1024)
#define XIP_NOCACHE_NOALLOC_BASE 0x13000000
typedef unsigned uint;
typedef uint16_t uarg_t;
typedef void *ptptr;
struct u_data { uint8_t u_error; };
extern struct u_data udata;
void kprintf(const char *, ...);
#endif
