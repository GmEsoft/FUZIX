#include <kernel.h>
void tusb_init(void);
void tud_task(void);
unsigned tud_cdc_n_write_available(unsigned);
unsigned tud_cdc_n_write_char(unsigned, char);
unsigned tud_cdc_n_write_flush(unsigned);
unsigned tud_cdc_n_available(unsigned);
int tud_cdc_n_read_char(unsigned);
