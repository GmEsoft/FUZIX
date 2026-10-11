#ifndef TTY_TEST_KERNEL_H
#define TTY_TEST_KERNEL_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#define CONFIG_H
#define NUM_DEV_TTY_UART 1
#define NUM_DEV_TTY_USB 4
#define NUM_DEV_TTY (NUM_DEV_TTY_UART+NUM_DEV_TTY_USB)
#define DEV_UART_0_TX_PIN 0
#define DEV_UART_0_RX_PIN 1
#define DEV_UART_0_CTS_PIN 2
#define DEV_UART_0_RTS_PIN 3
#define DEV_UART_1_TX_PIN 6
#define DEV_UART_1_RX_PIN 7
#define DEV_UART_1_CTS_PIN 8
#define DEV_UART_1_RTS_PIN 9
#define DEV_USB_DETECT_TIMEOUT 1
#define DEV_USB_INIT_TIMEOUT 1
#define used(x) ((void)(x))
typedef unsigned uint;
typedef uint16_t uarg_t;
typedef void *inoptr;
struct s_queue {
    uint8_t *q_base, *q_head, *q_tail;
    unsigned q_size, q_count, q_wakeup;
};
#include <string.h>
#define PROGBASE ((uintptr_t)0x20010000)
#define PROGLOAD (PROGBASE+1536)
#define PROGTOP (PROGBASE+65536)
#define USERMEM (160*1024)
#define UDATA_SIZE 1536
#define SRAM_BASE 0x20000000
#define SRAM_END 0x20042000
#define GPIO_OUT 1
#define CFG_TUD_CDC NUM_DEV_TTY_USB
typedef struct p_tab { uint32_t p_pid, p_status; } *ptptr;
struct u_data {
    uint32_t u_sp, u_isp, u_break, u_argn, u_argn1, u_argn2, u_argn3;
    ptptr u_ptab;
    uint8_t u_callno, u_insys, u_ininterrupt, u_error;
    uint32_t u_retval;
};
extern struct u_data udata;
extern unsigned ramsize, procmem;
void fuzix_main(void);
void unix_syscall(void);
int di(void);
void fpanic(const char *) __attribute__((noreturn));
void _sched_yield(void);
void kprintf(const char *, ...);
#endif
