#include <kernel.h>
typedef struct { unsigned locked; } critical_section_t;
void critical_section_init(critical_section_t *);
void critical_section_enter_blocking(critical_section_t *);
void critical_section_exit(critical_section_t *);
