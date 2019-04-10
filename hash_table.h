#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <inttypes.h>
#include "list.h"

/**
 * Closed-addressed general purpose hash table with
 * a customizable hash function and key comparator.
 */

typedef struct _hash_table_entry {
    void *key;
    void *value;
    uint8_t must_destroy;
} hash_table_entry;

typedef int (*key_cmp_func)(void *, void *);

typedef uint32_t (*key_hash_func)(void *, size_t);

typedef struct _hash_table {
    size_t size;
    list **index;
    key_cmp_func key_cmp;
    key_hash_func key_hash;
} hash_table;

int ht_init(hash_table *, uint32_t, key_cmp_func, key_hash_func);

hash_table_entry *ht_init_entry(void *, size_t, void *, size_t);

int ht_destroy_entry(hash_table_entry *);

int ht_set(hash_table *, void *, void *);

int ht_set_entry(hash_table *, hash_table_entry *);

void *ht_get(hash_table *, void *);

int ht_del(hash_table *, void *);

int ht_destroy(hash_table *);

void ht_dump(hash_table *);

int ht_keys(hash_table *, void **);

#endif
