#ifndef __LIST_H_DEFINED__
#define __LIST_H_DEFINED__

/**
 * Doubly-linked list.
 */

/**
 * Linked list node.
 */
typedef struct list_node {
    /**
     * Value of node.
     */
    void* value;

    /**
     * Pointers to previous and next entries in list.
     */
    struct list_node *prev, *next;
} list_node;

/**
 * List.
 */
typedef struct list {
    /**
     * First node in list.
     */
    list_node* head;

    /**
     * Number of items in list.
     */
    size_t size;
} list;

/**
 * Initialize list.
 *
 * @param lst list.
 * @return 0 on success, -1 on failure.
 */
int list_init(list* lst);

/**
 *
 * @param lst list.
 * @param value
 * @param pos
 * @return 0 on success, -1 on failure.
 */
int list_insert_at(list* lst, void* value, size_t pos);

/**
 *
 * @param lst list.
 * @param pos
 * @return
 */
void* list_get_at(const list* lst, size_t pos);

/**
 *
 * @param lst list.
 * @param value
 * @return 0 on success, -1 on failure.
 */
int list_shift(list* lst, void* value);

/**
 *
 * @param lst list.
 * @return
 */
void* list_unshift(list* lst);

/**
 *
 * @param lst list.
 * @param value
 * @return 0 on success, -1 on failure.
 */
int list_push(list* lst, void* value);

/**
 *
 * @param lst list.
 * @return
 */
list_node* list_tail_node(const list* lst);

/**
 *
 * @param lst list.
 * @return
 */
void* list_tail(const list* lst);

/**
 *
 * @param lst list.
 * @return
 */
void* list_pop(list* lst);

/**
 *
 * @param lst list.
 * @param pos
 * @return 0 on success, -1 on failure.
 */
int list_del_at(list* lst, size_t pos);

/**
 *
 */
typedef void (*list_iter_func)(
    const list_node* lst,
    size_t index,
    void* user_arg
);

/**
 * Iterate list items.
 *
 * @param lst list.
 * @param iter_func iterator callback function.
 * @param iter_func_user_arg optional argument to pass to callback function.
 * @return 0 on success, -1 on failure.
 */
int list_iter(
    const list* lst,
    list_iter_func iter_func,
    void* iter_func_user_arg
);

/**
 * Print the list to the console.
 * Assumes that items are stored as strings.
 *
 * @param lst list.
 */
void list_dump(const list* lst);

/**
 * Destroy list.
 *
 * @param lst list.
 * @return 0 on success, -1 on failure.
 */
int list_destroy(list* lst);

#endif
