#ifndef __LIST_H_DEFINED__
#define __LIST_H_DEFINED__

/**
 * Doubly-linked list node
 */
typedef struct linked_list_node {
    void* value;
    struct linked_list_node *prev, *next;
} list_node;

/**
 * Doubly-linked list
 */
typedef struct linked_list {
    list_node *head, *tail;
    size_t size;
} list;

/**
 * Initialize list
 *
 * @param lst Empty list to initialize
 * @return 0 on success, -1 on failure
 */
int linked_list_init(list* lst);

/**
 * Insert value into list at position
 *
 * @param lst List
 * @param value Value to insert
 * @param pos Position to insert at
 * @return 0 on success, -1 on failure
 */
int linked_list_insert_at(list* lst, void* value, size_t pos);

/**
 * Get value at position in list
 *
 * @param lst List
 * @param pos Position to get item at
 * @return Value of item (or NULL it not found)
 */
void* linked_list_get_at(const list* lst, size_t pos);

/**
 * Delete value at position in list
 *
 * @param lst List
 * @param pos Position to delete at
 * @return 0 on success, -1 on failure
 */
int linked_list_del_at(list* lst, size_t pos);

/**
 * Get first (head) value from list
 *
 * @param lst List
 * @return Value of head (or NULL if empty)
 */
void* linked_list_head(const list* lst);

/**
 * Push value to head of list (prepend)
 *
 * @param lst List
 * @param value Value
 * @return 0 on success, -1 on failure
 */
int linked_list_push_head(list* lst, void* value);

/**
 * Pop value from head of list
 *
 * @param lst List
 * @return Value that was popped (or NULL if not found)
 */
void* linked_list_pop_head(list* lst);

/**
 * Get last value in list
 *
 * @param lst List
 * @return Value of tail (or NULL if empty)
 */
void* linked_list_tail(const list* lst);

/**
 * Push value to tail of list (append)
 *
 * @param lst List
 * @param value Value
 * @return 0 on success, -1 on failure
 */
int linked_list_push_tail(list* lst, void* value);

/**
 * Pop last (tail) value from list
 *
 * @param lst List
 * @return Value from tail (or NULL if empty)
 */
void* linked_list_pop_tail(list* lst);

/**
 * List iterator callback function
 *
 * @param lst Iterated list node
 * @param index Iteration index
 * @param user_arg Optional user arg
 */
typedef void (*list_iter_func)(
    const list_node* lst,
    size_t index,
    void* user_arg
);

/**
 * Iterate list items
 *
 * @param lst List
 * @param iter_func Iterator callback function
 * @param iter_func_user_arg Optional argument to pass to callback function
 * @return 0 on success, -1 on failure
 */
int linked_list_iter(
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
void linked_list_dump(const list* lst);

/**
 * Destroy list
 *
 * @param lst List
 * @return 0 on success, -1 on failure
 */
int linked_list_destroy(list* lst);

#endif
