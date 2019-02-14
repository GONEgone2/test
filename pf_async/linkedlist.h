#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#define RES_LIST_MAX 10

typedef enum _cell_vect_kind{
  cell_vect_kind_normal,
  cell_vect_kind_up,
  cell_vect_kind_down,
  cell_vect_kind_max
} cell_vect_kind;

extern int cell_class_init(cell_vect_kind kind, void** cell_class);
extern void cell_class_add(void* cell_class, void* data);
extern void cell_class_del(void* cell_class, void* data);
extern void cell_class_get_next(void* cell_class, void* target_cell, void** cell);
extern void cell_class_get_top(void* cell_class, void** cell);
extern void cell_class_get_data_from_cell(void* cell, void** data);
extern void cell_class_display(void* cell_class);
extern void cell_class_finsh(void* cell_class);

#endif

