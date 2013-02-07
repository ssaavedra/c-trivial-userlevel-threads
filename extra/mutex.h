#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "../sthread.h"

typedef struct {
	int value;
	sthread_t owner;
	int counter;
	int type;
} sthread_mutex_t;

typedef struct {
	int type;
} sthread_mutexattr_t;

#define STHREAD_MUTEX_NORMAL PTHREAD_MUTEX_NORMAL
#define STHREAD_MUTEX_ERRORCHECK PTHREAD_MUTEX_ERRORCHECK
#define STHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE
#define STHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_DEFAULT


sthread_mutex_t STHREAD_MUTEX_INITIALIZER = { .value = 0 };

int sthraed_mutexattr_init(sthread_mutexattr_t **attr);
int sthread_mutexattr_gettype(sthread_mutexattr_t *attr, int *type);
int sthread_mutexattr_settype(sthread_mutexattr_t *attr, int type);
int sthread_mutex_lock(sthread_mutex_t *mutex);
int sthread_mutex_unlock(sthread_mutex_t *mutex);
int sthread_mutex_trylock(sthread_mutex_t *mutex);
int sthread_mutex_destroy(sthread_mutex_t *mutex);
int sthread_mutex_init(sthread_mutex_t **mutex, const sthread_mutexattr_t *attr);

