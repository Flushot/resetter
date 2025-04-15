#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

/**
 * Create a new list node
 *
 * @param value Value of node
 * @return List node
 */
static list_node* make_node(void* value) {
    list_node* item = malloc(sizeof(list_node));
    if (item == NULL) {
        perror("make_node: malloc() failed");
        return NULL;
    }

    item->value = value;
    item->prev = NULL;
    item->next = NULL;

    return item;
}

/**
 * Find list node at a given position
 *
 * This needs to traverse the entire list, so it's not very efficient for
 * large lists
 *
 * @param list List
 * @param pos Position
 * @return List node (or NULL if not found)
 */
static list_node* find_node_at(list_node* list, const size_t pos) {
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
    lst->head = NULL;
    lst->tail = NULL;
    lst->size = 0;

    return 0;
}

int list_insert_at(list* lst, void* value, const size_t pos) {
    list_node* p_head = lst->head;
    list_node* p_existing = find_node_at(p_head, pos);

    if (p_existing == NULL) {
        fprintf(stderr, "list_insert_at: node does not exist at index %zu\n", pos);
        return -1;
    }

    list_node* p_node = make_node(value);
    if (p_node == NULL) {
        return -1;
    }

    if (p_head == NULL) {
        lst->head = p_node;
    }

    p_node->prev = p_existing->prev;
    p_node->next = p_existing;

    if (p_node->next == NULL) {
        lst->tail = p_node;
    }

    ++lst->size;

    return 0;
}

void* list_get_at(const list* lst, const size_t pos) {
    list_node* p_node = find_node_at(lst->head, pos);

    if (p_node == NULL) {
        return NULL;
    }

    return p_node->value;
}

int list_del_at(list* lst, const size_t pos) {
    if (lst->head == NULL) {
        fprintf(stderr, "list_del_at: list is empty");
        return -1;
    }

    list_node* p_node = find_node_at(lst->head, pos);
    if (p_node == NULL) {
        fprintf(stderr, "list_del_at: node does not exist at index %zu", pos);
        return -1;
    }

    list_node* p_before = p_node->prev;
    list_node* p_after = p_node->next;

    if (p_before != NULL) {
        p_before->next = p_after;
    }
    else {
        lst->head = p_after;
    }

    if (p_after != NULL) {
        p_after->prev = p_before;
    }
    else {
        lst->tail = p_before;
    }

    p_node->prev = NULL;
    p_node->next = NULL;

    free(p_node);
    --lst->size;

    return 0;
}

void* list_head(const list* lst) {
    if (lst->head == NULL) {
        return NULL;
    }

    return lst->head->value;
}

int list_push_head(list* lst, void* value) {
    list_node* p_head = lst->head;

    list_node* p_node = make_node(value);
    if (p_node == NULL) {
        return -1;
    }

    if (p_head != NULL) {
        lst->head = p_node;
        p_node->next = p_head;
        p_head->prev = p_node;
    }
    else {
        lst->head = p_node;
    }

    if (p_node->next == NULL) {
        lst->tail = p_node;
    }

    ++lst->size;

    return 0;
}

void* list_pop_head(list* lst) {
    list_node* p_node = lst->head;

    if (p_node == NULL) {
        return NULL;
    }

    void* value = p_node->value;
    lst->head = p_node->next;
    p_node->next = NULL;

    free(p_node);
    --lst->size;

    return value;
}

void* list_tail(const list* lst) {
    if (lst->tail == NULL) {
        return NULL;
    }

    return lst->tail->value;
}

int list_push_tail(list* lst, void* value) {
    list_node* p_tail = lst->tail;

    list_node* p_node = make_node(value);
    if (p_node == NULL) {
        return -1;
    }

    if (p_tail != NULL) {
        p_tail->next = p_node;
        p_node->prev = p_tail;
    }
    else {
        lst->head = p_node;
    }

    lst->tail = p_node;

    ++lst->size;

    return 0;
}

void* list_pop_tail(list* lst) {
    list_node* p_node = lst->tail;

    if (p_node == NULL) {
        return NULL;
    }

    list_node* p_before = p_node->prev;

    lst->tail = p_before;

    if (p_before == NULL) {
        // List is now empty
        lst->head = NULL;
    }
    else {
        lst->tail = p_before;
        p_before->next = NULL;
        p_node->prev = NULL;
    }

    void* value = p_node->value;

    free(p_node);
    --lst->size;

    return value;
}

int list_iter(
    const list* lst,
    list_iter_func iter_func,
    void* iter_func_user_arg
) {
    list_node* p_node = lst->head;
    if (p_node == NULL) {
        fprintf(stderr, "list_iter: list is empty");
        return -1;
    }

    int i = 0;
    do {
        list_node* p_next = p_node->next;
        iter_func(p_node, i++, iter_func_user_arg);
        p_node = p_next;
    }
    while (p_node != NULL);

    return 0;
}

/**
 * Iterator callback function that prints the list to stdout
 *
 * @param item Iterated list node
 * @param _index Iteration index (ignored)
 * @param _user_arg Ignored
 */
static void dump_iter_func(
    const list_node* item,
    size_t _index,
    void* _user_arg
) {
    printf("\"%s\", ", (char *)item->value);
}

void list_dump(const list* lst) {
    printf("[ ");
    list_iter(lst, dump_iter_func, NULL);
    printf(" ]\n");
}

/**
 * Iterator callback function that destroys all list nodes
 *
 * @param item Iterated list node
 * @param _index Iteration index
 * @param _user_arg Ignored
 */
static void destroy_iter_func(
    const list_node* item,
    size_t _index,
    void* _user_arg
) {
    free((void *)item);
}

int list_destroy(list* lst) {
    list_iter(lst, destroy_iter_func, NULL);

    lst->head = NULL;
    lst->tail = NULL;
    lst->size = 0;

    return 0;
}
