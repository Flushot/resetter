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

static list_node *list_at(list_node *list, int pos) {
    list_node *iter = list;
    int idx;

    for (idx = 0; iter != NULL; ++idx) {
        if (idx == pos) {
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
    list_node *existing = list_at(head, pos);
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
    list_node *node = list_at(lst->head, pos);
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
    if (head == NULL) {
        return NULL;
    }

    void *value = head->value;
    list_del_at(lst, 0);
    return value;
}

int list_push(list *lst, void *value) {
    list_node *tail = lst->head;
    if (tail != NULL) {
        while (tail->next != NULL) {
            tail = tail->next;
        }
    }

    list_node *item = (list_node *)malloc(sizeof(list_node));
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

void *list_pop(list *lst) {
    if (lst->size == 0) {
        return NULL;
    }

    int pos = lst->size - 1;
    void *value = list_get_at(lst, pos);
    list_del_at(lst, pos);
    return value;
}

int list_del_at(list *lst, int pos) {
    if (lst->head == NULL) {
        return -1;
    }

    list_node *item = list_at(lst->head, pos);
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

void list_dump(list *lst) {
    printf("[ ");

    list_node *iter = lst->head;
    if (iter != NULL) {
        do {
            printf("\"%s\" ", iter->value);
            iter = iter->next;
        } while (iter != NULL);
    }

    printf("]\n");
}

int list_destroy(list *lst) {
    list_node *item, *iter = lst->head;

    if (iter != NULL) {
        while (iter->next != NULL) {
            item = iter;
            iter = iter->next;
            free(item);
        }
    }

    return 0;
}
