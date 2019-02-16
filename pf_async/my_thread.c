
#include <stdio.h>
#include <stdlib.h>
#include "my_thread.h"
#include "linkedlist.h"
#include <string.h>
#include <unistd.h>

#define UNUSED_PARAM_IGNORE_COMPILE_WARN(x) ((void) x)

/* --------------------- admin info -------------- */
#define MY_THREAD_REQ_MAX 10
#define MY_THREAD_EXE_MAX 1

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

typedef struct _my_thread_run_que_prm
{
    void* core;
	my_thread* thread;
}my_thread_run_que_prm;

typedef struct _my_thread_core
{
    void* req_list;
	void* req_list_admin; //my_thread->top
	int   req_list_cnt;
	pthread_t g_admin_tid;
	my_thread_run_que_prm thd_prm[MY_THREAD_EXE_MAX];
}my_thread_core;

static void my_thread_init(void* self);
static void my_thread_run(void* self);
static void my_thread_wait(void* self);

static my_thread_ctrl vect = {
  my_thread_init, /* init */
  my_thread_run,  /* run */
  my_thread_wait  /* wait */
};


/* --------------------- prottype func -------------- */
static void my_thread_search_que(void* my_thread_core_ptr);
static void my_thread_run_que(my_thread_run_que_prm* prm);
static int my_thread_chk_enable_que(my_thread* thd);

/* --------------------- function ------------------- */
void my_thread_sys_init(void** my_thread_core_ptr)
{
	my_thread_core* core;
	
    core = malloc(sizeof(my_thread_core));
	if(core == NULL){
		printf("malloc faild \n");
		return ;
	}
	memset(core, 0, sizeof(my_thread_core));
	core->req_list = malloc(sizeof(my_thread) * MY_THREAD_REQ_MAX);
	if(core->req_list == NULL){
		printf("malloc faild2 \n");
		return ;
	}
	memset(core->req_list, 0, sizeof(my_thread) * MY_THREAD_REQ_MAX);
	if(cell_class_init (cell_vect_kind_normal, &core->req_list_admin) < 0){
		printf("cell_class_init is fail. \n");
	}
	*my_thread_core_ptr = core;
}

void my_thread_sys_finish(void* my_thread_core_ptr)
{
	my_thread_core* core = my_thread_core_ptr;
	if( (core == NULL) || (core->req_list_admin == NULL)) return ;
	cell_class_finsh(core->req_list_admin);
	if(core->req_list != NULL){
		free(core->req_list);
	}
	if(core != NULL){
		free(core);
	}
	core->req_list_admin = NULL;
}

void my_thread_que_get_empty(void* my_thread_core_ptr, void** my_thread_ptr)
{
	my_thread_core* core = my_thread_core_ptr;
	my_thread* req_list = core->req_list;
	my_thread* thd = NULL;
	int i;
  
	for(int j=0; j < MY_THREAD_REQ_MAX; j++){
		i = j + core->req_list_cnt;
		if(i >= MY_THREAD_REQ_MAX){
			i -= MY_THREAD_REQ_MAX;
		}
		if(req_list[i].data.que_status == que_req_status_empty){
			thd = &req_list[i];
			thd->data.que_status = que_req_status_usr_set;
			break;
		}
	}

	*my_thread_ptr = thd;
}

int my_thread_que_add(
	void* my_thread_core_ptr, void* my_thread_ptr,
	void* entry_func, void* entry_param,
	void* cb_func, void* cb_param)
{
	my_thread_core* core = my_thread_core_ptr;
	my_thread* thd = my_thread_ptr;
	
	thd->data.entry_func = entry_func;
	thd->data.entry_param = entry_param;
	thd->data.cb_func = cb_func;
	thd->data.cb_param = cb_param;
	thd->ctrl = &vect;
	thd->data.que_status = que_req_status_wait_do;
	cell_class_add(core->req_list_admin, thd);
	return 0;
  
}

void my_thread_sys_wait_allque_done(void* my_thread_core_ptr)
{
  
  void* cell_next = (void*)0xff;
  my_thread_core* core = my_thread_core_ptr;
  
  /* signalかえたい */
  while(cell_next){
    cell_class_get_next(core->req_list_admin, NULL, &cell_next);
    if(cell_next != NULL){
      usleep(100 * 1000);
    }
  }
}

int my_thread_que_is_done(void* my_thread_core_ptr, void* my_thread_ptr)
{
	my_thread* thd = my_thread_ptr;
	my_thread_core* core = my_thread_core_ptr;
	
	UNUSED_PARAM_IGNORE_COMPILE_WARN(core);
	
  if(my_thread_chk_enable_que(thd)){
    if(thd->data.que_status != que_req_status_empty){
      return 1;
    }
  }
  return 0;
}

void my_thread_sys_run_start(void* my_thread_core_ptr)
{
	my_thread_core* core = my_thread_core_ptr;
	
	if(core->g_admin_tid) return;
	pthread_create(&core->g_admin_tid, NULL, (void*)my_thread_search_que, core);
}

void my_thread_sys_run_stop(void* my_thread_core_ptr)
{
	my_thread_core* core = my_thread_core_ptr;
	if(core->g_admin_tid != 0){
		pthread_cancel(core->g_admin_tid);
		core->g_admin_tid = 0;
	}
}

static void my_thread_search_que(void* my_thread_core_ptr)
{
	my_thread_core* core = my_thread_core_ptr;
	my_thread* thd;
	my_thread* req_list = core->req_list;
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
			my_thread_run_que_prm* thd_prm;
			thd_prm = &core->thd_prm[0];
		    thd_prm->core = core;
		    thd_prm->thread = thd;
			pthread_create(&thd->data.tid, NULL, (void*)my_thread_run_que, thd_prm);
			thd->data.que_status = que_req_status_doing;
			thd->ctrl->wait(thd);
		}
	}
}

static void my_thread_run_que(my_thread_run_que_prm* prm)
{
	my_thread_core* core = prm->core;
	my_thread* thd = prm->thread;
	
	if(my_thread_chk_enable_que(thd) == 0) return;

	/* run que */
	if(thd->ctrl->init != NULL){
		thd->ctrl->init(thd);
	}
	if(thd->ctrl->run != NULL){
		thd->ctrl->run(thd);
	}
	thd->data.que_status = que_req_status_empty;
	cell_class_del(core->req_list_admin, thd);
	
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


