#include <stdlib.h>

#include "hash_table_test.h"
#include "../utils/hash_table.h"

CU_TestInfo* get_hash_table_tests() {
    static CU_TestInfo tests[] = {
        {"test_hash_table_init_and_destroy", test_hash_table_init_and_destroy},
        {"test_hash_table_rehash", test_hash_table_rehash},
        {"test_hash_table_get_and_set", test_hash_table_get_and_set},
        {"test_hash_table_set_entry", test_hash_table_set_entry},
        {"test_hash_table_del", test_hash_table_del},
        {"test_hash_table_keys", test_hash_table_keys},
        {"test_hash_table_values", test_hash_table_values},
        {"test_hash_table_has_no_duplicates", test_hash_table_has_no_duplicates},
        {"test_hash_table_iter", test_hash_table_iter},
        {"test_hash_table_size", test_hash_table_size},
        CU_TEST_INFO_NULL,
    };

    return tests;
}

void test_hash_table_init_and_destroy() {
    int ret;
    hash_table ht;

    ret = ht_init(&ht, 50, NULL, NULL);
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_EQUAL(ht.index_size, 50)

    ht_set(&ht, "foo", "one");
    ht_set(&ht, "bar", "two");

    ret = ht_destroy(&ht);
    CU_ASSERT_EQUAL(ret, 0)

    // Get non-existant values (will print warnings)
    CU_ASSERT_PTR_NULL(ht_get(&ht, "foo"))
    CU_ASSERT_PTR_NULL(ht_get(&ht, "bar"))
}

void test_hash_table_rehash() {
    int ret;
    hash_table ht;

    ret = ht_init(&ht, 2, NULL, NULL);
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_EQUAL(ht.index_size, 2)

    ret = ht_set(&ht, "foo", "one");
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "foo"), "one")

    // Rehash
    ret = ht_rehash(&ht, 3);
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_EQUAL(ht.index_size, 3)
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "foo"), "one")

    ret = ht_destroy(&ht);
    CU_ASSERT_EQUAL(ret, 0)
}

void test_hash_table_get_and_set() {
    int ret;
    hash_table ht;

    ht_init(&ht, 50, NULL, NULL);

    ret = ht_set(&ht, "foo", "one");
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "foo"), "one")
    CU_ASSERT_PTR_NULL(ht_get(&ht, "doesnt_exist"))

    ret = ht_set(&ht, "bar", "two");
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "foo"), "one")
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "bar"), "two")

    ret = ht_set(&ht, "spangle", "fez");
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "foo"), "one")
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "bar"), "two")
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "spangle"), "fez")

    ht_destroy(&ht);
}

void test_hash_table_set_entry() {
    int key = 3;
    int value = 6;
    int ret;

    hash_table_entry* entry = ht_init_entry(&key, sizeof(int), &value, sizeof(int));
    CU_ASSERT_PTR_NOT_NULL_FATAL(entry)
    CU_ASSERT_EQUAL(*((int *)entry->key), key)
    CU_ASSERT_EQUAL(*((int *)entry->value), value)

    hash_table ht;
    ht_init(&ht, 50, NULL, NULL);

    ret = ht_set_entry(&ht, entry);
    CU_ASSERT_EQUAL(ret, 0)

    ret = ht_destroy(&ht);
    CU_ASSERT_EQUAL(ret, 0)
}

void test_hash_table_del() {
    int ret;
    hash_table ht;

    ht_init(&ht, 50, NULL, NULL);

    ht_set(&ht, "foo", "one");
    ht_set(&ht, "bar", "two");

    ret = ht_del(&ht, "bar");
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "foo"), "one")
    CU_ASSERT_PTR_NULL(ht_get(&ht, "bar"))

    ht_destroy(&ht);
}

void test_hash_table_keys() {
    srand(0);

    size_t ret;
    hash_table ht;
    char* keys[5];
    memset(keys, 0, sizeof(keys));

    ht_init(&ht, 50, NULL, NULL);

    ret = ht_keys(&ht, (void **)keys);
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_PTR_NULL(keys[0])

    ht_set(&ht, "foo", "one");
    ht_set(&ht, "bar", "two");
    ht_set(&ht, "spangle", "three");

    ret = ht_keys(&ht, (void **)keys);
    CU_ASSERT_EQUAL(ret, 3)
    CU_ASSERT_STRING_EQUAL(keys[0], "bar")
    CU_ASSERT_STRING_EQUAL(keys[1], "spangle")
    CU_ASSERT_STRING_EQUAL(keys[2], "foo")
    CU_ASSERT_PTR_NULL(keys[3])

    ht_destroy(&ht);
}

void test_hash_table_values() {
    srand(0);

    size_t ret;
    hash_table ht;
    char* values[5];
    memset(values, 0, sizeof(values));

    ht_init(&ht, 50, NULL, NULL);

    ret = ht_values(&ht, (void **)values);
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_PTR_NULL(values[0])

    ht_set(&ht, "foo", "one");
    ht_set(&ht, "bar", "two");
    ht_set(&ht, "spangle", "three");

    ret = ht_values(&ht, (void **)values);
    CU_ASSERT_EQUAL(ret, 3)
    CU_ASSERT_STRING_EQUAL(values[0], "two")
    CU_ASSERT_STRING_EQUAL(values[1], "three")
    CU_ASSERT_STRING_EQUAL(values[2], "one")
    CU_ASSERT_PTR_NULL(values[3])

    ht_destroy(&ht);
}

void test_hash_table_has_no_duplicates() {
    size_t ret;
    void* keys[2];
    hash_table ht;

    ht_init(&ht, 100, NULL, NULL);

    ht_set(&ht, "a", "one");
    ht_set(&ht, "a", "two");

    ret = ht_keys(&ht, (void **)keys);
    CU_ASSERT_EQUAL(ret, 1) // One only key

    CU_ASSERT_STRING_EQUAL(ht_get(&ht, "a"), "two") // Last value set

    ht_destroy(&ht);
}

static void test_hash_table_iter_func(const hash_table_entry* entry, const size_t index, void* result) {
    strcat(result, entry->key);
    strcat(result, entry->value);
}

void test_hash_table_iter() {
    int ret;
    hash_table ht;
    char result[500];
    memset(result, 0, sizeof(result));

    ht_init(&ht, 50, NULL, NULL);

    ht_set(&ht, "foo", "one");
    ht_set(&ht, "bar", "two");

    ret = ht_iter(&ht, test_hash_table_iter_func, result);
    CU_ASSERT_EQUAL(ret, 0)
    CU_ASSERT_STRING_EQUAL(result, "bartwofooone")

    ht_destroy(&ht);
}

void test_hash_table_size() {
    hash_table ht;

    ht_init(&ht, 50, NULL, NULL);
    CU_ASSERT_EQUAL(ht_size(&ht), 0)

    ht_set(&ht, "foo", "one");
    CU_ASSERT_EQUAL(ht_size(&ht), 1)

    ht_set(&ht, "bar", "two");
    CU_ASSERT_EQUAL(ht_size(&ht), 2)

    ht_destroy(&ht);
    CU_ASSERT_EQUAL(ht_size(&ht), 0)
}
