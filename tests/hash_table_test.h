#ifndef __HASH_TABLE_TEST_H__
#define __HASH_TABLE_TEST_H__

#include <CUnit/Basic.h>

CU_TestInfo *get_hash_table_tests();
void test_hash_table_init_and_destroy();
void test_hash_table_get_and_set();
void test_hash_table_del();

#endif