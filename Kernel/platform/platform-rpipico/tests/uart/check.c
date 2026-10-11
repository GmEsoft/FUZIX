#include <kernel.h>
#include <tty.h>
#include <hardware/uart.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../rawuart.h"
#include "../../devtty.h"
static uart_inst_t ports[2]={{.num=0},{.num=1}};
static unsigned inits[2], deinits, pins, yields, formats;
static uint rate, bits, stops;
static uart_parity_t parity;
struct tty ttydata[NUM_DEV_TTY+1];
uart_inst_t *uart_get_instance(unsigned n) { assert(n<2); return &ports[n]; }
uart_hw_t *uart_get_hw(uart_inst_t *p) { return &p->hw; }
uint uart_init(uart_inst_t *p, uint baud) { inits[p->num]++; p->hw.fr=UART_UARTFR_TXFE_BITS; return baud; }
void uart_deinit(uart_inst_t *p) { (void)p; deinits++; }
void gpio_set_function(unsigned pin, unsigned function) { assert(function==2 && pin<32); pins|=1u<<pin; }
void uart_set_translate_crlf(uart_inst_t *p, bool v) { (void)p; assert(!v); }
void uart_set_fifo_enabled(uart_inst_t *p, bool v) { (void)p; assert(v); }
void uart_set_hw_flow(uart_inst_t *p, bool cts, bool rts) { (void)p; assert(cts && rts); }
bool uart_is_writable(uart_inst_t *p) { (void)p; return true; }
bool uart_is_readable(uart_inst_t *p) { (void)p; return false; }
void uart_putc(uart_inst_t *p, char c) { (void)p; (void)c; }
uint uart_set_baudrate(uart_inst_t *p, uint baud) { assert(!(p->hw.fr&UART_UARTFR_BUSY_BITS)); rate=baud; return baud; }
void uart_set_format(uart_inst_t *p, uint data, uint stop, uart_parity_t mode)
{ assert(!(p->hw.fr&UART_UARTFR_BUSY_BITS)); formats++; bits=data; stops=stop; parity=mode; }
void _sched_yield(void) { yields++; assert(yields<5); ports[0].hw.fr &= ~UART_UARTFR_BUSY_BITS; }
void tight_loop_contents(void) {}
void kprintf(const char *fmt, ...) { (void)fmt; }
uint64_t get_absolute_time(void) { static uint64_t t; return ++t*1000; }
uint64_t delayed_by_ms(uint64_t t, uint ms) { return t+ms*1000; }
int64_t absolute_time_diff_us(uint64_t a, uint64_t b) { return b-a; }
void core1_init(void) {}
int usbconsole_getc(uint8_t n) { (void)n; return -1; }
void usbconsole_putc(uint8_t n, uint8_t c) { (void)n; (void)c; }
ttyready_t usbconsole_ready(uint8_t n) { (void)n; return TTY_READY_NOW; }
bool usbconsole_is_available(uint8_t n) { (void)n; return false; }
void usbconsole_sleeping(uint8_t n) { (void)n; }
uint_fast8_t tty_inproc(uint_fast8_t n, uint_fast8_t c) { (void)n; (void)c; return 0; }
void tty_outproc(uint_fast8_t n) { (void)n; }
int main(int argc, char **argv)
{
    assert(argc==2);
    ttydata[1].termios.c_cflag=B115200|CS8;
    if (!strcmp(argv[1],"pins")) {
        rawuart_setup(1,2,0);
        assert((pins&((1u<<6)|(1u<<7)|(1u<<8)|(1u<<9)))==((1u<<6)|(1u<<7)|(1u<<8)|(1u<<9)));
        assert(!(pins&((1u<<2)|(1u<<3))));
    } else if (!strcmp(argv[1],"drain")) {
        rawuart_early_init();
        ports[0].hw.fr=UART_UARTFR_TXFE_BITS|UART_UARTFR_BUSY_BITS;
        rawuart_setup(1,1,1);
        assert(yields==1 && inits[0]==1 && formats==1);
    } else if (!strcmp(argv[1],"settings")) {
        for (unsigned p=0; p<4; p++) {
            ttydata[1].termios.c_cflag=B9600|CS7|CSTOPB|((p&1)?PARENB:0)|((p&2)?PARODD:0);
            rawuart_setup(1,1,0);
            assert(rate==9600 && bits==7 && stops==2);
            assert(parity==((p&1)?((p&2)?UART_PARITY_ODD:UART_PARITY_EVEN):UART_PARITY_NONE));
        }
        assert(inits[0]==1);
    } else if (!strcmp(argv[1],"hangup")) {
        rawuart_early_init();
        ttydata[1].termios.c_cflag=B0;
        rawuart_setup(1,1,1);
        assert(deinits==1);
        ttydata[1].termios.c_cflag=B115200|CS8;
        rawuart_setup(1,1,1);
        assert(inits[0]==2);
    } else if (!strcmp(argv[1],"mask")) {
        devtty_init();
        assert(termios_mask[1]&CSTOPB);
    } else abort();
    puts(argv[1]);
}
