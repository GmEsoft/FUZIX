#ifndef TEST_GPIO_H
#define TEST_GPIO_H
#include <stdbool.h>
#include <stdint.h>
#define GPIO_IN 0
#define GPIO_OUT 1
#define GPIO_FUNC_SPI 1
#define GPIO_FUNC_SIO 5
#define GPIO_IRQ_EDGE_FALL 4
typedef void (*gpio_irq_callback_t)(unsigned, uint32_t);
void gpio_init(unsigned);
void gpio_set_dir(unsigned,bool);
void gpio_pull_up(unsigned);
void gpio_set_function(unsigned,unsigned);
void gpio_put(unsigned,bool);
bool gpio_get(unsigned);
void gpio_set_irq_callback(gpio_irq_callback_t);
void gpio_set_irq_enabled(unsigned,unsigned,bool);
#endif
