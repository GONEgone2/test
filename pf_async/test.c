#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "my_thread.h"
#include "linkedlist.h"

static void thread1(void* pParam);
static void thread1_cbfunc(void* pParam);

#define TEST_LIST_MAX 1

int main(void){
  void* list[TEST_LIST_MAX];
  my_thread* thd[TEST_LIST_MAX];

  /* declarative */
  my_thread_sys_init();
  my_thread_sys_run_start();

  for(int i = 0; i < TEST_LIST_MAX; i++){
    cell_class_init(i%3, &list[i]);
    printf("[%d] 0x%016lx\n", i, (unsigned long)list[i]);
    thd[i] = my_thread_que_get_empty();
    my_thread_que_add(thd[i], thread1, list[i], thread1_cbfunc, list[i]);
  }
  
  my_thread_sys_wait_allque_done();
  my_thread_sys_run_stop();
  my_thread_sys_finish();
  for(int i = 0; i < TEST_LIST_MAX; i++){
    cell_class_finsh(list[i]);
  }
  
  return 0;
}

static void thread1_cbfunc(void* pParam)
{
  printf("cb_func call. 0x%016lx \n", (unsigned long)pParam);
}

static void thread1(void* pParam)
{
  void* list = pParam;
  
  printf(" thread1 is 0x%016lx. \n", (unsigned long)list);
  for(int i=0;i<10;i++){
    cell_class_add(list, (void*)random());
  }
  cell_class_display(list);
}

int main2(void){
  void* list;

  for(int i=0;i<3;i++){
    printf("test start[%d]. \n", i);
    cell_class_init(i, &list);
    cell_class_add(list, (void*)1);
    cell_class_add(list, (void*)3);
    cell_class_add(list, (void*)2);
    cell_class_add(list, (void*)5);
    cell_class_add(list, (void*)4);
    cell_class_add(list, (void*)1);
    cell_class_display(list);
    cell_class_finsh(list);
    printf("test end. \n");
  }

  return 0;

}

