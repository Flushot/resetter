#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "murmur3.h"

/**
 * Array builder iterator user_arg
 */
struct ht_array_builder_arg {
    size_t index;

    /**
     * Array to store items in
     */
    void** items;
};

/**
 * Get hash table index pointer for given key
 * Hashes the key then computes its index offset
 *
 * @param ht Hash table
 * @param key Key to compute index for
 * @return Computed index
 */
static size_t find_index(const hash_table* ht, const void* key) {
    return (*ht->key_hash)(key, ht->index_size) % (ht->index_size - 1);
}

/**
 * Default key comparison function (strcmp)
 *
 * @param key_a Key A to compare
 * @param key_b Key B to compare
 * @return Same as strcmp()
 */
static int default_key_cmp(const void* key_a, const void* key_b) {
    return strcmp(key_a, key_b);
}

/**
 * Default hashing function (murmur3)
 *
 * @param key Key to hash
 * @param ht_size Size of hash table index
 * @return Hash value
 */
static uint32_t default_key_hash(const void* key, size_t ht_size) {
    static uint32_t hash_seed = -1;
    if (hash_seed == -1) {
        hash_seed = rand();
    }

    return murmur3(key, strlen(key), hash_seed) % (ht_size - 1);
}

int ht_init(
    hash_table* ht,
    const uint32_t size,
    const key_cmp_func key_cmp,
    const key_hash_func key_hash
) {
    memset(ht, 0, sizeof(hash_table));
    ht->index_size = size;

    size_t index_size = size * sizeof(list);
    ht->index = malloc(index_size);
    if (ht->index == NULL) {
        perror("ht_init: malloc() failed");
        return -1;
    }

    memset(ht->index, 0, index_size);

    ht->entry_size = 0;

    ht->key_cmp = key_cmp == NULL ? default_key_cmp : key_cmp;
    ht->key_hash = key_hash == NULL ? default_key_hash : key_hash;

    return 0;
}

int ht_rehash(hash_table* ht, const uint32_t new_size) {
    list** old_index = ht->index;
    const size_t old_index_size = ht->index_size;

    size_t index_size = new_size * sizeof(list);
    ht->index = malloc(index_size);
    if (ht->index == NULL) {
        perror("ht_rehash: malloc() failed");
        return -1;
    }

    memset(ht->index, 0, index_size);
    ht->index_size = new_size;

    // Rebuild index
    for (int i = 0; i < old_index_size; ++i) {
        const list* p_list = old_index[i];
        if (p_list != NULL) {
            const list_node* p_iter = p_list->head;
            if (p_iter != NULL) {
                do {
                    hash_table_entry* p_entry = p_iter->value;
                    ht_set_entry(ht, p_entry);
                    p_iter = p_iter->next;
                }
                while (p_iter != NULL);
            }
        }
    }

    free(old_index);

    return 0;
}

hash_table_entry* ht_init_entry(
    const void* key, size_t key_size,
    const void* value, size_t value_size
) {
    hash_table_entry* p_entry = malloc(sizeof(hash_table_entry));
    if (p_entry == NULL) {
        perror("ht_init_entry: malloc() failed for entry");
        return NULL;
    }

    p_entry->must_destroy = 1;

    p_entry->key = malloc(key_size);
    if (p_entry->key == NULL) {
        perror("ht_init_entry: malloc() failed for key");
        free(p_entry);
        return NULL;
    }

    p_entry->value = malloc(value_size);
    if (p_entry->value == NULL) {
        perror("ht_init_entry: malloc() failed for value");
        free(p_entry->key);
        free(p_entry);
        return NULL;
    }

    memcpy(p_entry->key, key, key_size);
    memcpy(p_entry->value, value, value_size);

    return p_entry;
}

int ht_destroy_entry(const hash_table_entry* entry) {
    free(entry->key);
    free(entry->value);
    free((void *)entry);

    return 0;
}

int ht_set(hash_table* ht, void* key, void* value) {
    hash_table_entry* p_entry = malloc(sizeof(hash_table_entry));
    if (p_entry == NULL) {
        perror("ht_set: malloc() failed");
        return -1;
    }

    memset(p_entry, 0, sizeof(hash_table_entry));
    p_entry->key = key;
    p_entry->value = value;

    return ht_set_entry(ht, p_entry);
}

int ht_set_entry(hash_table* ht, hash_table_entry* entry) {
    if (ht->index == NULL) {
        fprintf(stderr, "ht_set_entry: hash table not initialized\n");
        return -1;
    }

    const size_t index = find_index(ht, entry->key);
    list* p_list = *(ht->index + index);
    if (p_list == NULL) {
        // First entry: Start a new linked list
        p_list = *(ht->index + index) = malloc(sizeof(list));
        if (list_init(p_list) != 0) {
            free(p_list);
            return -1;
        }
    }
    else {
        list_node* p_curr = p_list->head;
        do {
            hash_table_entry* p_curr_ent = p_curr->value;
            if ((*ht->key_cmp)(p_curr_ent->key, entry->key) == 0) {
                // Update existing value
                p_curr_ent->value = entry->value;
                free(entry);
                return 0;
            }

            p_curr = p_curr->next;
        }
        while (p_curr != NULL);
    }

    ++ht->entry_size;

    // Add to list
    return list_push_tail(*(ht->index + index), entry);
}

void* ht_get(const hash_table* ht, const void* key) {
    if (ht->index == NULL) {
        fprintf(stderr, "ht_get: hash table not initialized\n");
        return NULL;
    }

    size_t index = find_index(ht, key);
    list* p_list = ht->index[index];
    if (p_list == NULL || p_list->size == 0) {
        // No entry
        return NULL;
    }

    list_node* p_curr = p_list->head;
    do {
        hash_table_entry* p_entry = p_curr->value;
        if ((*ht->key_cmp)(p_entry->key, key) == 0) {
            return p_entry->value;
        }

        p_curr = p_curr->next;
    }
    while (p_curr != NULL);

    // No entry
    return NULL;
}

int ht_del(hash_table* ht, const void* key) {
    if (ht->index == NULL) {
        fprintf(stderr, "ht_del: hash table not initialized\n");
        return -1;
    }

    size_t index = find_index(ht, key);
    list* p_list = ht->index[index];
    if (p_list == NULL || p_list->size == 0) {
        // No entry
        return -1;
    }

    int i = 0;
    size_t deleted_count = 0;
    list_node* p_curr = p_list->head;
    while (p_curr != NULL) {
        hash_table_entry* p_entry = p_curr->value;
        if ((*ht->key_cmp)(p_entry->key, key) == 0) {
            list_del_at(p_list, i);
            ++deleted_count;
        }
        else {
            ++i;
        }

        p_curr = p_curr->next;
    };

    ht->entry_size -= deleted_count;

    // No entry
    return deleted_count > 0 ? 0 : -1;
}

/**
 * Iterator callback function that destroys all entries
 *
 * @param entry Iterated hash table entry
 * @param _index Iteration index (ignored)
 * @param _user_arg Ignored
 */
static void destroy_iter_func(
    const hash_table_entry* entry,
    const size_t _index,
    void* _user_arg
) {
    if (entry->must_destroy) {
        ht_destroy_entry(entry);
    }
    else {
        free((void *)entry);
    }
}

int ht_destroy(hash_table* ht) {
    if (ht->index == NULL) {
        return -1;
    }

    ht_iter(ht, destroy_iter_func, NULL);
    free(ht->index);
    ht->index = NULL;

    ht->entry_size = 0;

    return 0;
}

int ht_iter(
    const hash_table* ht,
    ht_iter_func iter_func,
    void* iter_func_user_arg
) {
    if (ht->index == NULL) {
        fprintf(stderr, "ht_iter: hash table not initialized\n");
        return -1;
    }

    for (int i = 0; i < ht->index_size; ++i) {
        const list* p_list = ht->index[i];
        if (p_list != NULL) {
            const list_node* p_iter = p_list->head;
            if (p_iter != NULL) {
                do {
                    hash_table_entry* p_entry = p_iter->value;
                    p_iter = p_iter->next;
                    iter_func(p_entry, i, iter_func_user_arg);
                }
                while (p_iter != NULL);
            }
        }
    }

    return 0;
}

/**
 * Iterator callback function that prints the hash table to stdout
 *
 * @param entry Iterated hash table entry
 * @param index Iteration index
 * @param _user_arg Ignored
 */
static void dump_iter_func(
    const hash_table_entry* entry,
    const size_t index,
    void* _user_arg
) {
    printf("%zu: { \"%s\" => \"%s\" }\n", index, (char *)entry->key, (char *)entry->value);
}

void ht_dump(const hash_table* ht) {
    ht_iter(ht, dump_iter_func, NULL);
}

/**
 * Iterator callback function that builds an array of keys
 *
 * @param entry Iterated hash table entry
 * @param _index Iteration index
 * @param user_arg ht_array_builder_arg accumulator for iterated keys
 */
static void keys_iter_func(
    const hash_table_entry* entry,
    const size_t _index,
    void* user_arg
) {
    struct ht_array_builder_arg* arg = user_arg;

    arg->items[arg->index++] = entry->key;
}

size_t ht_keys(const hash_table* ht, void** keys) {
    struct ht_array_builder_arg user_arg;

    memset(&user_arg, 0, sizeof(user_arg));
    user_arg.items = keys;

    if (ht_iter(ht, keys_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}

size_t ht_size(const hash_table* ht) {
    return ht->entry_size;
}

/**
 * Iterator callback function that builds an array of values
 *
 * @param entry Iterated hash table entry
 * @param _index Iteration index
 * @param user_arg ht_array_builder_arg accumulator for iterated values
 */
static void ht_values_iter_func(
    const hash_table_entry* entry,
    const size_t _index,
    void* user_arg
) {
    struct ht_array_builder_arg* arg = user_arg;

    arg->items[arg->index++] = entry->value;
}

size_t ht_values(const hash_table* ht, void** values) {
    struct ht_array_builder_arg user_arg;

    memset(&user_arg, 0, sizeof(user_arg));
    user_arg.items = values;

    if (ht_iter(ht, ht_values_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}
