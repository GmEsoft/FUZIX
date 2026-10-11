#include <kernel.h>
#include <kdata.h>
#include <printf.h>
extern int abi_signal_callback(int);
extern void abi_fatal(const char *) __attribute__((noreturn));
static unsigned syscall_ok;

void abi_prepare(void)
{
    udata.u_insys = 1;
    udata.u_sp = PROGBASE + 8192;
    udata.u_ptab = NULL;
    syscall_ok = 0;
}
void unix_syscall(void)
{
    syscall_ok = udata.u_insys && udata.u_callno == 0xaa &&
        udata.u_argn == 2 && udata.u_argn1 == 3 &&
        udata.u_argn2 == 4 && udata.u_argn3 == 5;
    udata.u_retval = 0xa5;
    udata.u_error = 0x5a;
    udata.u_cursig = 2;
    udata.u_sigvec[2] = (uaddr_t)abi_signal_callback;
}
unsigned abi_syscall_ok(void) { return syscall_ok; }
unsigned abi_user_mode(void) { return !udata.u_insys; }
void kprintf(const char *fmt, ...) { (void)fmt; }
void panic(char *message) { abi_fatal(message); }
