#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

list_node *list_rewind(list_node *node) {
    list_node *head = node;

    while (head->prev != NULL) {
        head = head->prev;
    }

    return head;
}

static list_node *_list_at(list_node *list, int pos) {
    int i;
    list_node *iter = list;

    for (i = 0; iter != NULL; ++i) {
        if (i == pos) {
            return iter;
        }

        iter = iter->next;
    }

    return NULL;
}

int list_init(list *lst) {
    memset(lst, 0, sizeof(list));

    return 0;
}

int list_insert_at(list *lst, void *value, int pos) {
    list_node *head = lst->head;
    list_node *existing = _list_at(head, pos);

    if (existing == NULL && head != NULL) {
        fprintf(stderr, "node does not exist at index %d\n", pos);
        return -1;
    }

    list_node *item = (list_node *)malloc(sizeof(list_node));
    if (item == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(item, 0, sizeof(list_node));
    item->value = value;

    if (head == NULL) {
        lst->head = item;
    } else {
        // Shift existing right
        item->prev = existing->prev;
        item->next = existing;
        existing->prev = item;
        if (item->prev != NULL) {
            item->prev->next = item;
        }

        lst->head = list_rewind(item);
    }

    ++lst->size;

    return 0;
}

void *list_get_at(list *lst, int pos) {
    list_node *node = _list_at(lst->head, pos);

    if (node == NULL) {
        return NULL;
    }

    return node->value;
}

int list_shift(list *list, void *value) {
    return list_insert_at(list, value, 0);
}

void *list_unshift(list *lst) {
    list_node *head = lst->head;
    void *value;

    if (head == NULL) {
        return NULL;
    }

    value = head->value;
    list_del_at(lst, 0);

    return value;
}

int list_push(list *lst, void *value) {
    list_node *tail = lst->head;
    list_node *item;

    if (tail != NULL) {
        while (tail->next != NULL) {
            tail = tail->next;
        }
    }

    item = (list_node *)malloc(sizeof(list_node));
    if (item == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(item, 0, sizeof(list_node));
    item->value = value;

    if (tail != NULL) {
        tail->next = item;
        item->prev = tail;
    } else {
        lst->head = item;
    }

    ++lst->size;

    return 0;
}

list_node *list_tail_node(list *lst) {
    int pos;

    if (lst->size == 0) {
        return NULL;
    }

    pos = lst->size - 1;

    return _list_at(lst->head, pos);
}

void *list_tail(list *lst) {
    list_node *tail = list_tail_node(lst);

    if (tail == NULL) {
        return NULL;
    } else {
        return tail->value;
    }
}

void *list_pop(list *lst) {
    int pos;
    list_node *tail = list_tail_node(lst);

    if (tail == NULL) {
        return NULL;
    }

    pos = lst->size - 1;
    list_del_at(lst, pos);

    return tail->value;
}

int list_del_at(list *lst, int pos) {
    list_node *item;

    if (lst->head == NULL) {
        return -1;
    }

    item = _list_at(lst->head, pos);
    if (item == NULL) {
        fprintf(stderr, "node does not exist at index %d", pos);
        return -1;
    }

    if (item->prev != NULL) {
        item->prev->next = item->next;
    } else {
        lst->head = item->next;
    }

    if (item->next != NULL) {
        item->next->prev = item->prev;
    }

    free(item);
    --lst->size;

    return 0;
}

void list_iter(list *lst, list_iter_func iter_func, void *iter_func_user_arg) {
    list_node *curr, *iter = lst->head;
    if (iter != NULL) {
        int i = 0;
        do {
            curr = iter;
            iter = iter->next;
            iter_func(curr, i++, iter_func_user_arg);
        } while (iter != NULL);
    }
}

static void _list_dump_iter_func(list_node *item, int index, void *user_arg) {
    printf("\"%s\", ", item->value);
}

void list_dump(list *lst) {
    printf("[ ");
    list_iter(lst, _list_dump_iter_func, NULL);
    printf(" ]\n");
}

static void _list_destroy_iter_func(list_node *item, int index, void *user_arg) {
    free(item);
}

int list_destroy(list *lst) {
    list_iter(lst, _list_destroy_iter_func, NULL);

    lst->size = 0;
    lst->head = NULL;

    return 0;
}
