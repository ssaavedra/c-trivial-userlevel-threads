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


#ifndef NUM_THREADS
#define NUM_THREADS 4
#endif

static void _slow();

#ifdef INCLUDE_SLOW
void slow() { _slow(); }
#endif

/* Thread #0 is the "main" process. We'll return to that at sthread_init(), so we
 * don't schedule it at NOT_MORE 
 */
#define NOT_MORE(a, b) (a < b ? a : 1)

/* Create a stack with 64k */
#define DEFAULT_GROW_SIZE 1024*1024


/* Tiny logger */
enum log_level { LOG_DEBUG };

#ifdef DEBUG
#if (DEBUG > 0)
#define logf(lvl, ...) do { if(DEBUG > lvl) { fprintf(stderr, __VA_ARGS__); } } while(0)
#endif
#else
#define logf(lvl, ...)
#endif



enum thread_status { TS_NOTSPAWNED = 0, TS_RUNNING = 1, TS_NOTREADY = 0x10, TS_NOTHINGYET = 0x11, TS_FINISHED = 0x18};

/* Global variables for thread implementation */
static int num_threads = 2;
static int current_thread = 0;
static int main_frame; /* first activation record */
void *thread_ebp[NUM_THREADS + 1];
void *thread_ret[NUM_THREADS + 1];

static struct sthread_info {
	void (*f_ptr)(void *);
	void *arg;
	int started;
} thread_info[NUM_THREADS];


int sthread_func(const int tid, const fun_t f, const arg_t a)
{
	if(thread_info[tid].started == TS_RUNNING)
		return EPERM;

	thread_info[tid].f_ptr = f;
	thread_info[tid].arg = a;
	thread_info[tid].started = TS_NOTSPAWNED;
	return 0;
}

int sthread_init(const int n)
{
	int i;

	if(n > NUM_THREADS) {
		return EINVAL;
	}

	num_threads = n + 1;

	for(i = 0; i < num_threads; i++)
	{
		thread_info[i].f_ptr = NULL;
		thread_info[i].started = TS_NOTHINGYET;
	}
	return 0;
}

int sthread_start()
{
	int i, status = 1;
	i = NOT_MORE(current_thread + 1, num_threads);

	while(thread_info[i].started < TS_NOTREADY) {
		status = 0;
		current_thread = 0;
		yield();
		i = NOT_MORE(current_thread + 1, num_threads);
		logf(LOG_DEBUG, "We are at thread %d and its status will be checked (%d)\n", i, thread_info[i].started);
	}
	logf(LOG_DEBUG, "No more threads!\n");
	for(i = 1; i < num_threads; i++) {
		logf(LOG_DEBUG, "Thread %d status: %d\n", i, thread_info[i].started);
	}
	return status;
}


static void safeguard_launch() {
	void *a[10];

	thread_info[current_thread].started = TS_RUNNING;
	(*thread_info[current_thread].f_ptr)(thread_info[current_thread].arg);
	thread_info[current_thread].started = TS_FINISHED;

	/* Put an always-valid return address: sthread_start's (i.e. thread #0) */
	a[EBP_ADDR_IDX] = thread_ebp[0];
	a[RET_ADDR_IDX] = thread_ret[0];
}

static void grow_stack_and_safeguard_launch(int size) {
	char stack_grower[size];
	stack_grower[size-1] = 0; /* If we couldn't grow up to here, this will segfault */
	safeguard_launch();
	/* This function does not: */ return;
}

static void _yield() 
{
	static int i;
	void * a[10];

	thread_ebp[current_thread] = a[EBP_ADDR_IDX];
	thread_ret[current_thread] = a[RET_ADDR_IDX];

	/* We loop exactly once through our circular thread_info list.
	 * If we find a ready thread (or loop the list completely) we stop.
	 */
	for(i = NOT_MORE(current_thread + 1, num_threads); thread_info[i].started & TS_NOTREADY && i != current_thread; i = NOT_MORE(i + 1, num_threads));
	if(i == current_thread) {
		/* No more threads. */
		errno = EBUSY;
		return;
	}
	current_thread = i;

	/*	
	do {
		current_thread = NOT_MORE(current_thread + 1, num_threads);
	} while(thread_info[current_thread].started & TS_NOTREADY);
	*/

	if(!thread_info[current_thread].started) {
		/* Grow the stack to fit a new thread  and launch it */
		grow_stack_and_safeguard_launch(DEFAULT_GROW_SIZE);
		/* This is not a return point. The former will never return.
		 * Having the "return" word written, however, allows us to make
		 * a breakpoint in order to check this statement.
		 */
		return;
	}

	a[EBP_ADDR_IDX] = thread_ebp[current_thread];
	a[RET_ADDR_IDX] = thread_ret[current_thread];

}

void yield() {
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
}



#ifndef SLOW_LOOP
#define SLOW_LOOP 10000000
#endif
void slow()
{
	unsigned long i;
	for(i = 0; i < SLOW_LOOP; i++)
		42.*96179.;
}

