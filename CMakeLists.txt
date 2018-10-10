CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)

# compile the libraries
add_subdirectory(modules/DES)
add_subdirectory(modules/UDP-Server)

# compile this program
include_directories(modules/DES)
include_directories(modules/UDP-Server)
add_executable(nsp
    main.cc
    )

target_link_libraries(nsp
    des
    udp
    )
