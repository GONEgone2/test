#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "my_thread.h"
#include "linkedlist.h"

static void thread1(void* pParam);
static void thread1_cbfunc(void* pParam);

int main(void){
  cell_class list[6];
  my_thread* thd[6];

  /* declarative */
  my_thread_sys_init();
  my_thread_sys_run_start();

  for(int i = 0; i < 6; i++){
    printf("[%d] 0x%08lx\n", i, (unsigned long)&list[i]);
    cell_class_constructor(&list[i], (i % 3));
    thd[i] = my_thread_que_get_empty();
    my_thread_que_add(thd[i], thread1, &list[i], thread1_cbfunc, &list[i]);
  }
  
  my_thread_sys_wait_allque_done();
  my_thread_sys_run_stop();

  return 0;
}

static void thread1_cbfunc(void* pParam)
{
  printf("cb_func call. 0x%08lx \n", (unsigned long)pParam);
}

static void thread1(void* pParam)
{
  cell_class* list = pParam;
  printf(" thread1 is 0x%08lx. \n", (unsigned long)list);
  list->vect.init(list);
  for(int i=0;i<10;i++){
    list->vect.add(list, (void*)random());
  }
  list->vect.display(list);
}

int main2(void){
  cell_class list;

  for(int i=0;i<3;i++){
    printf("test start[%d]. \n", i);
    cell_class_constructor(&list, i);
    list.vect.init(&list);
    list.vect.add(&list, (void*)1);
    list.vect.add(&list, (void*)3);
    list.vect.add(&list, (void*)2);
    list.vect.add(&list, (void*)5);
    list.vect.add(&list, (void*)4);
    list.vect.add(&list, (void*)1);
    list.vect.display(&list);
    printf("test end. \n");
  }

  return 0;

}

