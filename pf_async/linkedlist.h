#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#define RES_LIST_MAX 10

typedef enum _cell_vect_kind{
  cell_vect_kind_normal,
  cell_vect_kind_up,
  cell_vect_kind_down,
  cell_vect_kind_max
} cell_vect_kind;

typedef struct _cell_vect{
  void (*init)(void* self);
  void (*add) (void* self, int input);
  void (*display)(void* self);
}cell_vect;

typedef struct _cell_data
{
  int value;
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

extern void cell_class_constructor(cell_class* self, cell_vect_kind kind);

#endif

