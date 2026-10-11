#include <kernel.h>
#include <kdata.h>
#include <gpio.h>
#include "picosdk.h"
#include <hardware/platform_defs.h>

/* The common ABI addresses eight-pin groups and uses byte masks. */
int gpio_ioctl(uarg_t request, char *data)
{
    struct gpioreq gr;
    struct gpio info = { 0 };
    unsigned first, pin;
    uint8_t mask;

    if (request == GPIOC_COUNT)
        return NUM_BANK0_GPIOS;
    if (request < GPIOC_SETBYTE || request > GPIOC_GETINFO) {
        udata.u_error = ENOTTY;
        return -1;
    }
    if (uget(data, &gr, sizeof(gr)) == -1)
        return -1;
    if (gr.pin >= NUM_BANK0_GPIOS) {
        udata.u_error = ENODEV;
        return -1;
    }
    first = gr.pin & ~7U;
    for (pin = first; pin < first + 8 && pin < NUM_BANK0_GPIOS; pin++) {
        mask = 1U << (pin - first);
        info.pinmask |= mask;
        if (gpio_get_function(pin) == GPIO_FUNC_SIO && gpio_get_dir(pin))
            info.wmask |= mask;
        if (gpio_get_out_level(pin))
            info.wdata |= mask;
    }

    switch (request) {
    case GPIOC_GETINFO:
        info.group = gr.pin >> 3;
        info.flags = GPIO_BIT_CONFIG;
        memcpy(info.name, "GPIO", 4);
        if (first >= 10) {
            info.name[4] = '0' + first / 10;
            info.name[5] = '0' + first % 10;
        } else
            info.name[4] = '0' + first;
        return uput(&info, data, sizeof(info));
    case GPIOC_GETBYTE:
        mask = 0;
        for (pin = first; pin < first + 8 && pin < NUM_BANK0_GPIOS; pin++)
            if (gpio_get(pin))
                mask |= 1U << (pin - first);
        return mask & ~info.wmask;
    case GPIOC_SETBYTE:
        info.wdata = gr.val;
        break;
    case GPIOC_SET:
        info.wdata |= gr.val;
        break;
    case GPIOC_CLR:
        info.wdata &= ~gr.val;
        break;
    case GPIOC_SETRW:
        for (pin = first; pin < first + 8 && pin < NUM_BANK0_GPIOS; pin++) {
            mask = 1U << (pin - first);
            if (gr.val & mask) {
                /* Preserve the output latch and unselected peripheral pins. */
                gpio_set_function(pin, GPIO_FUNC_SIO);
                gpio_set_dir(pin, GPIO_OUT);
            } else if (gpio_get_function(pin) == GPIO_FUNC_SIO)
                gpio_set_dir(pin, GPIO_IN);
        }
        return 0;
    }
    for (pin = first; pin < first + 8 && pin < NUM_BANK0_GPIOS; pin++)
        gpio_put(pin, (info.wdata >> (pin - first)) & 1);
    return 0;
}
