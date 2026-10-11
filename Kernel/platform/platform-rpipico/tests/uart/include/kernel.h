#ifndef UART_TEST_KERNEL_H
#define UART_TEST_KERNEL_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#define CONFIG_H
#define NUM_DEV_TTY_UART 2
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
void _sched_yield(void);
void kprintf(const char *, ...);
#endif
