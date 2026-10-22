#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

extern uint8_t fuller, kempston, kmouse, kempston_mbmask;

void pagemap_init(void)
{
	uint8_t i;

	/* Pentagon uses 7FFD as follows
	   7: 256K
	   6: 128K
	   2: 64K
	   1: 32K
	   0: 16K

	   The 1MB one uses bit 5 for 512K */

	/* Add the rest of the first 256K */
	pagemap_add(0x04);
	pagemap_add(0x06);

	/* 128-256K */
	for (i = 0; i < 8; i++)
		pagemap_add(0x40 | i);

	/* TODO: enable 512K support in pentagon.s */
	if (ramsize == 512) {
		for (i = 0; i < 8; i++) {
			pagemap_add(0x80 | i);
			pagemap_add(0xC0 | i);
		}
	}
	/* TODO: 512K is more complex.
	   - Pentagon 512 uses bits 7/6/2/1/0
	   - ATM uses FDFD bits 2-0 */
}

uint_fast8_t plt_param(char *p)
{
	if (strcmp(p, "kempston") == 0) {
		kempston = 1;
		return 1;
	}
	if (strcmp(p, "kmouse") == 0) {
		kmouse = 1;
		return 1;
	}
	if (strcmp(p, "fuller") == 0) {
		fuller = 1;
		return 1;
	}
	if (strcmp(p, "kmouse3") == 0) {
		kmouse = 1;
		kempston_mbmask = 7;
		return 1;
	}
	if (strcmp(p, "kmturbo") == 0) {
		/* For now rely on the turbo detect - may want to change this */
		kmouse = 1;
		return 1;
	}
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

void plt_copyright(void)
{
}
