#ifndef __ARRAY_LIST_H_DEFINED__
#define __ARRAY_LIST_H_DEFINED__

typedef struct array_list {
  size_t size;
  size_t value_size;
  size_t capacity;
  void** array;
} array_list;

/**
 * Initialize array list
 *
 * @param lst Empty list to initialize
 * @param value_size Size of each value in array
 * @param capacity Max capacity of array list (before it gets resized)
 * @return 0 on success, -1 on failure
 */
int array_list_init(array_list* lst, size_t value_size, size_t capacity);

/**
 * Find index of value
 *
 * @param lst Array list
 * @param value Value to search
 * @return Index or -1 if not found
 */
size_t array_list_index_of(array_list* lst, void* value);

/**
 * Insert value into array list at position
 *
 * @param lst Array list
 * @param value Value to insert
 * @param pos Position to insert at
 * @return 0 on success, -1 on failure
 */
int array_list_insert_at(array_list* lst, void* value, size_t pos);

/**
 * Get value at position in array list
 *
 * @param lst Array list
 * @param pos Position to get item at
 * @return Value of item (or NULL it not found)
 */
void* array_list_get_at(const array_list* lst, size_t pos);

/**
 * Delete value at position in array list
 *
 * @param lst Array list
 * @param pos Position to delete at
 * @return Value at position (or NULL if not found)
 */
void* array_list_del_at(array_list* lst, size_t pos);

/**
 * Delete value from array list
 *
 * @param lst Array list
 * @param value Value to delete
 * @return Deleted value (or NULL if not found)
 */
void* array_list_del_value(array_list* lst, void* value);

/**
 * Push value to head of array list (prepend)
 *
 * @param lst Array list
 * @param value Value
 * @return 0 on success, -1 on failure
 */
int array_list_push_head(array_list* lst, void* value);

/**
 * Pop value from head of array list
 *
 * @param lst Array list
 * @return Value that was popped (or NULL if not found)
 */
void* array_list_pop_head(array_list* lst);

/**
 * Push value to tail of array list (append)
 *
 * @param lst Array list
 * @param value Value
 * @return 0 on success, -1 on failure
 */
int array_list_push_tail(array_list* lst, void* value);

/**
 * Pop last (tail) value from array list
 *
 * @param lst Array list
 * @return Value from tail (or NULL if empty)
 */
void* array_list_pop_tail(array_list* lst);

/**
 * Resize array list
 *
 * @param lst Array list
 * @param capacity New capacity
 * @return 0 on success, -1 on failure
 */
int array_list_resize(array_list* lst, size_t capacity);

/**
 * Destroy array list
 *
 * @param lst Array list
 * @return 0 on success, -1 on failure
 */
int array_list_destroy(array_list* lst);

#endif
