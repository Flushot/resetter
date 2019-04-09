#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

static int find_index(hash_table *ht, void *key) {
    return (*ht->key_hash)(key, ht->size) % (ht->size - 1);
}

static int default_key_cmp(void *key_a, void *key_b) {
    return strcmp((char *)key_a, (char *)key_b);
}

static uint32_t default_key_hash(void *key, size_t ht_size) {
    int i;
    uint32_t hash = 0;
    char *str_key = (char *)key;

    for (i = 0; i < strlen(str_key); ++i) {
        hash += (uint8_t)str_key[i] % (ht_size - 1);
    }

    return hash;
}

int ht_init(hash_table *ht, uint32_t size, key_cmp_func key_cmp, key_hash_func key_hash) {
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

    ht->key_cmp = key_cmp == NULL ? default_key_cmp : key_cmp;
    ht->key_hash = key_hash == NULL ? default_key_hash : key_hash;

    return 0;
}

int ht_set(hash_table *ht, void *key, void *value) {
    int index;
    list *list;
    hash_table_entry *entry;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized");
        return -1;
    }

    entry = (hash_table_entry *)malloc(sizeof(hash_table_entry));
    if (entry == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(entry, 0, sizeof(hash_table_entry));
    entry->key = key;
    entry->value = value;

    index = find_index(ht, key);
    list = *(ht->index + index);
    if (list == NULL) {
        // First entry: Start a new linked list
        list = *(ht->index + index) = malloc(sizeof(list));
        if (list_init(list) != 0) {
            return -1;
        }
    }

    return list_push(*(ht->index + index), entry);
}

void *ht_get(hash_table *ht, void *key) {
    int index;
    list *list;
    list_node *curr;
    hash_table_entry *entry;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized");
        return NULL;
    }

    index = find_index(ht, key);
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
    } while (curr != NULL);

    // No entry
    return NULL;
}

int ht_del(hash_table *ht, void *key) {
    int index, i = 0;
    list *list;
    list_node *curr;
    hash_table_entry *entry;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized");
        return -1;
    }

    index = find_index(ht, key);
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
        } else {
            ++i;
        }

        curr = curr->next;
    } while (curr != NULL);

    // No entry
    return found ? 0 : -1;
}

int ht_destroy(hash_table *ht) {
    int i;
    list *list;
    list_node *curr;
    hash_table_entry *entry;

    if (ht->index != NULL) {
        for (i = 0; i < ht->size; ++i) {
            list = ht->index[i];
            if (list != NULL) {
                curr = list->head;
                if (curr != NULL) {
                    do {
                        entry = curr->value;
                        free(entry);
                        curr = curr->next;
                    } while (curr != NULL);
                }
            }
        }

        free(ht->index);
        ht->index = NULL;
    }

    return 0;
}

void ht_dump(hash_table *ht) {
    int i;
    list *list;
    list_node *iter;

    if (ht->index == NULL) {
        fprintf(stderr, "hash table not initialized");
        return;
    }

    for (i = 0; i < ht->size; ++i) {
        list = ht->index[i];
        if (list != NULL) {
            printf("%d: ", i);
            printf("[ ");

            iter = list->head;
            if (iter != NULL) {
                do {
                    hash_table_entry *entry = iter->value;
                    printf("\"%s\" => \"%s\", ", entry->key, entry->value);
                    iter = iter->next;
                } while (iter != NULL);
            }

            printf("]\n");
        }
    }
}
