#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

/**
 * Closed-addressed general purpose hash table with
 * a customizable hash function and key comparator.
 */

#include <inttypes.h>
#include "list.h"

/**
 * Hash table entry.
 */
typedef struct _hash_table_entry {
    /**
     * Pointer to key.
     */
    void *key;

    /**
     * Pointer to value.
     */
    void *value;

    /**
     * If set to 1, then ht_destroy_entry() will automatically
     * be called on this entry when ht_destroy() was called.
     */
    uint8_t must_destroy;
} hash_table_entry;

/**
 * Key comparator function.
 */
typedef int (*key_cmp_func)(void *key_a, void *key_b);

/**
 * Key hash function.
 */
typedef uint32_t (*key_hash_func)(void *key, size_t ht_size);

/**
 * Hash table.
 */
typedef struct _hash_table {
    /**
     * Size of the index.
     * This is NOT the size of all stored entries.
     */
    size_t size;

    /**
     * Hash table index.
     * Pointers to linked lists of entries.
     */
    list **index;

    /**
     * Key comparator function.
     * Default: String comparator.
     */
    key_cmp_func key_cmp;

    /**
     * Key hash function.
     * Default: String hash function.
     */
    key_hash_func key_hash;
} hash_table;

/**
 * Initialize hash table.
 *
 * @param ht hash table.
 * @param size index size.
 * @param key_cmp key comparator (or NULL to use default).
 * @param key_hash key hash function (or NULL to use default).
 * @return 0 on success, -1 on failure.
 */
int ht_init(
    hash_table *ht,
    uint32_t size,
    key_cmp_func key_cmp,
    key_hash_func key_hash
);

/**
 * Initialize a new hash table entry by creating a copy of key/value.
 *
 * @param key pointer to key (to copy).
 * @param key_size size of key.
 * @param value pointer to value (to copy).
 * @param value_size size of value.
 * @return hash table entry (must ht_destroy_entry() on this when finished with it).
 */
hash_table_entry *ht_init_entry(
    void *key,
    size_t key_size,
    void *value,
    size_t value_size
);

/**
 * Destroy a hash table entry created with ht_init_entry().
 *
 * @param entry hash table entry.
 * @return 0 on success, -1 on failure.
 */
int ht_destroy_entry(hash_table_entry *entry);

/**
 * Set value in hash table.
 *
 * @param ht hash table.
 * @param key pointer to key.
 * @param value pointer to value.
 * @return 0 on success, -1 on failure.
 */
int ht_set(hash_table *ht, void *key, void *value);

/**
 *
 * @param ht
 * @param entry
 * @return 0 on success, -1 on failure.
 */
int ht_set_entry(hash_table *ht, hash_table_entry *entry);

/**
 * Get value from hash table.
 *
 * @param ht hash table.
 * @param key entry key to get value for.
 * @return value pointer.
 */
void *ht_get(hash_table *ht, void *key);

/**
 * Delete entry from hash table.
 *
 * @param ht hash table.
 * @param key entry key to delete.
 * @return 0 on success, -1 on failure.
 */
int ht_del(hash_table *ht, void *key);

/**
 * Destroy hash table.
 *
 * @param ht hash table.
 * @return 0 on success, -1 on failure.
 */
int ht_destroy(hash_table *ht);

/**
 *
 */
typedef void (*ht_iter_func)(
    hash_table_entry *entry,
    int index,
    void *user_arg
);

/**
 * Iterate hash table keys and values.
 *
 * @param ht hash table.
 * @param iter_func iterator callback function.
 * @param iter_func_user_arg optional argument to pass to callback function.
 * @return 0 on success, -1 on failure.
 */
int ht_iter(
    hash_table *ht,
    ht_iter_func iter_func,
    void *iter_func_user_arg
);

/**
 * Print the hash table to the console.
 * Assumes that keys and values are stored as strings.
 *
 * @param ht hash table.
 */
void ht_dump(hash_table *ht);

/**
 * Get all keys in hash table.
 *
 * @param ht hash table.
 * @param keys pointer to array to store key pointers in.
 * @return number of keys.
 */
int ht_keys(hash_table *ht, void **keys);

/**
 * Get all values in hash table.
 *
 * @param ht hash table.
 * @param values pointer to array to store value pointers in.
 * @return number of values.
 */
int ht_values(hash_table *ht, void **values);

/**
 * Get number of entries in hash table.
 *
 * @param ht hash table.
 * @return number of entries.
 */
int ht_size(hash_table *ht);

#endif
