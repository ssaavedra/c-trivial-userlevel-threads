/**
 * Simple User-level Threads implementation in raw C compatible with IA-32 and
 * x86_64 instruction sets for the common caller/callee convention with frame
 * pointer enabled.
 *
 * This implementation will not work when compiled in GCC with the
 * -fno-frame-pointer compilation flag.
 *
 * Author: Santiago Saavedra <santiago.saavedral@udc.es>
 * Developed for Subject: Design and Implementation of Operating Systems.
 * Taught by: Ramón P. Otero and Alberto Illobre.
 *
 * Universidade da Coruña - 2012-2013
 */

#ifndef _STHREAD_H_
#define _STHREAD_H_
#include "yield.h"

/**
 * This flag is returned on sthread_join(thread, *ret) when you perform a
 * cancellation via sthread_cancel(thread)
 */
#define STHREAD_CANCELED ((void*)0xFFFF0000)

typedef int sthread_once_t;
typedef struct sthread_key {
	void (*destructor)(void *);
	void *specific_values[MAX_THREADS];
	struct sthread_key *nxt;
	struct sthread_key *prv;
} *sthread_key_t;

#define STHREAD_ONCE_INIT _sthread_once_init_value

/**
 * sthread_cancel permits cancellation of threads, in a means similar to those
 * provided by POSIX.1, i.e., the following steps occur:
 *   1. Cancellation clean-up handlers are popped (in the reverse of the order
 *   in which they were pushed) and called.
 *   2. Thread-specific data destructors are called, in an unspecified order.
 *   3. The thread is terminated.
 */
int sthread_cancel(sthread_t thread);

/**
 * sthread_cleanup_push pushes a routine to the cleanup stack, which will be
 * called whenever the thread is cancelled (if it ever is).
 */
void sthread_cleanup_push(sthread_fun_t routine, sthread_arg_t arg);

/**
 * sthread_cleanup_pop removes the last routine pushed onto the cleanup stack,
 * and it can be called if the execute flag is set to non-zero.
 */
void sthread_cleanup_pop(int execute);

/**
 * This function remains here for compatibility with pthreads. Actually,
 * sthreads can be compared via the arithmetic "=" operator.
 */
int sthread_equal(sthread_t t1, sthread_t t2);

/* Data structures handling */
int sthread_once(sthread_once_t *once_control, void (*init_routine)(void));

/**
 * creates a thread-specific data key visible to all threads in the process.
 */
int sthread_key_create(sthread_key_t *key, void (*destructor)(void*));

/**
 * Deletes a thread-specific data key. Any access to this data key from any
 * thread may break the program.
 */
int sthread_key_delete(sthread_key_t key);

/**
 * Retrieves a specific value for the current thread for the provided key.
 * If the value is not yet assigned, NULL is returned.
 */
void *sthread_getspecific(sthread_key_t key);

/**
 * Sets a specific value for the current thread and the provided key.
 */
int sthread_setspecific(sthread_key_t key, void *value);

/* Overload sthread_create in a nasty way to support sthread_cleanup functions
 */
extern void _sthread_cleanup_init(sthread_t);
#define sthread_create4(t, f, a, y) do { sthread_create4(t, f, a, 0); _sthread_cleanup_init(*(t)); if(y) yield(); } while(0)


/**
 * This library uses alarm to implement an automatic thread scheduling, so
 * that even when there are no yield points explicitly created in the
 * threads, they will get preempted whenever a signal can arrive.
 */
void sthread_auto_yield(int value);

#endif

