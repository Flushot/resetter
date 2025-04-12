#ifndef __LIST_H_DEFINED__
#define __LIST_H_DEFINED__

/**
 * Doubly-linked list
 */

/**
 * Linked list node
 */
typedef struct list_node {
    /**
     * Value of node
     */
    void* value;

    /**
     * Pointers to previous and next entries in list
     */
    struct list_node *prev, *next;
} list_node;

/**
 * List.
 */
typedef struct list {
    /**
     * First node in list
     */
    list_node *head, *tail;

    /**
     * Number of nodes in list
     */
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
 * Insert item in list at position
 *
 * @param lst List
 * @param value Value to insert
 * @param pos Position to insert at
 * @return 0 on success, -1 on failure
 */
int list_insert_at(list* lst, void* value, const size_t pos);

/**
 * Get item at position in list
 *
 * @param lst List
 * @param pos Position to get item at
 * @return Value of item (or NULL it not found)
 */
void* list_get_at(const list* lst, const size_t pos);

/**
 *
 * @param lst List
 * @param pos Position to delete at
 * @return 0 on success, -1 on failure
 */
int list_del_at(list* lst, const size_t pos);

/**
 * Get first (head) item from list
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
 * Pop item from head of list
 *
 * @param lst List
 * @return Value that was popped (or NULL if not found)
 */
void* list_pop_head(list* lst);

/**
 * Get last item in list
 *
 * @param lst List
 * @return Value of tail (or NULL if empty)
 */
void* list_tail(const list* lst);

/**
 *
 * @param lst List
 * @param value Value
 * @return 0 on success, -1 on failure
 */
int list_push_tail(list* lst, void* value);

/**
 * Pop last (tail) item from list
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
