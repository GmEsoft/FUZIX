/* Flash lockout is mocked; USB queue tests do not exercise hardware lockout. */
static inline bool flash_safe_execute_core_init(void) { return true; }
