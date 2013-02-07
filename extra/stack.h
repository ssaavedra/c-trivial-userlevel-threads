#ifndef _STHREAD_CTX_STACK_
/** To be included from other files **/
#define _STHREAD_CTX_STACK_
#define _STHREAD_INTERNALS_

#include "../sthread.h"

#define DEFAULT_STACK_THREADSIZE 8192

void sthread_stack_save(int cur_thread, void **reloc_information, void **ebp, void **ret);
static void sthread_stack_safeguard_launch(sthread_fun_t ptr, sthread_arg_t arg);
static void sthread_stack_grow_and_launch(int size, sthread_fun_t ptr, sthread_arg_t arg);
void sthread_stack_start(int cur_thread, void **reloc_information, void **ebp, void **ret, sthread_fun_t ptr, sthread_arg_t arg);
void sthread_stack_restore(int cur_thread, void **reloc_information, void **ebp, void **ret);
void sthread_stack_exit(int cur_thread, void **reloc_information);


#undef _STHREAD_INTERNALS_
#endif
