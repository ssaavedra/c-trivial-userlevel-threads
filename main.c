#include <stdio.h>

#include "yield.c"

void t1(void *arg)
{
	printf("t1 started!\n");
	yield();
	printf("t1 ended!\n");
}

void t2(void *arg)
{
	printf("t2 started!\n");
	yield();
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

