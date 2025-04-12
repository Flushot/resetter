#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

list_node* list_rewind(list_node* node) {
    list_node* p_head = node;

    while (p_head->prev != NULL) {
        p_head = p_head->prev;
    }

    return p_head;
}

static list_node* list_at(list_node* list, size_t pos) {
    list_node* p_iter = list;

    for (int i = 0; p_iter != NULL; ++i) {
        if (i == pos) {
            return p_iter;
        }

        p_iter = p_iter->next;
    }

    return NULL;
}

int list_init(list* lst) {
    memset(lst, 0, sizeof(list));

    return 0;
}

int list_insert_at(list* lst, void* value, size_t pos) {
    list_node* p_head = lst->head;
    list_node* p_existing = list_at(p_head, pos);

    if (p_existing == NULL && p_head != NULL) {
        fprintf(stderr, "node does not exist at index %d\n", (int)pos);
        return -1;
    }

    list_node* item = (list_node *)malloc(sizeof(list_node));
    if (item == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(item, 0, sizeof(list_node));
    item->value = value;

    if (p_head == NULL) {
        lst->head = item;
    }
    else {
        // Shift existing right
        item->prev = p_existing->prev;
        item->next = p_existing;
        p_existing->prev = item;
        if (item->prev != NULL) {
            item->prev->next = item;
        }

        lst->head = list_rewind(item);
    }

    ++lst->size;

    return 0;
}

void* list_get_at(const list* lst, size_t pos) {
    list_node* node = list_at(lst->head, pos);

    if (node == NULL) {
        return NULL;
    }

    return node->value;
}

int list_shift(list* list, void* value) {
    return list_insert_at(list, value, 0);
}

void* list_unshift(list* lst) {
    list_node* p_head = lst->head;

    if (p_head == NULL) {
        return NULL;
    }

    void* value = p_head->value;
    list_del_at(lst, 0);

    return value;
}

int list_push(list* lst, void* value) {
    list_node* p_tail = lst->head;

    if (p_tail != NULL) {
        while (p_tail->next != NULL) {
            p_tail = p_tail->next;
        }
    }

    list_node* p_item = (list_node *)malloc(sizeof(list_node));
    if (p_item == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(p_item, 0, sizeof(list_node));
    p_item->value = value;

    if (p_tail != NULL) {
        p_tail->next = p_item;
        p_item->prev = p_tail;
    }
    else {
        lst->head = p_item;
    }

    ++lst->size;

    return 0;
}

list_node* list_tail_node(const list* lst) {
    if (lst->size == 0) {
        return NULL;
    }

    size_t pos = lst->size - 1;

    return list_at(lst->head, pos);
}

void* list_tail(const list* lst) {
    list_node* tail = list_tail_node(lst);

    if (tail == NULL) {
        return NULL;
    }
    else {
        return tail->value;
    }
}

void* list_pop(list* lst) {
    list_node* tail = list_tail_node(lst);

    if (tail == NULL) {
        return NULL;
    }

    size_t pos = lst->size - 1;
    list_del_at(lst, pos);

    return tail->value;
}

int list_del_at(list* lst, size_t pos) {
    if (lst->head == NULL) {
        return -1;
    }

    list_node* p_item = list_at(lst->head, pos);
    if (p_item == NULL) {
        fprintf(stderr, "node does not exist at index %d", (int)pos);
        return -1;
    }

    if (p_item->prev != NULL) {
        p_item->prev->next = p_item->next;
    }
    else {
        lst->head = p_item->next;
    }

    if (p_item->next != NULL) {
        p_item->next->prev = p_item->prev;
    }

    free(p_item);
    --lst->size;

    return 0;
}

int list_iter(
    const list* lst,
    list_iter_func iter_func,
    void* iter_func_user_arg
) {
    list_node* p_iter = lst->head;
    if (p_iter == NULL) {
        fprintf(stderr, "list is empty");
        return -1;
    }

    int i = 0;
    do {
        list_node* p_curr = p_iter;
        p_iter = p_iter->next;
        iter_func(p_curr, i++, iter_func_user_arg);
    }
    while (p_iter != NULL);

    return 0;
}

static void list_dump_iter_func(
    const list_node* item,
    size_t _index,
    void* _user_arg
) {
    printf("\"%s\", ", (char *)item->value);
}

void list_dump(const list* lst) {
    printf("[ ");
    list_iter(lst, list_dump_iter_func, NULL);
    printf(" ]\n");
}

static void list_destroy_iter_func(
    const list_node* item,
    size_t _index,
    void* _user_arg
) {
    free((void *)item);
}

int list_destroy(list* lst) {
    list_iter(lst, list_destroy_iter_func, NULL);

    lst->size = 0;
    lst->head = NULL;

    return 0;
}
