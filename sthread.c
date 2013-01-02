/**
 * Simple User-level Threads implementation in raw C compatible with IA-32 and
 * x86_64 instruction sets for the common caller/callee convention with frame
 * pointer enabled.
 *
 * This implementation will not work when compiled in GCC with the
 * -fno-frame-pointer compilation flag.
 *
 *  This file extends the capabilities of the sthread library, providining
 *  means for cancelling threads, comparing sthread_t values and handling
 *  data-structures, in a way compatible to the one defined by POSIX.1 with
 *  the use of pthread_once, pthread_key_create, pthread_key_delete,
 *  pthread_getspecific and pthread_setspecific.
 *
 *  We will setup automatically the first cancellation function, which is
 *  a wrapper for sthread_exit. This is our way to guarantee that sthread_exit
 *  will be called as the last part of the cancellation process, and it will
 *  do so from the thread's stack.
 *
 * Author: Santiago Saavedra <santiago.saavedral@udc.es>
 * Developed for Subject: Design and Implementation of Operating Systems.
 * Taught by: Ramón P. Otero and Alberto Illobre.
 *
 * Universidade da Coruña - 2012-2013
 */

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "sthread.h"

#ifndef MAX_CANCEL_FUNS
#define MAX_CANCEL_FUNS 10
#endif

static struct sthread_cancel_funs {
	struct sthread_cancel_info {
		sthread_fun_t f;
		sthread_arg_t a;
		sthread_t thread_ref;
	} info[MAX_CANCEL_FUNS];
	char used[MAX_CANCEL_FUNS];
} cleanup_functions[MAX_THREADS];

static struct sthread_key *root_sthread_key = NULL;

int _sthread_once_init_value = 0;

void sthread_cleanup_push(sthread_fun_t routine, sthread_arg_t arg)
{
	int i;
	sthread_t id = sthread_self();

	for(i = 0; i < MAX_CANCEL_FUNS && cleanup_functions[id].used[i]; i++);
	if(i == MAX_CANCEL_FUNS) {
		errno = ENOMEM;
		return;
	}

	cleanup_functions[id].used[i] = 1;
	cleanup_functions[id].info[i].f = routine;
	cleanup_functions[id].info[i].a = arg;
	root_sthread_key = NULL;

}

void sthread_cleanup_pop(int execute)
{
	int i;
	sthread_t id = sthread_self();
	for(i = MAX_CANCEL_FUNS; i > 1 && !cleanup_functions[id].used; i--);
	if(i == 0) return;

	if(execute)
		cleanup_functions[id].info[i].f(cleanup_functions[id].info[i].a);
	cleanup_functions[id].used[i] = 0;
}

int sthread_cancel(sthread_t thread)
{
	int i;
	for(i = MAX_CANCEL_FUNS; i > 0; i--)
	{
		if(!cleanup_functions[thread].used[i])
			continue;
		cleanup_functions[thread].info[i].f(cleanup_functions[thread].info[i].a);
	}
	return 0;
}

/* We include here the extern def., as this is not meant to be used outside the
 * cancellation scope, and there's no reason to include it in the yield.h
 * interface.
 */
extern void _sthread_force_exit(sthread_t thread, void *retval);
static void *_sthread_cancel_exitpoint(void *arg)
{
	struct sthread_cancel_info *info = (struct sthread_cancel_info *) arg;
	struct sthread_key *key = root_sthread_key;
	while(key != NULL) {
		key->destructor(key->specific_values[info->thread_ref]);
		key->specific_values[info->thread_ref] = NULL;
		key = key->nxt;
	}
	_sthread_force_exit(info->thread_ref, info->a);
}

void _sthread_cleanup_init(sthread_t thread)
{
	int i;
	struct sthread_cancel_info *info = cleanup_functions[thread].info;

	for(i = 1; i < MAX_CANCEL_FUNS; i++)
		cleanup_functions[thread].used[i] = 0;
	cleanup_functions[thread].used[0] = 1;

	info->thread_ref = thread;
	info->a = STHREAD_CANCELED;

	cleanup_functions[thread].info[0].f = _sthread_cancel_exitpoint;
	cleanup_functions[thread].info[0].a = &cleanup_functions[thread].info[0];
}

int sthread_equal(sthread_t t1, sthread_t t2)
{
	return t1 == t2;
}

int sthread_once(sthread_once_t *once_control, void (*init_routine)(void))
{
	if(*once_control)
		return 0;
	(*once_control)++;
	init_routine();
	return 0;
}


int sthread_key_create(sthread_key_t *key, void (*destructor)(void*))
{
	struct sthread_key *s;
	s = calloc(1, sizeof(s));
	if(s == NULL)
		return -ENOMEM;
	s->destructor = destructor;

	if(root_sthread_key == NULL) {
		root_sthread_key = s;
		s->nxt = NULL;
		s->prv = NULL;
	} else {
		s->nxt = root_sthread_key;
		s->nxt->prv = s;
		s->prv = NULL;
		root_sthread_key = s;
	}

	*key = s;
	return 0;
}

int sthread_key_delete(sthread_key_t key)
{
	int i;
	for(i = 0; i < MAX_THREADS; i++) {
		if(key->specific_values[i] != NULL) {
			key->destructor(key->specific_values[i]);
		}
	}
	if(key->prv != NULL) {
		key->prv->nxt = key->nxt;
		key->nxt->prv = key->prv;
	}
	free(key);
	return 0;
}

void *sthread_getspecific(sthread_key_t key)
{
	return key->specific_values[sthread_self()];
}

int sthread_setspecific(sthread_key_t key, void *value)
{
	key->specific_values[sthread_self()] = value;
	return 0;
}
	

void _auto_yield(int signal) {
	logf(LOG_SIGNAL, "signal\n");

	logf(LOG_SIGNAL, "Auto-yielding due to signal.\n");

	alarm(1);
	yield();

	logf(LOG_SIGNAL, "Returning to sighandler\n");
}


void sthread_auto_yield(int value) {
	static char yield_stack[8000];
	struct sigaction s;
	stack_t altstack;
	altstack.ss_sp = yield_stack;
	altstack.ss_flags = 0;
	altstack.ss_size = 32000;
	s.sa_handler = _auto_yield;
	s.sa_flags = SA_RESTART | SA_NODEFER;
	sigemptyset(&s.sa_mask);
	if(!value) {
		s.sa_handler = SIG_IGN;
	} else {
		sigaltstack(&altstack, NULL);
	}

	sigaction(SIGALRM, &s, NULL);
	alarm(1);
}
		

