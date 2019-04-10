#include "list_test.h"
#include "../list.h"

CU_TestInfo *get_list_tests() {
    static CU_TestInfo tests[] = {
        { "test_list_init_and_destroy", test_list_init_and_destroy },
        { "test_list", test_list },
        { "test_list_iter", test_list_iter },
        CU_TEST_INFO_NULL,
    };

    return tests;
}

void test_list_init_and_destroy() {
    int ret;
    list lst;

    ret = list_init(&lst);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(lst.size, 0);
    CU_ASSERT_PTR_NULL(lst.head);

    list_push(&lst, "foo");
    list_push(&lst, "bar");

    ret = list_destroy(&lst);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(lst.size, 0);
    CU_ASSERT_PTR_NULL(lst.head);
}

void test_list() {
    int ret;
    list lst;

    list_init(&lst);

    ret = list_push(&lst, "foo");
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(lst.size, 1);
    CU_ASSERT_STRING_EQUAL(lst.head->value, "foo");
    CU_ASSERT_PTR_NULL(lst.head->next);

    ret = list_push(&lst, "bar");
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(lst.size, 2);
    CU_ASSERT_STRING_EQUAL(lst.head->value, "foo");
    CU_ASSERT_STRING_EQUAL(lst.head->next->value, "bar");
    CU_ASSERT_PTR_NULL(lst.head->next->next);

    ret = list_shift(&lst, "spangle");
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(lst.size, 3);
    CU_ASSERT_STRING_EQUAL(lst.head->value, "spangle");
    CU_ASSERT_STRING_EQUAL(lst.head->next->value, "foo");
    CU_ASSERT_STRING_EQUAL(lst.head->next->next->value, "bar");
    CU_ASSERT_PTR_NULL(lst.head->next->next->next);

    ret = list_del_at(&lst, 0);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(lst.size, 2);
    CU_ASSERT_STRING_EQUAL(lst.head->value, "foo");
    CU_ASSERT_STRING_EQUAL(lst.head->next->value, "bar");
    CU_ASSERT_PTR_NULL(lst.head->next->next);

    list_pop(&lst);
    CU_ASSERT_EQUAL(lst.size, 1);
    CU_ASSERT_STRING_EQUAL(lst.head->value, "foo");
    CU_ASSERT_PTR_NULL(lst.head->next);

    list_destroy(&lst);
}

static void test_list_iter_func(list_node *item, int index, void *result) {
    strcat(result, item->value);
}

void test_list_iter() {
    list lst;
    char result[500];
    memset(result, 0, sizeof(result));

    list_init(&lst);
    list_push(&lst, "foo");
    list_push(&lst, "bar");
    list_push(&lst, "spangle");

    list_iter(&lst, test_list_iter_func, result);
    CU_ASSERT_STRING_EQUAL(result, "foobarspangle");

    list_destroy(&lst);
}
