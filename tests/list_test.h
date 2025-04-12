#ifndef __LIST_TEST_H__
#define __LIST_TEST_H__

#include <CUnit/Basic.h>

CU_TestInfo* get_list_tests();

void test_list_init_and_destroy();

void test_list();

void test_list_iter();

#endif
