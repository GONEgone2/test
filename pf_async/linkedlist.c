#include <stdio.h>
#include <string.h>
#include "linkedlist.h"

/* prototype declaration */
static void list_init(void* self);
static void list_push_data_more(void* self, int input);
static void list_push_data_less(void* self, int input);
static void list_push_data_normal(void* self, int input);
static void list_display(void* self);

/* func type */
static cell_vect vect_up={
  list_init,           /* init */
  list_push_data_more, /* add */
  list_display         /* display */
};
static cell_vect vect_down={
  list_init,           /* init */
  list_push_data_less, /* add */
  list_display         /* display */
};
static cell_vect vect_normal={
  list_init,           /* init */
  list_push_data_normal,      /* add */
  list_display         /* display */
};

void cell_class_constructor(cell_class* self, cell_vect_kind kind)
{
  switch (kind) {
  case cell_vect_kind_up: self->vect = vect_up; break;
  case cell_vect_kind_down: self->vect = vect_down; break;
  default: self->vect = vect_normal; break;
  }
}

static void list_init(void* self)
{
  cell_class* this = self;
  memset(this->resouce_list, 0, sizeof(cell) * RES_LIST_MAX);
  this->list_top = NULL;
}

static cell* list_get_empty_cell(void* self)
{
  cell_class* this = self;
  int i;

  for( i = 0; i < RES_LIST_MAX; i++){
    if((this->resouce_list[i].next == NULL) &&
       (this->resouce_list[i].prev == NULL) &&
       (
	(this->list_top != &this->resouce_list[i]) ||
	(this->list_top == NULL)
	)
       ){
      return &this->resouce_list[i];
    }
  }
  return NULL;
}

static void list_push_tail(void* self, cell* p)
{
  cell_class* this = self;
  cell* target;

  target = this->list_top;
  if(target == NULL){
    this->list_top = p;
    return ;
  }

  while (target->next != NULL) {
    target = target->next;
  }
  target->next = p;
  p->prev = target;

}

static void list_push_before_target(void* self, cell* p, cell* target)
{
  cell_class* this = self;
  if((target == NULL) || (this->list_top == NULL)){
    list_push_tail(self, p);
    return ;
  }

  if(target->prev == NULL){
    if(this->list_top == target){
      p->next = target;
      p->prev = NULL;
      target->prev = p;
      this->list_top = p;
    }
    return ;
  }

  cell* prev = target->prev;
  target->prev = p;
  p->prev = prev;
  prev->next = p;
  p->next = target;
}



static void list_remove(void* self, cell* p)
{
  cell_class* this = self;
  if(p == NULL) return;

  cell* prev = p->prev;
  cell* next = p->next;

  if((prev != NULL) && (next != NULL)){
    prev->next = next;
    next->prev = prev;
  }else if(next == NULL){
    prev->next = NULL;
  }else if(prev == NULL){
    this->list_top = next;
    this->list_top->prev = NULL;
  }

  p->next = NULL;
  p->prev = NULL;
}
static cell* list_get_tail_cell(void* self)
{
  cell_class* this = self;
  cell* target = this->list_top;

  if(target == NULL) return NULL;

  while(target != NULL){
    if(target->next == NULL){
      return target;
    }
  }
  return NULL;
}
static cell* list_get_morevalue_cell(void* self, int input)
{
  cell_class* this = self;
  cell* target = this->list_top;

  if(target == NULL) return target;
  while(target != NULL){
    if(target->data.value == input){
      cell* del_target;
      del_target = target;
      target = del_target->prev;
      list_remove(self, del_target);
      if(target == NULL){
	target = this->list_top;
      }
      break;
    }else if(target->data.value > input){
      break;
    }
    target = target->next;
  }

  return target;
}
static cell* list_get_lessvalue_cell(void* self, int input)
{
  cell_class* this = self;
  cell* target = this->list_top;

  if(target == NULL) return target;
  while(target != NULL){
    if(target->data.value == input){
      cell* del_target;
      del_target = target;
      target = del_target->next;
      list_remove(self, del_target);
      break;
    }else if(target->data.value < input){
      break;
    }
    target = target->next;
  }

  return target;
}
static void list_push_data_normal(void* self, int input)
{
  cell_class* this = self;
  cell* target = list_get_empty_cell(self);

  if(target == NULL){
    target = list_get_tail_cell(self);
    if(target == NULL) return;
    list_remove(self, target);
  }

  target->data.value = input;
  list_push_tail(self, target);
}

static void list_push_data_more(void* self, int input)
{
  cell_class* this = self;
  cell* target = list_get_empty_cell(self);
  cell* list_pos_insert;

  if(target == NULL){
    target = list_get_tail_cell(self);
    if(target == NULL) return;
    list_remove(self, target);
  }
  list_pos_insert = list_get_morevalue_cell(self, input);

  target->data.value = input;
  list_push_before_target(self, target, list_pos_insert);
}
static void list_push_data_less(void* self, int input)
{
  cell_class* this = self;
  cell* target = list_get_empty_cell(self);
  cell* list_pos_insert;

  if(target == NULL){
    target = list_get_tail_cell(self);
    if(target == NULL) return;
    list_remove(self, target);
  }
  list_pos_insert = list_get_lessvalue_cell(self, input);
  target->data.value = input;
  list_push_before_target(self, target, list_pos_insert);
}

static void list_display(void* self)
{
  cell_class* this = self;
  cell* target;
  int i = 0;

  target = this->list_top;
  if(target == NULL){
    printf(" None list.\n");
    return ;
  }

  while (target != NULL) {
    printf("list[%d]=%d \n", i, target->data.value);
    if(target->next == NULL){
      break;
    }
    target = target->next;
    i++;
  }
  #if 0
  for(i=0; i <RES_LIST_MAX; i++){
    printf("list_liner[%d]=%d. adr=0x%08x, next=0x%08x, prev=0x%08x \n", i,
	   this->resouce_list[i].data.value, &this->resouce_list[i],
	   this->resouce_list[i].next, this->resouce_list[i].prev);
  }
  #endif
}

/* not use*/
static void list_push_after_target(void* self, cell* p, cell* target)
{
  cell_class* this = self;
  if((target == NULL) || (this->list_top == NULL)){
    list_push_tail(self, p);
    return ;
  }

  if(target->next == NULL){
    list_push_tail(self, p);
    return ;
  }

  cell* next = target->next;
  target->next = p;
  p->next = next;
  next->prev = p;
  p->prev = target;
}
