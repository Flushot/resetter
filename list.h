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

int list_init(list *);

int list_insert_at(list *, void *, int);

void *list_get_at(list *, int);

int list_shift(list *, void *);

void *list_unshift(list *);

int list_push(list *, void *);

list_node *list_tail_node(list *lst);

void *list_tail(list *);

void *list_pop(list *);

int list_del_at(list *, int pos);

void list_dump(list *);

int list_destroy(list *);

#endif
