#include <stdio.h>
#include <stdlib.h>

#include "array_list.h"

/**
 * Allocate new array in array list
 *
 * @param lst Array list
 * @return 0 on success, -1 on failure
 */
static int alloc_array(array_list* lst) {
    void* new_array;

    if (lst->array == NULL) {
        // First allocation
        new_array = malloc(lst->capacity * lst->value_size);
    }
    else {
        // Subsequent allocation
        new_array = realloc(lst->array, lst->capacity * lst->value_size);
    }

    if (new_array == NULL) {
        perror("alloc_array: malloc() failed");
        return -1;
    }

    lst->array = new_array;

    return 0;
}

int array_list_init(array_list* lst, const size_t value_size, const size_t capacity) {
    lst->size = 0;
    lst->value_size = value_size;
    lst->capacity = capacity;
    lst->array = NULL;

    if (alloc_array(lst) != 0) {
        return -1;
    }

    return 0;
}

size_t array_list_index_of(const array_list* lst, const void* value) {
    for (size_t i = 0; i < lst->size; ++i) {
        if (lst->array[i] == value) {
            return i;
        }
    }

    return -1;
}

/**
 * Remove item at pos and shift items after it left
 *
 * @param lst List
 * @param pos Position to remove
 */
static void shift_left(array_list* lst, size_t pos) {
    --lst->size;

    for (size_t i = pos; i < lst->size; ++i) {
        lst->array[i] = lst->array[i + 1];
    }
}

/**
 * Remove item at pos and shift items before it right
 *
 * @param lst List
 * @param pos Position to remove
 */
static void shift_right(array_list* lst, size_t pos) {
    if ((pos + 1) >= lst->capacity) {
        array_list_resize(lst, pos + 1);
    }

    for (size_t i = lst->size; i > pos; --i) {
        lst->array[i] = lst->array[i - 1];
    }
}

int array_list_insert_at(array_list* lst, void* value, const size_t pos) {
    if (pos > lst->size) {
        fprintf(stderr, "array_list_insert_at: index %zu is out of bounds %zu\n", pos, lst->size);
        return -1;
    }

    if (lst->size == 0 && pos == 0) {
        lst->array[0] = value;
        ++lst->size;
        return 0;
    }

    shift_right(lst, pos);

    lst->array[pos] = value;
    ++lst->size;

    return 0;
}

void* array_list_get_at(const array_list* lst, const size_t pos) {
    if (pos >= lst->size) {
        return NULL;
    }

    return lst->array[pos];
}

void* array_list_del_at(array_list* lst, const size_t pos) {
    if (pos >= lst->size) {
        return NULL;
    }

    void* value = array_list_get_at(lst, pos);

    shift_left(lst, pos);

    return value;
}

void* array_list_del_value(array_list* lst, const void* value) {
    size_t index = array_list_index_of(lst, value);
    if (index == -1) {
        return NULL;
    }

    return array_list_del_at(lst, index);
}

int array_list_push_head(array_list* lst, void* value) {
    return array_list_insert_at(lst, value, 0);
}

void* array_list_pop_head(array_list* lst) {
    if (lst->size == 0) {
        return NULL;
    }

    return array_list_del_at(lst, 0);
}

int array_list_push_tail(array_list* lst, void* value) {
    return array_list_insert_at(lst, value, lst->size);
}

void* array_list_pop_tail(array_list* lst) {
    if (lst->size == 0) {
        return NULL;
    }

    return array_list_del_at(lst, lst->size - 1);
}

int array_list_resize(array_list* lst, const size_t capacity) {
    const size_t old_capacity = lst->capacity;

    if (capacity == lst->capacity) {
        // ignore
        return 0;
    }

    if (capacity < lst->capacity && capacity < lst->size) {
        fprintf(stderr, "array_list_resize: new capacity %zu is smaller than list size %zu\n", capacity, lst->size);
        return -1;
    }

    printf("array_list_resize: resizing from %zu to %zu\n", lst->capacity, capacity);
    lst->capacity = capacity;

    if (alloc_array(lst) == -1) {
        lst->capacity = old_capacity;
        return -1;
    }

    return 0;
}

int array_list_destroy(array_list* lst) {
    if (lst->array != NULL) {
        free(lst->array);
        lst->array = NULL;
    }

    lst->size = 0;
    lst->value_size = 0;
    lst->capacity = 0;

    return 0;
}
