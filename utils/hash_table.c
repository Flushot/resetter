#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "murmur3.h"

static uint32_t hash_seed = 0x00000000; // TODO: randomize

struct ht_array_builer_arg_t {
    int index;
    void** items;
};

static int _find_index(hash_table* ht, void* key) {
    return (*ht->key_hash)(key, ht->size) % (ht->size - 1);
}

static int _default_key_cmp(void* key_a, void* key_b) {
    return strcmp((char *)key_a, (char *)key_b);
}

static uint32_t _default_key_hash(void* key, size_t ht_size) {
    return murmur3((uint8_t *)key, strlen(key), hash_seed) % (ht_size - 1);
}

int ht_init(
    hash_table* ht,
    uint32_t size,
    key_cmp_func key_cmp,
    key_hash_func key_hash
) {
    size_t index_size;

    memset(ht, 0, sizeof(hash_table));
    ht->size = size;

    index_size = size * sizeof(list);
    ht->index = malloc(index_size);
    if (ht->index == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(ht->index, 0, index_size);

    ht->key_cmp = key_cmp == NULL ? _default_key_cmp : key_cmp;
    ht->key_hash = key_hash == NULL ? _default_key_hash : key_hash;

    return 0;
}

hash_table_entry* ht_init_entry(
    void* key, size_t key_size,
    void* value, size_t value_size
) {
    hash_table_entry* entry = malloc(sizeof(hash_table_entry));
    if (entry == NULL) {
        perror("malloc() failed");
        return NULL;
    }

    entry->must_destroy = 1;

    entry->key = malloc(key_size);
    if (entry->key == NULL) {
        perror("malloc() failed");
        free(entry);
        return NULL;
    }

    entry->value = malloc(value_size);
    if (entry->value == NULL) {
        perror("malloc() failed");
        free(entry->key);
        free(entry);
        return NULL;
    }

    memcpy(entry->key, key, key_size);
    memcpy(entry->value, value, value_size);

    return entry;
}

int ht_destroy_entry(hash_table_entry* entry) {
    free(entry->key);
    free(entry->value);
    free(entry);

    return 0;
}

int ht_set(hash_table* ht, void* key, void* value) {
    hash_table_entry* entry = malloc(sizeof(hash_table_entry));
    if (entry == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(entry, 0, sizeof(hash_table_entry));
    entry->key = key;
    entry->value = value;

    return ht_set_entry(ht, entry);
}

int ht_set_entry(hash_table* ht, hash_table_entry* entry) {
    int index;
    list* list;
    list_node* curr;
    hash_table_entry* curr_entry;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return -1;
    }

    index = _find_index(ht, entry->key);
    list = *(ht->index + index);
    if (list == NULL) {
        // First entry: Start a new linked list
        list = *(ht->index + index) = malloc(sizeof(list));
        if (list_init(list) != 0) {
            return -1;
        }
    }
    else {
        curr = list->head;
        do {
            curr_entry = curr->value;
            if ((*ht->key_cmp)(curr_entry->key, entry->key) == 0) {
                // Update existing value
                curr_entry->value = entry->value;
                free(entry);
                return 0;
            }

            curr = curr->next;
        }
        while (curr != NULL);
    }

    // Add to list
    return list_push(*(ht->index + index), entry);
}

void* ht_get(hash_table* ht, void* key) {
    int index;
    list* list;
    list_node* curr;
    hash_table_entry* entry;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return NULL;
    }

    index = _find_index(ht, key);
    list = ht->index[index];
    if (list == NULL || list->size == 0) {
        // No entry
        return NULL;
    }

    curr = list->head;
    do {
        entry = curr->value;
        if ((*ht->key_cmp)(entry->key, key) == 0) {
            return entry->value;
        }

        curr = curr->next;
    }
    while (curr != NULL);

    // No entry
    return NULL;
}

int ht_del(hash_table* ht, void* key) {
    int index, i = 0;
    list* list;
    list_node* curr;
    hash_table_entry* entry;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return -1;
    }

    index = _find_index(ht, key);
    list = ht->index[index];
    if (list == NULL || list->size == 0) {
        // No entry
        return -1;
    }

    int found = 0;
    curr = list->head;
    do {
        entry = curr->value;
        if ((*ht->key_cmp)(entry->key, key) == 0) {
            list_del_at(list, i);
            found = 1;
        }
        else {
            ++i;
        }

        curr = curr->next;
    }
    while (curr != NULL);

    // No entry
    return found ? 0 : -1;
}

static void _ht_destroy_iter_func(
    hash_table_entry* entry,
    int _index,
    void* _user_arg
) {
    if (entry->must_destroy) {
        ht_destroy_entry(entry);
    }
    else {
        free(entry);
    }
}

int ht_destroy(hash_table* ht) {
    if (ht->index == NULL) {
        return -1;
    }

    ht_iter(ht, _ht_destroy_iter_func, NULL);
    free(ht->index);
    ht->index = NULL;

    return 0;
}

int ht_iter(
    hash_table* ht,
    ht_iter_func iter_func,
    void* iter_func_user_arg
) {
    int i;
    list* list;
    list_node* iter;
    hash_table_entry* entry;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return -1;
    }

    for (i = 0; i < ht->size; ++i) {
        list = ht->index[i];
        if (list != NULL) {
            iter = list->head;
            if (iter != NULL) {
                do {
                    entry = iter->value;
                    iter = iter->next;
                    iter_func(entry, i, iter_func_user_arg);
                }
                while (iter != NULL);
            }
        }
    }

    return 0;
}

static void _ht_dump_iter_func(
    hash_table_entry* entry,
    int index,
    void* _user_arg
) {
    printf("%d: { \"%s\" => \"%s\" }\n", index, (char *)entry->key, (char *)entry->value);
}

void ht_dump(hash_table* ht) {
    ht_iter(ht, _ht_dump_iter_func, NULL);
}

static void _ht_keys_iter_func(
    hash_table_entry* entry,
    int _index,
    void* user_arg
) {
    struct ht_array_builer_arg_t* arg = (struct ht_array_builer_arg_t *)user_arg;

    arg->items[arg->index++] = entry->key;
}

int ht_keys(hash_table* ht, void** keys) {
    struct ht_array_builer_arg_t user_arg;

    memset(&user_arg, 0, sizeof(user_arg));
    user_arg.items = keys;

    if (ht_iter(ht, _ht_keys_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}

static void _ht_size_iter_func(
    hash_table_entry* _entry,
    int _index,
    void* user_arg
) {
    struct ht_array_builer_arg_t* arg = (struct ht_array_builer_arg_t *)user_arg;

    ++arg->index;
}

int ht_size(hash_table* ht) {
    struct ht_array_builer_arg_t user_arg;

    memset(&user_arg, 0, sizeof(user_arg));

    if (ht_iter(ht, _ht_size_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}

static void _ht_values_iter_func(
    hash_table_entry* entry,
    int _index,
    void* user_arg
) {
    struct ht_array_builer_arg_t* arg = (struct ht_array_builer_arg_t *)user_arg;

    arg->items[arg->index++] = entry->value;
}

int ht_values(hash_table* ht, void** values) {
    struct ht_array_builer_arg_t user_arg;

    memset(&user_arg, 0, sizeof(user_arg));
    user_arg.items = values;

    if (ht_iter(ht, _ht_values_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}
