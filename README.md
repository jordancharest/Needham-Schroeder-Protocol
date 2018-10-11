# Needham-Schroeder Protocol

## Cloning
This repo relies on two submodules that I wrote, UDP-Server and DES, so you must be sure to clone the submodules as well. There are two options:
```bash
git clone --recurse-submodules -j8 <this-repository>
```
or
```bash
git clone <this-repository>
cd path/to/repo
git submodule update --init --recursive
```

## Building
To build you will need a c++14 compiler and CMake, both of which are built-in to many Linux distributions. I used GNU 5.4.0. To build:
```bash
cd path/to/repo
mkdir -p build
cd build
cmake ..
make
```
This will build three executables: KDC (Key Distribution Center), alice, and bob in the `build` directory.

## Running
To complete the entire Diffie-Hellman key exchange and then spin up the secure messaging channel using Needham-Schroeder Protocol, you will need to run all three executables. The order is important, since the KDC expects messages to arrive in a particular order. The program isn't optimized for user experience, since that's not the point of the exercise and I have to draw the line somewhere. This is the procedure:

Run the KDC. From the repository root:
```bash
./build/kdc alice.txt bob.txt
```
Contained in the text file command line arguments are Alice and Bob's public information (P and G) for Computational Diffie-Hellman Key Exchange. THE KDC will read the text files and generate public keys for Alice and Bob. Then it will wait for a connection request from Alice. To send the connection request from Alice, in another terminal, from repository root, run:
```bash
./build/alice alice.txt
```
