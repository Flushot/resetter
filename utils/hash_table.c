#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "murmur3.h"

static uint32_t hash_seed = 0x00000000; // TODO: randomize

struct ht_array_builder_arg_t {
    int index;
    void** items;
};

static size_t _find_index(const hash_table* ht, const void* key) {
    return (*ht->key_hash)(key, ht->size) % (ht->size - 1);
}

static int _default_key_cmp(const void* key_a, const void* key_b) {
    return strcmp(key_a, key_b);
}

static uint32_t _default_key_hash(const void* key, size_t ht_size) {
    return murmur3(key, strlen(key), hash_seed) % (ht_size - 1);
}

int ht_init(
    hash_table* ht,
    uint32_t size,
    key_cmp_func key_cmp,
    key_hash_func key_hash
) {
    memset(ht, 0, sizeof(hash_table));
    ht->size = size;

    size_t index_size = size * sizeof(list);
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
    const void* key, size_t key_size,
    const void* value, size_t value_size
) {
    hash_table_entry* p_entry = malloc(sizeof(hash_table_entry));
    if (p_entry == NULL) {
        perror("malloc() failed");
        return NULL;
    }

    p_entry->must_destroy = 1;

    p_entry->key = malloc(key_size);
    if (p_entry->key == NULL) {
        perror("malloc() failed");
        free(p_entry);
        return NULL;
    }

    p_entry->value = malloc(value_size);
    if (p_entry->value == NULL) {
        perror("malloc() failed");
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

int ht_set(const hash_table* ht, void* key, void* value) {
    hash_table_entry* p_entry = malloc(sizeof(hash_table_entry));
    if (p_entry == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(p_entry, 0, sizeof(hash_table_entry));
    p_entry->key = key;
    p_entry->value = value;

    return ht_set_entry(ht, p_entry);
}

int ht_set_entry(const hash_table* ht, hash_table_entry* entry) {
    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return -1;
    }

    size_t index = _find_index(ht, entry->key);
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

    // Add to list
    return list_push(*(ht->index + index), entry);
}

void* ht_get(const hash_table* ht, const void* key) {
    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return NULL;
    }

    size_t index = _find_index(ht, key);
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

int ht_del(const hash_table* ht, const void* key) {
    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return -1;
    }

    size_t index = _find_index(ht, key);
    list* p_list = ht->index[index];
    if (p_list == NULL || p_list->size == 0) {
        // No entry
        return -1;
    }

    int i = 0;
    int found = 0;
    list_node* p_curr = p_list->head;
    do {
        hash_table_entry* p_entry = p_curr->value;
        if ((*ht->key_cmp)(p_entry->key, key) == 0) {
            list_del_at(p_list, i);
            found = 1;
        }
        else {
            ++i;
        }

        p_curr = p_curr->next;
    }
    while (p_curr != NULL);

    // No entry
    return found ? 0 : -1;
}

static void _ht_destroy_iter_func(
    const hash_table_entry* entry,
    int _index,
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

    ht_iter(ht, _ht_destroy_iter_func, NULL);
    free(ht->index);
    ht->index = NULL;

    return 0;
}

int ht_iter(
    const hash_table* ht,
    ht_iter_func iter_func,
    void* iter_func_user_arg
) {
    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized\n");
        return -1;
    }

    for (int i = 0; i < ht->size; ++i) {
        list* p_list = ht->index[i];
        if (p_list != NULL) {
            list_node* p_iter = p_list->head;
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

static void _ht_dump_iter_func(
    const hash_table_entry* entry,
    int index,
    void* _user_arg
) {
    printf("%d: { \"%s\" => \"%s\" }\n", index, (char *)entry->key, (char *)entry->value);
}

void ht_dump(const hash_table* ht) {
    ht_iter(ht, _ht_dump_iter_func, NULL);
}

static void _ht_keys_iter_func(
    const hash_table_entry* entry,
    int _index,
    void* user_arg
) {
    struct ht_array_builder_arg_t* arg = user_arg;

    arg->items[arg->index++] = entry->key;
}

int ht_keys(const hash_table* ht, void** keys) {
    struct ht_array_builder_arg_t user_arg;

    memset(&user_arg, 0, sizeof(user_arg));
    user_arg.items = keys;

    if (ht_iter(ht, _ht_keys_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}

static void _ht_size_iter_func(
    const hash_table_entry* _entry,
    int _index,
    void* user_arg
) {
    struct ht_array_builder_arg_t* arg = user_arg;

    ++arg->index;
}

int ht_size(const hash_table* ht) {
    struct ht_array_builder_arg_t user_arg;

    memset(&user_arg, 0, sizeof(user_arg));

    if (ht_iter(ht, _ht_size_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}

static void _ht_values_iter_func(
    const hash_table_entry* entry,
    int _index,
    void* user_arg
) {
    struct ht_array_builder_arg_t* arg = user_arg;

    arg->items[arg->index++] = entry->value;
}

int ht_values(const hash_table* ht, void** values) {
    struct ht_array_builder_arg_t user_arg;

    memset(&user_arg, 0, sizeof(user_arg));
    user_arg.items = values;

    if (ht_iter(ht, _ht_values_iter_func, &user_arg) != 0) {
        return -1;
    }

    return user_arg.index;
}
