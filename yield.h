/**
 * Used to get yield()
 */


typedef void (*fun_t)(void *);
typedef void *arg_t;

void yield();
int sthread_init(const int n);
int sthread_func(const int i, const fun_t worker, const arg_t arg);

