#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#define _TINYDISK_PRIVATE
/* Our swap hack has to stick its nose in places it shouldn't */
#include <tinydisk.h>
#include <tinyide.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint16_t swap_dev = 0xFFFF;

void plt_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}

void map_init(void)
{
}

uint_fast8_t plt_param(char *p)
{
    return 0;
}

void device_init(void)
{
	extern uint8_t swapdevh, swaplba[3];
	uint8_t minor;
#ifdef CONFIG_TD_IDE
	ide_probe();
#endif
	minor = swap_dev & 0x0F;
	/* Our custom swap logic is a bit restrictive */
	if ((swap_dev & 0xFF00) != 0 || minor > CONFIG_TD_MAX_PART)
		panic("swap required on primary device");
	/* Configure the swap helper values */
	swapdevh = 0xE0;	/* primary only for now */
	if (minor) {
		/* LBA offset of partition for 24bits */
		swaplba[0] = td_lba[0][minor];
		swaplba[1] = td_lba[0][minor] >> 8;
		swaplba[2] = td_lba[0][minor] >> 16;
	}
}

void plt_interrupt(void)
{
	uint8_t dummy;

	tty_poll();
#if 0
	if (via[13] & 0x40) {
		dummy = via[4]; /* Reset interrupt */
		timer_interrupt();
	}
#endif
}

/* TODO: put these into the asm bits */
unsigned strlen(const char *p)
{
	unsigned n = 0;
	while(*p++)
		n++;
	return n;
}

void *memcpy(void *to, const void *from, unsigned len)
{
	const uint8_t *f = from;
	uint8_t *t = to;
	while(len--)
		*t++ = *f++;
	return to;
}

void *memset(void *to, int v, unsigned len)
{
	uint8_t *t = to;
	while(len--)
		*t++ = v;
	return to;
}

