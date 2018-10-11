# Needham-Schroeder Protocol

## Cloning
This repo relies on two submodules that I wrote, UDP-Server and DES, so you must be sure to clone the submodules as well. There are two options:
```bash
git clone --recurse-submodules -j8 <this-repository>
```
or
```bash
git clone <this-repository>
cd <path-to-repo>
git submodule update --init --recursive
```
