cmake_minimum_required(VERSION 3.13)
project(resetter C)

set(resetter_VERSION_MAJOR 1)
set(resetter_VERSION_MINOR 0)

# Enable for debugging
# set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(-DUSE_ZEROMQ)

# C compiler flags
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ggdb -fPIC")

# Source files
set(
        SOURCES
        arp_mitm.c
        listener.c
        resetter.c
        thread_mgr.c
        utils/array_list.c
        utils/hash_table.c
        utils/list.c
        utils/murmur3.c
        utils/net_utils.c
)

# Main program
add_library(resetter_shared ${SOURCES}) # Shared lib for tests
add_executable(resetter main.c) # Main executable
install(TARGETS resetter DESTINATION bin)
target_link_libraries(resetter resetter_shared) # Link main executable to shared lib

# Unit tests
add_executable(
        test
        test.c
        tests/hash_table_test.c
        tests/list_test.c
        tests/murmur3_test.c
        tests/net_utils_test.c
)

# libnet
find_library(NET NAMES net libnet.a REQUIRED)
find_path(NET_INCLUDE_DIR NAMES libnet.h REQUIRED)
target_include_directories(resetter_shared PUBLIC ${NET_INCLUDE_DIR})
target_link_libraries(resetter_shared ${NET})
execute_process(
        COMMAND libnet-config --defines
        OUTPUT_VARIABLE NET_DEFINES
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " " ";" NET_DEFINES_LIST "${NET_DEFINES}")
target_compile_definitions(resetter_shared PUBLIC ${NET_DEFINES_LIST})

# libpcap
find_library(PCAP NAMES pcap libpcap.a REQUIRED)
target_link_libraries(resetter_shared ${PCAP})

# zeromq
get_directory_property(MYDEFS COMPILE_DEFINITIONS)
if (MYDEFS MATCHES "USE_ZEROMQ")
    find_library(ZMQ NAMES zmq libzmq.a REQUIRED)
    target_include_directories(resetter_shared PUBLIC ${ZMQ_INCLUDE_DIR})
    target_link_libraries(resetter_shared ${ZMQ})
endif ()

# cunit
find_library(CUNIT NAMES cunit cunit.a REQUIRED)
find_path(CUNIT_INCLUDE_DIR NAMES CUnit/CUnit.h REQUIRED)
target_include_directories(test PRIVATE ${CUNIT_INCLUDE_DIR})
target_link_libraries(test PRIVATE ${CUNIT})

# link test against main program
target_link_libraries(test PRIVATE resetter_shared)
