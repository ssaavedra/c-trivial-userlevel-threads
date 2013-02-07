#include <setjmp.h>

#define HEAP_BASEPTR 0x20000000
#define DEFAULT_HEAP_THREADSIZE 8192
#define DEFAULT_HEAP_GROW_SIZE 8192

struct sthread_heap_info {
	void *stackptr;
	off_t stacksize;
	off_t maxstacksize;
	sthread_t threadnum;
	jmp_buf lastenv;
	jmp_buf myenv;
	void *cur_ebp;
	void *cur_ret;
	struct sthread_heap_info *nxt;
};

void sthread_heap_save(int cur_thread, void **reloc_information, void **ebp, void **ret);
static void sthread_heap_launch(sthread_fun_t ptr, sthread_arg_t arg);
void sthread_heap_start_signal(int signal);
void sthread_heap_start(int cur_thread, void **reloc_information, void **ebp, void **ret, sthread_fun_t ptr, sthread_arg_t arg);

void sthread_heap_restore(int cur_thread, void **reloc_information, void **ebp, void **ret);
void sthread_heap_exit(int cur_thread, void **reloc_information);

