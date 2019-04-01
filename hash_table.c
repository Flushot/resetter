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
    char *strkey = (char *)key;
    uint32_t hash = 0;
    int i;

    for (i = 0; i < strlen(strkey); ++i) {
        hash += (uint8_t)strkey[i] % (ht_size - 1);
    }

    return hash;
}

int ht_init(hash_table *ht, uint32_t size, key_cmp_func key_cmp, key_hash_func key_hash) {
    memset(ht, 0, sizeof(hash_table));
    ht->size = size;

    size_t index_size = size * sizeof(hash_table_entry);
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
    hash_table_entry *entry = (hash_table_entry *)malloc(sizeof(hash_table_entry));
    if (entry == NULL) {
        perror("malloc() failed");
        return -1;
    }

    memset(entry, 0, sizeof(hash_table_entry));
    entry->key = key;
    entry->value = value;
    entry->next = NULL;

    int index = find_index(ht, key);
    void *curr = ht->index[index];
    if (curr == NULL) {
        // First entry
        *(ht->index + index) = entry;
        return 0;
    } else {
        // Collision: Append to tail or update existing value
        hash_table_entry *tail, *list = curr;
        do {
            tail = list;
            if ((*ht->key_cmp)(tail->key, key) == 0) {
                // Update existing value
                tail->value = value;
                return 0;
            }

            if (list->next == NULL) {
                // Append new value to tail
                list->next = entry;
                return 0;
            }

            list = list->next;
        } while (list != NULL);
    }

    return -1;
}

void *ht_get(hash_table *ht, void *key) {
    int index = find_index(ht, key);
    hash_table_entry *entry = ht->index[index];
    if (entry == NULL) {
        // No entry
        return NULL;
    }

    hash_table_entry *tail, *list = entry;
    do {
        tail = list;
        if ((*ht->key_cmp)(tail->key, key) == 0) {
            return tail->value;
        }

        list = list->next;
    } while (list != NULL);

    // No entry
    return NULL;
}

int ht_del(hash_table *ht, void *key) {
    int index = find_index(ht, key);
    hash_table_entry *entry = ht->index[index];
    if (entry == NULL) {
        // No entry
        return -1;
    }

    if (entry->next == NULL) {
        // Single entry
        if ((*ht->key_cmp)(entry->key, key) == 0) {
            free(entry);
            ht->index[index] = NULL;
            return 0;
        } else {
            // No entry
            return -1;
        }
    }

    // Multiple entries
    hash_table_entry *curr, *prev = NULL, *list = entry;
    do {
        curr = list;
        if ((*ht->key_cmp)(curr->key, key) == 0) {
            // Remove item from linked list
            if (prev != NULL) {
                prev->next = curr->next;
            }

            free(curr);
        }

        prev = curr;
        list = list->next;
    } while (list != NULL);

    // No entry
    return -1;
}

int ht_destroy(hash_table *ht) {
    int i;

    for (i = 0; i < ht->size; ++i) {
        hash_table_entry *curr, *list = ht->index[i];
        if (list != NULL) {
            do {
                curr = list;
                list = curr->next;
                free(curr);
            } while (list != NULL);
        }
    }

    free(ht->index);

    return 0;
}

void ht_dump(hash_table *ht) {
    int i;

    for (i = 0; i < ht->size; ++i) {
        hash_table_entry *curr, *list = ht->index[i];
        if (list != NULL) {
            do {
                curr = list;
                printf("%s (%d): %s\n", (char *)curr->key, i, (char *)curr->value);
                list = curr->next;
            } while (list != NULL);
        }
    }
}
