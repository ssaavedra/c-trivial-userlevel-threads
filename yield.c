
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "yield.h"

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
#define DEFAULT_GROW_SIZE 64*1024


typedef void (*fun_t)(void *);
typedef void *arg_t;

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


int sthread_func(int tid, fun_t f, arg_t a)
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
	current_thread = 0;
	for(i = NOT_MORE(current_thread + 1, num_threads);
			thread_info[i].started < TS_NOTREADY
			&& i != current_thread; i = NOT_MORE(i + 1, num_threads))
	{
		status = 0;
		yield();
	}
	return status;
}


static void safeguard_launch() {
	void *a[10];

	thread_info[current_thread].started = TS_RUNNING;
	(*thread_info[current_thread].f_ptr)(thread_info[current_thread].arg);
	thread_info[current_thread].started = TS_FINISHED;

	/* Put an always-valid return address: sthread_start's (i.e. thread #0) */
	a[10] = thread_ebp[0];
	a[11] = thread_ret[0];
}

static void grow_stack_and_safeguard_launch(int size) {
	char stack_grower[size];
	stack_grower[size-1] = 0; /* If we couldn't grow up to here, this will segfault */
	safeguard_launch();
	/* This function does not: */ return;
}

void yield() 
{
	static int i;
	void * a[10];

	thread_ebp[current_thread] = a[10];
	thread_ret[current_thread] = a[11];

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

	a[10] = thread_ebp[current_thread];
	a[11] = thread_ret[current_thread];
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

