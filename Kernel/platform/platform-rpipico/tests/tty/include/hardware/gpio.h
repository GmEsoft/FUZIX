#include <kernel.h>
#define GPIO_FUNC_UART 2
void gpio_set_function(unsigned, unsigned);

void gpio_init(unsigned);
void gpio_set_dir(unsigned, bool);
void gpio_put(unsigned, bool);
