#ifndef GLOBALS_H
#define GLOBALS_H

#include "flash-layout.h"

#define FLASH_OFFSET FUZIX_FLASH_FTL_OFFSET

extern void flash_dev_init(void);
extern void sd_rawinit(void);

extern void contextswitch(ptptr p);

struct svc_frame
{
	uint32_t r12;
	uint32_t pc;
	uint32_t lr;
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
};

struct exception_frame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
};

struct extended_exception_frame
{
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t cause;
	uint32_t sp;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
};

int rpipico_flash_program(uint32_t offset, const uint8_t *data, uint32_t len);
int rpipico_flash_erase(uint32_t offset, uint32_t len);

#endif
