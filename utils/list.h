#ifndef __LIST_H_DEFINED__
#define __LIST_H_DEFINED__

/**
 * Doubly-linked list
 */

/**
 * Linked list node
 */
typedef struct list_node {
    void* value;
    struct list_node *prev, *next;
} list_node;

/**
 * List.
 */
typedef struct list {
    list_node *head, *tail;
    size_t size;
} list;

/**
 * Initialize list
 *
 * @param lst Empty list to initialize
 * @return 0 on success, -1 on failure
 */
int list_init(list* lst);

/**
 * Insert value into list at position
 *
 * @param lst List
 * @param value Value to insert
 * @param pos Position to insert at
 * @return 0 on success, -1 on failure
 */
int list_insert_at(list* lst, void* value, size_t pos);

/**
 * Get value at position in list
 *
 * @param lst List
 * @param pos Position to get item at
 * @return Value of item (or NULL it not found)
 */
void* list_get_at(const list* lst, size_t pos);

/**
 * Delete value at position in list
 *
 * @param lst List
 * @param pos Position to delete at
 * @return 0 on success, -1 on failure
 */
int list_del_at(list* lst, size_t pos);

/**
 * Get first (head) value from list
 *
 * @param lst List
 * @return Value of head (or NULL if empty)
 */
void* list_head(const list* lst);

/**
 * Push value to head of list (prepend)
 *
 * @param lst List
 * @param value Value
 * @return 0 on success, -1 on failure
 */
int list_push_head(list* lst, void* value);

/**
 * Pop value from head of list
 *
 * @param lst List
 * @return Value that was popped (or NULL if not found)
 */
void* list_pop_head(list* lst);

/**
 * Get last value in list
 *
 * @param lst List
 * @return Value of tail (or NULL if empty)
 */
void* list_tail(const list* lst);

/**
 * Push value to tail of list (append)
 *
 * @param lst List
 * @param value Value
 * @return 0 on success, -1 on failure
 */
int list_push_tail(list* lst, void* value);

/**
 * Pop last (tail) value from list
 *
 * @param lst List
 * @return Value from tail (or NULL if empty)
 */
void* list_pop_tail(list* lst);

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
 * @param lst List
 * @param iter_func Iterator callback function
 * @param iter_func_user_arg Optional argument to pass to callback function
 * @return 0 on success, -1 on failure
 */
int list_iter(
    const list* lst,
    list_iter_func iter_func,
    void* iter_func_user_arg
);

/**
 * Print the list to the console
 * Assumes that items are stored as strings
 *
 * @param lst List
 */
void list_dump(const list* lst);

/**
 * Destroy list
 *
 * @param lst List
 * @return 0 on success, -1 on failure
 */
int list_destroy(list* lst);

#endif
