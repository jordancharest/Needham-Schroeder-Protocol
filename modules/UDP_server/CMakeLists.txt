CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(UDP_server)
add_library(udp STATIC
    udp_server.cc
    )

install(TARGETS udp DESTINATION ../../lib)