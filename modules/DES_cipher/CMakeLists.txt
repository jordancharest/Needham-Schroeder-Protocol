CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(DES_cipher)
add_library(des STATIC
    des_cipher.cc
    )

install(TARGETS des DESTINATION ../../lib)