#include <CUnit/Basic.h>

#include "tests/list_test.h"
#include "tests/murmur3_test.h"
#include "tests/hash_table_test.h"
#include "tests/net_utils_test.h"

int main(int argc, char** argv) {
    // Initialize the CUnit test registry
    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    CU_SuiteInfo suites[] = {
        {"list", NULL, NULL, NULL, NULL, get_list_tests()},
        {"murmur3", NULL, NULL, NULL, NULL, get_murmur3_tests()},
        {"hash_table", NULL, NULL, NULL, NULL, get_hash_table_tests()},
        {"net_utils", NULL, NULL, NULL, NULL, get_net_utils_tests()},
        CU_SUITE_INFO_NULL,
    };

    if (CU_register_suites(suites) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run all tests using the CUnit Basic interface
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
