CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)

# compile the libraries
add_subdirectory(modules/DES)
add_subdirectory(modules/UDP-Server)

# compile the key distribution center program
include_directories(modules/DES)
include_directories(modules/UDP-Server)
add_executable(kdc
    key_distribution_center.cc
)

target_link_libraries(kdc
    des
    udp
)



# build user 1 (Alice)
add_executable(alice
    alice.cc
)

target_link_libraries(alice
    des
    udp
)

# build user 2 (Bob)
add_executable(bob
    bob.cc
)

target_link_libraries(bob
    des
    udp
)
