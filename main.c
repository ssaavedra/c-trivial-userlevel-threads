#include <stdio.h>

#include "yield.h"

void t1(void *arg)
{
	int i;

	if(arg == NULL) {
		i = 0;
		printf("[T1]: Calling recursively.\n");
		t1(&i);
		printf("[T1]: END.\n");
		return;
	}

	if(*(int*)arg < 2) {
		(*(int*)arg)++;
		printf("[T1]: Calling recursively (arg=%d).\n", *(int*)arg);
		yield();
		t1(arg);
		if(arg == NULL) {
			printf("[T1]: ARG IS NULL, WTF?!\n");
			return;
		}
		printf("[T1]: END (arg=%d).\n", (*(int*)arg)--);
		return;
	}

	printf("t1 started!\n");
	for(i = 0; i < 10; i++) {
		printf("I am thread 1 and I'm looping at %d\n", i);
		yield();
	}
	yield();
	printf("t1 ended!\n");
}

void t2(void *arg)
{
	int i;

	if(arg == NULL) {
		i = 0;
		printf("[T2]: Calling recursively.\n");
		t2((int*)&i);
		printf("[T2]: END.\n");
		return;
	}

	if(*(int*)arg < 2) {
		(*(int*)arg)++;
		printf("[T2]: Calling recursively (arg=%d).\n", *(int*)arg);
		yield();
		t2(arg);
		if(*(&arg) == NULL) {
			printf("[T2]: ARG IS NULL, WTF?!\n");
			return;
		}
		printf("[T2]: END (arg=%d).\n", (*(int*)arg)--);
		return;
	}


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

