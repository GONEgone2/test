#ifndef __MY_THREAD_H__
#define __MY_THREAD_H__

#include <pthread.h>

typedef struct _my_thread_data
{
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

extern void my_thread_constructor(my_thread* thd, void* entry_func, void* entry_param, void* cb_func, void* cb_param);
extern int my_thread_add_que(my_thread* thd);


#endif
