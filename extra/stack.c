#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../log.h"
#include "stack.h"

#define DEFAULT_STACK_THREADSIZE 8192


void sthread_stack_save(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	register void ***info;
	if(*reloc_information == NULL)
		*reloc_information = malloc(sizeof(void**) * 2);
	if(*reloc_information == NULL) {
		perror("sthread_stack_save");
		exit(7);
	}

	info = *reloc_information;

	if(cur_thread > -1) {
		info[0] = *ebp;
		info[1] = *ret;
	}
}

static void sthread_stack_safeguard_launch(sthread_fun_t ptr, sthread_arg_t arg)
{
	register void *retval = (*ptr)(arg);
	sthread_exit(retval);
}

static void sthread_stack_grow_and_launch(int size, sthread_fun_t ptr, sthread_arg_t arg)
{
	void *reservation[size];
	reservation[0] = 0;
	reservation[size - 1] = 0;
	sthread_stack_safeguard_launch(ptr, arg);
}

void sthread_stack_start(int cur_thread, void **reloc_information, void **ebp, void **ret, sthread_fun_t ptr, sthread_arg_t arg)
{
	register void ***info = *reloc_information;

	logf(LOG_DEBUG, "Spawning stuff..\n");

	if(info != NULL) {
		// Should not happen.
	}
	info = malloc(sizeof(void**) * 2);
	*reloc_information = info;

	info[0] = *ebp;
	info[1] = *ret;

	sthread_stack_grow_and_launch(DEFAULT_STACK_THREADSIZE, ptr, arg);
}


void sthread_stack_restore(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	register void ***info = *reloc_information;
	
	if(info == NULL) {
		sthread_stack_start(cur_thread, reloc_information, ebp, ret, sthread__get_function(cur_thread), sthread__get_arg(cur_thread));
	}

	logf(LOG_DEBUG, "Changing stuff..\n");

	*ebp = info[0];
	*ret = info[1];
}

void sthread_stack_exit(int cur_thread, void **reloc_information)
{
	free(*reloc_information);
}

