cmake_minimum_required(VERSION 3.13)
project(resetter C)

set(resetter_VERSION_MAJOR 1)
set(resetter_VERSION_MINOR 0)

# Enable for debugging
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(
        SOURCES
        arp_mitm.c
        asmtest.asm
        listener.c
        resetter.c
        thread_mgr.c
        utils/murmur3.c
        utils/hash_table.c
        utils/list.c
        utils/net_utils.c
)

#add_definitions(-DUSE_ZEROMQ)
add_executable(resetter main.c ${SOURCES})
install(TARGETS resetter DESTINATION bin)

# tests
add_executable(
        test
        test.c
        tests/murmur3_test.c
        tests/hash_table_test.c
        tests/list_test.c
        ${SOURCES}
)

# C compiler
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ggdb -fPIC")

# x86 compiler
enable_language(ASM_NASM)
if (CMAKE_ASM_NASM_COMPILER_LOADED)
    set(CAN_USE_ASSEMBLER TRUE)
    file(GLOB_RECURSE ENGINE_ASM_FILES "*.asm")
    set(ENGINE_SOURCES ${ENGINE_SOURCES} ${ENGINE_ASM_FILES})
endif (CMAKE_ASM_NASM_COMPILER_LOADED)

# libnet
# TODO: figure out how to run $(libnet-config --defines)
set(net_HAVE_SOCKADDR_SA_LEN ON)
set(net_LIBNET_BSD_BYTE_SWAP ON)
find_library(NET_LIB NAMES net libnet.a REQUIRED)
find_path(NET_LIB_INCLUDE_DIR NAMES libnet.h REQUIRED)
target_include_directories(resetter PRIVATE ${NET_LIB_INCLUDE_DIR})
target_include_directories(test PRIVATE ${NET_LIB_INCLUDE_DIR})
target_link_libraries(resetter ${NET_LIB})
target_link_libraries(test ${NET_LIB})

# libpcap
find_library(PCAP_LIB NAMES pcap libpcap.a REQUIRED)
target_link_libraries(resetter ${PCAP_LIB})
target_link_libraries(test ${PCAP_LIB})

#get_directory_property(MYDEFS COMPILE_DEFINITIONS)
#if(MYDEFS MATCHES "USE_ZEROMQ")
# zeromq
#find_library(zmqp libzmq.a)
#target_link_libraries(resetter zmq)
#endif()

# cunit
find_library(CUNIT_LIB NAMES cunit cunit.a REQUIRED)
find_path(CUNIT_INCLUDE_DIR NAMES CUnit/CUnit.h REQUIRED)
target_include_directories(test PRIVATE ${CUNIT_INCLUDE_DIR})
target_link_libraries(test ${CUNIT_LIB})
