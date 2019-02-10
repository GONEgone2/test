#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "my_thread.h"
#include "linkedlist.h"

static void* thread1(void* pParam);

int main(void){
  cell_class list;
  int list_method = random();
  my_thread thd;

  /* initial */
  list_method = list_method  % 3;
  printf("list method is %d. \n", list_method);
  cell_class_constructor(&list, list_method);

  /* declarative */
  my_thread_constructor(&thd, thread1, &list, NULL, NULL);
  my_thread_add_que(&thd);

  return 0;
}

static void* thread1(void* pParam)
{
  cell_class* list = pParam;

  list->vect.init(list);
  for(int i=0;i<10;i++){
    list->vect.add(list, random());
  }
  list->vect.display(list);
}

int main2(void){
  cell_class list;

  for(int i=0;i<3;i++){
    printf("test start[%d]. \n", i);
    cell_class_constructor(&list, i);
    list.vect.init(&list);
    list.vect.add(&list, 1);
    list.vect.add(&list, 3);
    list.vect.add(&list, 2);
    list.vect.add(&list, 5);
    list.vect.add(&list, 4);
    list.vect.add(&list, 1);
    list.vect.display(&list);
    printf("test end. \n");
  }

  return 0;

}

