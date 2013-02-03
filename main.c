#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "yield.h"
#include "sthread.h"

void slow() { usleep(500000); }

void *t1(void *arg)
{
	int i;

	if(arg == NULL) {
		i = 0;
		printf("[T1]: Calling recursively.\n");
		t1(&i);
		printf("[T1]: END.\n");
		return NULL;
	}

	if(*(int*)arg < 2) {
		(*(int*)arg)++;
		printf("[T1]: Calling recursively (arg=%d).\n", *(int*)arg);
		yield();
		t1(arg);
		if(arg == NULL) {
			printf("[T1]: ARG IS NULL, WTF?!\n");
			return NULL;
		}
		printf("[T1]: END (arg=%d).\n", (*(int*)arg)--);
		return NULL;
	}

	printf("t1 started!\n");
	for(i = 0; i < 10; i++) {
		printf("I am thread 1 and I'm looping at %d\n", i);
		slow();
		yield();
	}
	printf("t1 ended!\n");
	return NULL;
}

void *t2(void *arg)
{
	int i;

	if(arg == NULL) {
		i = 0;
		printf("[T2]: Calling recursively.\n");
		t2((int*)&i);
		printf("[T2]: END.\n");
		return NULL;
	}

	if(*(int*)arg < 2) {
		(*(int*)arg)++;
		printf("[T2]: Calling recursively (arg=%d).\n", *(int*)arg);
		//yield();
		t2(arg);
		if(*(&arg) == NULL) {
			printf("[T2]: ARG IS NULL, WTF?!\n");
			return NULL;
		}
		printf("[T2]: END (arg=%d).\n", (*(int*)arg)--);
		return NULL;
	}


	printf("t2 started!\n");
	for(i = 10; i < 20; i++) {
		printf("I am thread 2 and I'm looping at %d\n", i);
		slow();
		yield();
	}
	printf("t2 ended!\n");
	return NULL;
}

int main()
{
	sthread_t th1, th2;
	int r1, r2;

	sthread_init(2);
	/*
	sthread_func(1, t1, NULL);
	sthread_func(2, t2, NULL);
	sthread_start();
	*/

	sthread_auto_yield(0);
	sthread_create4(&th1, t1, NULL, 1);
	sthread_create4(&th2, t2, NULL, 1);
	sthread_wait_all();
	sthread_join(th1, (void**) &r1);
	sthread_join(th2, (void**) &r2);
	
	printf("The main has ended their work.\n");
	return 0;
}

