#ifndef TEST_KERNEL_H
#define TEST_KERNEL_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#define CONFIG_H
#define CONFIG_NET
#define CONFIG_NET_WIZNET
#define CONFIG_NET_W5500
#define CONFIG_RC2040
#define TICKSPERSEC 100
typedef unsigned uint;
typedef unsigned irqflags_t;
typedef void *ptptr;
typedef int ttyready_t;
struct u_data { unsigned u_ininterrupt; };
extern struct u_data udata;
struct devsw { int (*a)(void), (*b)(void), (*c)(void), (*d)(void), (*e)(void); };
int fixture_noop(void);
#define no_open fixture_noop
#define no_close fixture_noop
#define no_rdwr fixture_noop
#define no_ioctl fixture_noop
#define sys_close fixture_noop
#define sys_read fixture_noop
#define sys_write fixture_noop
#define sys_ioctl fixture_noop
#define blkdev_open fixture_noop
#define blkdev_read fixture_noop
#define blkdev_write fixture_noop
#define blkdev_ioctl fixture_noop
#define tty_open fixture_noop
#define tty_close fixture_noop
#define tty_read fixture_noop
#define tty_write fixture_noop
#define tty_ioctl fixture_noop
irqflags_t di(void);
void irqrestore(irqflags_t);
void timer_interrupt(void);
void tty_interrupt(void);
#endif
