
#define FILE_BASEPTR  0x20000000
#define FILE_STACKSIZE 0x0FFFFFFF

struct sthread_file_info {
	void *stackptr;
	off_t stacksize;
	sthread_t threadnum;
	int fd;
	void *cur_ebp;
	void *cur_ret;
	int dirty;
	struct sthread_file_info *nxt;
};

void sthread_file_save(int cur_thread, void **reloc_information, void **ebp, void **ret);
static void sthread_file_launch(sthread_fun_t ptr, sthread_arg_t arg);
void sthread_file_start_signal(int signal);
void sthread_file_start(int cur_thread, void **reloc_information, void **ebp, void **ret, sthread_fun_t ptr, sthread_arg_t arg);

void sthread_file_restore(int cur_thread, void **reloc_information, void **ebp, void **ret);
void sthread_file_exit(int cur_thread, void **reloc_information);
void sthread_file_protect_yield(int cur_thread, void **reloc_information);

