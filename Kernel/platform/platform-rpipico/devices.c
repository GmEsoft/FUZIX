#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devsys.h>
#include <blkdev.h>
#include <tty.h>
#include <devtty.h>
#include <dev/devsd.h>
#include <printf.h>
#include "globals.h"
#include "picosdk.h"
#include <hardware/irq.h>
#include "core1.h"
#ifdef CONFIG_NET_WIZNET
#include "wiznet_spi.h"
#endif

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write           ioctl
// ---------------------------------------------------------------------
  /* 0: /dev/hd - block device interface */
  {  blkdev_open,   no_close,   blkdev_read,    blkdev_write,	blkdev_ioctl},
  /* 1: /dev/fd - Floppy disk block devices */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,     no_close,   no_rdwr,   no_rdwr,  no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      sys_close,    sys_read, sys_write, sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
};

static uint64_t tick_deadline;
#define TICK_US (1000000 / TICKSPERSEC)

bool validdev(uint16_t dev)
{
    /* This is a bit uglier than needed but the right hand side is
       a constant this way */
    if(dev > ((sizeof(dev_tab)/sizeof(struct devsw)) << 8) - 1)
	return false;
    else
        return true;
}

static void timer_tick_cb(unsigned alarm)
{
    irqflags_t irq = di();
    udata.u_ininterrupt = 1;

    absolute_time_t next;
    tick_deadline += TICK_US;

    tty_interrupt();
    timer_interrupt();
#ifdef CONFIG_NET_WIZNET
    wiz_poll();
#endif

    /* Keep the original phase rather than adding handler time every tick.
     * If interrupts were delayed, skip expired deadlines to the next period. */
    for (;;) {
        update_us_since_boot(&next, tick_deadline);
        if (!hardware_alarm_set_target(alarm, next))
            break;
        uint64_t current = time_us_64();
        if (current >= tick_deadline)
            tick_deadline += ((current - tick_deadline) / TICK_US + 1) * TICK_US;
    }

    udata.u_ininterrupt = 0;
    irqrestore(irq);
}

#ifdef CONFIG_NET
extern void netdev_init(void);
#endif

void device_init(void)
{
    /* Timer interrup must be initialized before blcok devices.
       set_boot_line uses pause syscall which will not be operational otherwise. */
    hardware_alarm_claim(0);
    tick_deadline = time_us_64();
    hardware_alarm_set_callback(0, timer_tick_cb);
    hardware_alarm_force_irq(0);

    /* The flash device is too small to be useful, and a corrupt flash will
     * cause a crash on startup... oddly. */
#ifdef CONFIG_PICO_FLASH
    flash_dev_init();
#endif
#ifdef CONFIG_NET
	netdev_init();
#endif
    sd_rawinit();
    devsd_init();
}

/* vim: sw=4 ts=4 et: */

