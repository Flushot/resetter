cmake_minimum_required(VERSION 3.13)
project(resetter C)

include(FetchContent)

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
        utils/linked_list.c
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
        tests/array_list_test.c
        tests/hash_table_test.c
        tests/linked_list_test.c
        tests/murmur3_test.c
        tests/net_utils_test.c
)

# libnet
FetchContent_Declare(
        libnet
        GIT_REPOSITORY https://github.com/libnet/libnet.git
        GIT_TAG v1.3
)
FetchContent_MakeAvailable(libnet)
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
FetchContent_Declare(
        libpcap
        GIT_REPOSITORY https://github.com/the-tcpdump-group/libpcap.git
        GIT_TAG libpcap-1.10.5
)
FetchContent_MakeAvailable(libpcap)
find_library(PCAP NAMES pcap libpcap.a REQUIRED)
target_link_libraries(resetter_shared ${PCAP})

# zeromq
FetchContent_Declare(
        zeromq
        GIT_REPOSITORY https://github.com/zeromq/libzmq.git
        GIT_TAG v4.3.5
)
set(ZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZMQ_ENABLE_GNUTLS OFF CACHE BOOL "" FORCE)
set(ENABLE_CURVE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zeromq)
get_directory_property(MYDEFS COMPILE_DEFINITIONS)
if (MYDEFS MATCHES "USE_ZEROMQ")
    find_library(ZMQ NAMES zmq libzmq.a REQUIRED)
    target_include_directories(resetter_shared PUBLIC ${ZMQ_INCLUDE_DIR})
    target_link_libraries(resetter_shared ${ZMQ})
endif ()

# cunit
FetchContent_Declare(
        cunit
        GIT_REPOSITORY https://gitlab.com/cunity/cunit.git
        GIT_TAG 3.5.2
)
FetchContent_MakeAvailable(cunit)
find_library(CUNIT NAMES cunit cunit.a REQUIRED)
find_path(CUNIT_INCLUDE_DIR NAMES CUnit/CUnit.h REQUIRED)
target_include_directories(test PRIVATE ${CUNIT_INCLUDE_DIR})
target_link_libraries(test PRIVATE ${CUNIT})

# link test against main program
target_link_libraries(test PRIVATE resetter_shared)
