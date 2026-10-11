#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/gpio.h>
#include "../../pico_ioctl.h"

int fixture_open(const char *path, int flags, ...)
{
    (void)flags;
    assert(!strcmp(path,"/dev/sys") || !strcmp(path,"/dev/gpio"));
    puts("TEST OPEN");
    if (getenv("FAIL_OPEN")) { errno=EIO; return -1; }
    return 42;
}
int fixture_ioctl(int fd, unsigned long request, ...)
{
    assert(fd==42);
    assert(request==PICOIOC_FLASH || request==GPIOC_SET);
    if (request==GPIOC_SET) {
        va_list ap;
        va_start(ap,request);
        struct gpioreq *gr=va_arg(ap,struct gpioreq *);
        printf("TEST GPIO %u %u\n",gr->pin,gr->val);
        va_end(ap);
    }
    puts("TEST IOCTL");
    if (getenv("FAIL_IOCTL")) { errno=EIO; return -1; }
    return 0;
}
int fixture_close(int fd)
{
    assert(fd==42);
    puts("TEST CLOSE");
    return 0;
}
