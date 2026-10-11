/* Run the production context/SVC/fork assembly on a Pico, without an SD card. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "pico/bootrom.h"

uint8_t progbase[160 * 1024] __attribute__((aligned(8)));
uint16_t runticks;
uint8_t abi_ptab[32] __attribute__((aligned(8)));
uint32_t abi_saved_sp, abi_mode, abi_result[11];
uint32_t abi_kernel_sp, abi_signal_sp, abi_callback_sp, abi_switch_sp;
uint32_t abi_clone_sp, abi_makeproc_sp, abi_exec_sp;
extern void abi_run_switch(void), abi_run_fork(void), abi_run_svc(void), abi_run_exec(void);
extern void abi_prepare(void);
extern unsigned abi_syscall_ok(void), abi_user_mode(void);
static unsigned failures;

void abi_fatal(const char *why)
{
    printf("FAIL unexpected exception: %s\n", why);
    stdio_flush();
    for (;;) tight_loop_contents();
}
static void check(unsigned yes, const char *what)
{
    if (!yes) { printf("FAIL %s\n", what); failures++; }
}
static void registers(void)
{
    for (unsigned i = 0; i < 9; i++) {
        uint32_t expected = 0x44444444u + i * 0x11111111u;
        if (abi_result[i + 2] != expected) {
            printf("FAIL r%u: %08lx expected %08lx\n", i + 4,
                   (unsigned long)abi_result[i + 2], (unsigned long)expected);
            failures++;
        }
    }
}
static void run(void)
{
    failures = 0;
    memset(progbase, 0, sizeof(progbase));
    memset(abi_ptab, 0, sizeof(abi_ptab));
    abi_ptab[4] = 7;
    abi_prepare();
    abi_run_switch();
    registers();
    check(abi_switch_sp && !(abi_switch_sp & 7), "switch C stack alignment");
    check(abi_ptab[0] == 1, "switched process running");
    abi_run_fork();
    registers();
    check(abi_result[0] == 0, "fork child return value");
    check(abi_clone_sp && !(abi_clone_sp & 7), "clone C stack alignment");
    check(abi_makeproc_sp && !(abi_makeproc_sp & 7), "makeproc C stack alignment");
    abi_run_svc();
    registers();
    check(abi_result[0] == 0xa5 && abi_result[1] == 0x5a, "SVC return frame");
    check(abi_syscall_ok(), "SVC argument frame and kernel mode");
    check(abi_kernel_sp && !(abi_kernel_sp & 7), "syscall C stack alignment");
    check(abi_signal_sp && !(abi_signal_sp & 7), "signal C stack alignment");
    check(abi_callback_sp && !(abi_callback_sp & 7), "user signal callback alignment");
    check(abi_user_mode(), "SVC clears u_insys");
    abi_prepare();
    abi_run_exec();
    check(abi_exec_sp == (uint32_t)(progbase + 8192), "exec user stack");
    check(abi_user_mode(), "exec clears u_insys");
    printf("ABI RESULT: %s (%u failures)\n", failures ? "FAIL" : "PASS", failures);
    stdio_flush();
}
int main(void)
{
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(10);
    sleep_ms(100);
    puts("RPIPICO ABI TEST: send t to run");
    stdio_flush();
    for (;;) {
        int c = getchar();
        if (c == 't') run();
        else if (c == 'b') reset_usb_boot(0, 0);
    }
}
