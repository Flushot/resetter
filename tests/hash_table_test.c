#include "hash_table_test.h"
#include "../hash_table.h"

CU_TestInfo *get_hash_table_tests() {
    static CU_TestInfo tests[] = {
        { "test_hash_table_init_and_destroy", test_hash_table_init_and_destroy },
        { "test_hash_table_get_and_set", test_hash_table_get_and_set },
        { "test_hash_table_del", test_hash_table_del },
        CU_TEST_INFO_NULL,
    };

    return tests;
}

void test_hash_table_init_and_destroy() {
    int ret;
    hash_table ht;

    ret = ht_init(&ht, 50, NULL, NULL);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(ht.size, 50);

    ht_set(&ht, "foo", "one");
    ht_set(&ht, "bar", "two");

    ret = ht_destroy(&ht);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_PTR_NULL(ht_get(&ht, "foo"));
    CU_ASSERT_PTR_NULL(ht_get(&ht, "bar"));
}

void test_hash_table_get_and_set() {
    int ret;
    hash_table ht;

    ht_init(&ht, 50, NULL, NULL);

    ret = ht_set(&ht, "foo", "one");
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(ht_get(&ht, "foo"), "one");
    CU_ASSERT_PTR_NULL(ht_get(&ht, "doesnt_exist"));

    ret = ht_set(&ht, "bar", "two");
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(ht_get(&ht, "foo"), "one");
    CU_ASSERT_EQUAL(ht_get(&ht, "bar"), "two");

    ret = ht_set(&ht, "spangle", "fez");
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(ht_get(&ht, "foo"), "one");
    CU_ASSERT_EQUAL(ht_get(&ht, "bar"), "two");
    CU_ASSERT_EQUAL(ht_get(&ht, "spangle"), "fez");

    ht_destroy(&ht);
}

void test_hash_table_del() {
    int ret;
    hash_table ht;

    ht_init(&ht, 50, NULL, NULL);

    ht_set(&ht, "foo", "one");
    ht_set(&ht, "bar", "two");

    ret = ht_del(&ht, "bar");
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(ht_get(&ht, "foo"), "one");
    CU_ASSERT_PTR_NULL(ht_get(&ht, "bar"));

    ht_destroy(&ht);
}
