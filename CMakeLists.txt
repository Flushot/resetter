cmake_minimum_required(VERSION 3.13)
project(resetter C)

set(resetter_VERSION_MAJOR 1)
set(resetter_VERSION_MINOR 0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ggdb")

add_executable(resetter resetter.c)

# libnet
# libnet-config --defines
set(net_HAVE_SOCKADDR_SA_LEN ON)
set(net_LIBNET_BSDISH_OS ON)
set(net_LIBNET_BSD_BYTE_SWAP ON)
find_library(net libnet.a)
target_link_libraries(resetter net)

# libpcap
find_library(pcap libpcap.a)
target_link_libraries(resetter pcap)

# libzmq
find_library(zmqp libzmq.a)
target_link_libraries(resetter zmq)

install(TARGETS resetter DESTINATION bin)
