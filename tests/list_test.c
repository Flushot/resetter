#include "list_test.h"
#include "../list.h"

CU_TestInfo *get_list_tests() {
    static CU_TestInfo tests[] = {
        { "test_list_init_and_destroy", test_list_init_and_destroy },
        { "test_list", test_list },
        CU_TEST_INFO_NULL,
    };

    return tests;
}

void test_list_init_and_destroy() {
    int ret;
    list lst;

    ret = list_init(&lst);
    CU_ASSERT(ret == 0);
    CU_ASSERT(lst.size == 0);
    CU_ASSERT(lst.head == NULL);

    list_push(&lst, "foo");
    list_push(&lst, "bar");

    ret = list_destroy(&lst);
    CU_ASSERT(ret == 0);
    CU_ASSERT(lst.size == 0);
    CU_ASSERT(lst.head == NULL);
}

void test_list() {
    int ret;
    list lst;

    list_init(&lst);

    ret = list_push(&lst, "foo");
    CU_ASSERT(ret == 0);
    CU_ASSERT(lst.size == 1);
    CU_ASSERT(lst.head->value == "foo");
    CU_ASSERT(lst.head->next == NULL);

    ret = list_push(&lst, "bar");
    CU_ASSERT(ret == 0);
    CU_ASSERT(lst.size == 2);
    CU_ASSERT(lst.head->value == "foo");
    CU_ASSERT(lst.head->next->value == "bar");
    CU_ASSERT(lst.head->next->next == NULL);

    ret = list_shift(&lst, "spangle");
    CU_ASSERT(ret == 0);
    CU_ASSERT(lst.size == 3);
    CU_ASSERT(lst.head->value == "spangle");
    CU_ASSERT(lst.head->next->value == "foo");
    CU_ASSERT(lst.head->next->next->value == "bar");
    CU_ASSERT(lst.head->next->next->next == NULL);

    ret = list_del_at(&lst, 0);
    CU_ASSERT(ret == 0);
    CU_ASSERT(lst.size == 2);
    CU_ASSERT(lst.head->value == "foo");
    CU_ASSERT(lst.head->next->value == "bar");
    CU_ASSERT(lst.head->next->next == NULL);

    list_pop(&lst);
    CU_ASSERT(lst.size == 1);
    CU_ASSERT(lst.head->value == "foo");
    CU_ASSERT(lst.head->next == NULL);

    list_destroy(&lst);
}
