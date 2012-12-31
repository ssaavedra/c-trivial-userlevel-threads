
#include <stdio.h>
#include <string.h>

#include "yield.h"

/*
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

void _factorial(void *arg)
{
	int m, n = (int) arg;
	printf("Starting factorial(%d)...\n", n);
	yielding_factorial(n, 1, n);
}
*/

struct factorial {
	int wanted;
	int current;
};

int factorial(struct factorial *tmp)
{
	int f, n = tmp->current;

	if(n == 0) {
		yield();
		return 1;
	}

	struct factorial stmp = { tmp->wanted, n - 1 };
	f = n * factorial( &stmp );

	if(n == tmp->wanted) {
		printf("Factorial of %d = %ld\n", tmp->wanted, f);
	}
	return f;
}

int main(int argc, char *argv[])
{
	struct factorial fstr[3] = {
		{ 5, 5 },
		{ 6, 6 },
		{ 9, 9 } };

	sthread_init(3);
	sthread_func(1, (fun_t) factorial, fstr);
	sthread_func(2, (fun_t) factorial, fstr + 1);
	sthread_func(3, (fun_t) factorial, fstr + 2);
	sthread_start();
	printf("Work done.\n");

	return 0;
}

