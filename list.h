#ifndef __LIST_H_DEFINED__
#define __LIST_H_DEFINED__

typedef struct _list_node {
    void *value;

    struct _list_node *prev, *next;
} list_node;

typedef struct _list {
    list_node *head;
    size_t size;
} list;

int list_init(list *list);

int list_insert_at(list *list, void *value, int pos);

void *list_get_at(list *list, int pos);

int list_shift(list *list, void *value);

void *list_unshift(list *list);

int list_push(list *list, void *value);

void *list_pop(list *list);

int list_del_at(list *list, int pos);

size_t list_size(list *list);

void list_dump(list *list);

int list_destroy(list *list);

#endif
