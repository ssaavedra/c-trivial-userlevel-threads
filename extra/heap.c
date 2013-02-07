#define _GNU_SOURCE
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

#include "../log.h"
#include "stack.h"
#include "heap.h"

#ifndef MIN
#define MIN(a,b) (a>b?b:a)
#endif

static struct sthread_heap_info *root_heap_info = NULL;


static int sthread_heap_enlarge()
{
	struct sthread_heap_info *ptr;
	off_t newsize;
	ptr = root_heap_info;

	while(ptr != NULL && ptr->threadnum != sthread_self())
		ptr = ptr->nxt;
	if(ptr == NULL) {
		logf(LOG_DEBUG, "ERROR: Heap instance not found.\n");
		exit(5);
	}

	if(ptr->stacksize < ptr->maxstacksize)
	{
		newsize = MIN(ptr->stacksize + DEFAULT_HEAP_GROW_SIZE, ptr->maxstacksize);

		ptr->stackptr = mremap(ptr->stackptr, ptr->stacksize, newsize, 0);
	}

}


static void sthread_heap_handle_segfault(int signum)
{
	sthread_t thread = sthread_self();
	if(sthread_heap_enlarge())
	{
		return;
	}
	sthread_exit(NULL);
}

static void sthread_heap_attach_signals()
{
	static char signal_stack[8000];
	struct sigaction s;
	stack_t altstack;
	altstack.ss_sp = signal_stack;
	altstack.ss_flags = 0;
	altstack.ss_size = 8000;
	s.sa_handler = sthread_heap_handle_segfault;
	s.sa_flags = SA_RESTART | SA_NODEFER | SA_ONSTACK;
	sigemptyset(&s.sa_mask);
	sigaltstack(&altstack, NULL);
	sigaction(SIGSEGV, &s, NULL);
}

static void sthread_heap_restore_signals()
{
	struct sigaction s;
	s.sa_handler = SIG_IGN;
	s.sa_flags = SA_RESTART | SA_NODEFER;
	sigemptyset(&s.sa_mask);
	sigaction(SIGSEGV, &s, NULL);
}

static struct sthread_heap_info *sthread_heap_find_and_link_info(int thread, void **reloc_information)
{
	struct sthread_heap_info *ptr;

	if(*reloc_information == NULL) {
		ptr = root_heap_info;
		while(ptr != NULL && ptr->threadnum != thread)
			ptr = ptr->nxt;
		if(ptr != NULL)
			*reloc_information = ptr;
	}
	if(*reloc_information == NULL) {
		if(root_heap_info == NULL) {
			root_heap_info = calloc(1, sizeof(struct sthread_heap_info));
			ptr = root_heap_info;
		} else {
			ptr = root_heap_info;
			while(ptr->nxt != NULL)
				ptr = ptr->nxt;
			ptr->nxt = calloc(1, sizeof(struct sthread_heap_info));
			ptr = ptr->nxt;
		}
		*reloc_information = ptr;
		ptr->nxt = NULL;
		ptr->threadnum = thread;
	}

	if( ((struct sthread_heap_info *) *reloc_information)->threadnum == thread)
		return (struct sthread_heap_info *) *reloc_information;
}

void sthread_heap_save(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	register struct sthread_heap_info *info;
	info = sthread_heap_find_and_link_info(cur_thread, reloc_information);

	sthread_heap_restore_signals();

	if(cur_thread > -1) {
		info->cur_ebp = *ebp;
		info->cur_ret = *ret;
	}
}

static stack_t sthread_old_altstack;

void sthread_heap_start_signal(int signal)
{
	void *retval;
	void *reloc_information;
	register struct sthread_heap_info *info;
	struct sigaction s;
	sigaltstack(&sthread_old_altstack, NULL);
	s.sa_handler = NULL;
	s.sa_flags = 0;
	sigaction(SIGUSR1, &s, NULL);

	logf(LOG_DEBUG, "Received signal. Spawning thread.\n");

	retval = (* (sthread__get_function(sthread_self())))(sthread__get_arg(sthread_self()));

	sthread_exit(retval);
}

void sthread_heap_start(int cur_thread, void **reloc_information, void **ebp, void **ret, sthread_fun_t ptr, sthread_arg_t arg)
{
	register struct sthread_heap_info *info;
	struct sigaction s;
	stack_t altstack;

	info = sthread_heap_find_and_link_info(cur_thread, reloc_information);

	logf(LOG_DEBUG, "Spawning stuff..\n");

	info->stackptr = mmap((void*) (long) (HEAP_BASEPTR + (cur_thread << 24)), DEFAULT_HEAP_THREADSIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS | MAP_GROWSDOWN | MAP_PRIVATE, -1, 0);
	info->stacksize = DEFAULT_HEAP_THREADSIZE;
	info->maxstacksize = info->stacksize;

	s.sa_handler = sthread_heap_start_signal;
	s.sa_flags = SA_ONSTACK | SA_NODEFER;
	altstack.ss_sp = info->stackptr;
	altstack.ss_size = info->stacksize;
	altstack.ss_flags = 0;
	sigaltstack(&altstack, &sthread_old_altstack);
	sigaction(SIGUSR1, &s, NULL);
	raise(SIGUSR1);
}


void sthread_heap_restore(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	register struct sthread_heap_info *info;
	info = sthread_heap_find_and_link_info(cur_thread, reloc_information);
	
	sthread_heap_attach_signals();
	if(info->stackptr == NULL) {
		sthread_heap_start(cur_thread, reloc_information, ebp, ret, sthread__get_function(cur_thread), sthread__get_arg(cur_thread));
	} else {

		logf(LOG_DEBUG, "Restoring stuff..\n");

		*ebp = info->cur_ebp;
		*ret = info->cur_ret;
	}
}

void sthread_heap_exit(int cur_thread, void **reloc_information)
{
	register struct sthread_heap_info *info;
	register struct sthread_heap_info *ptr;
	info = sthread_heap_find_and_link_info(cur_thread, reloc_information);
	if(info->nxt != NULL) {
		if(info != root_heap_info) {
			ptr = root_heap_info;
			while(ptr->nxt != info && ptr->nxt != NULL)
				ptr = ptr->nxt;
		}
		if(ptr->nxt == info)
			ptr->nxt = info->nxt;
	}
	munmap(info->stackptr, info->stacksize);
	free(info);
}

