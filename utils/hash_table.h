#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

/**
 * Closed-addressed general purpose hash table with
 * a customizable hash function and key comparator
 */

#include <inttypes.h>
#include "list.h"

/**
 * Hash table entry.
 */
typedef struct hash_table_entry {
    /**
     * Pointer to key
     */
    void* key;

    /**
     * Pointer to value
     */
    void* value;

    /**
     * If set to 1, then ht_destroy_entry() will automatically
     * be called on this entry when ht_destroy() was called
     */
    uint8_t must_destroy;
} hash_table_entry;

/**
 * Key comparator function
 */
typedef int (*key_cmp_func)(const void* key_a, const void* key_b);

/**
 * Key hash function
 */
typedef uint32_t (*key_hash_func)(const void* key, size_t ht_size);

/**
 * Hash table
 */
typedef struct hash_table {
    /**
     * Size of the index
     * This is NOT the size of all stored entries
     */
    size_t index_size;

    /**
     * Size of the total stored hash table entries
     */
    size_t entry_size;

    /**
     * Hash table index
     * Pointers to linked lists of entries
     */
    list** index;

    /**
     * Key comparator function
     * Default: String comparator
     */
    key_cmp_func key_cmp;

    /**
     * Key hash function
     * Default: String hash function
     */
    key_hash_func key_hash;
} hash_table;

/**
 * Initialize hash table
 *
 * @param ht Hash table
 * @param size Index size
 * @param key_cmp Key comparator (or NULL to use default)
 * @param key_hash Key hash function (or NULL to use default)
 * @return 0 on success, -1 on failure
 */
int ht_init(
    hash_table* ht,
    uint32_t size,
    key_cmp_func key_cmp,
    key_hash_func key_hash
);

/**
 * Resize and rebuild the hash table
 *
 * @param ht Hash table
 * @param new_size New size of hash table index
 * @return 0 on success, -1 on failure
 */
int ht_rehash(hash_table* ht, uint32_t new_size);

/**
 * Initialize a new hash table entry by creating a copy of key/value
 *
 * @param key Pointer to key (to copy)
 * @param key_size Size of key
 * @param value Pointer to value (to copy)
 * @param value_size Size of value
 * @return Hash table entry (must ht_destroy_entry() on this when finished with it)
 */
hash_table_entry* ht_init_entry(
    const void* key,
    size_t key_size,
    const void* value,
    size_t value_size
);

/**
 * Destroy a hash table entry created with ht_init_entry()
 *
 * @param entry Hash table entry
 * @return 0 on success, -1 on failure
 */
int ht_destroy_entry(const hash_table_entry* entry);

/**
 * Set value in hash table
 *
 * @param ht Hash table
 * @param key Pointer to key
 * @param value Pointer to value
 * @return 0 on success, -1 on failure
 */
int ht_set(hash_table* ht, void* key, void* value);

/**
 *
 * @param ht Hash table
 * @param entry Entry to set
 * @return 0 on success, -1 on failure
 */
int ht_set_entry(hash_table* ht, hash_table_entry* entry);

/**
 * Get value from hash table
 *
 * @param ht Hash table
 * @param key Entry key to get value for
 * @return Value pointer
 */
void* ht_get(const hash_table* ht, const void* key);

/**
 * Delete entry from hash table
 *
 * @param ht Hash table
 * @param key Entry key to delete
 * @return 0 on success, -1 on failure
 */
int ht_del(hash_table* ht, const void* key);

/**
 * Destroy hash table
 *
 * @param ht hash table
 * @return 0 on success, -1 on failure
 */
int ht_destroy(hash_table* ht);

/**
 *
 */
typedef void (*ht_iter_func)(
    const hash_table_entry* entry,
    size_t index,
    void* user_arg
);

/**
 * Iterate hash table keys and values
 *
 * @param ht Hash table
 * @param iter_func Iterator callback function
 * @param iter_func_user_arg Optional argument to pass to callback function
 * @return 0 on success, -1 on failure
 */
int ht_iter(
    const hash_table* ht,
    ht_iter_func iter_func,
    void* iter_func_user_arg
);

/**
 * Print the hash table to the console
 * Assumes that keys and values are stored as strings
 *
 * @param ht hash table
 */
void ht_dump(const hash_table* ht);

/**
 * Get all keys in hash table
 *
 * @param ht Hash table
 * @param keys Pointer to array to store key pointers in
 * @return Number of keys
 */
size_t ht_keys(const hash_table* ht, void** keys);

/**
 * Get all values in hash table
 *
 * @param ht Hash table
 * @param values Pointer to array to store value pointers in
 * @return Number of values
 */
size_t ht_values(const hash_table* ht, void** values);

/**
 * Get number of entries in hash table
 *
 * @param ht Hash table
 * @return Number of entries
 */
size_t ht_size(const hash_table* ht);

#endif
