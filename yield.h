/**
 * Simple User-level Threads implementation in raw C compatible with IA-32 and
 * x86_64 instruction sets for the common caller/callee convention with frame
 * pointer enabled.
 *
 * This implementation will not work when compiled in GCC with the
 * -fno-frame-pointer compilation flag.
 *
 *  File: yield.h
 *  Contains headers and typedefs available to user programs.
 *
 * Author: Santiago Saavedra <santiago.saavedral@udc.es>
 * Developed for Subject: Design and Implementation of Operating Systems.
 * Taught by: Ramón P. Otero and Alberto Illobre.
 *
 * Universidade da Coruña - 2012-2013
 */

#ifndef _YIELD_H_
#define _YIELD_H_

/**
 * This define allows the user to know how yield.so was compiled.
 * Default: 0 (no yield on create)
 */
#define DEFAULT_YIELD_ON_CREATE 0

/**
 * This flag's meaning alters shtread_create when called from within this
 * header file, which will expand the macro into a call to sthread_create4 with
 * the 4th argument set to this flag's value.
 */
#ifndef YIELD_ON_CREATE
#define YIELD_ON_CREATE 0
#endif

/**
 * You can specify the max threads that may run concurrently.
 * Increasing this value enlarges static variables defined in the .data zone.
 */
#ifndef MAX_THREADS
#define MAX_THREADS 16 
#endif

/**
 * This is the type for thread functions.
 */
typedef void *(*sthread_fun_t)(void *);

/**
 * sthreads may receive a single void* argument.
 */
typedef void *sthread_arg_t;

typedef int sthread_t;

/**
 * This function allows the user to change the context and begin execution of
 * another thread. It should be called from within the different threads to
 * create context-switching points.
 */
void sthread_yield(int n);
#define yield() sthread_yield(0)

/**
 * This was created for internal usage. It was used to test that all threads
 * were executed before returning to main().
 */
int sthread_wait_all();

/**
 * This initializes the main thread. You don't need to call this function, as
 * it is called internally by sthread_create4.
 */
int sthread_init();

/**
 * This function creates a thread. As these threads are run from user-level,
 * we add an argument that is not available in POSIX.1 pthreads, which is
 * whether we should immediately change the context to the newly created thread
 * or instead wait until there's an explicit yield() (or maybe another
 * sthread_create4 call).
 */
int sthread_create4(sthread_t *thread, sthread_fun_t start_routine, sthread_arg_t arg, int yield_now);
/**
 * sthread_create remains as a means of compatibility with POSIX.
 * We have sthread_create4 which does the same, but specifying whether to yield
 * or not.
 * If you include this function via the dynamic loader (dlsym), it will yield
 * or not depending on how the yield.so was compiled (namely, it will yield if
 * the flag -DDEFAULT_YIELD_ON_CREATE=1 was used).
 *
 * However, if you include this header file, as the function is also defined as
 * a macro, it will yield if you specify this flag in *your* program's
 * compilation. Actually, it does a call to sthread_create4.
 */
int sthread_create(sthread_t *thread, sthread_fun_t start_routine, sthread_arg_t arg);
#define sthread_create(t, f, a) sthread_create4(t, f, a, YIELD_ON_CREATE)

void sthread_exit(void *retval);
int sthread_join(sthread_t thread, void **retval);

/**
 * Obtains the current thread id
 */
sthread_t sthread_self();

#endif

