#include <stdio.h>

#include "yield.h"

void t1(void *arg)
{
	int i;

	printf("t1 started!\n");
	for(i = 0; i < 10; i++) {
		printf("I am thread 1 and I'm looping at %d\n", i);
		yield();
	}
	printf("t1 ended!\n");
}

void t2(void *arg)
{
	int i;

	printf("t2 started!\n");
	for(i = 10; i < 20; i++) {
		printf("I am thread 2 and I'm looping at %d\n", i);
		yield();
	}
	printf("t2 ended!\n");
}

int main()
{
	sthread_init(2);
	sthread_func(1, t1, NULL);
	sthread_func(2, t2, NULL);
	sthread_start();
	printf("The threads have ended their work.\n");
}

