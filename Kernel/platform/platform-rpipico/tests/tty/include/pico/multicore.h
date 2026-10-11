void multicore_launch_core1(void (*entry)(void));

/* Core startup/lockout is mocked; these tests exercise USB queues and wakeups. */
static inline void multicore_lockout_victim_init(void) {}
static inline bool multicore_lockout_victim_is_initialized(unsigned core) { (void)core; return true; }
