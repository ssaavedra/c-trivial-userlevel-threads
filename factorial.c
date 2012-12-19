
#include <stdio.h>
#include <string.h>

#ifdef LINUX
#include "show_stack.c"
#endif

void t2();
void t1();
void yield();
void slow();

#define N 3

int ct = 1;
int mf;
int tf[N];
int tr[N];

char s[1000];


int main(int argc, char *argv[])
{
	int i;
	mf = &i;

sprintf(s, "%s %d\n", "-----main", ct);
write(1, s, strlen(s));

	t1();
	return 0;
}

void t1()
{
	char a[10];
	volatile int i;
	for(i = 1; i < 10; i++) {
		ct = 1;
		sprintf(a, "\t%s\n", "a");
		write(1, a, strlen(a));
		sprintf(a, "%d %x %dM", (void*)mf - (void*)&a, (void*) a, ((void*)mf - (void*)&a) / 1000000 );
		write(1, a, strlen(a));
		slow();
//sprintf(s, "%s %d\n", "-----t1-before-yield", ct);
//write(1, s, strlen(s));
		yield();
//sprintf(s, "%s %d (t1.i = %d)\n", "-----t1-after-yield", ct, i);
//write(1, s, strlen(s));
	}
}

void t2()
{
	char a[10];
	volatile int i;
	for(i = 1; i < 10; i++) {
		ct = 2;
		sprintf(a, "%s", "b");
		write(1, a, strlen(a));
		slow();
//sprintf(s, "%s %d\n", "-----t2-before-yield", ct);
//write(1, s, strlen(s));
		yield();
//sprintf(s, "%s %d (t2.i = %d)\n", "-----t2-after-yield", ct, i);
//write(1, s, strlen(s));
	}
}


int startt2 = 1;
void yield()
{
	int a[10];
	//show_stack(a);

	tf[ct] = a[12];
	tr[ct] = a[13];
	
//sprintf(s, "%s %d\n", "-----yield", ct);
//write(1, s, strlen(s));


	if(startt2)
	{
	
//sprintf(s, "%s %d\n", "-----yield+startt2", ct);
//write(1, s, strlen(s));

		startt2 = 0;
		t2();
	
//sprintf(s, "%s %d\n", "-----yield-aftert2", ct);
//write(1, s, strlen(s));
	}

	
//sprintf(s, "%s %d\n", "-----yield-out", ct);
//write(1, s, strlen(s));

	ct = (ct % 2) + 1;

//sprintf(s, "a[12] = tf[%d] = %p\n", ct, tf[ct]);
//write(1, s, strlen(s));

//sprintf(s, "a[13] = tr[%d] = %p\n", ct, tr[ct]);
//write(1, s, strlen(s));

	a[12] = tf[ct];
	a[13] = tr[ct];

	//show_stack(a);
}



#define XXXX_SLOW 10000000
void slow()
{
	unsigned long i;
	for(i = 0; i < XXXX_SLOW; i++)
		42.*96179.;
}

