#include "murmur3_test.h"
#include "../utils/murmur3.h"

CU_TestInfo* get_murmur3_tests() {
    static CU_TestInfo tests[] = {
        {"test_murmur3", test_murmur3},
        CU_TEST_INFO_NULL,
    };

    return tests;
}

void test_murmur3() {
    CU_ASSERT_EQUAL(murmur3("", 0, 0x00000000), 0x00000000)
    CU_ASSERT_EQUAL(murmur3("", 0, 0x00000001), 0x514e28b7)
    CU_ASSERT_EQUAL(murmur3("", 0, 0xffffffff), 0x81f16f39)

    CU_ASSERT_EQUAL(murmur3("test", 4, 0x00000000), 0xba6bd213)
    CU_ASSERT_EQUAL(murmur3("test", 4, 0x9747b28c), 0x704b81dc)

    CU_ASSERT_EQUAL(murmur3("Hello, world!", 13, 0x00000000), 0xc0363e43)
    CU_ASSERT_EQUAL(murmur3("Hello, world!", 13, 0x9747b28c), 0x24884cba)

    CU_ASSERT_EQUAL(murmur3("The quick brown fox jumps over the lazy dog", 43, 0x00000000), 0x2e4ff723)
    CU_ASSERT_EQUAL(murmur3("The quick brown fox jumps over the lazy dog", 43, 0x9747b28c), 0x2fa826cd)
}
