#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

#define IDE_DSR		0x70

static uint8_t old_dsr;

void ide_select(uint8_t drive)
{
    if (drive > 1)
        old_dsr = 0x81;		/* Powered on, second interface */
    else
        old_dsr = 0x80;		/* Powered on, first interface */
    out(IDE_DSR, old_dsr);
}

static void devide_delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
       plt_idle();
}

void devide_reset(void)
{
    out(IDE_DSR, 0xC0);		/* Reset */
    devide_delay();
    out(IDE_DSR, old_dsr);
}

