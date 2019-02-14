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
  
typedef struct _my_thread_data
{
  que_req_status que_status;
  pthread_t tid;
  void*     entry_func;
  void*     entry_param;
  void*     cb_func;
  void*     cb_param;
}my_thread_data;

typedef struct _my_thread_ctrl
{
  void (*init)(void* self);
  void (*run)(void* self);
  void (*wait)(void* self);
}my_thread_ctrl;

typedef struct _my_thread
{
  my_thread_data data;
  my_thread_ctrl* ctrl;
}my_thread;

/* system all method */
extern void my_thread_sys_init(void);
extern void my_thread_sys_finish(void);
extern void my_thread_sys_run_start(void);
extern void my_thread_sys_run_stop(void);
extern void my_thread_sys_wait_allque_done(void);

/* que method */
extern my_thread* my_thread_que_get_empty(void);
extern int my_thread_que_add(my_thread* thd, void* entry_func, void* entry_param, void* cb_func, void* cb_param);
extern int my_thread_que_is_done(my_thread* thd);

#endif
