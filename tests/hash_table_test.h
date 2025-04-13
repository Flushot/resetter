#ifndef __HASH_TABLE_TEST_H__
#define __HASH_TABLE_TEST_H__

#include <CUnit/Basic.h>

CU_TestInfo* get_hash_table_tests();

void test_hash_table_init_and_destroy();

void test_hash_table_rehash();

void test_hash_table_get_and_set();

void test_hash_table_set_entry();

void test_hash_table_del();

void test_hash_table_keys();

void test_hash_table_values();

void test_hash_table_has_no_duplicates();

void test_hash_table_iter();

void test_hash_table_size();

#endif
