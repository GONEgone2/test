
#include <stdio.h>
#include <stdlib.h>
#include "my_thread.h"
#include "linkedlist.h"
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define handle_error_en(en, msg)									\
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define UNUSED_PARAM_IGNORE_COMPILE_WARN(x) ((void) x)
#define DBG_ON 0

/* --------------------- admin info -------------- */
#define MY_THREAD_REQ_MAX 10
#define MY_THREAD_EXE_MAX 2
#define MY_THREAD_PARAM_MAX (MY_THREAD_EXE_MAX * 2)

#define SIG_ADMIN     (SIGRTMIN + 1)
#define SIG_WAIT_QUE  SIG_ADMIN //(SIGRTMIN + 2)

typedef struct _my_thread_data
{
	que_req_status que_status;
	pthread_t tid;
	pthread_mutex_t que_status_mutex;
	my_thread_event_handler evt_hdl;
}my_thread_data;

typedef struct _my_thread
{
  my_thread_data data;
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
	pthread_mutex_t req_list_admin_mutex;
	int   req_list_cnt;
	pthread_t g_admin_tid;
	sigset_t  g_admin_signal;
	pid_t     g_wait_que_empty_pid;
	sigset_t  g_wait_que_empty_signal;
	my_thread_run_que_prm thd_prm[MY_THREAD_PARAM_MAX];
	void* thd_prm_admin;
	int   thd_prm_cnt;
	int   thd_exec_cnt;
}my_thread_core;

/* --------------------- prottype func -------------- */
static void my_thread_search_que(void* my_thread_core_ptr);
static void my_thread_run_que(my_thread_run_que_prm* prm);
static int my_thread_chk_enable_que(my_thread* thd);
static void my_thread_init(void* self);
static void my_thread_run(void* self);
static void my_thread_wait(void* self);

/* --------------------- function ------------------- */
void my_thread_sys_init(void** my_thread_core_ptr)
{
	my_thread_core* core;
	my_thread* req_list;
	
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

    req_list = core->req_list;
	for(int i = 0; i < MY_THREAD_REQ_MAX; i++){
		pthread_mutex_init(&req_list[i].data.que_status_mutex, NULL);
	}
	pthread_mutex_init(&core->req_list_admin_mutex, NULL);
	cell_class_init(cell_vect_kind_normal, &core->thd_prm_admin);
	*my_thread_core_ptr = core;
}

void my_thread_sys_finish(void* my_thread_core_ptr)
{
	my_thread_core* core = my_thread_core_ptr;
	my_thread* req_list;
	
	if( (core == NULL) || (core->req_list_admin == NULL)) return ;
	cell_class_finsh(core->req_list_admin);
	pthread_mutex_destroy(&core->req_list_admin_mutex);
	req_list = core->req_list;
	for(int i = 0; i < MY_THREAD_REQ_MAX; i++){
	    pthread_mutex_destroy(&req_list[i].data.que_status_mutex);
	}
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
			pthread_mutex_lock(&thd->data.que_status_mutex);
			thd->data.que_status = que_req_status_usr_set;
			pthread_mutex_unlock(&thd->data.que_status_mutex);
			if(++core->req_list_cnt >= MY_THREAD_REQ_MAX){
				core->req_list_cnt = 0;
			}
			break;
		}
	}

	*my_thread_ptr = thd;
}

static my_thread_run_que_prm* my_thread_thdprm_get_empty(my_thread_core* core)
{
	my_thread_run_que_prm* thdprm_target;
	
	for(int i = 0;i < MY_THREAD_PARAM_MAX; i++){
		thdprm_target = &core->thd_prm[i];
		if(cell_class_is_data(core->thd_prm_admin, thdprm_target) == 0){
			return thdprm_target;
		}
	}
	return NULL;
}

static void my_thread_thdprm_use(my_thread_core* core, my_thread_run_que_prm* thd_prm)
{
	cell_class_add(core->thd_prm_admin, thd_prm);	
}

static void my_thread_thdprm_unuse(my_thread_core* core, my_thread_run_que_prm* thd_prm)
{
	cell_class_del(core->thd_prm_admin, thd_prm);	
}

int my_thread_que_add(
	void* my_thread_core_ptr, void* my_thread_ptr,
	my_thread_event_handler* evt_hdl)
{
	my_thread_core* core = my_thread_core_ptr;
	my_thread* thd = my_thread_ptr;
	int s;
	
	if(evt_hdl == NULL){
		memset(&thd->data.evt_hdl, 0, sizeof(my_thread_event_handler));
	}else{
		thd->data.evt_hdl = *evt_hdl;
	}
	pthread_mutex_lock(&thd->data.que_status_mutex);
	thd->data.que_status = que_req_status_wait_do;
	pthread_mutex_unlock(&thd->data.que_status_mutex);
	pthread_mutex_lock(&core->req_list_admin_mutex);	
	cell_class_add(core->req_list_admin, thd);
	pthread_mutex_unlock(&core->req_list_admin_mutex);
    s = pthread_kill(core->g_admin_tid, SIG_ADMIN);
#if DBG_ON
	printf("singnal send.pid=0x%lx, SIG_ADMIN ret=%d \n", core->g_admin_tid, s);
#else
	UNUSED_PARAM_IGNORE_COMPILE_WARN(s);
#endif
	return 0;
	
}

void my_thread_sys_wait_allque_done(void* my_thread_core_ptr)
{
  
  void* cell_next = (void*)0xff;
  my_thread_core* core = my_thread_core_ptr;
  siginfo_t siginfo;
  struct timespec sig_wait_time;
  
  core->g_wait_que_empty_pid = getpid();
  sigemptyset(&core->g_wait_que_empty_signal);
  sigaddset(&core->g_wait_que_empty_signal, SIG_WAIT_QUE);	
  sigprocmask(SIG_BLOCK, &core->g_wait_que_empty_signal, NULL);
  sig_wait_time.tv_sec = 0;
  sig_wait_time.tv_nsec = 100*1000; //100us
  while(cell_next){
	  cell_class_get_next(core->req_list_admin, NULL, &cell_next);
	  if(cell_next != NULL){
#if DBG_ON
		  printf("signal recieve. g_wait_que_empty_signal pid = 0x%08lx. waiting... ", (unsigned long)core->g_wait_que_empty_pid);
#endif
		  sigtimedwait(&core->g_wait_que_empty_signal, &siginfo, &sig_wait_time);
#if DBG_ON
		  printf("signal_val= 0x%x.\n", siginfo.si_signo);
#endif		  
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
	siginfo_t siginfo;
	struct timespec sig_wait_time;

	s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if(s != 0){
		printf("thread can't cncel. \n");
	}
	
	sigemptyset(&core->g_admin_signal);
	sigaddset(&core->g_admin_signal, SIG_ADMIN);
	sig_wait_time.tv_sec = 0;
	sig_wait_time.tv_nsec = 100*1000; //100us

	s = pthread_sigmask(SIG_BLOCK, &core->g_admin_signal, NULL);
#if DBG_ON	
	if(s != 0){
		handle_error_en(s, "pthread_sigmask");
	}	
#endif	
	while(1){
		/* signal wait */
#if DBG_ON
		printf("signal recieve. g_admin_signal pid=0x%lx. waiting... ", pthread_self());
#endif
		s = sigtimedwait(&core->g_admin_signal, &siginfo, &sig_wait_time);
#if DBG_ON
		printf("signal_val = 0x%x. \n", siginfo.si_signo);
#endif
		
		/* search que*/
		thd = NULL;
		for(int i = 0; i <MY_THREAD_REQ_MAX; i++){
			if(
				(req_list[i].data.evt_hdl.entry_func != NULL)
				&& (req_list[i].data.que_status == que_req_status_wait_do)
				&& (req_list[i].data.tid == 0)
				){
				thd = &req_list[i];
				break;
			}
		}
    
		if(
			(my_thread_chk_enable_que(thd) == 0)
			|| (core->thd_exec_cnt >= MY_THREAD_EXE_MAX)
			){
			/* none request or over requet. waiting... */
		}else{
			my_thread_run_que_prm* thd_prm;
			thd_prm = my_thread_thdprm_get_empty(core);
			my_thread_thdprm_use(core, thd_prm);
			core->thd_prm_cnt++;
			thd_prm->core = core;
			thd_prm->thread = thd;
			pthread_create(&thd->data.tid, NULL, (void*)my_thread_run_que, thd_prm);
			core->thd_exec_cnt++;
			pthread_mutex_lock(&thd->data.que_status_mutex);
			thd->data.que_status = que_req_status_doing;
			pthread_mutex_unlock(&thd->data.que_status_mutex);
#if (MY_THREAD_EXE_MAX == 1)
			my_thread_wait(thd);
#endif
		}
	}
}

static void my_thread_run_que(my_thread_run_que_prm* thd_prm)
{
	my_thread_core* core = thd_prm->core;
	my_thread* thd = thd_prm->thread;
	int s;
	
	if(my_thread_chk_enable_que(thd) == 0) return;

	/* run que */
	my_thread_init(thd);
    my_thread_run(thd);
	pthread_mutex_lock(&core->req_list_admin_mutex);
	cell_class_del(core->req_list_admin, thd);
	pthread_mutex_unlock(&core->req_list_admin_mutex);
	pthread_mutex_lock(&thd->data.que_status_mutex);
	thd->data.que_status = que_req_status_empty;
	pthread_mutex_unlock(&thd->data.que_status_mutex);
	core->thd_exec_cnt--;
	s = pthread_kill(core->g_admin_tid, SIG_ADMIN);
#if DBG_ON
	printf("pthread_kill pid=0x%lx, ret=%d \n", core->g_admin_tid, s);
#else
	UNUSED_PARAM_IGNORE_COMPILE_WARN(s);
#endif
	s = kill(core->g_wait_que_empty_pid, SIG_WAIT_QUE);
#if DBG_ON
	printf("kill pid=0x%08lx, ret=%d \n", (unsigned long)core->g_wait_que_empty_pid, s);
#endif
	my_thread_thdprm_unuse(core, thd_prm);
}

static void my_thread_init(void* self){
  my_thread* this = self;

  if(this == NULL) return;
}

static void my_thread_run(void* self){
	my_thread* this = self;
	void (*init_func)(void* init_parm);
	void (*entry_func)(void* entry_param);
	void (*cb_func)(void* cb_param);
	
	if(this == NULL) return;
	
    init_func = this->data.evt_hdl.init_func;
	if(init_func != NULL){
	    init_func(this->data.evt_hdl.init_parm);
	}
	entry_func = this->data.evt_hdl.entry_func;
	if(entry_func != NULL){
		entry_func(this->data.evt_hdl.entry_param);
	}
	cb_func = this->data.evt_hdl.cb_func;
	if(cb_func != NULL){
		cb_func(this->data.evt_hdl.cb_param);
	}
}

static void my_thread_wait(void* self){
  my_thread* this = self;

  if(this == NULL) return;
  pthread_join(this->data.tid, NULL);
}

static int my_thread_chk_enable_que(my_thread* thd)
{
  
  if(thd == NULL){
    return 0;
  }
  return 1;
}


