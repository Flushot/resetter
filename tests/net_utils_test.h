#ifndef __NET_UTILS_TEST_H__
#define __NET_UTILS_TEST_H__

#include <CUnit/Basic.h>

CU_TestInfo* get_net_utils_tests();

void test_net_utils_ip2long();

void test_net_utils_long2ip();

void test_net_utils_ip_matches();

void test_net_utils_ether_ntoa();

#endif
