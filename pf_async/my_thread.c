
#include <stdio.h>
#include "my_thread.h"
#include "linkedlist.h"
#include <string.h>
#include <unistd.h>

static void my_thread_init(void* self);
static void my_thread_run(void* self);
static void my_thread_wait(void* self);

static my_thread_ctrl vect = {
  my_thread_init, /* init */
  my_thread_run,  /* run */
  my_thread_wait  /* wait */
};

/* --------------------- admin info -------------- */
#define MY_THREAD_REQ_MAX 10
static my_thread req_list[MY_THREAD_REQ_MAX];
static void* req_list_admin = NULL;
static int req_list_cnt = 0;
static pthread_t g_admin_tid = 0;

/* --------------------- prottype func -------------- */
static void my_thread_search_que(void);
static void my_thread_run_que(my_thread* thd);
static int my_thread_chk_enable_que(my_thread* thd);

/* --------------------- function ------------------- */
void my_thread_sys_init(void)
{
  if(req_list_admin != NULL) return ;
  memset(req_list, 0, sizeof(my_thread) * MY_THREAD_REQ_MAX);
  if(cell_class_init (cell_vect_kind_normal, &req_list_admin) < 0){
    printf("cell_class_init is fail. \n");
  }
}
void my_thread_sys_finish(void)
{
  if( req_list_admin == NULL) return ;
  cell_class_finsh(req_list_admin);
  req_list_admin = NULL;
  
}
my_thread* my_thread_que_get_empty(void)
{
  my_thread* thd = NULL;
  int i;
  
  for(int j=0; j < MY_THREAD_REQ_MAX; j++){
    i = j + req_list_cnt;
    if(i >= MY_THREAD_REQ_MAX){
      i -= MY_THREAD_REQ_MAX;
    }
    if(req_list[i].data.que_status == que_req_status_empty){
      thd = &req_list[i];
      thd->data.que_status = que_req_status_usr_set;
      break;
    }
  }

  return thd;
}

int my_thread_que_add(my_thread* thd, void* entry_func, void* entry_param,
		      void* cb_func, void* cb_param)
{
  thd->data.entry_func = entry_func;
  thd->data.entry_param = entry_param;
  thd->data.cb_func = cb_func;
  thd->data.cb_param = cb_param;
  thd->ctrl = &vect;
  thd->data.que_status = que_req_status_wait_do;
  cell_class_add(req_list_admin, thd);
  return 0;
  
}

void my_thread_sys_wait_allque_done(void)
{
  
  void* cell_next = (void*)0xff;
  
  /* signalかえたい */
  while(cell_next){
    cell_class_get_next(req_list_admin, NULL, &cell_next);
    if(cell_next != NULL){
      usleep(100 * 1000);
    }
  }
}

int my_thread_que_is_done(my_thread* thd)
{
  if(my_thread_chk_enable_que(thd)){
    if(thd->data.que_status != que_req_status_empty){
      return 1;
    }
  }
  return 0;
}

void my_thread_sys_run_start(void)
{
  if(g_admin_tid) return;
  pthread_create(&g_admin_tid, NULL, (void*)my_thread_search_que, NULL);
}

void my_thread_sys_run_stop(void)
{
  if(g_admin_tid != 0){
    pthread_cancel(g_admin_tid);
    g_admin_tid = 0;
  }
}

static void my_thread_search_que(void)
{
  my_thread* thd;
  int s;

  s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  if(s != 0){
    printf("thread can't cncel. \n");
  }

  while(1){
    /* search que*/
    thd = NULL;
    for(int i = 0; i <MY_THREAD_REQ_MAX; i++){
      if(
	 (req_list[i].data.entry_func != NULL)
	 && (req_list[i].data.que_status == que_req_status_wait_do)
	 && (req_list[i].data.tid == 0)
	 ){
	thd = &req_list[i];
	break;
      }
    }
    
    if(my_thread_chk_enable_que(thd) == 0){
      /* none request...waiting. */
      usleep(100 * 1000); //100ms wait    
    }else{
      pthread_create(&thd->data.tid, NULL, (void*)my_thread_run_que, thd);
      thd->data.que_status = que_req_status_doing;
      thd->ctrl->wait(thd);
    }
  }
}

static void my_thread_run_que(my_thread* thd)
{
  if(my_thread_chk_enable_que(thd) == 0) return;

  /* run que */
  if(thd->ctrl->init != NULL){
    thd->ctrl->init(thd);
  }
  if(thd->ctrl->run != NULL){
    thd->ctrl->run(thd);
  }
  thd->data.que_status = que_req_status_empty;
  cell_class_del(req_list_admin, thd);

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
  //pthread_create(&this->data.tid, NULL, (void*)my_thread_entry, self);
  my_thread_entry(self);
  
}

static void my_thread_wait(void* self){
  my_thread* this = self;

  if(this == NULL) return;
  pthread_join(this->data.tid, NULL);
}

static int my_thread_chk_enable_que(my_thread* thd)
{
  
  if((thd == NULL) || (thd->ctrl == NULL)){
    return 0;
  }
  return 1;
}


