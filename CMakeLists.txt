CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# compile the libraries
add_subdirectory(modules/DES_cipher)
add_subdirectory(modules/UDP_server)

# compile this program
include_directories(include)
link_directories(lib)

set(CMAKE_CXX_STANDARD 14)

add_executable(nsp
    main.cc
    )
