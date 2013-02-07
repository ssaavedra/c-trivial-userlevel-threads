#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "../log.h"
#include "stack.h"
#include "file.h"

#ifndef MIN
#define MIN(a,b) (a>b?b:a)
#endif

static struct sthread_file_info *root_file_info = NULL;

static struct sthread_file_info *sthread_file_find_and_link_info(int thread, void **reloc_information)
{
	struct sthread_file_info *ptr;

	if(*reloc_information == NULL) {
		ptr = root_file_info;
		while(ptr != NULL && ptr->threadnum != thread)
			ptr = ptr->nxt;
		if(ptr != NULL)
			*reloc_information = ptr;
	}
	if(*reloc_information == NULL) {
		if(root_file_info == NULL) {
			root_file_info = calloc(1, sizeof(struct sthread_file_info));
			ptr = root_file_info;
		} else {
			ptr = root_file_info;
			while(ptr->nxt != NULL)
				ptr = ptr->nxt;
			ptr->nxt = calloc(1, sizeof(struct sthread_file_info));
			ptr = ptr->nxt;
		}
		*reloc_information = ptr;
		ptr->nxt = NULL;
		ptr->threadnum = thread;
		ptr->fd = open(tmpnam(NULL), O_RDWR | O_CREAT, 0644);
	}

	if( ((struct sthread_file_info *) *reloc_information)->threadnum == thread)
		return (struct sthread_file_info *) *reloc_information;
}

void sthread_file_save(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	register struct sthread_file_info *info;
	info = sthread_file_find_and_link_info(cur_thread, reloc_information);

	if(cur_thread > -1) {
		info->cur_ebp = *ebp;
		info->cur_ret = *ret;
		msync(info->stackptr, info->stacksize, MS_SYNC);
		info->dirty = 0;
	}
}

static stack_t sthread_old_altstack;

void sthread_file_start_signal(int signal)
{
	void *retval;
	void *reloc_information;
	register struct sthread_file_info *info;
	struct sigaction s;
	sigaltstack(&sthread_old_altstack, NULL);
	s.sa_handler = NULL;
	s.sa_flags = 0;
	sigaction(SIGUSR1, &s, NULL);

	logf(LOG_DEBUG, "Received signal. Spawning thread.\n");

	retval = (* (sthread__get_function(sthread_self())))(sthread__get_arg(sthread_self()));

	sthread_exit(retval);
}

void sthread_file_start(int cur_thread, void **reloc_information, void **ebp, void **ret, sthread_fun_t ptr, sthread_arg_t arg)
{
	register struct sthread_file_info *info;
	struct sigaction s;
	stack_t altstack;

	info = sthread_file_find_and_link_info(cur_thread, reloc_information);

	logf(LOG_DEBUG, "Spawning stuff..\n");

	ftruncate(info->fd, FILE_STACKSIZE);
	info->stackptr = mmap((void*) (long) (FILE_BASEPTR), FILE_STACKSIZE, PROT_READ|PROT_WRITE, MAP_SHARED, info->fd, 0);
	info->stacksize = FILE_STACKSIZE;
	info->dirty = 1;

	s.sa_handler = sthread_file_start_signal;
	s.sa_flags = SA_ONSTACK | SA_NODEFER;
	altstack.ss_sp = info->stackptr;
	altstack.ss_size = info->stacksize;
	altstack.ss_flags = 0;
	sigaltstack(&altstack, &sthread_old_altstack);
	sigaction(SIGUSR1, &s, NULL);
	raise(SIGUSR1);
}


void sthread_file_restore(int cur_thread, void **reloc_information, void **ebp, void **ret)
{
	register struct sthread_file_info *info;
	info = sthread_file_find_and_link_info(cur_thread, reloc_information);
	

	if(info->stackptr == NULL) {
		sthread_file_start(cur_thread, reloc_information, ebp, ret, sthread__get_function(cur_thread), sthread__get_arg(cur_thread));
	} else {

		logf(LOG_DEBUG, "Restoring stuff..\n");

		info->stackptr = mmap((void*) (long) (FILE_BASEPTR), FILE_STACKSIZE, PROT_READ|PROT_WRITE, MAP_SHARED, info->fd, 0);
		*ebp = info->cur_ebp;
		*ret = info->cur_ret;
	}
}

void sthread_file_exit(int cur_thread, void **reloc_information)
{
	register struct sthread_file_info *info;
	register struct sthread_file_info *ptr;
	info = sthread_file_find_and_link_info(cur_thread, reloc_information);
	if(info->nxt != NULL) {
		if(info != root_file_info) {
			ptr = root_file_info;
			while(ptr->nxt != info && ptr->nxt != NULL)
				ptr = ptr->nxt;
		}
		if(ptr->nxt == info)
			ptr->nxt = info->nxt;
	}
	munmap(info->stackptr, info->stacksize);
	free(info);
}


static char sthread_file_yield_stack[5000];


void sthread_file_protect_yield(int cur_thread, void **reloc_information)
{
	register struct sthread_file_info *info;
	struct sigaction s;
	stack_t altstack;

	s.sa_handler = sthread_file_protect_yield_signal;
	s.sa_flags = SA_ONSTACK | SA_NODEFER;
	altstack.ss_sp = malloc(5000);
	altstack.ss_size = 5000;
	altstack.ss_flags = 0;
	sigaltstack(&altstack, &sthread_old_altstack);
	sigaction(SIGUSR2, &s, NULL);
	raise(SIGUSR2);
}

