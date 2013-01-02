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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "yield.h"

#ifndef EBP_ADDR_IDX

#if __LP64__
#define EBP_ADDR_IDX 12
#define RET_ADDR_IDX 13
#else
#define EBP_ADDR_IDX 12
#define RET_ADDR_IDX 13
#endif

#endif

#define NOT_MORE(a, b) (a < b ? a : 0)

/* Create a stack with 64k */
#define DEFAULT_GROW_SIZE 1024*1024


#define DEBUG 10



enum thread_status { TS_NOTSPAWNED = 0, TS_RUNNING = 1, TS_NOTREADY = 0x10, TS_NOTHINGYET = 0x11, TS_FINISHED = 0x18, TS_JOINED = 0x1C};

/* Global variables for thread implementation */
int _sthread_num_threads = 1;
static int _initd = 0;
int current_thread = 0;
static void *thread_ebp[MAX_THREADS + 1];
static void *thread_ret[MAX_THREADS + 1];

static struct sthread_info {
	sthread_fun_t f_ptr;
	sthread_arg_t arg;
	int started;
	void *retval;
} thread_info[MAX_THREADS];

/* Get an available thread. If we can reuse an already-joined thread, we'll use
 * that.
 */
static int _alloc_avail_thread() {
	int i;
	for(i = 0; i < _sthread_num_threads; i++) {
		if(thread_info[i].started == TS_NOTHINGYET || thread_info[i].started == TS_JOINED)
		{
			thread_info[i].started = TS_NOTHINGYET;
			return i;
		}
	}
	return _sthread_num_threads++;
}


int sthread_create4(sthread_t *tid, const sthread_fun_t f, const sthread_arg_t a, int yield_now)
{
	if(!_initd) sthread_init();

	*tid = _alloc_avail_thread();

	if(thread_info[*tid].started == TS_RUNNING)
		return EAGAIN;

	thread_info[*tid].f_ptr = f;
	thread_info[*tid].arg = a;
	thread_info[*tid].started = TS_NOTSPAWNED;
	if(yield_now)
		yield();
	return 0;
}

int shtread_create(sthread_t *tid, const sthread_fun_t f, const sthread_arg_t a)
{
	return sthread_create4(tid, f, a, DEFAULT_YIELD_ON_CREATE);
}

int sthread_init()
{
	_initd = 1;
	_sthread_num_threads = 1;
	thread_info[0].f_ptr = NULL;
	thread_info[0].started = TS_RUNNING;
	return 0;
}

int sthread_wait_all()
{
	int i, status = 1;
	i = (current_thread + 1) % _sthread_num_threads;
	if(i == 0) {
		i = (i + 1) % _sthread_num_threads;
	}
	if(i == 0) {
		return ENOENT;
	}

	while(thread_info[i].started < TS_NOTREADY) {
		status = 0;
		current_thread = 0;
		yield();
		i = (current_thread + 1) % _sthread_num_threads;
		if(i == 0) i = (i + 1) % _sthread_num_threads;
		if(i == 0) break;
		logf(LOG_DEBUG, "We are at thread %d and its status will be checked (%d)\n", i, thread_info[i].started);
	}
	logf(LOG_DEBUG, "No more threads!\n");
	for(i = 1; i < _sthread_num_threads; i++) {
		logf(LOG_DEBUG, "Thread %d status: %d\n", i, thread_info[i].started);
	}
	return status;
}

int sthread_join(sthread_t thread, void **retval)
{
	if(thread_info[thread].started == TS_NOTSPAWNED)
		return ENOENT;
	while(thread_info[thread].started != TS_FINISHED) {
		yield();
	}
	*retval = thread_info[thread].retval;
	thread_info[thread].started = TS_JOINED;
	return 0;
}

void sthread_exit(void *retval)
{
	thread_info[current_thread].retval = retval;
	thread_info[current_thread].started = TS_FINISHED;
	yield();
	/* NORETURN */
}

void _sthread_force_exit(sthread_t thread, void *retval)
{
	thread_info[thread].retval = retval;
	thread_info[thread].started = TS_FINISHED;
}


static void safeguard_launch() {
	void *a[10];
	void *retval;

	thread_info[current_thread].started = TS_RUNNING;
	logf(LOG_DEBUG, "Spawning thread %d\n", current_thread);
	retval = 
		(*thread_info[current_thread].f_ptr)(thread_info[current_thread].arg);
	logf(LOG_DEBUG, "Gathering output from thread %d\n", current_thread);
	logf(LOG_DEBUG, "Value = %d. Putting into struct.\n", retval);
	thread_info[current_thread].retval = retval;

	thread_info[current_thread].started = TS_FINISHED;
	logf(LOG_DEBUG, "Thread %d killed.\n", current_thread);
	logf(LOG_DEBUG, "Returning to where we were called <%p>.\n", thread_ret[0]);

	/* Put an always-valid return address: (i.e. thread #0) */
	current_thread = 0;
	/* In this case, the values are deeper in the stack because we are
	 * executing a function which has a return value (the f_ptr pointer).
	 */
	a[EBP_ADDR_IDX + 1] = thread_ebp[0];
	a[RET_ADDR_IDX + 1] = thread_ret[0];
	/* NORETURN: Jumps to thread#0 */
}

static void grow_stack_and_safeguard_launch(int size) {
	char stack_grower[size];
	stack_grower[size-1] = 0; /* If we couldn't grow up to here, this will segfault */
	safeguard_launch();
	/* NORETURN: safeguard_launch */
}


static char _yield_mutex = 0;
static void _yield() 
{
	static int i;
	void * a[10];

	if(_yield_mutex) {
		logf(LOG_DEBUG, "RARE! Exiting from nested _yield!\n");
		errno = EAGAIN;
		return;
	}
	while(__sync_lock_test_and_set(&_yield_mutex, 1));

	if(current_thread > -1) {
		thread_ebp[current_thread] = a[EBP_ADDR_IDX];
		thread_ret[current_thread] = a[RET_ADDR_IDX];
	}

	/* We loop exactly once through our circular thread_info list.
	 * If we find a ready thread (or loop the list completely) we stop.
	 */
	for(i = NOT_MORE(current_thread + 1, _sthread_num_threads); thread_info[i].started & TS_NOTREADY && i != current_thread; i = NOT_MORE(i + 1, _sthread_num_threads));
	if(i == current_thread) {
		__sync_lock_release(&_yield_mutex);
		/* No more threads. */
		errno = EBUSY;
		return;
	}
	current_thread = i;

	/*	
	do {
		current_thread = NOT_MORE(current_thread + 1, _sthread_num_threads);
	} while(thread_info[current_thread].started & TS_NOTREADY);
	*/

	if(!thread_info[current_thread].started) {
		/* Grow the stack to fit a new thread  and launch it */
		__sync_lock_release(&_yield_mutex);
		grow_stack_and_safeguard_launch(DEFAULT_GROW_SIZE);
		/* This is not a return point. The former will never return.
		 * Having the "return" word written, however, allows us to make
		 * a breakpoint in the debugger so we can check this statement.
		 */
		return;
	}

	a[EBP_ADDR_IDX] = thread_ebp[current_thread];
	a[RET_ADDR_IDX] = thread_ret[current_thread];
}

void sthread_yield(int n) {
	/* This creates an artificial frame atop, in order
	 * to restore the %sp value too. 
	 * If we didn't call this, %sp would have any other value.
	 *
	 * This is the same than adding something like
	 * push %esp
	 * mov %esp,%ebp
	 * before the real function (push %esp ...) starts
	 * and
	 * mov %ebp,%esp  # \
	 * pop %esp       # -- or IA-32 leave
	 * push &&yield_return
	 * ret
	 * ret
	 */
	_yield();
	__sync_lock_release(&_yield_mutex);
}

sthread_t sthread_self(void)
{
	return current_thread;
}


#ifndef SLOW_LOOP
#define SLOW_LOOP 10000000
#endif
void _slow()
{
	unsigned long i;
	for(i = 0; i < SLOW_LOOP; i++)
		42.*96179.;
}

