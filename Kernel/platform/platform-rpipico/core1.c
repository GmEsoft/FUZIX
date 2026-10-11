/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <kernel.h>
#include <printf.h>
#include "picosdk.h"
#include "config.h"
#include "core1.h"

#define ssize_t __ssize_t
#define time_t __time_t
#include <tusb.h>
#undef ssize_t
#undef time_t

#include <pico/critical_section.h>
#include <pico/multicore.h>
#include <pico/flash.h>
#include <hardware/uart.h>
#include <hardware/irq.h>

#define CONFIG_CONSOLE_BUF_LEN 16

#if CONFIG_CONSOLE_BUF_LEN > 255
#error "Invalid console buffer length"
#endif

struct console_buf_t
{
	uint8_t buffer[CONFIG_CONSOLE_BUF_LEN];
	uint8_t rindex;
	uint8_t windex;
	uint8_t len;
};

struct console_t
{
	struct console_buf_t rbuf;
	struct console_buf_t wbuf;
	bool sleeping;
};

static struct console_t usb_console_table[NUM_DEV_TTY_USB];
int usb_host_connected;
critical_section_t critical_section;

static void console_buf_write(struct console_buf_t *buf, uint8_t b)
{
	critical_section_enter_blocking(&critical_section);
	if (buf->len < CONFIG_CONSOLE_BUF_LEN)
	{
		buf->buffer[buf->windex] = b;
		buf->windex = (buf->windex + 1) % CONFIG_CONSOLE_BUF_LEN;
		buf->len++;
	}
	critical_section_exit(&critical_section);
}

static uint8_t console_buf_read(struct console_buf_t *buf)
{
	critical_section_enter_blocking(&critical_section);
	uint8_t b = 0;
	if (buf->len != 0)
	{
		b = buf->buffer[buf->rindex];
		buf->rindex = (buf->rindex + 1) % CONFIG_CONSOLE_BUF_LEN;
		buf->len--;
	}
	critical_section_exit(&critical_section);
	return b;
}

int usbconsole_getc(uint8_t usb_num)
{
	int c = -1;
	if (!usb_num || usb_num > NUM_DEV_TTY_USB)
		return -1;
	struct console_buf_t *buf = &usb_console_table[usb_num - 1].rbuf;
	if (buf->len > 0)
	{
		c = console_buf_read(buf);
	}
	return c;
}

ttyready_t usbconsole_ready(uint8_t usb_num)
{
    if (!usb_num || usb_num > NUM_DEV_TTY_USB)
        return TTY_READY_LATER;
    critical_section_enter_blocking(&critical_section);
    bool ready = usb_host_connected &&
        usb_console_table[usb_num - 1].wbuf.len < CONFIG_CONSOLE_BUF_LEN;
    critical_section_exit(&critical_section);
    return ready ? TTY_READY_NOW : TTY_READY_LATER;
}

bool usbconsole_is_available(uint8_t usb_num)
{
    critical_section_enter_blocking(&critical_section);
    bool available = usb_host_connected && usb_num && usb_num <= NUM_DEV_TTY_USB;
    critical_section_exit(&critical_section);
    return available;
}

void usbconsole_putc(uint8_t usb_num, uint8_t c)
{
    if (!usb_num || usb_num > NUM_DEV_TTY_USB)
        return;
    /* The generic TTY writer waits before calling us. Echo/diagnostics can
     * arrive from an interrupt and must drop a byte if the queue is full. */
    if (usbconsole_is_available(usb_num))
        console_buf_write(&usb_console_table[usb_num - 1].wbuf, c);
}

void usbconsole_sleeping(uint8_t usb_num)
{
    if (!usb_num || usb_num > NUM_DEV_TTY_USB)
        return;
    critical_section_enter_blocking(&critical_section);
    usb_console_table[usb_num - 1].sleeping = true;
    critical_section_exit(&critical_section);
}

bool usbconsole_wakeup(uint8_t usb_num)
{
    if (!usb_num || usb_num > NUM_DEV_TTY_USB)
        return false;
    critical_section_enter_blocking(&critical_section);
    struct console_t *console = &usb_console_table[usb_num - 1];
    bool wake = console->sleeping && usb_host_connected &&
        console->wbuf.len < CONFIG_CONSOLE_BUF_LEN;
    if (wake)
        console->sleeping = false;
    critical_section_exit(&critical_section);
    return wake;
}


void tud_mount_cb(void)
{
	critical_section_enter_blocking(&critical_section);
	usb_host_connected = 1;
	critical_section_exit(&critical_section);
}

void tud_umount_cb(void)
{
	critical_section_enter_blocking(&critical_section);
	usb_host_connected = 0;
	critical_section_exit(&critical_section);
}

static void core1_main(void)
{
	/* Install the SDK's RAM-resident FIFO handler before enabling USB.
	 * It parks this core without flash access while core 0 programs flash. */
	if (!flash_safe_execute_core_init())
		panic("USB flash lockout init");
	tusb_init();

	for (;;)
	{
		tud_task();

		/*
		We ignore tud_cdc_n_connected because if DTR is not set by the host
		all tty's will appear disconnected, which seems to be the case for pi pico
		*/

		for (int i = 0; i < CFG_TUD_CDC; i++)
		{
			if (tud_cdc_n_write_available(i) && usb_console_table[i].wbuf.len)
			{
				uint8_t b = console_buf_read(&usb_console_table[i].wbuf);
				tud_cdc_n_write_char(i, b);
				tud_cdc_n_write_flush(i);
			}
		}

		for (int i = 0; i < CFG_TUD_CDC; i++)
		{
			if (!tud_cdc_n_available(i))
			{
				continue;
			}
			struct console_buf_t *buf = &usb_console_table[i].rbuf;
			if (buf->len < CONFIG_CONSOLE_BUF_LEN)
			{
				uint8_t b = tud_cdc_n_read_char(i);
				console_buf_write(buf, b);
			}
		}
	}
}

void core1_init(void)
{
	//multicore_reset_core1();
	critical_section_init(&critical_section);
#if NUM_DEV_TTY_USB > 0
	multicore_launch_core1(core1_main);
	/* No filesystem write may race the victim-handler initialization. */
	while (!multicore_lockout_victim_is_initialized(1))
		tight_loop_contents();
#endif
}
