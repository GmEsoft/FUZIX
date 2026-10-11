#include <kernel.h>
typedef uint64_t absolute_time_t;
absolute_time_t get_absolute_time(void);
absolute_time_t delayed_by_ms(absolute_time_t, uint);
int64_t absolute_time_diff_us(absolute_time_t, absolute_time_t);
void tight_loop_contents(void);
