CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)

# compile the libraries
add_subdirectory(modules/DES_cipher)
add_subdirectory(modules/UDP_server)

# compile this program
include_directories(include)
add_executable(nsp
    main.cc
    )

target_link_libraries(nsp
    des
    udp
    )
