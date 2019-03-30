cmake_minimum_required(VERSION 3.13)
project(resetter C)

set(resetter_VERSION_MAJOR 1)
set(resetter_VERSION_MINOR 0)

# Enable for debugging
#set(CMAKE_VERBOSE_MAKEFILE ON)

#add_definitions(-DUSE_ZEROMQ)
add_executable(resetter main.c core.c thread_mgr.c resetter.c listener.c arp_mitm.c asmtest.asm)
install(TARGETS resetter DESTINATION bin)

# C compiler
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ggdb -fPIC")

# x86 compiler
enable_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER_LOADED)
    set(CAN_USE_ASSEMBLER TRUE)
    file(GLOB_RECURSE ENGINE_ASM_FILES "*.asm")
    set(ENGINE_SOURCES ${ENGINE_SOURCES} ${ENGINE_ASM_FILES})
endif(CMAKE_ASM_NASM_COMPILER_LOADED)

# libnet
# TODO: figure out how to run $(libnet-config --defines)
set(net_HAVE_SOCKADDR_SA_LEN ON)
set(net_LIBNET_BSDISH_OS ON)
set(net_LIBNET_BSD_BYTE_SWAP ON)
find_library(net libnet.a)
target_link_libraries(resetter net)

# pcap
find_library(pcap libpcap.a)
target_link_libraries(resetter pcap)

#get_directory_property(MYDEFS COMPILE_DEFINITIONS)
#if(MYDEFS MATCHES "USE_ZEROMQ")
    # zeromq
    #find_library(zmqp libzmq.a)
    #target_link_libraries(resetter zmq)
#endif()
