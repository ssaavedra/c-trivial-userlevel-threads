
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../yield.h"

struct sthread_heap_info {
	void *baseptr;
	unsigned stacksize;
	jmp_buf env;
};

static void safeguard_launch();

void sthread_heap_save(int tid, void **information, void **ebp, void **ret)
{
	struct sthread_heap_info *i;
	i = *information;

}

void sthread_heap_init_handler(int signal, void *altstack, size_t size)
{
}

void sthread_heap_clear_handler(int snum)
{
	signal(snum, SIG_DFL);
}

void sthread_heap_restore(int tid, void **information)
{
	void *newpage;
	struct sthread_heap_info *i;
	i = *information;

	if(*information == NULL) {
		// Launching for the first time
		*information = calloc(1, sizeof(struct sthread_heap_info));
		i->stacksize = 8*1024*1024;
		newpage = mmap(NULL, i->stacksize, PROT_READ|PROT_WRITE, MAP_GROWSDOWN|MAP_SHARED|MAP_ANONYMOUS, 0, 0);
		i->baseptr = newpage;
		
		sthread_heap_init_handler(SIGUSR1, newpage, i->stacksize);
		raise(SIGUSR1);
		sthread_heap_clear_handler(SIGUSR1);
		// In the alt stack...
		// put handler for (SIGUSR1, sthread_heap_startthread);
		// and then raise(SIGUSR1);
	} else {
		longjmp(i->env, 1);
	}

}

