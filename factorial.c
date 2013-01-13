/**
 * Example of usage for the sthreads implementation.
 *
 * File: dbuf.c
 *
 * This is a double-buffered copy which uses two copy-workers and aio
 * (see <aio.h>) in order to copy files.
 * It is very simple, and it doesn't accept any options, nor works for
 * anything outside a regular file (and will exit if it finds you trying
 * to copy to/form anything but a file).
 *
 * Author: Santiago Saavedra <santiago.saavedral@udc.es>
 * Developed for Subject: Design and Implementation of Operating Systems.
 * Taught by: Ramón P. Otero and Alberto Illobre.
 *
 * Universidade da Coruña - 2012-2013
 */

#include <stdio.h>
#include <string.h>

#include "sthread.h"

int *p[3];

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

int new_factorial(int n)
{
	int i, j;

	if(n <= 0) {
		for(i = 1; i < 10000000; i++)
		{
			j += 23958.23*182935 * i ;
		}
		printf("T = %d Factorial de %d = yield %d\n", sthread_self(), n, (p[sthread_self()] - &i) / 1000000);
		yield();
		return 1;
	}

	//yield();
	return n + new_factorial(n - 1);
}

#define k 500000
int clase_factorial(int n)
{
	int i;
	p[sthread_self()] = &i;
	for(;;) {
		if(n == 12) {
			n = k;
		} else {
			n = 12;
		}
		printf("T = %d Entrando en factorial(%d)\n", sthread_self(), n);
		i = new_factorial(n);
		printf("T = %d ", sthread_self());
		printf("Factorial de %d = %d\n", n, i);
	}
	return i;
}
	

int main(int argc, char *argv[])
{
	sthread_t th1, th2, th3;
	int r1, r2, r3;

	struct factorial fstr[3] = {
		{ 5, 5 },
		{ 6, 6 },
		{ 9, 9 } };

	sthread_init(3);

	/*
	sthread_create4(&th1, (sthread_fun_t) factorial, fstr, 1);
	sthread_create4(&th2, (sthread_fun_t) factorial, fstr + 1, 1);
	sthread_create4(&th3, (sthread_fun_t) factorial, fstr + 2, 1);
	sthread_join(th1, (void**) &r1);
	sthread_join(th2, (void**) &r2);
	sthread_join(th3, (void**) &r3);

	sthread_create4(&th1, (sthread_fun_t) new_factorial, (void*) 12, 1);
	sthread_create4(&th2, (sthread_fun_t) new_factorial, (void*) 12, 1);
	sthread_create4(&th3, (sthread_fun_t) new_factorial, (void*) 25, 1);
	sthread_join(th1, (void**) &r1);
	sthread_join(th2, (void**) &r2);
	sthread_join(th3, (void**) &r3);
	*/

	sthread_auto_yield(1);
	sthread_create4(&th1, (sthread_fun_t) clase_factorial, (void*) 12, 1);
	sthread_create4(&th2, (sthread_fun_t) clase_factorial, (void*) 12, 1);
	sthread_create4(&th3, (sthread_fun_t) clase_factorial, (void*) 12, 1);
	sthread_join(th1, (void**) &r1);
	sthread_join(th2, (void**) &r2);
	sthread_join(th3, (void**) &r3);


	printf("f($1) = %d\nf($2) = %d\nf($3) = %d\n", r1, r2, r3);
	printf("Work done.\n");

	return 0;
}

