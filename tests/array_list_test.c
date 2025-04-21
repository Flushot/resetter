#include "array_list_test.h"
#include "../utils/array_list.h"

CU_TestInfo* get_array_list_tests() {
    static CU_TestInfo tests[] = {
        {"test_array_list_init_and_destroy", test_array_list_init_and_destroy},
        {"test_array_list", test_array_list},
        CU_TEST_INFO_NULL,
    };

    return tests;
}

void test_array_list_init_and_destroy() {
    array_list lst;
    const size_t value_size = sizeof(int);
    const size_t capacity = 3;

    CU_ASSERT_EQUAL(array_list_init(&lst, value_size, capacity), 0)
    CU_ASSERT_PTR_NOT_NULL(lst.array)
    CU_ASSERT_EQUAL(lst.size, 0)
    CU_ASSERT_EQUAL(lst.value_size, value_size)
    CU_ASSERT_EQUAL(lst.capacity, capacity)

    CU_ASSERT_EQUAL(array_list_destroy(&lst), 0)
    CU_ASSERT_PTR_NULL(lst.array)
    CU_ASSERT_EQUAL(lst.size, 0)
    CU_ASSERT_EQUAL(lst.value_size, 0)
    CU_ASSERT_EQUAL(lst.capacity, 0)
}

void test_array_list() {
    array_list lst;
    int values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    CU_ASSERT_EQUAL(array_list_init(&lst, sizeof(int), 3), 0); // []

    CU_ASSERT_EQUAL(array_list_push_tail(&lst, &values[5]), 0) // [5]
    CU_ASSERT_EQUAL(array_list_push_tail(&lst, &values[7]), 0) // [5, 7]
    CU_ASSERT_EQUAL(array_list_push_tail(&lst, &values[9]), 0) // [5, 7, 9]
    CU_ASSERT_EQUAL(lst.size, 3)

    CU_ASSERT_EQUAL(array_list_index_of(&lst, &values[7]), 1)
    CU_ASSERT_EQUAL(array_list_index_of(&lst, &values[11]), -1)

    CU_ASSERT_EQUAL(*(int *)array_list_get_at(&lst, 2), 9)
    CU_ASSERT_EQUAL(*(int *)array_list_del_at(&lst, 1), 7) // [5, 9]
    CU_ASSERT_EQUAL(lst.size, 2)

    CU_ASSERT_EQUAL(array_list_push_tail(&lst, &values[11]), 0) // [5, 9, 11]
    CU_ASSERT_EQUAL(*(int *)array_list_del_at(&lst, 1), 9) // [5, 11]
    CU_ASSERT_PTR_NULL(array_list_del_at(&lst, 9))
    CU_ASSERT_EQUAL(*(int *)array_list_pop_head(&lst), 5) // [11]
    CU_ASSERT_EQUAL(*(int *)array_list_pop_head(&lst), 11) // []
    CU_ASSERT_EQUAL(lst.size, 0)

    CU_ASSERT_EQUAL(array_list_push_head(&lst, &values[5]), 0) // [5]
    CU_ASSERT_EQUAL(array_list_push_head(&lst, &values[7]), 0) // [7, 5]
    CU_ASSERT_EQUAL(array_list_push_head(&lst, &values[9]), 0) // [9, 7, 5]

    CU_ASSERT_EQUAL(*(int *)array_list_get_at(&lst, 2), 5)
    CU_ASSERT_EQUAL(*(int *)array_list_get_at(&lst, 0), 9)
    CU_ASSERT_EQUAL(*(int *)array_list_del_value(&lst, &values[9]), 9) // [7, 5]
    CU_ASSERT_EQUAL(lst.size, 2)
    CU_ASSERT_EQUAL(*(int *)array_list_get_at(&lst, 0), 7)

    CU_ASSERT_EQUAL(array_list_destroy(&lst), 0)
}
