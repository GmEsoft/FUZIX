#include <kernel.h>
#include <tty.h>
#include <pico/critical_section.h>
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../devtty.h"
#include "../../core1.h"
uint_fast8_t plt_param(char *);
void kputchar(uint_fast8_t);
void tud_mount_cb(void), tud_umount_cb(void);
struct u_data udata;
struct tty ttydata[NUM_DEV_TTY+1];
unsigned ramsize, procmem;
static jmp_buf escape;
static bool expecting_panic;
static void (*core_entry)(void);
static unsigned usb_room, usb_sent, usb_tasks, usb_passes, wakes, wake_bitmap;
static uint8_t last_byte;
static unsigned wake_minor;
void fpanic(const char *msg) { (void)msg; assert(expecting_panic); longjmp(escape,1); }
void kprintf(const char *fmt, ...) { (void)fmt; }
void tight_loop_contents(void) { fputs("unexpected busy wait\n",stderr); abort(); }
void _sched_yield(void) { abort(); }
uint64_t get_absolute_time(void) { static uint64_t t; return ++t*1000; }
uint64_t delayed_by_ms(uint64_t t, uint ms) { return t+ms*1000; }
int64_t absolute_time_diff_us(uint64_t a, uint64_t b) { return b-a; }
void rawuart_early_init(void) {}
void rawuart_putc(uint8_t n,uint8_t c) { (void)n; (void)c; }
ttyready_t rawuart_ready(uint8_t n) { (void)n; return TTY_READY_NOW; }
void rawuart_sleeping(uint8_t n) { (void)n; }
int rawuart_getc(uint8_t n) { (void)n; return -1; }
void rawuart_setup(uint_fast8_t a,uint_fast8_t b,uint_fast8_t c) { (void)a;(void)b;(void)c; }
uint_fast8_t tty_inproc(uint_fast8_t n,uint_fast8_t c) { (void)n;(void)c;return 1; }
void tty_outproc(uint_fast8_t n) { wakes++; wake_minor=n; wake_bitmap|=1u<<n; }
void critical_section_init(critical_section_t *c) { c->locked=0; }
void critical_section_enter_blocking(critical_section_t *c) { assert(!c->locked); c->locked=1; }
void critical_section_exit(critical_section_t *c) { assert(c->locked); c->locked=0; }
void multicore_launch_core1(void (*entry)(void)) { core_entry=entry; }
void tusb_init(void) {}
void tud_task(void) { if (++usb_tasks > usb_passes) longjmp(escape,1); }
unsigned tud_cdc_n_write_available(unsigned n) { return n==0 ? usb_room : 0; }
unsigned tud_cdc_n_write_char(unsigned n,char b) { assert(!n && usb_room); usb_room--; usb_sent++; last_byte=b; return 1; }
unsigned tud_cdc_n_write_flush(unsigned n) { (void)n; return 0; }
unsigned tud_cdc_n_available(unsigned n) { (void)n; return 0; }
int tud_cdc_n_read_char(unsigned n) { (void)n; abort(); }
static void pump(unsigned passes)
{
    usb_tasks=0; usb_passes=passes;
    if (!setjmp(escape)) core_entry();
}
static void parse(const char *input,bool valid)
{
    char value[256];
    assert(strlen(input)<sizeof(value));
    strcpy(value,input);
    expecting_panic=!valid;
    if (!setjmp(escape)) {
        assert(plt_param(value)==1);
        assert(valid);
    } else assert(!valid);
    expecting_panic=false;
}
int main(int argc,char **argv)
{
    assert(argc==2);
    core1_init();
    if (!strcmp(argv[1],"parse")) {
        parse("tty=usb1,usb2,usb3,usb4,uart1",true);
        assert(ttymap_count==NUM_DEV_TTY);
        assert(ttymap[5].tty==1 && ttymap[5].drv==TTYDRV_UART);
        for (unsigned i=1;i<5;i++) assert(ttymap[i].tty==i && ttymap[i].drv==TTYDRV_USB);
    } else if (!strcmp(argv[1],"invalid")) {
        const char *bad[]={"tty=usb0","tty=usb5","tty=uart2","tty=uart255","tty=usbgarbage",
                          "tty=usb1x","tty=usb-1","tty=usb99999999999999999999",
                          "tty=usb1,usb2,usb3,usb4,uart1,usb1"};
        for (unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++) parse(bad[i],false);
    } else if (!strcmp(argv[1],"remap")) {
        devtty_init();
        assert(ttymap_count==NUM_DEV_TTY);
        parse("tty=usb2,uart1",true);
        assert(ttymap_count==2 && termios_mask[1]==_CSYS);
        assert(termios_mask[2]&CBAUD);
        for (unsigned i=3;i<=NUM_DEV_TTY;i++) assert(!ttymap[i].tty && !termios_mask[i]);
    } else if (!strcmp(argv[1],"full")) {
        tud_mount_cb();
        for (unsigned i=0;i<16;i++) usbconsole_putc(1,'a'+i);
        assert(usbconsole_ready(1)==TTY_READY_LATER);
        usbconsole_putc(1,'X'); /* IRQ echo must not wait or overwrite. */
        usb_room=16; pump(16);
        assert(usb_sent==16 && last_byte=='p');
    } else if (!strcmp(argv[1],"irq-echo")) {
        tud_mount_cb(); parse("tty=usb1",true);
        ttydata[1].termios.c_lflag=ECHO;
        for (unsigned i=0;i<16;i++) usbconsole_putc(1,'a'+i);
        udata.u_ininterrupt=1;
        tty_echo(1,'X'); /* Actual common IRQ path must not wait for a full queue. */
        assert(udata.u_ininterrupt);
        usb_room=16; pump(16);
        assert(usb_sent==16 && last_byte=='p'); /* Echo cannot overwrite pending output. */
        tty_echo(1,'Y'); usb_room=1; pump(1);
        assert(usb_sent==17 && last_byte=='Y');
        tud_umount_cb(); tty_echo(1,'Z'); /* Disconnected host must not block IRQ echo. */
        assert(usb_sent==17 && udata.u_ininterrupt);
    } else if (!strcmp(argv[1],"disconnect")) {
        assert(usbconsole_ready(1)==TTY_READY_LATER);
        for (unsigned i=0;i<100;i++) usbconsole_putc(1,'X');
        tud_mount_cb(); usb_room=16; pump(16);
        assert(!usb_sent && usbconsole_ready(1)==TTY_READY_NOW);
        tud_umount_cb(); assert(usbconsole_ready(1)==TTY_READY_LATER);
    } else if (!strcmp(argv[1],"wakeup")) {
        tud_mount_cb(); parse("tty=usb1",true);
        for (unsigned i=0;i<16;i++) usbconsole_putc(1,'X');
        usbconsole_sleeping(1);
        tty_interrupt(); assert(!wakes);
        usb_room=1; pump(1);
        tty_interrupt(); assert(wakes==1 && wake_minor==1);
        tty_interrupt(); assert(wakes==1);
    } else if (!strcmp(argv[1],"aliases")) {
        tud_mount_cb(); parse("tty=usb1,usb1",true);
        for (unsigned i=0;i<16;i++) usbconsole_putc(1,'X');
        usbconsole_sleeping(1);
        usb_room=1; pump(1);
        tty_interrupt(); assert(wakes==2 && wake_bitmap==6);
    } else if (!strcmp(argv[1],"diagnostic")) {
        tud_mount_cb(); parse("tty=usb1",true);
        for (unsigned i=0;i<16;i++) usbconsole_putc(1,'X');
        kputchar('\n'); /* Must not wait for a non-consuming host. */
    } else if (!strcmp(argv[1],"bounds")) {
        tud_mount_cb();
        for (unsigned n=0;n<2;n++) {
            unsigned invalid=n ? NUM_DEV_TTY_USB+1 : 0;
            assert(usbconsole_getc(invalid)==-1);
            assert(usbconsole_ready(invalid)==TTY_READY_LATER);
            assert(!usbconsole_is_available(invalid));
            usbconsole_putc(invalid,'X'); usbconsole_sleeping(invalid);
            assert(!usbconsole_wakeup(invalid));
        }
    } else abort();
    puts(argv[1]);
}
