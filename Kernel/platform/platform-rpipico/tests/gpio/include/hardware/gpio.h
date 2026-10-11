#include <stdbool.h>
#define GPIO_FUNC_SIO 5
#define GPIO_IN 0
#define GPIO_OUT 1
unsigned gpio_get_function(unsigned);
bool gpio_get_dir(unsigned);
bool gpio_get_out_level(unsigned);
bool gpio_get(unsigned);
void gpio_set_function(unsigned, unsigned);
void gpio_set_dir(unsigned, bool);
void gpio_put(unsigned, bool);
void gpio_init(unsigned);
