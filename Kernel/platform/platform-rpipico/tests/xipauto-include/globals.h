#ifndef XIP_TEST_GLOBALS_H
#define XIP_TEST_GLOBALS_H
#include <stdint.h>

#define FUZIX_FLASH_FLASH_BASE 0u
#define FUZIX_FLASH_XIP_OFFSET 0x18000u
#define FUZIX_FLASH_XIP_BYTES 0x40000u
#define FUZIX_FLASH_PAGE_BYTES 512u
#define FUZIX_FLASH_ERASE_BYTES 4096u

int rpipico_flash_program(uint32_t offset, const uint8_t *data, uint32_t len);
int rpipico_flash_erase(uint32_t offset, uint32_t len);
#endif
