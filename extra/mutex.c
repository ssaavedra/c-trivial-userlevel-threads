#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "../sthread.h"

#include "mutex.h"

int sthread_mutexattr_init(sthread_mutexattr_t **attr)
{
	*attr = calloc(1, sizeof(sthread_mutexattr_t));
	if(*attr == NULL)
		return ENOMEM;
	return 0;
}

int sthread_mutexattr_destroy(sthread_mutexattr_t *attr)
{
	free(attr);
	return 0;
}

int sthread_mutexattr_gettype(sthread_mutexattr_t *attr, int *type)
{
	if(attr == NULL)
		return -EINVAL;
	if(type == NULL)
		return -EINVAL;
	return attr->type;
}

int sthread_mutexattr_settype(sthread_mutexattr_t *attr, int type)
{
	if(attr == NULL)
		return -EINVAL;
	
	attr->type = type;
	return 0;
}


static int sthread_mutex_lock_(sthread_mutex_t *mutex, int nonblock)
{
	if(mutex == NULL)
		return EINVAL;

	switch(mutex->type) {
		case STHREAD_MUTEX_NORMAL:
			break;
		case STHREAD_MUTEX_ERRORCHECK:
			if(mutex->value)
				return EBUSY;
			break;
		case STHREAD_MUTEX_RECURSIVE:
			if(mutex->counter) {
				if(sthread_self() == mutex->owner) {
					mutex->counter++;
					return 0;
				}
			}
			break;
	}

	while(!nonblock && mutex->value)
		yield();
	if(!nonblock)
		return EBUSY;

	while(__sync_lock_test_and_set(&mutex->value, 1));
	mutex->counter = 1;
	mutex->owner = sthread_self();
	return 0;
}

int sthread_mutex_lock(sthread_mutex_t *mutex) {
	return sthread_mutex_lock_(mutex, 0);
}

int sthread_mutex_unlock(sthread_mutex_t *mutex)
{
	if(mutex == NULL)
		return EINVAL;

	switch(mutex->type)
	{
		case STHREAD_MUTEX_NORMAL:
			break;
		case STHREAD_MUTEX_ERRORCHECK:
			if(!mutex->value)
				return EINVAL;
			if(mutex->owner != sthread_self())
				return EPERM;
			break;
		case STHREAD_MUTEX_RECURSIVE:
			if(mutex->counter > 1) {
				if(mutex->owner != sthread_self())
					return EPERM;
				mutex->counter--;
				return 0;
			}
			break;
	}

	__sync_lock_release(&mutex->value);
	mutex->counter = 0;
}

int sthread_mutex_trylock(sthread_mutex_t *mutex)
{
	return sthread_mutex_lock_(mutex, 0);
}

int sthread_mutex_destroy(sthread_mutex_t *mutex)
{
	if(sthread_mutex_trylock(mutex))
		if(mutex)
			free(mutex);
}

int sthread_mutex_init(sthread_mutex_t **mutex, const sthread_mutexattr_t *attr)
{
	*mutex = calloc(1, sizeof(sthread_mutex_t));
	// Unneeded because of calloc(3) // mutex.value = 0;
}

