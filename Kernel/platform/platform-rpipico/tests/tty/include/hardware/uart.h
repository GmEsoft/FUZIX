#ifndef UART_TEST_SDK_H
#define UART_TEST_SDK_H
#include <kernel.h>
#define PICO_DEFAULT_UART_BAUD_RATE 115200
#define UART_UARTFR_TXFE_BITS 0x80
#define UART_UARTFR_BUSY_BITS 0x08
typedef enum { UART_PARITY_NONE, UART_PARITY_EVEN, UART_PARITY_ODD } uart_parity_t;
typedef struct { uint32_t fr, dr; } uart_hw_t;
typedef struct { unsigned num; uart_hw_t hw; } uart_inst_t;
uart_inst_t *uart_get_instance(unsigned);
uart_hw_t *uart_get_hw(uart_inst_t *);
uint uart_init(uart_inst_t *, uint);
void uart_deinit(uart_inst_t *);
void uart_set_translate_crlf(uart_inst_t *, bool);
void uart_set_fifo_enabled(uart_inst_t *, bool);
void uart_set_hw_flow(uart_inst_t *, bool, bool);
bool uart_is_writable(uart_inst_t *);
bool uart_is_readable(uart_inst_t *);
void uart_putc(uart_inst_t *, char);
uint uart_set_baudrate(uart_inst_t *, uint);
void uart_set_format(uart_inst_t *, uint, uint, uart_parity_t);
#endif
