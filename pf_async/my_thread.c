
#include "my_thread.h"

static void my_thread_init(void* self);
static void my_thread_run(void* self);
static void my_thread_wait(void* self);

static my_thread_ctrl vect = {
  my_thread_init, /* init */
  my_thread_run,  /* run */
  my_thread_wait  /* wait */
};

void my_thread_constructor(my_thread* thd, void* entry_func, void* entry_param, void* cb_func, void* cb_param)
{
  thd->data.entry_func = entry_func;
  thd->data.entry_param = entry_param;
  thd->data.cb_func = cb_func;
  thd->data.cb_param = cb_param;
  thd->ctrl = &vect;
}

int my_thread_add_que(my_thread* thd)
{

  if((thd->ctrl == NULL) || (thd == NULL)) return -1;

  if(thd->ctrl->init != NULL)
    {
      thd->ctrl->init(thd);
    }
  if(thd->ctrl->run != NULL){
    thd->ctrl->run(thd);
  }
  if(thd->ctrl->wait != NULL){
    thd->ctrl->wait(thd);
  }

  return 0;

}

static void my_thread_init(void* self){
  my_thread* this = self;

  if(this == NULL) return;
}

static void my_thread_entry(void* self){
  my_thread* this = self;
  void (*entry_func)(void* entry_param);
  void (*cb_func)(void* cb_param);

  entry_func = this->data.entry_func;
  if(entry_func != NULL){
    entry_func(this->data.entry_param);
  }
  cb_func = this->data.cb_func;
  if(cb_func != NULL){
    cb_func(this->data.cb_param);
  }
}

static void my_thread_run(void* self){
  my_thread* this = self;

  if(this == NULL) return;
  pthread_create(&this->data.tid, NULL, (void*)my_thread_entry, self);
}

static void my_thread_wait(void* self){
  my_thread* this = self;

  if(this == NULL) return;
  pthread_join(this->data.tid, NULL);
}


