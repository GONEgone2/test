#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

#define UNUSED_PARAM_IGNORE_COMPILE_WARN(x) ((void) x)

typedef struct _cell_vect{
  void (*init)(void* self);
  void (*add) (void* self, void* data);
  void (*del) (void* self, void* data);
  void (*get_next)(void* self, void* target_cell, void** next_cell);
  void (*display)(void* self);
}cell_vect;

typedef struct _cell_data
{
  void* data;
}cell_data;

typedef struct _cell
{
  struct _cell* next;
  struct _cell* prev;
  cell_data     data;
}cell;

typedef struct _cell_class
{
  cell_vect  vect;
  cell*      list_top;
  cell       resouce_list[RES_LIST_MAX];
}cell_class;

/* prototype declaration */
static void list_init(void* self);
static void list_push_data_more(void* self, void* data);
static void list_push_data_less(void* self, void* data);
static void list_push_data_normal(void* self, void* data);
static void list_get_next_cell(void* self, void* target_cell, void** next_cell);
static void list_delete_data(void* self, void* data);
static void list_display(void* self);

/* func type */
static cell_vect vect_up={
  list_init,           /* init */
  list_push_data_more, /* add */
  list_delete_data,    /* del */
  list_get_next_cell,  /* get_next */
  list_display         /* display */
};
static cell_vect vect_down={
  list_init,           /* init */
  list_push_data_less, /* add */
  list_delete_data,    /* del */
  list_get_next_cell,  /* get_next */
  list_display         /* display */
};
static cell_vect vect_normal={
  list_init,           /* init */
  list_push_data_normal,      /* add */
  list_delete_data,    /* del */
  list_get_next_cell,  /* get_next */
  list_display         /* display */
};

/* public functin */
int cell_class_init(cell_vect_kind kind, void** cell_class_ptr)
{
  cell_class* self;
  
  self = (cell_class*)malloc(sizeof(cell_class));
  if(self == NULL){
    return -1;
  }
  memset(self, 0, sizeof(cell_class));
  switch (kind) {
  case cell_vect_kind_up:   self->vect = vect_up;     break;
  case cell_vect_kind_down: self->vect = vect_down;   break;
  default:                  self->vect = vect_normal; break;
  }
  *cell_class_ptr = (void*)self;
  
  return 0;
}

void cell_class_add(void* cell_class_ptr, void* data)
{
  cell_class* self = cell_class_ptr;
  self->vect.add(self, data);
}
void cell_class_del(void* cell_class_ptr, void* data)
{
  cell_class* self = cell_class_ptr;
  self->vect.del(self, data);
}
int cell_class_is_data(void* cell_class_ptr, void* data)
{
  cell_class* self = cell_class_ptr;
  cell* target_cell = self->list_top;
  while(target_cell != NULL){
	  if(target_cell->data.data == data){
		  return 1;
	  }
	  target_cell = target_cell->next;
  }
  return 0;
}

void cell_class_get_next(void* cell_class_ptr, void* target_cell, void** cell_ptr)
{
  cell* ret;
  cell_class* self = cell_class_ptr;
  self->vect.get_next(self, target_cell, (void**)&ret);
  *cell_ptr = ret;
}
void cell_class_get_top(void* cell_class_ptr, void** cell_ptr)
{
  cell* ret;
  cell_class* self = cell_class_ptr;  
  self->vect.get_next(self, NULL, (void**)&ret);
  *cell_ptr = ret;
}
void cell_class_get_data_from_cell(void* cell_ptr, void** data_ptr)
{
  cell* pcell = cell_ptr;
  if(pcell != NULL){
    *data_ptr = pcell->data.data;
  }else{
    *data_ptr = NULL;
  }
}
void cell_class_display(void* cell_class_ptr)
{
  cell_class* self = cell_class_ptr;
  self->vect.display(self);
}
void cell_class_finsh(void* cell_class_ptr){
  if(cell_class_ptr != NULL){
    free(cell_class_ptr);
  }
}

static void list_init(void* self)
{
  cell_class* this = self;
  UNUSED_PARAM_IGNORE_COMPILE_WARN(this);
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
  }else if((next == NULL) && (prev == NULL)){
    if(this == NULL) return;
    if(this->list_top == p){
      this->list_top = NULL;
    }
  }else if(next == NULL){
    prev->next = NULL;
  }else if(prev == NULL){
    if(this == NULL) return;
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
static cell* list_get_morevalue_cell(void* self, void* input)
{
  cell_class* this = self;
  cell* target = this->list_top;

  if(target == NULL) return target;
  while(target != NULL){
    if((unsigned long)target->data.data == (unsigned long)input){
      cell* del_target;
      del_target = target;
      target = del_target->prev;
      list_remove(self, del_target);
      if(target == NULL){
	target = this->list_top;
      }
      break;
    }else if((unsigned long)target->data.data > (unsigned long)input){
      break;
    }
    target = target->next;
  }

  return target;
}
static cell* list_get_lessvalue_cell(void* self, void* input)
{
  cell_class* this = self;
  cell* target = this->list_top;

  if(target == NULL) return target;
  while(target != NULL){
    if((unsigned long)target->data.data == (unsigned long)input){
      cell* del_target;
      del_target = target;
      target = del_target->next;
      list_remove(self, del_target);
      break;
    }else if((unsigned long)target->data.data < (unsigned long)input){
      break;
    }
    target = target->next;
  }

  return target;
}
static void list_push_data_normal(void* self, void* data)
{
  cell_class* this = self;
  cell* target = list_get_empty_cell(self);
  
  UNUSED_PARAM_IGNORE_COMPILE_WARN(this);
  
  if(target == NULL){
    target = list_get_tail_cell(self);
    if(target == NULL) return;
    list_remove(self, target);
  }

  target->data.data = data;
  list_push_tail(self, target);
}

static void list_push_data_more(void* self, void* data)
{
  cell_class* this = self;
  cell* target = list_get_empty_cell(self);
  cell* list_pos_insert;
  
  UNUSED_PARAM_IGNORE_COMPILE_WARN(this);
  
  if(target == NULL){
    target = list_get_tail_cell(self);
    if(target == NULL) return;
    list_remove(self, target);
  }
  list_pos_insert = list_get_morevalue_cell(self, data);

  target->data.data = data;
  list_push_before_target(self, target, list_pos_insert);
}
static void list_push_data_less(void* self, void* data)
{
  cell_class* this = self;
  cell* target = list_get_empty_cell(self);
  cell* list_pos_insert;
  
  UNUSED_PARAM_IGNORE_COMPILE_WARN(this);
  
  if(target == NULL){
    target = list_get_tail_cell(self);
    if(target == NULL) return;
    list_remove(self, target);
  }
  list_pos_insert = list_get_lessvalue_cell(self, data);
  target->data.data = data;
  list_push_before_target(self, target, list_pos_insert);
}

static void list_get_next_cell(void* self, void* target_cell, void** next_cell)
{
  cell_class* this = self;
  cell* target = (cell*)target_cell;
  
  if(target == NULL){
    *next_cell = this->list_top;
    return ;
  }
  *next_cell = target->next;
}
static cell* list_get_cell_from_data(void* self, void* data)
{
  cell_class* this = self;
  cell* target = this->list_top;
  
  while(target != NULL){
    if(target->data.data == data){
      return target;
    }
    target = target->next;
  }

  return NULL;
}

static void list_delete_data(void* self, void* data)
{
  cell* target = list_get_cell_from_data(self, data);
  list_remove(self, target);
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
    printf("list[%d]=%lu \n", i, (unsigned long)target->data.data);
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

