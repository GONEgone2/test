#ifndef __MY_THREAD_H__
#define __MY_THREAD_H__

#include <pthread.h>

typedef enum _que_req_status
{
  que_req_status_empty,
  que_req_status_usr_set,
  que_req_status_wait_do,
  que_req_status_doing,
  que_req_status_done,
  que_req_status_max
}que_req_status;

/* system all method */
extern void my_thread_sys_init(void** my_thread_core_ptr);
extern void my_thread_sys_finish(void* my_thread_core_ptr);
extern void my_thread_sys_run_start(void* my_thread_core_ptr);
extern void my_thread_sys_run_stop(void* my_thread_core_ptr);
extern void my_thread_sys_wait_allque_done(void* my_thread_core_ptr);

/* que method */
extern void my_thread_que_get_empty(void* my_thread_core_ptr, void** my_thread);
extern int my_thread_que_add(void* my_thread_core_ptr, void* my_thread, void* entry_func, void* entry_param, void* cb_func, void* cb_param);
extern int my_thread_que_is_done(void* my_thread_core_ptr, void* my_thread);

#endif
