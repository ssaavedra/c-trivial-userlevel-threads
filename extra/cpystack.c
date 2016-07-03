#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../log.h"
#include "cpystack.h"

#define DEFAULT_CPYSTACK_THREADSIZE 8192

struct sthread_cpystack_info {
	void *begin;
	void *copy;
	off_t size;
	void *ebp;
	void *ret;
};

struct sthread_cpystack_info *root_info;

void sthread_cpystack_save(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	struct sthread_cpystack_info *info;
	if(*reloc_information == NULL)
		*reloc_information = malloc(sizeof(struct sthread_cpystack_info));
	if(*reloc_information == NULL) {
		perror("sthread_cpystack_save");
		exit(7);
	}

	info = *reloc_information;

	if(cur_thread > -1) {
		info->ebp = *ebp;
		info->ret = *ret;
		info->size = ebp - info->begin;

		if(info->copy != NULL) {
			info->copy = realloc(info->size);
		}

		memcpy(info->copy, ebp);
	}
}

static void sthread_cpystack_safeguard_launch(sthread_fun_t ptr, sthread_arg_t arg)
{
	register void *retval;

	retval = (*ptr)(arg);
	sthread_exit(retval);
}

static void sthread_cpystack_grow_and_launch(int size, sthread_fun_t ptr, sthread_arg_t arg)
{
	void *reservation[size];
	reservation[0] = 0;
	reservation[size - 1] = 0;
	sthread_cpystack_safeguard_launch(ptr, arg);
}

void sthread_cpystack_start(int cur_thread, void **reloc_information, void **ebp, void **ret, sthread_fun_t ptr, sthread_arg_t arg)
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

	sthread_cpystack_grow_and_launch(DEFAULT_cpystack_THREADSIZE, ptr, arg);
}


void sthread_cpystack_restore(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	register void ***info = *reloc_information;
	
	if(info == NULL) {
		sthread_cpystack_start(cur_thread, reloc_information, ebp, ret, sthread__get_function(cur_thread), sthread__get_arg(cur_thread));
	}

	logf(LOG_DEBUG, "Changing stuff..\n");

	*ebp = info[0];
	*ret = info[1];
}

void sthread_cpystack_exit(int cur_thread, void **reloc_information)
{
	free(*reloc_information);
}

