#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

static char tbuf1[TTYSIZ];

uint8_t vtattr_cap;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS	/* TODO */
};

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
        return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	vtoutput(&c, 1);
}

void tty_setup(uint8_t minor, uint8_t flag)
{
}

void tty_sleeping(uint8_t minor)
{
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

#define POKEY	((volatile uint8_t *)0xD200)

static uint8_t kdown;

static void kbd_poll(void)
{
	extern uint8_t keydecode[];
	uint8_t r;
	/* Key still down: TODO autorepeat etc */
	if (POKEY[0x0F] & 4)
		return;
	r = POKEY[0x09];
	if (kdown)
		return;
	if (r & 0x80)
		r = keydecode[r & 0x7F] & 0x1F;
	else
		r = keydecode[r];
	if (r)
		vt_inproc(1, r);
}

void tty_poll(void)
{
	uint8_t r = POKEY[0x0E];
//	kprintf("Pokey %x %x\n", r, POKEY[0x0F]);
	/* Disable the interrupts to clear them */
	POKEY[0x0E] = r;

	/* Check what we got */

	if (!(r & 0x80))
		vt_inproc(1, 3);	/* Map break to ^C */
	if (!(r & 0x40))
		kbd_poll();
	else
		kdown = 0;
	/* TODO: 0x20 is serial ready, low bits are timers etc */
	/* Reset error flags */
	r = POKEY[0x0F];
	if (r & 0xE0)
		POKEY[0x0A] = 1;
	/* Interruptds back on */
	POKEY[0x0E] = 0xC0;
}

