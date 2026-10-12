#ifndef TEST_STDLIB_H
#define TEST_STDLIB_H
#include <stdint.h>
#include <stdbool.h>
#include <hardware/irq.h>
#include <kernel.h>
typedef uint64_t absolute_time_t;
void update_us_since_boot(absolute_time_t *,uint64_t);
uint64_t to_us_since_boot(absolute_time_t);
uint64_t time_us_64(void);
void hardware_alarm_claim(unsigned);
void hardware_alarm_set_callback(unsigned,void (*)(unsigned));
void hardware_alarm_force_irq(unsigned);
bool hardware_alarm_set_target(unsigned,absolute_time_t);
#endif
