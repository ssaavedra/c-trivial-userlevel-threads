
#include <stdio.h>
#include <string.h>

#include "yield.h"

#ifndef NUM_THREADS
#define NUM_THREADS 2
#endif

static void _slow();

#ifdef INCLUDE_SLOW
void slow() { _slow(); }
#endif

/* Global variables for thread implementation */
static int current_thread = 1;
static int main_frame; /* first activation record */
int thread_ebp[NUM_THREADS];
int thread_ret[NUM_THREADS];

void yield()
{
	int a[10];

	thread_ebp[current_thread] = a[12];
	thread_ret[current_thread] = a[13];

	current_thread = (current_thread % 2) + 1;

	a[12] = thread_ebp[current_thread];
	a[13] = thread_ret[current_thread];
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

