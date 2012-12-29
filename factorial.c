
#include <stdio.h>
#include <string.h>

#include "yield.h"

int yielding_factorial(int n, int m, int orig)
{
	if(n == 0) {
		yield();
		printf("Factorial de %d = %d\n", orig, m);
		return 0;
	}

	yielding_factorial(n - 1, n * m, orig);
	return 0;
}

void factorial(void *arg)
{
	int m, n = (int) arg;
	printf("Starting factorial(%d)...\n", n);
	yielding_factorial(n, 1, n);
}

int main(int argc, char *argv[])
{
	sthread_init(3);
	sthread_func(1, factorial, (void*) 5);
	sthread_func(2, factorial, (void*) 6);
	sthread_func(3, factorial, (void*) 9);
	sthread_start();
	printf("Work done.\n");

	return 0;
}

