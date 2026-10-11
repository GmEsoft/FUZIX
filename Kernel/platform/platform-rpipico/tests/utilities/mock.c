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
    assert(request==PICOIOC_FLASH || request==GPIOC_SET || request==GPIOC_CLR || request==GPIOC_GETINFO || request==GPIOC_SETRW);
    static unsigned pin, calls;
    calls++;
    if (request!=PICOIOC_FLASH) {
        va_list ap;
        va_start(ap,request);
        void *data=va_arg(ap,void *);
        if (request==GPIOC_GETINFO) {
            pin=*(uint8_t *)data;
            struct gpio *info=data;
            memset(info,0,sizeof(*info));
            info->wmask=0x02;
            if (pin>=30) { errno=ENODEV; puts("TEST IOCTL"); return -1; }
        } else {
            struct gpioreq *gr=data;
            assert(gr->pin==pin);
            unsigned mask=1U << (pin & 7);
            assert(gr->val==(request==GPIOC_SETRW ? 0x02 | mask : mask));
            if (request!=GPIOC_SETRW)
                printf("TEST GPIO %u %u\n",gr->pin,request==GPIOC_SET);
        }
        va_end(ap);
    }
    puts("TEST IOCTL");
    if (getenv("FAIL_IOCTL") || (getenv("FAIL_IOCTL_STAGE") && calls==(unsigned)atoi(getenv("FAIL_IOCTL_STAGE")))) { errno=EIO; return -1; }
    return 0;
}
int fixture_close(int fd)
{
    assert(fd==42);
    puts("TEST CLOSE");
    return 0;
}
