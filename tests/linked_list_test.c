#include "linked_list_test.h"
#include "../utils/linked_list.h"

CU_TestInfo* get_list_tests() {
    static CU_TestInfo tests[] = {
        {"test_linked_list_init_and_destroy", test_linked_list_init_and_destroy},
        {"test_linked_list", test_linked_list},
        {"test_linked_list_iter", test_linked_list_iter},
        CU_TEST_INFO_NULL,
    };

    return tests;
}

void test_linked_list_init_and_destroy() {
    list lst;

    CU_ASSERT_EQUAL(linked_list_init(&lst), 0)
    CU_ASSERT_EQUAL(lst.size, 0)
    CU_ASSERT_PTR_NULL(lst.head)

    CU_ASSERT_EQUAL(linked_list_push_tail(&lst, "foo"), 0) // ["foo"]
    CU_ASSERT_EQUAL(linked_list_push_tail(&lst, "bar"), 0) // ["foo", "bar"]

    CU_ASSERT_EQUAL(linked_list_destroy(&lst), 0)
    CU_ASSERT_EQUAL(lst.size, 0)
    CU_ASSERT_PTR_NULL(lst.head)
}

void test_linked_list() {
    list lst;

    CU_ASSERT_EQUAL(linked_list_init(&lst), 0)

    CU_ASSERT_EQUAL(linked_list_push_tail(&lst, "foo"), 0) // ["foo"]
    CU_ASSERT_EQUAL(lst.size, 1)
    CU_ASSERT_STRING_EQUAL(lst.head->value, "foo")
    CU_ASSERT_PTR_NULL(lst.head->next)

    CU_ASSERT_EQUAL(linked_list_push_tail(&lst, "bar"), 0) // ["foo", "bar"]
    CU_ASSERT_EQUAL(lst.size, 2)

    CU_ASSERT_STRING_EQUAL(lst.tail->value, "bar")
    CU_ASSERT_STRING_EQUAL(lst.tail->prev->value, "foo")
    CU_ASSERT_PTR_NULL(lst.tail->prev->prev)

    CU_ASSERT_EQUAL(linked_list_push_head(&lst, "spangle"), 0) // ["spangle", "foo", "bar"]
    CU_ASSERT_EQUAL(lst.size, 3)
    CU_ASSERT_STRING_EQUAL(lst.head->value, "spangle")
    CU_ASSERT_STRING_EQUAL(lst.head->next->value, "foo")
    CU_ASSERT_STRING_EQUAL(lst.head->next->next->value, "bar")
    CU_ASSERT_PTR_NULL(lst.head->next->next->next)

    CU_ASSERT_EQUAL(linked_list_del_at(&lst, 0), 0) // ["foo", "bar"]
    CU_ASSERT_EQUAL(lst.size, 2)
    CU_ASSERT_STRING_EQUAL(lst.head->value, "foo")
    CU_ASSERT_STRING_EQUAL(lst.head->next->value, "bar")
    CU_ASSERT_PTR_NULL(lst.head->next->next)

    CU_ASSERT_STRING_EQUAL(linked_list_pop_tail(&lst), "bar") // ["foo"]
    CU_ASSERT_EQUAL(lst.size, 1)
    CU_ASSERT_STRING_EQUAL(lst.head->value, "foo")
    CU_ASSERT_PTR_NULL(lst.head->next)

    CU_ASSERT_EQUAL(linked_list_destroy(&lst), 0)
}

static void test_list_iter_func(const list_node* item, size_t index, void* result) {
    strcat(result, "(");
    strcat(result, item->value);
    strcat(result, ")");
}

void test_linked_list_iter() {
    list lst;
    char result[500];
    memset(result, 0, sizeof(result));

    CU_ASSERT_EQUAL(linked_list_init(&lst), 0)
    CU_ASSERT_EQUAL(linked_list_push_tail(&lst, "foo"), 0) // ["foo"]
    CU_ASSERT_EQUAL(linked_list_push_tail(&lst, "bar"), 0) // ["foo", "bar"]
    CU_ASSERT_EQUAL(linked_list_push_tail(&lst, "spangle"), 0) // ["foo", "bar", "spangle"]

    CU_ASSERT_EQUAL(linked_list_iter(&lst, test_list_iter_func, result), 0)
    CU_ASSERT_STRING_EQUAL(result, "(foo)(bar)(spangle)")

    CU_ASSERT_EQUAL(linked_list_destroy(&lst), 0)
}
